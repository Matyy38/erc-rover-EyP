// Ej3_CuatroRuedas.ino
// 4 encoders x4 (una unidad PCNT por rueda), posicion de 64 bits y velocidad en rpm.
// La lectura se hace en una tarea FreeRTOS a periodo fijo; loop() solo imprime.
#include <Arduino.h>
#include "esp_arduino_version.h"
#include "driver/pulse_cnt.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Requiere el core Arduino-ESP32 3.x"
#endif

// ------------------------- Parametros del rover -------------------------
constexpr int      CPR_MOTOR      = 64;      // cuentas x4 por vuelta del eje del motor
constexpr int      REDUCCION      = 90;
constexpr int      CUENTAS_VUELTA = CPR_MOTOR * REDUCCION;   // 5760 por vuelta de rueda
constexpr int      PERIODO_MS     = 10;      // periodo de muestreo de velocidad
constexpr int      LIMITE         = 32767;   // limite simetrico del contador de 16 bits
constexpr uint32_t FILTRO_NS      = 1250;    // 100 ciclos de APB; 0 = sin filtro
constexpr float    ALFA_EMA       = 0.2f;    // suavizado de la velocidad (1 = sin filtro)

struct ConfigRueda { const char* nombre; int pinA; int pinB; int8_t signo; };

// OJO: PINES DE EJEMPLO. Reemplazar por los de 40-firmware/mapa-gpio.md.
// OJO: SIGNOS A DEFINIR EN BANCO: +1 si la cuenta crece cuando esa rueda empuja
//      al rover hacia adelante. Las ruedas de un lado giran al reves que las del otro.
const ConfigRueda RUEDAS[4] = {
  {"FR", 34, 35, +1},   // J9
  {"FL", 36, 39, -1},   // J16
  {"RL", 32, 33, -1},   // J18
  {"RR", 25, 26, +1},   // J8
};

static bool ok(esp_err_t e, const char* que) {
  if (e != ESP_OK) Serial.printf("  [ERROR] %s: %s\n", que, esp_err_to_name(e));
  return e == ESP_OK;
}

// =========================== Clase EncoderPCNT ===========================
class EncoderPCNT {
public:
  bool begin(int pinA, int pinB, int8_t signo) {
    signo_ = signo;

    pcnt_unit_config_t cu = {};
    cu.high_limit = LIMITE;
    cu.low_limit  = -LIMITE;
    if (!ok(pcnt_new_unit(&cu, &unidad_), "pcnt_new_unit")) return false;

    if (FILTRO_NS > 0) {
      pcnt_glitch_filter_config_t cf = {};
      cf.max_glitch_ns = FILTRO_NS;
      if (!ok(pcnt_unit_set_glitch_filter(unidad_, &cf), "glitch_filter")) return false;
    }

    pcnt_channel_handle_t chA = nullptr, chB = nullptr;
    pcnt_chan_config_t cc = {};
    cc.edge_gpio_num = pinA;  cc.level_gpio_num = pinB;
    if (!ok(pcnt_new_channel(unidad_, &cc, &chA), "canal A")) return false;
    cc.edge_gpio_num = pinB;  cc.level_gpio_num = pinA;
    if (!ok(pcnt_new_channel(unidad_, &cc, &chB), "canal B")) return false;

    pcnt_channel_set_edge_action(chA, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                      PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(chA, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                       PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(chB, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                      PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(chB, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                       PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

    // Watch points en +-LIMITE: dejan bien definido el reinicio a 0 en los limites
    // (lo mismo que exige accum_count). Sin callback registrado no hay interrupciones.
    if (!ok(pcnt_unit_add_watch_point(unidad_,  LIMITE), "watch +")) return false;
    if (!ok(pcnt_unit_add_watch_point(unidad_, -LIMITE), "watch -")) return false;

    if (!ok(pcnt_unit_enable(unidad_), "enable")) return false;
    pcnt_unit_clear_count(unidad_);
    pcnt_unit_start(unidad_);
    ultimo_ = 0;
    pos_ = 0;
    return true;
  }

  // Llamar a periodo fijo. Devuelve las cuentas avanzadas desde la llamada anterior,
  // ya con el signo de la rueda, y acumula la posicion en 64 bits.
  int32_t actualizar() {
    int ahora = 0;
    pcnt_unit_get_count(unidad_, &ahora);
    int32_t d = ahora - ultimo_;
    ultimo_ = ahora;
    // El registro es periodico con periodo LIMITE: llevamos d al rango (-LIMITE/2, LIMITE/2]
    while (d >  LIMITE / 2) d -= LIMITE;
    while (d < -LIMITE / 2) d += LIMITE;
    d *= signo_;
    pos_ += d;
    return d;
  }

  int64_t posicion() const { return pos_; }

private:
  pcnt_unit_handle_t unidad_ = nullptr;
  int     ultimo_ = 0;
  int64_t pos_    = 0;
  int8_t  signo_  = 1;
};

// ======================= Estado compartido y tarea =======================
EncoderPCNT enc[4];

struct Medicion { int64_t pos[4]; float rpm[4]; };
Medicion compartida = {};                       // la escribe la tarea, la lee loop()
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void tareaEncoders(void*) {
  const float K_RPM = 60000.0f / (CUENTAS_VUELTA * PERIODO_MS);   // rpm por cuenta
  float rpmFiltrada[4] = {0, 0, 0, 0};
  TickType_t t0 = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&t0, pdMS_TO_TICKS(PERIODO_MS));   // periodo fijo, sin deriva
    Medicion m;
    for (int i = 0; i < 4; i++) {
      int32_t d = enc[i].actualizar();
      rpmFiltrada[i] += ALFA_EMA * (d * K_RPM - rpmFiltrada[i]);
      m.pos[i] = enc[i].posicion();
      m.rpm[i] = rpmFiltrada[i];
    }
    portENTER_CRITICAL(&mux);
    compartida = m;
    portEXIT_CRITICAL(&mux);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  for (int i = 0; i < 4; i++) {
    Serial.printf("Rueda %s: A=IO%d B=IO%d\n", RUEDAS[i].nombre, RUEDAS[i].pinA,
                  RUEDAS[i].pinB);
    if (!enc[i].begin(RUEDAS[i].pinA, RUEDAS[i].pinB, RUEDAS[i].signo)) {
      Serial.println("Fallo la configuracion. Programa detenido.");
      while (true) delay(1000);
    }
  }
  xTaskCreatePinnedToCore(tareaEncoders, "encoders", 4096, nullptr, 5, nullptr, 1);
}

void loop() {
  Medicion m;
  portENTER_CRITICAL(&mux);
  m = compartida;
  portEXIT_CRITICAL(&mux);
  for (int i = 0; i < 4; i++) {
    Serial.printf("%s %9lld (%7.2f v) %7.1f rpm | ", RUEDAS[i].nombre,
                  (long long)m.pos[i], m.pos[i] / (float)CUENTAS_VUELTA, m.rpm[i]);
  }
  Serial.println();
  delay(200);
}
