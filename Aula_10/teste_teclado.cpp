#include <stdio.h>
#include <wiringPi.h>
#include "Keypad.hpp"

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Numeração BCM utilizada no exemplo da Freenove.
byte rowPins[ROWS] = {16, 20, 21, 26};
byte colPins[COLS] = {19, 13, 6, 5};

Keypad keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    ROWS,
    COLS
);

int main()
{
    printf("Iniciando teste do teclado...\n");
    printf("Pressione as teclas.\n");
    printf("Pressione D para encerrar.\n");

    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    keypad.setDebounceTime(50);

    while (true) {
        char key = keypad.getKey();

        if (key) {
            printf("Tecla detectada: %c\n", key);

            if (key == 'D') {
                printf("Teste encerrado.\n");
                break;
            }
        }

        delay(10);
    }

    return 0;
}