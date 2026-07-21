#include <stdio.h>
#include <string>

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

byte rowPins[ROWS] = {16, 20, 21, 26};
byte colPins[COLS] = {19, 13, 6, 5};

Keypad keypad(
    makeKeymap(keys),
    rowPins,
    colPins,
    ROWS,
    COLS
);

const std::string CORRECT_PASSWORD = "1234";
const size_t MAX_PASSWORD_LENGTH = 8;

int main()
{
    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    keypad.setDebounceTime(50);

    std::string typedPassword;

    printf("Digite a senha.\n");
    printf("# confirma, * apaga e D encerra.\n");

    while (true) {
        char key = keypad.getKey();

        if (!key) {
            delay(10);
            continue;
        }

        if (key >= '0' && key <= '9') {
            if (typedPassword.length() < MAX_PASSWORD_LENGTH) {
                typedPassword += key;

                printf(
                    "Senha: %s\n",
                    std::string(
                        typedPassword.length(),
                        '*'
                    ).c_str()
                );
            }
        } else if (key == '*') {
            if (!typedPassword.empty()) {
                typedPassword.pop_back();
            }

            printf(
                "Senha: %s\n",
                std::string(
                    typedPassword.length(),
                    '*'
                ).c_str()
            );
        } else if (key == '#') {
            if (typedPassword == CORRECT_PASSWORD) {
                printf("ACESSO LIBERADO\n");
            } else {
                printf("ACESSO NEGADO\n");
            }

            typedPassword.clear();

            printf("Digite uma nova senha.\n");
        } else if (key == 'D') {
            break;
        }
    }

    printf("Teste encerrado.\n");

    return 0;
}