// Ej1_PCNT_Minimo.ino
// Un encoder en cuadratura x4 con el driver nuevo de PCNT (core Arduino-ESP32 3.x).
#include <Arduino.h>
#include "esp_arduino_version.h"
#include "driver/pulse_cnt.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Este sketch usa driver/pulse_cnt.h: requiere el core Arduino-ESP32 3.x"
#endif

constexpr int PIN_A = 34;        // OJO: pines de ejemplo, completar desde mapa-gpio.md
constexpr int PIN_B = 35;
constexpr int LIMITE = 32767;    // maximo del contador de 16 bits con signo

pcnt_unit_handle_t unidad = nullptr;

void setup() {
  Serial.begin(115200);

  // 1) Unidad: rango del contador. El hardware vuelve a 0 al llegar a un limite.
  pcnt_unit_config_t cfgUnidad = {};
  cfgUnidad.high_limit = LIMITE;
  cfgUnidad.low_limit  = -LIMITE;
  ESP_ERROR_CHECK(pcnt_new_unit(&cfgUnidad, &unidad));

  // 2) Filtro de glitch: ignora pulsos de menos de 1250 ns (100 ciclos de APB a 80 MHz)
  pcnt_glitch_filter_config_t cfgFiltro = {};
  cfgFiltro.max_glitch_ns = 1250;
  ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unidad, &cfgFiltro));

  // 3) Dos canales cruzados: canal A cuenta flancos de A mirando el nivel de B, y viceversa
  pcnt_chan_config_t cfgA = {};
  cfgA.edge_gpio_num  = PIN_A;
  cfgA.level_gpio_num = PIN_B;
  pcnt_channel_handle_t canalA = nullptr;
  ESP_ERROR_CHECK(pcnt_new_channel(unidad, &cfgA, &canalA));

  pcnt_chan_config_t cfgB = {};
  cfgB.edge_gpio_num  = PIN_B;
  cfgB.level_gpio_num = PIN_A;
  pcnt_channel_handle_t canalB = nullptr;
  ESP_ERROR_CHECK(pcnt_new_channel(unidad, &cfgB, &canalB));

  // 4) Acciones para x4. Orden de argumentos: (subida, bajada) y (nivel alto, nivel bajo)
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(canalA,
      PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
  ESP_ERROR_CHECK(pcnt_channel_set_level_action(canalA,
      PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
  ESP_ERROR_CHECK(pcnt_channel_set_edge_action(canalB,
      PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
  ESP_ERROR_CHECK(pcnt_channel_set_level_action(canalB,
      PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

  // 5) Habilitar, poner en cero y arrancar
  ESP_ERROR_CHECK(pcnt_unit_enable(unidad));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(unidad));
  ESP_ERROR_CHECK(pcnt_unit_start(unidad));
}

void loop() {
  int cuenta = 0;
  pcnt_unit_get_count(unidad, &cuenta);
  // Ojo: con el motor a 120 rpm este valor vuelve a 0 cada ~2,8 s (ver Ej2 y Ej3)
  Serial.printf("cuenta = %6d   (%.3f vueltas de rueda)\n", cuenta, cuenta / 5760.0f);
  delay(100);
}
