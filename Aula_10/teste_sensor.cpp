#include <stdio.h>
#include <sys/time.h>
#include <wiringPi.h>

#define TRIG_PIN 14
#define ECHO_PIN 15

#define MAX_DISTANCE 220
#define TIMEOUT (MAX_DISTANCE * 60)

int pulseIn(int pin, int level, int timeout)
{
    struct timeval currentTime;
    struct timeval initialTime;
    struct timeval pulseStartTime;

    long micros;

    gettimeofday(&initialTime, NULL);

    micros = 0;

    // Aguarda o pino atingir o nível desejado.
    while (digitalRead(pin) != level) {
        gettimeofday(&currentTime, NULL);

        if (currentTime.tv_sec > initialTime.tv_sec) {
            micros = 1000000L;
        } else {
            micros = 0;
        }

        micros += currentTime.tv_usec - initialTime.tv_usec;

        if (micros > timeout) {
            return 0;
        }
    }

    gettimeofday(&pulseStartTime, NULL);

    // Mede quanto tempo o pino permanece no nível desejado.
    while (digitalRead(pin) == level) {
        gettimeofday(&currentTime, NULL);

        if (currentTime.tv_sec > initialTime.tv_sec) {
            micros = 1000000L;
        } else {
            micros = 0;
        }

        micros += currentTime.tv_usec - initialTime.tv_usec;

        if (micros > timeout) {
            return 0;
        }
    }

    if (currentTime.tv_sec > pulseStartTime.tv_sec) {
        micros = 1000000L;
    } else {
        micros = 0;
    }

    micros += currentTime.tv_usec - pulseStartTime.tv_usec;

    return micros;
}

float getDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long echoTime = pulseIn(
        ECHO_PIN,
        HIGH,
        TIMEOUT
    );

    if (echoTime == 0) {
        return -1.0f;
    }

    // Distância em centímetros.
    return static_cast<float>(echoTime)
        * 340.0f
        / 2.0f
        / 10000.0f;
}

int main()
{
    printf("Iniciando teste do sensor ultrassonico...\n");
    printf("Pressione Ctrl+C para encerrar.\n");

    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(TRIG_PIN, LOW);

    delay(100);

    const float LIMIT_DISTANCE = 10.0f;

    while (true) {
        float distance = getDistance();

        if (distance < 0) {
            printf("Erro ou timeout na leitura.\n");
        } else {
            const char* status;

            if (distance <= LIMIT_DISTANCE) {
                status = "TRANCADA";
            } else {
                status = "ABERTA";
            }

            printf(
                "Distancia: %.2f cm | Estado: %s\n",
                distance,
                status
            );
        }

        delay(500);
    }

    return 0;
}