// Ej2_PCNT_Acumulado.ino
// Igual que Ej1, pero el driver acumula los desbordes: la cuenta pasa a ser de 32 bits.
#include <Arduino.h>
#include "esp_arduino_version.h"
#include "driver/pulse_cnt.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Requiere el core Arduino-ESP32 3.x"
#endif

constexpr int PIN_A = 34;        // OJO: pines de ejemplo
constexpr int PIN_B = 35;
constexpr int LIMITE = 32767;
constexpr int CUENTAS_VUELTA = 64 * 90;   // 5760

pcnt_unit_handle_t unidad = nullptr;

void setup() {
  Serial.begin(115200);

  pcnt_unit_config_t cfgUnidad = {};
  cfgUnidad.high_limit = LIMITE;
  cfgUnidad.low_limit  = -LIMITE;
  cfgUnidad.flags.accum_count = 1;          // (1) el driver suma cada desborde
  ESP_ERROR_CHECK(pcnt_new_unit(&cfgUnidad, &unidad));

  pcnt_glitch_filter_config_t cfgFiltro = {};
  cfgFiltro.max_glitch_ns = 1250;
  ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unidad, &cfgFiltro));

  pcnt_chan_config_t cfg = {};
  pcnt_channel_handle_t canalA = nullptr, canalB = nullptr;
  cfg.edge_gpio_num = PIN_A;  cfg.level_gpio_num = PIN_B;
  ESP_ERROR_CHECK(pcnt_new_channel(unidad, &cfg, &canalA));
  cfg.edge_gpio_num = PIN_B;  cfg.level_gpio_num = PIN_A;
  ESP_ERROR_CHECK(pcnt_new_channel(unidad, &cfg, &canalB));

  pcnt_channel_set_edge_action(canalA, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                               PCNT_CHANNEL_EDGE_ACTION_INCREASE);
  pcnt_channel_set_level_action(canalA, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
  pcnt_channel_set_edge_action(canalB, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                               PCNT_CHANNEL_EDGE_ACTION_DECREASE);
  pcnt_channel_set_level_action(canalB, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

  // (2) watch points en ambos limites: asi el driver se entera de cada desborde
  ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unidad,  LIMITE));
  ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unidad, -LIMITE));

  ESP_ERROR_CHECK(pcnt_unit_enable(unidad));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(unidad));   // necesario tras agregar watch points
  ESP_ERROR_CHECK(pcnt_unit_start(unidad));
}

void loop() {
  int posicion = 0;
  pcnt_unit_get_count(unidad, &posicion);   // (3) ya incluye los desbordes acumulados
  Serial.printf("posicion = %ld cuentas = %.3f vueltas\n",
                (long)posicion, posicion / (float)CUENTAS_VUELTA);
  delay(200);
}
