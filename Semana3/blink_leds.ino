#include <Arduino.h>

// Definindo os pinos das saídas
const int pinos[] = {4, 5, 6, 7};
const int totalPinos = 4;

void setup() {
  // Configura todos os pinos da lista como SAÍDA (OUTPUT)
  for (int i = 0; i < totalPinos; i++) {
    pinMode(pinos[i], OUTPUT);
  }
}

void loop() {
  // Liga todas as saídas (HIGH)
  for (int i = 0; i < totalPinos; i++) {
    digitalWrite(pinos[i], HIGH);
  }
  
  delay(1000); // Espera 1 segundo ligado

  // Desliga todas as saídas (LOW)
  for (int i = 0; i < totalPinos; i++) {
    digitalWrite(pinos[i], LOW);
  }
  
  delay(1000); // Espera 1 segundo desligado
}