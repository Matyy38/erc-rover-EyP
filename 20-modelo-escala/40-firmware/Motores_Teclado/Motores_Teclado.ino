// Motores_Teclado.ino
// Prueba de los 4 canales BTS7960 desde el monitor serie, leyendo los 4 encoders por PCNT.
// Base: pcnt/Ej3_CuatroRuedas.ino (encoders) y Test_Motors/Test_Motors.ino (LEDC).
// Pines segun 40-firmware/mapa-gpio.md (26-oct-2026).
//
// Monitor serie a 115200. Cada letra se manda con Enter (se ignoran \r y \n):
//   q / a   canal 1 sentido R / sentido L        t / g   los 4 canales sentido R / L
//   w / s   canal 2                              1..4    frena ese canal
//   e / d   canal 3                              x o ' ' frena todo
//   r / f   canal 4                              + / -   duty +-10 %
//   p       pausa / reanuda la telemetria        0       pone a cero las posiciones
//   h o ?   ayuda
//
// "Sentido R" = R_PWM con duty y L_PWM en 0. Que eso sea adelante o atras depende del cableado
// del motor: se define en banco. Freno = las dos entradas en 0 (las dos llaves bajas del
// BTS7960 encendidas, motor en cortocircuito).
#include <Arduino.h>
#include "esp_arduino_version.h"
#include "driver/pulse_cnt.h"

#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Requiere el core Arduino-ESP32 3.x"
#endif

// ------------------------- Parametros de motores -------------------------
constexpr uint32_t PWM_FREQ       = 20000;   // 20 kHz: inaudible, y es lo que suponen el
                                             // filtro de Vout_CS y el ADS1115
constexpr uint8_t  PWM_RES        = 10;      // 10 bits; a 20 kHz el maximo son 11
constexpr int      DUTY_MAX       = (1 << PWM_RES) - 1;
constexpr int      DUTY_INICIAL   = 30;      // % al arrancar
constexpr int      PASO_DUTY      = 10;      // % por cada + o -
constexpr int      RAMPA_MS       = 300;     // tiempo de 0 a 100 % (y de +100 a -100 el doble)
constexpr uint32_t TIEMPO_MAX_MS  = 5000;    // parada automatica sin comandos; 0 = desactivada

struct ConfigCanal { const char* nombre; uint8_t pinR; uint8_t pinL; char teclaR; char teclaL; };

// La correspondencia driver <-> rueda todavia no esta fijada: los canales van por driver.
const ConfigCanal CANALES[4] = {
  {"M1", 25, 26, 'q', 'a'},   // J10
  {"M2", 27, 14, 'w', 's'},   // J11
  {"M3", 13,  4, 'e', 'd'},   // J15
  {"M4",  5, 23, 'r', 'f'},   // J19. R_PWM_4 pasa por U1 (NLAS4157); L_PWM_4 con pull-up (V1)
};

// ------------------------- Parametros de encoders ------------------------
constexpr int      CPR_MOTOR      = 64;      // cuentas x4 por vuelta del eje del motor
constexpr int      REDUCCION      = 90;
constexpr int      CUENTAS_VUELTA = CPR_MOTOR * REDUCCION;   // 5760 por vuelta de rueda
constexpr int      PERIODO_MS     = 10;      // periodo de la tarea de control
constexpr int      LIMITE         = 32767;   // limite simetrico del contador de 16 bits
constexpr uint32_t FILTRO_NS      = 1250;    // 100 ciclos de APB; 0 = sin filtro
constexpr float    ALFA_EMA       = 0.2f;    // suavizado de la velocidad (1 = sin filtro)

struct ConfigRueda { const char* nombre; int pinA; int pinB; int8_t signo; };

// OJO: SIGNOS A DEFINIR EN BANCO: +1 si la cuenta crece cuando esa rueda empuja
//      al rover hacia adelante. Las ruedas de un lado giran al reves que las del otro.
const ConfigRueda RUEDAS[4] = {
  {"FR", 36, 39, +1},   // J9
  {"FL", 34, 35, -1},   // J16
  {"RL", 16, 17, -1},   // J18
  {"RR", 18, 19, +1},   // J8
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

  void    cero()           { pos_ = 0; }
  int64_t posicion() const { return pos_; }

private:
  pcnt_unit_handle_t unidad_ = nullptr;
  int     ultimo_ = 0;
  int64_t pos_    = 0;
  int8_t  signo_  = 1;
};

// ======================= Estado compartido y tarea =======================
EncoderPCNT enc[4];

struct Medicion { int64_t pos[4]; float rpm[4]; int duty[4]; };
Medicion compartida = {};                       // la escribe la tarea, la lee loop()

int      objetivo[4] = {0, 0, 0, 0};            // duty con signo pedido; lo escribe loop()
uint32_t vence[4]    = {0, 0, 0, 0};            // millis de parada automatica; 0 = no vence
uint8_t  parados     = 0;                       // bit i: la tarea paro el canal i por tiempo
bool     pedirCero   = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void aplicarDuty(int i, int d) {
  ledcWrite(CANALES[i].pinR, d > 0 ?  d : 0);
  ledcWrite(CANALES[i].pinL, d < 0 ? -d : 0);
}

void tareaControl(void*) {
  const float K_RPM = 60000.0f / (CUENTAS_VUELTA * PERIODO_MS);   // rpm por cuenta
  const int   PASO  = max(1, DUTY_MAX * PERIODO_MS / RAMPA_MS);    // duty por periodo
  float rpmFiltrada[4] = {0, 0, 0, 0};
  int   aplicado[4]    = {0, 0, 0, 0};
  TickType_t t0 = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&t0, pdMS_TO_TICKS(PERIODO_MS));   // periodo fijo, sin deriva

    // --- Motores: parada automatica y rampa hacia el objetivo ---
    int  obj[4];
    bool cero;
    uint32_t ahora = millis();
    portENTER_CRITICAL(&mux);
    for (int i = 0; i < 4; i++) {
      if (vence[i] != 0 && (int32_t)(ahora - vence[i]) >= 0) {
        objetivo[i] = 0;
        vence[i] = 0;
        parados |= 1 << i;
      }
      obj[i] = objetivo[i];
    }
    cero = pedirCero;
    pedirCero = false;
    portEXIT_CRITICAL(&mux);

    for (int i = 0; i < 4; i++) {
      int d = aplicado[i];
      if      (obj[i] > d) d = min(obj[i], d + PASO);
      else if (obj[i] < d) d = max(obj[i], d - PASO);
      if (d != aplicado[i]) {
        aplicado[i] = d;
        aplicarDuty(i, d);
      }
    }

    // --- Encoders ---
    Medicion m;
    for (int i = 0; i < 4; i++) {
      if (cero) enc[i].cero();
      int32_t d = enc[i].actualizar();
      rpmFiltrada[i] += ALFA_EMA * (d * K_RPM - rpmFiltrada[i]);
      m.pos[i]  = enc[i].posicion();
      m.rpm[i]  = rpmFiltrada[i];
      m.duty[i] = aplicado[i];
    }
    portENTER_CRITICAL(&mux);
    compartida = m;
    portEXIT_CRITICAL(&mux);
  }
}

// ============================ Comandos serie =============================
int  dutyPct     = DUTY_INICIAL;
bool telemetria  = true;

int pctACuentas(int pct) { return pct * DUTY_MAX / 100; }

// sentido: +1 = R, -1 = L, 0 = freno
void mandar(int i, int sentido) {
  portENTER_CRITICAL(&mux);
  objetivo[i] = sentido * pctACuentas(dutyPct);
  vence[i] = (sentido != 0 && TIEMPO_MAX_MS > 0) ? (millis() + TIEMPO_MAX_MS) | 1 : 0;
  portEXIT_CRITICAL(&mux);
  if (sentido == 0) Serial.printf(">> %s freno\n", CANALES[i].nombre);
  else Serial.printf(">> %s sentido %c al %d %%\n", CANALES[i].nombre,
                     sentido > 0 ? 'R' : 'L', dutyPct);
}

void frenarTodo() {
  portENTER_CRITICAL(&mux);
  for (int i = 0; i < 4; i++) { objetivo[i] = 0; vence[i] = 0; }
  portEXIT_CRITICAL(&mux);
  Serial.println(">> FRENO todos los canales");
}

// Cambia el duty y lo aplica a los canales que estan girando, conservando el sentido.
void cambiarDuty(int delta) {
  dutyPct = constrain(dutyPct + delta, PASO_DUTY, 100);
  portENTER_CRITICAL(&mux);
  for (int i = 0; i < 4; i++) {
    if (objetivo[i] > 0) objetivo[i] =  pctACuentas(dutyPct);
    if (objetivo[i] < 0) objetivo[i] = -pctACuentas(dutyPct);
  }
  portEXIT_CRITICAL(&mux);
  Serial.printf(">> duty = %d %%\n", dutyPct);
}

void ayuda() {
  Serial.println();
  Serial.println("Canal  pin R  pin L  sentido R  sentido L  frena");
  for (int i = 0; i < 4; i++) {
    Serial.printf("%-5s  IO%-3d  IO%-3d      %c          %c        %d\n", CANALES[i].nombre,
                  CANALES[i].pinR, CANALES[i].pinL, CANALES[i].teclaR, CANALES[i].teclaL, i + 1);
  }
  Serial.println("t / g: los 4 en sentido R / L   x o espacio: frena todo   + / -: duty");
  Serial.println("p: pausa telemetria   0: posiciones a cero   h: esta ayuda");
  Serial.printf("Duty actual %d %%, rampa %d ms, PWM %lu Hz", dutyPct, RAMPA_MS,
                (unsigned long)PWM_FREQ);
  if (TIEMPO_MAX_MS > 0) Serial.printf(", parada automatica a los %lu ms",
                                       (unsigned long)TIEMPO_MAX_MS);
  Serial.println("\n");
}

void procesar(char c) {
  for (int i = 0; i < 4; i++) {
    if (c == CANALES[i].teclaR) { mandar(i, +1); return; }
    if (c == CANALES[i].teclaL) { mandar(i, -1); return; }
    if (c == '1' + i)           { mandar(i,  0); return; }
  }
  switch (c) {
    case 't': for (int i = 0; i < 4; i++) mandar(i, +1); break;
    case 'g': for (int i = 0; i < 4; i++) mandar(i, -1); break;
    case 'x': case ' ': frenarTodo(); break;
    case '+': cambiarDuty(+PASO_DUTY); break;
    case '-': cambiarDuty(-PASO_DUTY); break;
    case 'p': telemetria = !telemetria; break;
    case '0':
      portENTER_CRITICAL(&mux);
      pedirCero = true;
      portEXIT_CRITICAL(&mux);
      Serial.println(">> posiciones a cero");
      break;
    case 'h': case '?': ayuda(); break;
    default: Serial.printf(">> tecla '%c' sin uso (h = ayuda)\n", c);
  }
}

// ================================ Setup ==================================
void setup() {
  // Primero el PWM, y en 0: cuanto antes se toma el control de las lineas de los drivers,
  // menos tiempo queda el canal 4 en el estado de arranque de U1 (V1).
  for (int i = 0; i < 4; i++) {
    ledcAttach(CANALES[i].pinR, PWM_FREQ, PWM_RES);
    ledcAttach(CANALES[i].pinL, PWM_FREQ, PWM_RES);
    aplicarDuty(i, 0);
  }

  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Prueba de motores por teclado (BTS7960 + PCNT) ===");

  for (int i = 0; i < 4; i++) {
    Serial.printf("Rueda %s: A=IO%d B=IO%d\n", RUEDAS[i].nombre, RUEDAS[i].pinA,
                  RUEDAS[i].pinB);
    if (!enc[i].begin(RUEDAS[i].pinA, RUEDAS[i].pinB, RUEDAS[i].signo)) {
      Serial.println("Fallo la configuracion. Programa detenido.");
      while (true) delay(1000);
    }
  }
  xTaskCreatePinnedToCore(tareaControl, "control", 4096, nullptr, 5, nullptr, 1);
  ayuda();
}

// ================================= Loop ==================================
void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') continue;
    procesar(tolower(c));
  }

  uint8_t p;
  portENTER_CRITICAL(&mux);
  p = parados;
  parados = 0;
  portEXIT_CRITICAL(&mux);
  for (int i = 0; i < 4; i++) {
    if (p & (1 << i)) Serial.printf(">> %s parada automatica\n", CANALES[i].nombre);
  }

  static uint32_t tImpresion = 0;
  if (telemetria && millis() - tImpresion >= 200) {
    tImpresion = millis();
    Medicion m;
    portENTER_CRITICAL(&mux);
    m = compartida;
    portEXIT_CRITICAL(&mux);
    for (int i = 0; i < 4; i++) {
      Serial.printf("%s %+4ld%% ", CANALES[i].nombre, lroundf(m.duty[i] * 100.0f / DUTY_MAX));
    }
    Serial.print("||");
    for (int i = 0; i < 4; i++) {
      Serial.printf(" %s %8lld (%6.2f v) %6.1f rpm |", RUEDAS[i].nombre,
                    (long long)m.pos[i], m.pos[i] / (float)CUENTAS_VUELTA, m.rpm[i]);
    }
    Serial.println();
  }
  delay(5);
}
