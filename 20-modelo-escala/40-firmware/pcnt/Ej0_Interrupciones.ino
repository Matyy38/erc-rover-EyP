// Ej0_Interrupciones.ino
// Decodificacion x4 por interrupciones. SOLO para comparar con el PCNT:
// funciona, pero cada flanco interrumpe a la CPU.
#include <Arduino.h>

constexpr int PIN_A = 34;  // OJO: pines de ejemplo, completar desde 40-firmware/mapa-gpio.md
constexpr int PIN_B = 35;

volatile int32_t cuenta = 0;
volatile uint32_t interrupciones = 0;
volatile uint8_t estadoPrevio = 0;

// Tabla de transicion. Estado = (A << 1) | B. Indice = (previo << 2) | nuevo.
// +1: A adelanta a B | -1: B adelanta a A | 0: sin cambio o salto ilegal (2 bits a la vez)
DRAM_ATTR const int8_t TABLA[16] = {
   0, -1, +1,  0,     // previo = 00
  +1,  0,  0, -1,     // previo = 01
  -1,  0,  0, +1,     // previo = 10
   0, +1, -1,  0      // previo = 11
};

void IRAM_ATTR isrEncoder() {
  uint8_t nuevo = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);
  cuenta += TABLA[(estadoPrevio << 2) | nuevo];
  estadoPrevio = nuevo;
  interrupciones++;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_A, INPUT);   // IO34..39 no tienen pull-up interno: usar el externo de la placa
  pinMode(PIN_B, INPUT);
  estadoPrevio = (digitalRead(PIN_A) << 1) | digitalRead(PIN_B);
  attachInterrupt(digitalPinToInterrupt(PIN_A), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), isrEncoder, CHANGE);
}

void loop() {
  static uint32_t irqPrevias = 0;
  uint32_t irq = interrupciones;
  Serial.printf("cuenta = %ld   interrupciones/s = %lu\n",
                (long)cuenta, (unsigned long)(irq - irqPrevias));
  irqPrevias = irq;
  delay(1000);
}
