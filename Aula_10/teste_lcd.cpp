#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pcf8574.h>
#include <lcd.h>

// Endereço padrão. O programa também testa 0x3F.
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

int detectI2C(int address)
{
    int fd = wiringPiI2CSetup(address);

    if (fd < 0) {
        return 0;
    }

    if (wiringPiI2CWrite(fd, 0) < 0) {
        return 0;
    }

    return 1;
}

int main()
{
    printf("Iniciando teste do LCD...\n");

    if (wiringPiSetupGpio() == -1) {
        printf("Erro ao inicializar wiringPi.\n");
        return 1;
    }

    if (detectI2C(0x27)) {
        pcf8574Address = 0x27;
    } else if (detectI2C(0x3F)) {
        pcf8574Address = 0x3F;
    } else {
        printf("LCD nao encontrado em 0x27 ou 0x3F.\n");
        printf("Execute: i2cdetect -y 1\n");
        return 1;
    }

    printf(
        "LCD encontrado no endereco 0x%X\n",
        pcf8574Address
    );

    pcf8574Setup(BASE, pcf8574Address);

    for (int i = 0; i < 8; i++) {
        pinMode(BASE + i, OUTPUT);
    }

    digitalWrite(LED, HIGH);
    digitalWrite(RW, LOW);

    int lcdHandle = lcdInit(
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

    if (lcdHandle == -1) {
        printf("Erro ao inicializar o LCD.\n");
        return 1;
    }

    lcdClear(lcdHandle);

    lcdPosition(lcdHandle, 0, 0);
    lcdPrintf(lcdHandle, "Fechadura");

    lcdPosition(lcdHandle, 0, 1);
    lcdPrintf(lcdHandle, "TRANCADA");

    delay(3000);

    lcdClear(lcdHandle);

    lcdPosition(lcdHandle, 0, 0);
    lcdPrintf(lcdHandle, "Teste LCD OK");

    delay(2000);

    lcdClear(lcdHandle);

    printf("Teste concluido.\n");

    return 0;
}