#include <stdio.h>
#include <time.h> // Incluída para medir o tempo de CPU
#include <stdint.h>

// Definindo o número de iterações para o benchmark
#define ITERACOES 10000000 

// Função para calcular o fatorial
int fatorial(int n) {
    int fat = 1;
    for (int i = 1; i <= n; i++) {
        fat *= i;
    }
    return fat;
}

int main() {
    int opcao;
    int bits;
    
    // Variáveis para medição de tempo
    clock_t inicio, fim;
    double tempo_gasto;

    printf("=== Calculadora de n bits com Benchmark ===\n");

    printf("Escolha a quantidade de bits:\n");
    printf("1 - 8 bits\n");
    printf("2 - 16 bits\n");
    printf("3 - 32 bits\n");
    scanf("%d", &bits);

    switch(bits){
    case 1: {
        printf("Valores permitidos: -128 a 127\n\n");

        printf("Escolha a operacao:\n");
        printf("1 - Soma\n");
        printf("2 - Subtracao\n");
        printf("3 - Multiplicacao\n");
        printf("4 - Fatorial\n");
        printf("Opcao: ");
        scanf("%d", &opcao);

        int temp;
        int8_t a, b;
        volatile int8_t resultado;

        switch(opcao) {

            case 1:
                printf("Digite o primeiro numero (-128 a 127): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-128 a 127): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Soma
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a + b; 
                }
                fim = clock();
                
                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d somas: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 2:
                printf("Digite o primeiro numero (-128 a 127): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-128 a 127): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Subtração
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a - b;
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d subtracoes: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 3:
                printf("Digite o primeiro numero (-128 a 127): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-128 a 127): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Multiplicação
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a * b;
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d multiplicacoes: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 4:
                printf("Digite um numero (-128 a 127): ");
                scanf("%d", &a);

                if(a < 0 || a > 15) {
                    printf("Erro: numero fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Fatorial
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = fatorial(a);
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("%d! = %d\n", a, resultado);
                printf("Tempo total para %d fatoriais: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            default:
                printf("Opcao invalida!\n");
        }
        break;
    }

    case 2: {
        printf("Valores permitidos: -32.768 a 32.767\n\n");

        printf("Escolha a operacao:\n");
        printf("1 - Soma\n");
        printf("2 - Subtracao\n");
        printf("3 - Multiplicacao\n");
        printf("4 - Fatorial\n");
        printf("Opcao: ");
        scanf("%d", &opcao);

        int temp;
        int16_t a, b;
        volatile int16_t resultado;

        switch(opcao) {

            case 1:
                printf("Digite o primeiro numero (-32.768 a 32.767): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-32.768 a 32.767): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Soma
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a + b; 
                }
                fim = clock();
                
                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d somas: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 2:
                printf("Digite o primeiro numero (-32.768 a 32.767): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-32.768 a 32.767): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Subtração
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a - b;
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d subtracoes: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 3:
                printf("Digite o primeiro numero (-32.768 a 32.767): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-32.768 a 32.767): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Multiplicação
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a * b;
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d multiplicacoes: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 4:
                printf("Digite um numero (-32.768 a 32.767): ");
                scanf("%d", &a);

                if(a < 0 || a > 15) {
                    printf("Erro: numero fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Fatorial
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = fatorial(a);
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("%d! = %d\n", a, resultado);
                printf("Tempo total para %d fatoriais: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            default:
                printf("Opcao invalida!\n");
        }
        break;
    }

    case 3: {
        printf("Valores permitidos: -2.147.483.648 a 2.147.483.647\n\n");

        printf("Escolha a operacao:\n");
        printf("1 - Soma\n");
        printf("2 - Subtracao\n");
        printf("3 - Multiplicacao\n");
        printf("4 - Fatorial\n");
        printf("Opcao: ");
        scanf("%d", &opcao);

        int temp;
        int32_t a, b;
        volatile int32_t resultado;

        switch(opcao) {

            case 1:
                printf("Digite o primeiro numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Soma
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a + b; 
                }
                fim = clock();
                
                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d somas: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 2:
                printf("Digite o primeiro numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Subtração
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a - b;
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d subtracoes: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 3:
                printf("Digite o primeiro numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &a);
                printf("Digite o segundo numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &b);

                if(a < 0 || a > 15 || b < 0 || b > 15) {
                    printf("Erro: numeros fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Multiplicação
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = a * b;
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("Resultado = %d\n", resultado);
                printf("Tempo total para %d multiplicacoes: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            case 4:
                printf("Digite um numero (-2.147.483.648 a 2.147.483.647): ");
                scanf("%d", &a);

                if(a < 0 || a > 15) {
                    printf("Erro: numero fora do intervalo de 4 bits.\n");
                    return 1;
                }

                // Início do Benchmark para Fatorial
                inicio = clock();
                for (int i = 0; i < ITERACOES; i++) {
                    resultado = fatorial(a);
                }
                fim = clock();

                tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
                printf("%d! = %d\n", a, resultado);
                printf("Tempo total para %d fatoriais: %f segundos\n", ITERACOES, tempo_gasto);
                break;

            default:
                printf("Opcao invalida!\n");
        }
        break;

    }
    }
    return 0;
}
