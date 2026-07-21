#include <stdio.h>
#include <signal.h>
#include <string>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pcf8574.h>
#include <lcd.h>
#include <softTone.h>

#include "Keypad.hpp"

// ============================================================
// CONFIGURACOES GERAIS
// ============================================================

const std::string CORRECT_PASSWORD = "1234";

const int MAX_PASSWORD_LENGTH = 8;
const int MAX_FAILED_ATTEMPTS = 3;

const unsigned int BLOCK_TIME_MS = 30000;       // 30 segundos
const unsigned int AUTHORIZATION_TIME_MS = 10000; // 10 segundos
const unsigned int SENSOR_INTERVAL_MS = 300;

// Ajuste depois da calibracao do sensor.
const float DISTANCE_LIMIT_CM = 10.0f;

// ============================================================
// LCD I2C
// ============================================================

#define LCD_ADDRESS 0x27

#define BASE 64
#define RS   (BASE + 0)
#define RW   (BASE + 1)
#define EN   (BASE + 2)
#define LED  (BASE + 3)
#define D4   (BASE + 4)
#define D5   (BASE + 5)
#define D6   (BASE + 6)
#define D7   (BASE + 7)

int lcdHandle = -1;

// ============================================================
// TECLADO MATRICIAL
// ============================================================

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Numeracao BCM.
byte rowPins[ROWS] = {16, 20, 21, 26};
byte colPins[COLS] = {19, 13, 6, 5};

Keypad keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    ROWS,
    COLS
);

// ============================================================
// BUZZER
// ============================================================

#define BUZZER_PIN 4

// ============================================================
// SENSOR ULTRASSONICO
// ============================================================

#define TRIG_PIN 14
#define ECHO_PIN 15

#define SENSOR_TIMEOUT_US 30000

// ============================================================
// VARIAVEIS DE ESTADO
// ============================================================

volatile sig_atomic_t running = 1;

std::string typedPassword;

int failedAttempts = 0;

unsigned int blockedUntil = 0;
unsigned int authorizedUntil = 0;
unsigned int nextSensorReading = 0;
unsigned int temporaryMessageUntil = 0;
unsigned int nextBlockedDisplayUpdate = 0;

bool temporaryMessageActive = false;
bool previousDoorOpen = false;
bool unauthorizedAlertActive = false;

// ============================================================
// FUNCOES DO LCD
// ============================================================

void clearLCDLine(int line)
{
    lcdPosition(lcdHandle, 0, line);
    lcdPrintf(lcdHandle, "                ");
}

void showTwoLines(const char* line1, const char* line2)
{
    lcdClear(lcdHandle);

    lcdPosition(lcdHandle, 0, 0);
    lcdPrintf(lcdHandle, "%-16.16s", line1);

    lcdPosition(lcdHandle, 0, 1);
    lcdPrintf(lcdHandle, "%-16.16s", line2);
}

void showPasswordPrompt()
{
    lcdClear(lcdHandle);

    lcdPosition(lcdHandle, 0, 0);
    lcdPrintf(lcdHandle, "Digite a senha:");

    lcdPosition(lcdHandle, 0, 1);

    for (size_t i = 0; i < typedPassword.length(); i++) {
        lcdPutchar(lcdHandle, '*');
    }
}

void showTemporaryMessage(
    const char* line1,
    const char* line2,
    unsigned int durationMs
)
{
    showTwoLines(line1, line2);

    temporaryMessageActive = true;
    temporaryMessageUntil = millis() + durationMs;
}

int initializeLCD()
{
    int fd = wiringPiI2CSetup(LCD_ADDRESS);

    if (fd < 0) {
        printf(
            "Erro: dispositivo I2C 0x%X nao encontrado.\n",
            LCD_ADDRESS
        );

        return -1;
    }

    pcf8574Setup(BASE, LCD_ADDRESS);

    for (int pin = BASE; pin < BASE + 8; pin++) {
        pinMode(pin, OUTPUT);
    }

    digitalWrite(LED, HIGH);
    digitalWrite(RW, LOW);

    return lcdInit(
        2,
        16,
        4,
        RS,
        EN,
        D4,
        D5,
        D6,
        D7,
        0,
        0,
        0,
        0
    );
}

// ============================================================
// FUNCOES DO BUZZER
// ============================================================

void stopBuzzer()
{
    softToneWrite(BUZZER_PIN, 0);
}

void successSound()
{
    softToneWrite(BUZZER_PIN, 2000);
    delay(150);
    stopBuzzer();
}

void errorSound()
{
    softToneWrite(BUZZER_PIN, 700);
    delay(500);
    stopBuzzer();
}

void alertSound()
{
    for (int i = 0; i < 3; i++) {
        softToneWrite(BUZZER_PIN, 1500);
        delay(180);

        stopBuzzer();
        delay(120);
    }
}

// ============================================================
// FUNCOES DO SENSOR ULTRASSONICO
// ============================================================

long pulseInWithTimeout(
    int pin,
    int expectedLevel,
    unsigned int timeoutUs
)
{
    unsigned int startTime = micros();

    while (digitalRead(pin) != expectedLevel) {
        if (micros() - startTime > timeoutUs) {
            return 0;
        }
    }

    unsigned int pulseStart = micros();

    while (digitalRead(pin) == expectedLevel) {
        if (micros() - pulseStart > timeoutUs) {
            return 0;
        }
    }

    return static_cast<long>(micros() - pulseStart);
}

float readDistanceCm()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long pulseDuration = pulseInWithTimeout(
        ECHO_PIN,
        HIGH,
        SENSOR_TIMEOUT_US
    );

    if (pulseDuration == 0) {
        return -1.0f;
    }

    // Velocidade aproximada do som: 34300 cm/s.
    return pulseDuration * 34300.0f / 2000000.0f;
}

void processSensor(unsigned int currentTime)
{
    if (currentTime < nextSensorReading) {
        return;
    }

    nextSensorReading = currentTime + SENSOR_INTERVAL_MS;

    float distance = readDistanceCm();

    if (distance < 0) {
        printf("Sensor: timeout de leitura.\n");
        return;
    }

    bool doorOpen = distance > DISTANCE_LIMIT_CM;

    printf(
        "Distancia: %.2f cm | Estado: %s\n",
        distance,
        doorOpen ? "ABERTA" : "TRANCADA"
    );

    // Detecta a transicao de fechada para aberta.
    if (doorOpen && !previousDoorOpen) {
        bool openingAuthorized = currentTime < authorizedUntil;

        if (openingAuthorized) {
            printf("Abertura autorizada detectada.\n");

            showTemporaryMessage(
                "Porta aberta",
                "Acesso autorizado",
                2000
            );
        } else {
            printf("ALERTA: abertura nao autorizada.\n");

            unauthorizedAlertActive = true;

            showTemporaryMessage(
                "ALERTA!",
                "Abertura indevida",
                3000
            );

            alertSound();
        }
    }

    // Quando a porta volta a fechar, o alerta pode ser liberado.
    if (!doorOpen && previousDoorOpen) {
        printf("Fechadura voltou ao estado trancado.\n");

        unauthorizedAlertActive = false;
        authorizedUntil = 0;

        if (currentTime >= blockedUntil) {
            showTemporaryMessage(
                "Fechadura",
                "TRANCADA",
                1500
            );
        }
    }

    previousDoorOpen = doorOpen;
}

// ============================================================
// AUTENTICACAO
// ============================================================

void clearPassword()
{
    typedPassword.clear();
}

void validatePassword(unsigned int currentTime)
{
    if (typedPassword == CORRECT_PASSWORD) {
        printf("ACESSO LIBERADO\n");

        failedAttempts = 0;
        authorizedUntil = currentTime + AUTHORIZATION_TIME_MS;

        successSound();

        showTemporaryMessage(
            "Acesso liberado",
            "Abra em 10 seg.",
            2500
        );
    } else {
        failedAttempts++;

        printf(
            "ACESSO NEGADO - tentativa %d de %d\n",
            failedAttempts,
            MAX_FAILED_ATTEMPTS
        );

        errorSound();

        if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
            blockedUntil = currentTime + BLOCK_TIME_MS;
            nextBlockedDisplayUpdate = 0;

            showTwoLines(
                "Sist. bloqueado",
                "Aguarde 30 seg."
            );

            temporaryMessageActive = false;

            printf("Sistema bloqueado por 30 segundos.\n");
        } else {
            char attemptsText[17];

            snprintf(
                attemptsText,
                sizeof(attemptsText),
                "Tentativa %d de %d",
                failedAttempts,
                MAX_FAILED_ATTEMPTS
            );

            showTemporaryMessage(
                "Acesso negado",
                attemptsText,
                2000
            );
        }
    }

    clearPassword();
}

// ============================================================
// BLOQUEIO TEMPORARIO
// ============================================================

bool processBlockingState(unsigned int currentTime)
{
    if (currentTime >= blockedUntil) {
        return false;
    }

    if (currentTime >= nextBlockedDisplayUpdate) {
        unsigned int remainingMs = blockedUntil - currentTime;
        unsigned int remainingSeconds = (remainingMs + 999) / 1000;

        char secondLine[17];

        snprintf(
            secondLine,
            sizeof(secondLine),
            "Restam %u seg.",
            remainingSeconds
        );

        showTwoLines(
            "Sist. bloqueado",
            secondLine
        );

        nextBlockedDisplayUpdate = currentTime + 1000;
    }

    return true;
}

void finishBlockingIfNecessary(unsigned int currentTime)
{
    static bool wasBlocked = false;

    bool currentlyBlocked = currentTime < blockedUntil;

    if (currentlyBlocked) {
        wasBlocked = true;
        return;
    }

    if (wasBlocked) {
        wasBlocked = false;
        failedAttempts = 0;
        blockedUntil = 0;

        printf("Bloqueio encerrado.\n");

        showTemporaryMessage(
            "Bloqueio encerr.",
            "Digite a senha",
            1500
        );
    }
}

// ============================================================
// TECLADO
// ============================================================

void processKey(char key, unsigned int currentTime)
{
    printf("Tecla detectada: %c\n", key);

    if (key >= '0' && key <= '9') {
        if (
            static_cast<int>(typedPassword.length())
            < MAX_PASSWORD_LENGTH
        ) {
            typedPassword += key;
            temporaryMessageActive = false;
            showPasswordPrompt();
        }

        return;
    }

    if (key == '*') {
        if (!typedPassword.empty()) {
            typedPassword.pop_back();
        }

        temporaryMessageActive = false;
        showPasswordPrompt();

        return;
    }

    if (key == 'D') {
        clearPassword();
        temporaryMessageActive = false;
        showPasswordPrompt();

        return;
    }

    if (key == '#') {
        if (typedPassword.empty()) {
            showTemporaryMessage(
                "Senha vazia",
                "Digite a senha",
                1500
            );

            return;
        }

        validatePassword(currentTime);
    }
}

// ============================================================
// ENCERRAMENTO
// ============================================================

void handleSignal(int)
{
    running = 0;
}

void finishProgram()
{
    stopBuzzer();

    if (lcdHandle >= 0) {
        showTwoLines(
            "Sistema",
            "encerrado"
        );

        delay(800);
        lcdClear(lcdHandle);
    }

    printf("\nPrograma encerrado.\n");
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    printf("Inicializando fechadura eletronica...\n");

    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    lcdHandle = initializeLCD();

    if (lcdHandle == -1) {
        printf("Erro ao inicializar LCD.\n");
        return 1;
    }

    keypad.setDebounceTime(50);

    pinMode(BUZZER_PIN, OUTPUT);

    if (softToneCreate(BUZZER_PIN) != 0) {
        printf("Erro ao inicializar buzzer.\n");
        return 1;
    }

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);
    stopBuzzer();

    showTwoLines(
        "Fechadura",
        "Inicializando..."
    );

    delay(1500);

    showPasswordPrompt();

    printf("Sistema iniciado.\n");
    printf("# confirma, * apaga e D limpa.\n");
    printf("Pressione Ctrl+C para encerrar.\n");

    while (running) {
        unsigned int currentTime = millis();

        // O sensor continua sendo monitorado mesmo durante o bloqueio.
        processSensor(currentTime);

        finishBlockingIfNecessary(currentTime);

        if (processBlockingState(currentTime)) {
            delay(10);
            continue;
        }

        if (
            temporaryMessageActive
            && currentTime >= temporaryMessageUntil
        ) {
            temporaryMessageActive = false;
            showPasswordPrompt();
        }

        char key = keypad.getKey();

        if (key) {
            processKey(key, currentTime);
        }

        delay(10);
    }

    finishProgram();

    return 0;
}