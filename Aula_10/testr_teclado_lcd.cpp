#include <stdio.h>
#include <string>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pcf8574.h>
#include <lcd.h>

#include "Keypad.hpp"

// ---------------- LCD ----------------

int pcf8574Address = 0x27;

#define BASE 64
#define RS   (BASE + 0)
#define RW   (BASE + 1)
#define EN   (BASE + 2)
#define LED  (BASE + 3)
#define D4   (BASE + 4)
#define D5   (BASE + 5)
#define D6   (BASE + 6)
#define D7   (BASE + 7)

// ---------------- Teclado ----------------

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {16, 20, 21, 26};
byte colPins[COLS] = {19, 13, 6, 5};

Keypad keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    ROWS,
    COLS
);

int detectI2C(int address)
{
    int fd = wiringPiI2CSetup(address);

    if (fd < 0) {
        return 0;
    }

    return wiringPiI2CWrite(fd, 0) >= 0;
}

int initializeLCD()
{
    if (detectI2C(0x27)) {
        pcf8574Address = 0x27;
    } else if (detectI2C(0x3F)) {
        pcf8574Address = 0x3F;
    } else {
        return -1;
    }

    pcf8574Setup(BASE, pcf8574Address);

    for (int i = 0; i < 8; i++) {
        pinMode(BASE + i, OUTPUT);
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

void showPassword(int lcdHandle, const std::string& password)
{
    lcdClear(lcdHandle);

    lcdPosition(lcdHandle, 0, 0);
    lcdPrintf(lcdHandle, "Digite a senha:");

    lcdPosition(lcdHandle, 0, 1);

    for (size_t i = 0; i < password.length(); i++) {
        lcdPutchar(lcdHandle, '*');
    }
}

int main()
{
    printf("Iniciando teste do teclado com LCD...\n");

    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    int lcdHandle = initializeLCD();

    if (lcdHandle == -1) {
        printf("LCD nao encontrado.\n");
        return 1;
    }

    keypad.setDebounceTime(50);

    std::string password;

    showPassword(lcdHandle, password);

    while (true) {
        char key = keypad.getKey();

        if (!key) {
            delay(10);
            continue;
        }

        printf("Tecla: %c\n", key);

        if (key >= '0' && key <= '9') {
            if (password.length() < 8) {
                password += key;
                showPassword(lcdHandle, password);
            }
        } else if (key == '*') {
            if (!password.empty()) {
                password.pop_back();
            }

            showPassword(lcdHandle, password);
        } else if (key == '#') {
            lcdClear(lcdHandle);

            lcdPosition(lcdHandle, 0, 0);
            lcdPrintf(lcdHandle, "Senha recebida");

            lcdPosition(lcdHandle, 0, 1);
            lcdPrintf(
                lcdHandle,
                "%d digitos",
                static_cast<int>(password.length())
            );

            printf(
                "Senha recebida com %zu digitos.\n",
                password.length()
            );

            delay(2000);

            password.clear();
            showPassword(lcdHandle, password);
        } else if (key == 'D') {
            break;
        }
    }

    lcdClear(lcdHandle);

    printf("Teste encerrado.\n");

    return 0;
}