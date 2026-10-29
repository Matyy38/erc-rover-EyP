// Ej4_DiagnosticoCanales.ino
// Valida los 8 cables (A y B de las 4 ruedas) por separado: una unidad PCNT por cable,
// contando subida Y bajada. Sirve para el pendiente "repetir la prueba en los 4 canales".
// Procedimiento: enviar 'r' por el monitor serie, girar cada rueda UNA vuelta a mano.
// Esperado por cable: 16 pulsos x 2 flancos x 90 = 2880 flancos por vuelta de rueda.
#include <Arduino.h>
#include "esp_arduino_version.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Requiere el core Arduino-ESP32 3.x"
#endif

constexpr int LIMITE = 32767;
// OJO: pines de ejemplo, en el mismo orden que mapa-gpio.md
const int   PINES[8]   = { 34,     35,     36,     39,     32,     33,     25,     26   };
const char* NOMBRES[8] = {"FR.A", "FR.B", "FL.A", "FL.B", "RL.A", "RL.B", "RR.A", "RR.B"};

pcnt_unit_handle_t unidad[8];

void setup() {
  Serial.begin(115200);
  delay(300);
  for (int i = 0; i < 8; i++) {
    pcnt_unit_config_t cu = {};
    cu.high_limit = LIMITE;
    cu.low_limit  = -LIMITE;
    ESP_ERROR_CHECK(pcnt_new_unit(&cu, &unidad[i]));

    pcnt_glitch_filter_config_t cf = {};
    cf.max_glitch_ns = 1250;
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unidad[i], &cf));

    pcnt_chan_config_t cc = {};
    cc.edge_gpio_num  = PINES[i];
    cc.level_gpio_num = -1;              // sin senial de control: IO virtual...
    cc.flags.virt_level_io_level = 1;    // ...fija en alto
    pcnt_channel_handle_t ch = nullptr;
    ESP_ERROR_CHECK(pcnt_new_channel(unidad[i], &cc, &ch));
    // Ambos flancos suman: medimos actividad del cable, no sentido de giro
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(ch, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                                     PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_unit_enable(unidad[i]));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unidad[i]));
    ESP_ERROR_CHECK(pcnt_unit_start(unidad[i]));
  }
  Serial.println("Listo. 'r' = poner contadores en cero.");
}

void loop() {
  if (Serial.available() && Serial.read() == 'r') {
    for (int i = 0; i < 8; i++) pcnt_unit_clear_count(unidad[i]);
  }
  for (int i = 0; i < 8; i += 2) {         // de a pares: A y B de la misma rueda
    int fa = 0, fb = 0;
    pcnt_unit_get_count(unidad[i], &fa);
    pcnt_unit_get_count(unidad[i + 1], &fb);
    int na = gpio_get_level((gpio_num_t)PINES[i]);
    int nb = gpio_get_level((gpio_num_t)PINES[i + 1]);
    Serial.printf("%s %5d [%d]  %s %5d [%d]  dif %3d || ", NOMBRES[i], fa, na,
                  NOMBRES[i + 1], fb, nb, fa - fb);
  }
  Serial.println();
  delay(500);
}
