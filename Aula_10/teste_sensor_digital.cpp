#include <stdio.h>
#include <wiringPi.h>

#define SENSOR_PIN 23

int main()
{
    printf("Iniciando teste do sensor digital...\n");
    printf("Pressione Ctrl+C para encerrar.\n");

    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    pinMode(SENSOR_PIN, INPUT);

    // Pode ser PUD_UP ou PUD_DOWN, dependendo do sensor.
    pullUpDnControl(SENSOR_PIN, PUD_UP);

    while (true) {
        int sensorValue = digitalRead(SENSOR_PIN);

        if (sensorValue == LOW) {
            printf("Fechadura: TRANCADA\n");
        } else {
            printf("Fechadura: ABERTA\n");
        }

        delay(300);
    }

    return 0;
}