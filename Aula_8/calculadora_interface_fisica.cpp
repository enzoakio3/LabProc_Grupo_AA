#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pcf8574.h>
#include <lcd.h>
#include "Keypad.hpp" // Biblioteca do teclado fornecida no kit

#define ITERACOES 10000000

// --- Configurações do LCD I2C ---
int pcf8574_address = 0x27; 
#define BASE 64         
#define RS      BASE+0
#define RW      BASE+1
#define EN      BASE+2
#define LED     BASE+3
#define D4      BASE+4
#define D5      BASE+5
#define D6      BASE+6
#define D7      BASE+7

int lcdhd; // Handler do LCD

// --- Configurações do Teclado Matricial ---
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {16, 20, 21, 26}; 
byte colPins[COLS] = {19, 13, 6, 5};   
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Função Fatorial do Código Base ---
int fatorial(int n) {
    int fat = 1;
    for (int i = 1; i <= n; i++) {
        fat *= i;
    }
    return fat;
}

// --- Detecção de Endereço I2C ---
int detectI2C(int addr){
    int _fd = wiringPiI2CSetup(addr);   
    if (_fd < 0) return 0;
    else {    
        if(wiringPiI2CWrite(_fd,0) < 0) return 0;
        else return 1;
    }
}

int main() {
    printf("Iniciando Calculadora Standalone com Benchmark... \n");
    
    // Inicializações de Hardware
    wiringPiSetupGpio(); // Inicialização exigida pelo Keypad (BCM)
    
    if(detectI2C(0x27)){
        pcf8574_address = 0x27;
    } else if(detectI2C(0x3F)){
        pcf8574_address = 0x3F;
    } else {
        printf("LCD I2C nao encontrado!\n");
        return -1;
    }
    
    pcf8574Setup(BASE, pcf8574_address);
    for(int i=0; i<8; i++){
        pinMode(BASE+i, OUTPUT);
    } 
    digitalWrite(LED, HIGH);
    digitalWrite(RW, LOW);
    
    lcdhd = lcdInit(2, 16, 4, RS, EN, D4, D5, D6, D7, 0, 0, 0, 0);
    if(lcdhd == -1){
        printf("lcdInit falhou!\n");
        return 1;
    }

    keypad.setDebounceTime(50);
    
    // Variáveis de controle de estado da calculadora
    int a = 0, b = 0, resultado = 0;
    double resultado_div = 0.0;
    int opcao = 0; // 1:+, 2:-, 3:*, 4:/, 5:!
    int estado = 0; // 0: Digitando A, 1: Escolhendo Op, 2: Digitando B, 3: Executando/Pronto
    
    lcdClear(lcdhd);
    lcdPosition(lcdhd, 0, 0);
    lcdPrintf(lcdhd, "A: ");

    while(1) {
        char key = keypad.getKey();
        
        if (key) {
            // Se for pressionado um número (0 a 9)
            if (key >= '0' && key <= '9') {
                int digito = key - '0';
                if (estado == 0) {
                    a = (a * 10) + digito;
                    lcdPosition(lcdhd, 3, 0);
                    lcdPrintf(lcdhd, "%d   ", a);
                } else if (estado == 2) {
                    b = (b * 10) + digito;
                    lcdPosition(lcdhd, 3, 1);
                    lcdPrintf(lcdhd, "%d   ", b);
                }
            }
            // Seleção de Operações (Letras e *)
            else if (key == 'A' || key == 'B' || key == 'C' || key == 'D' || key == '*') {
                if (key == 'A') { opcao = 1; lcdPosition(lcdhd, 0, 1); lcdPrintf(lcdhd, "+ "); estado = 2; }
                else if (key == 'B') { opcao = 2; lcdPosition(lcdhd, 0, 1); lcdPrintf(lcdhd, "- "); estado = 2; }
                else if (key == 'C') { opcao = 3; lcdPosition(lcdhd, 0, 1); lcdPrintf(lcdhd, "* "); estado = 2; }
                else if (key == 'D') { opcao = 4; lcdPosition(lcdhd, 0, 1); lcdPrintf(lcdhd, "/ "); estado = 2; }
                else if (key == '*') { 
                    opcao = 5; 
                    lcdPosition(lcdhd, 0, 1); 
                    lcdPrintf(lcdhd, "! (Pressione #)"); 
                    estado = 2; 
                }
                
                if (opcao != 5) {
                    lcdPosition(lcdhd, 0, 1);
                    lcdPrintf(lcdhd, "B: ");
                }
            }
            // Executar Cálculo e Benchmark (Tecla #)
            else if (key == '#') {
                // Validação de divisão por zero
                if (opcao == 4 && b == 0) {
                    lcdClear(lcdhd);
                    lcdPosition(lcdhd, 0, 0);
                    lcdPrintf(lcdhd, "Erro: Div por 0");
                    delay(2000);
                    // Reseta
                    a = 0; b = 0; estado = 0;
                    lcdClear(lcdhd); lcdPosition(lcdhd, 0, 0); lcdPrintf(lcdhd, "A: ");
                    continue;
                }

                lcdClear(lcdhd);
                lcdPosition(lcdhd, 0, 0);
                lcdPrintf(lcdhd, "Processando BM...");

                // Medição de Tempo Baseada no Código Original
                clock_t inicio = clock();

                for (int i = 0; i < ITERACOES; i++) {
                    switch (opcao) {
                        case 1: resultado = a + b; break;
                        case 2: resultado = a - b; break;
                        case 3: resultado = a * b; break;
                        case 4: resultado_div = (double)a / (double)b; break;
                        case 5: resultado = fatorial(a); break;
                    }
                }

                clock_t fim = clock();
                double tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;

                // Mostra o resultado final no LCD1602
                lcdClear(lcdhd);
                lcdPosition(lcdhd, 0, 0);
                if (opcao == 4) {
                    lcdPrintf(lcdhd, "R: %lf", resultado_div);
                } else {
                    lcdPrintf(lcdhd, "R: %d", resultado);
                }
                
                lcdPosition(lcdhd, 0, 1);
                lcdPrintf(lcdhd, "T: %f s", tempo_gasto);

                // Aguarda 5 segundos antes de permitir uma nova operação
                delay(5000);

                // Reseta variáveis para nova execução
                a = 0; b = 0; resultado = 0; resultado_div = 0.0; estado = 0;
                lcdClear(lcdhd);
                lcdPosition(lcdhd, 0, 0);
                lcdPrintf(lcdhd, "A: ");
            }
        }
        delay(20); // Evita pooling agressivo de CPU
    }
    return 0;
}