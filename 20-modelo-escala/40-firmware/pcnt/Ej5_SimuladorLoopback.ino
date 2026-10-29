// Ej5_SimuladorLoopback.ino
// Prueba del codigo SIN motor: el mismo ESP32 genera una cuadratura por dos salidas
// y la cuenta con PCNT por dos entradas. Puentear OUT_A->IN_A y OUT_B->IN_B con cables.
// Ideal: hacerlo en un DevKit suelto. No usar pines de encoders ni de arranque.
#include <Arduino.h>
#include "esp_arduino_version.h"
#include "driver/pulse_cnt.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Requiere el core Arduino-ESP32 3.x"
#endif

constexpr int OUT_A = 16, OUT_B = 17;      // salidas del "encoder falso" (ejemplo)
constexpr int IN_A  = 27, IN_B  = 14;      // entradas del PCNT           (ejemplo)
constexpr int CUARTO_US = 86;              // 1/4 de periodo: 2,9 kHz como el CQRobot
constexpr int CICLOS = 2000;               // ciclos completos por prueba
constexpr int LIMITE = 32767;

pcnt_unit_handle_t unidad = nullptr;

void generar(int ciclos, bool adelante) {
  // Secuencia AB para A adelantando a B: 10 -> 11 -> 01 -> 00
  static const uint8_t SEC[4][2] = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
  for (int c = 0; c < ciclos; c++) {
    for (int k = 0; k < 4; k++) {
      int j = adelante ? k : (6 - k) % 4;   // atras: 01 -> 11 -> 10 -> 00
      digitalWrite(OUT_A, SEC[j][0]);
      digitalWrite(OUT_B, SEC[j][1]);
      delayMicroseconds(CUARTO_US);
    }
  }
}

int leer() { int c = 0; pcnt_unit_get_count(unidad, &c); return c; }

void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(OUT_A, OUTPUT); digitalWrite(OUT_A, LOW);
  pinMode(OUT_B, OUTPUT); digitalWrite(OUT_B, LOW);

  pcnt_unit_config_t cu = {};
  cu.high_limit = LIMITE;
  cu.low_limit  = -LIMITE;
  ESP_ERROR_CHECK(pcnt_new_unit(&cu, &unidad));
  pcnt_glitch_filter_config_t cf = {};
  cf.max_glitch_ns = 1250;
  ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unidad, &cf));

  pcnt_chan_config_t cc = {};
  pcnt_channel_handle_t chA = nullptr, chB = nullptr;
  cc.edge_gpio_num = IN_A;  cc.level_gpio_num = IN_B;
  ESP_ERROR_CHECK(pcnt_new_channel(unidad, &cc, &chA));
  cc.edge_gpio_num = IN_B;  cc.level_gpio_num = IN_A;
  ESP_ERROR_CHECK(pcnt_new_channel(unidad, &cc, &chB));
  pcnt_channel_set_edge_action(chA, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                    PCNT_CHANNEL_EDGE_ACTION_INCREASE);
  pcnt_channel_set_level_action(chA, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                     PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
  pcnt_channel_set_edge_action(chB, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                    PCNT_CHANNEL_EDGE_ACTION_DECREASE);
  pcnt_channel_set_level_action(chB, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                     PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
  ESP_ERROR_CHECK(pcnt_unit_enable(unidad));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(unidad));
  ESP_ERROR_CHECK(pcnt_unit_start(unidad));
}

void loop() {
  pcnt_unit_clear_count(unidad);
  generar(CICLOS, true);
  int ida = leer();
  generar(CICLOS, false);
  int vuelta = leer();
  bool pasa = (ida == 4 * CICLOS) && (vuelta == 0);
  Serial.printf("adelante: %d (esperado %d)  ida y vuelta: %d (esperado 0)  -> %s\n",
                ida, 4 * CICLOS, vuelta, pasa ? "PASA" : "FALLA");
  delay(2000);
}
