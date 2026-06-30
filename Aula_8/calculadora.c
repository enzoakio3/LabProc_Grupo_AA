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
    double resultado_div;

    clock_t inicio, fim;
    double tempo_gasto;

    printf("=== Calculadora ===\n");

    printf("\nEscolha operacao:\n");
    printf("1 - Soma\n");
    printf("2 - Subtracao\n");
    printf("3 - Multiplicacao\n");
    printf("4 - Divisao\n");
    printf("5 - Fatorial\n");
    printf("Opcao: ");
    scanf("%d", &opcao);

    printf("Digite a: ");
    scanf("%d", &a);

    if (opcao != 5) {
        do {
            printf("Digite b: ");
            scanf("%d", &b);
            if (opcao == 4 && b == 0){
                printf("Erro: divisao por zero. Tente novamente.\n");
            }
        } while (opcao == 4 && b == 0);
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
                resultado_div = (double)a / (double)b;
                break;
            case 5:
                resultado = fatorial(a);
                break;
            default:
                printf("Opcao invalida\n");
                return 1;
        }
    }

    fim = clock();

    tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;
    if(opcao == 4)
        printf("\nResultado final: %lf\n", resultado);
    printf("\nResultado final: %d\n", resultado);
    printf("Tempo total: %f segundos (%d iteracoes)\n", tempo_gasto, ITERACOES);

    return 0;
}