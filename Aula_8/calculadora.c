#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define ITERACOES 10000000

int fatorial(int n) {
    int fat = 1;
    for (int i = 1; i <= n; i++) {
        fat *= i;
    }
    return fat;
}

int main() {
    int opcao, bits;
    int a, b, resultado;

    clock_t inicio, fim;
    double tempo_gasto;

    printf("=== Benchmark de Operacoes Inteiras ===\n");

    printf("Escolha o modo (apenas comparativo):\n");
    printf("1 - 8 bits (simulado)\n");
    printf("2 - 16 bits (simulado)\n");
    printf("3 - 32 bits (nativo int)\n");
    scanf("%d", &bits);

    printf("\nEscolha operacao:\n");
    printf("1 - Soma\n");
    printf("2 - Subtracao\n");
    printf("3 - Multiplicacao\n");
    printf("4 - Fatorial\n");
    printf("Opcao: ");
    scanf("%d", &opcao);

    printf("Digite a e b (ou apenas a no fatorial): ");
    scanf("%d", &a);
    if (opcao != 4) scanf("%d", &b);

    // Simulação simples de bit-width (opcional)
    if (bits == 1) { // 8 bits
        a = (int8_t)a;
        b = (int8_t)b;
    } else if (bits == 2) { // 16 bits
        a = (int16_t)a;
        b = (int16_t)b;
    }

    inicio = clock();

    for (int i = 0; i < ITERACOES; i++) {
        switch (opcao) {
            case 1:
                resultado = a + b;
                break;
            case 2:
                resultado = a - b;
                break;
            case 3:
                resultado = a * b;
                break;
            case 4:
                resultado = fatorial(a);
                break;
            default:
                printf("Opcao invalida\n");
                return 1;
        }
    }

    fim = clock();

    tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;

    printf("\nResultado final: %d\n", resultado);
    printf("Tempo total: %f segundos (%d iteracoes)\n", tempo_gasto, ITERACOES);

    return 0;
}