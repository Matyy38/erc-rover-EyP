/*
 * C2b_ACS712_ADS1115.ino — ACS712-30A leído por el ADS1115 y por el ADC del ESP32 en el mismo
 * instante, comparados contra el multímetro (guía de labo, punto 8, paso 4).
 *
 * estado: SIN PROBAR. Borrador para el labo, 29-oct-2026.
 * ESP32 suelto en protoboard, NO la placa del rover (única prueba donde se admite protoboard).
 *
 * Armado (el de C2a, más el ADS1115):
 *   ACS712 VCC <- 5 V del devkit (VIN)      ACS712 GND -> GND
 *   ACS OUT -> 10 k -> nodo X -> 20 k -> GND     nodo X -> GPIO34 y AIN1, 100 nF a GND   (2/3)
 *   5 V     -> 20 k -> nodo Y -> 10 k -> GND     nodo Y -> GPIO35 y AIN2, 100 nF a GND   (1/3)
 *   ADS1115: VDD <- 3V3 (NUNCA 5 V), GND, ADDR -> GND (0x48), SDA -> GPIO21, SCL -> GPIO22
 *   Con los divisores ninguna entrada pasa de 3,33 V (ACS saturado a 5 V).
 *   Motor: la corriente entra y sale por la bornera del ACS712, con cable de potencia.
 *   Multímetro de 10 A en serie como referencia.
 *
 * Cómo compara "en el mismo instante": se dispara la conversión del ADS (integra ~7,8 ms a
 * 128 muestras/s) y, mientras convierte, el ESP32 promedia su ADC en esa misma ventana. Las dos
 * vías ven el mismo tramo de corriente, con el mismo PWM promediado.
 *
 * Corriente (salida ratiométrica): S = 66 mV/A · VCC/5,   I = (Vout − cero·VCC) / S
 *
 * Comandos por serie (terminar con Enter):
 *   t          tara: sin corriente por el sensor (hacerlo al arrancar y cada tanto)
 *   m<valor>   anota el multímetro en A, p. ej. "m2.35"; imprime los errores de cada vía
 *   r          resumen: sensibilidad ajustada de cada vía con los puntos anotados
 *   b          borrar los puntos anotados
 *
 * Salida: etiqueta:valor, sirve en el Serial Plotter. Las líneas con '#' son comentarios.
 * Sin bibliotecas externas: solo Wire (núcleo Arduino-ESP32 2.x o 3.x).
 */

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ---------------- Configuración ----------------
constexpr int      PIN_SDA    = 21;
constexpr int      PIN_SCL    = 22;
constexpr uint32_t I2C_HZ     = 100000;
constexpr uint8_t  ADS_ADDR   = 0x48;
constexpr int      PIN_SALIDA = 34;      // ADC1 (GPIO15 no: es ADC2 y pin de arranque)
constexpr int      PIN_VCC    = 35;      // ADC1
constexpr uint8_t  CANAL_SALIDA = 1;     // AIN1, como en la placa
constexpr uint8_t  CANAL_VCC    = 2;     // AIN2, como en la placa

constexpr float K_SALIDA = 1.5f;         // inversa del divisor 2/3
constexpr float K_VCC    = 3.0f;         // inversa del divisor 1/3
constexpr float SENS_5V  = 0.066f;       // V/A con VCC = 5,0 V (hoja de datos: 63 a 69 mV/A)
constexpr int   N_PROM   = 4;            // pares de conversiones promediados por lectura (~65 ms)
constexpr uint32_t PERIODO_MS = 250;
constexpr int   N_TARA   = 10;
constexpr int   MAX_PUNTOS = 20;

// Límites para avisos
constexpr float V_ADC_LINEAL = 2.45f;    // ESP32 con 11 dB: lineal entre 0,15 y 2,45 V (Espressif)
constexpr float V_ADS_MAX    = 3.30f;    // no pasar de VDD del ADS

// ---------------- ADS1115 (misma configuración que test-ads1115.ino) ----------------
constexpr uint8_t  REG_CONVERSION = 0x00;
constexpr uint8_t  REG_CONFIG     = 0x01;
constexpr uint16_t CFG_OS         = 0x8000;   // escribir: iniciar; leer: 1 = terminó
constexpr uint16_t CFG_PGA_4096   = 0x0200;   // ±4,096 V, 125 uV por cuenta
constexpr uint16_t CFG_UN_DISPARO = 0x0100;
constexpr uint16_t CFG_DR_128     = 0x0080;   // 128 muestras/s
constexpr uint16_t CFG_COMP_OFF   = 0x0003;
constexpr float    V_POR_CUENTA   = 4.096f / 32768.0f;
constexpr uint32_t T_INTEGRA_US   = 7500;     // ventana en la que el ESP32 muestrea (< 7,8 ms nominal)
constexpr uint32_t T_CONV_MAX_MS  = 20;

// ---------------- Estado ----------------
float ceroAds = 0.5f, ceroEsp = 0.5f;          // salida/VCC sin corriente
uint32_t erroresI2c = 0;

struct Lectura {
  float vsAds, vcAds;    // salida y VCC del ACS reconstruidos (antes del divisor), vía ADS
  float vsEsp, vcEsp;    // ídem, vía ADC del ESP32
  float xAds, xEsp;      // tensión en el nodo X (en el pin), para los avisos
};

struct Punto { float iMult, nAds, nEsp; };     // n = (Vout − cero·VCC)·5/VCC  [V], numerador normalizado
Punto puntos[MAX_PUNTOS];
int nPuntos = 0;

// ---------------- I2C ----------------
bool escribirReg16(uint8_t reg, uint16_t val) {
  Wire.beginTransmission(ADS_ADDR);
  Wire.write(reg);
  Wire.write(val >> 8);
  Wire.write(val & 0xFF);
  return Wire.endTransmission() == 0;
}

bool leerReg16(uint8_t reg, uint16_t &val) {
  Wire.beginTransmission(ADS_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom((int)ADS_ADDR, 2) != 2) return false;
  val = (uint16_t)Wire.read() << 8;
  val |= Wire.read();
  return true;
}

bool iniciarConversion(uint8_t canal) {
  uint16_t cfg = CFG_OS | ((uint16_t)(0x4 + canal) << 12) | CFG_PGA_4096
               | CFG_UN_DISPARO | CFG_DR_128 | CFG_COMP_OFF;
  return escribirReg16(REG_CONFIG, cfg);
}

bool terminarConversion(float &voltios) {
  uint32_t t0 = millis();
  uint16_t r = 0;
  for (;;) {
    if (!leerReg16(REG_CONFIG, r)) return false;
    if (r & CFG_OS) break;
    if (millis() - t0 > T_CONV_MAX_MS) return false;
    delayMicroseconds(200);
  }
  uint16_t c;
  if (!leerReg16(REG_CONVERSION, c)) return false;
  voltios = (int16_t)c * V_POR_CUENTA;
  return true;
}

// ---------------- Medición simultánea ----------------
// Convierte un canal del ADS y, durante su integración, acumula el ADC del ESP32 en los dos pines.
bool ventana(uint8_t canal, float &vAds, uint32_t &sumaX, uint32_t &sumaY, uint32_t &n) {
  if (!iniciarConversion(canal)) return false;
  uint32_t t0 = micros();
  while (micros() - t0 < T_INTEGRA_US) {
    sumaX += analogReadMilliVolts(PIN_SALIDA);
    sumaY += analogReadMilliVolts(PIN_VCC);
    n++;
  }
  return terminarConversion(vAds);
}

bool medir(Lectura &L) {
  float aX = 0, aY = 0, v;
  uint32_t sX = 0, sY = 0, n = 0;
  for (int i = 0; i < N_PROM; i++) {
    if (!ventana(CANAL_SALIDA, v, sX, sY, n)) { erroresI2c++; return false; }
    aX += v;
    if (!ventana(CANAL_VCC, v, sX, sY, n)) { erroresI2c++; return false; }
    aY += v;
  }
  L.xAds  = aX / N_PROM;
  L.vsAds = L.xAds * K_SALIDA;
  L.vcAds = aY / N_PROM * K_VCC;
  L.xEsp  = (float)sX / n / 1000.0f;
  L.vsEsp = L.xEsp * K_SALIDA;
  L.vcEsp = (float)sY / n / 1000.0f * K_VCC;
  return true;
}

// Numerador normalizado a VCC = 5 V: I = n / SENS_5V
float numerador(float vs, float vc, float cero) { return (vs - cero * vc) * 5.0f / vc; }
float corriente(float vs, float vc, float cero) { return numerador(vs, vc, cero) / SENS_5V; }

// ---------------- Comandos ----------------
void tarar() {
  Serial.println("# Tarando: SIN corriente por el sensor...");
  float sa = 0, se = 0;
  Lectura L;
  for (int i = 0; i < N_TARA; i++) {
    if (!medir(L)) { Serial.println("# Tara fallida: ADS1115 sin respuesta"); return; }
    sa += L.vsAds / L.vcAds;
    se += L.vsEsp / L.vcEsp;
  }
  ceroAds = sa / N_TARA;
  ceroEsp = se / N_TARA;
  Serial.printf("# Tara: ADS %.4f, ESP32 %.4f de VCC (ideal 0,500)\n", ceroAds, ceroEsp);
  if (fabsf(ceroAds - 0.5f) > 0.02f || fabsf(ceroEsp - 0.5f) > 0.02f)
    Serial.println("# OJO: tara lejos de 0,5. ¿Circula corriente? ¿Divisores bien armados?");
}

void anotarMultimetro(float iMult) {
  Lectura L;
  if (!medir(L)) { Serial.println("# Sin lectura del ADS"); return; }
  float nA = numerador(L.vsAds, L.vcAds, ceroAds);
  float nE = numerador(L.vsEsp, L.vcEsp, ceroEsp);
  float iA = nA / SENS_5V, iE = nE / SENS_5V;
  Serial.printf("# PUNTO mult=%.3f A | ADS=%.3f A (%+.1f %%) | ESP=%.3f A (%+.1f %%) | ADS-ESP=%.3f A\n",
                iMult, iA, 100.0f * (iA - iMult) / iMult, iE, 100.0f * (iE - iMult) / iMult, iA - iE);
  if (nPuntos < MAX_PUNTOS) puntos[nPuntos++] = {iMult, nA, nE};
  else Serial.println("# Tabla llena: 'b' para borrar");
}

// Recta por el origen (la tara ya fijó el cero): S = Σ(n·I) / Σ(I²)
void resumen() {
  if (nPuntos < 2) { Serial.println("# Hacen falta al menos 2 puntos ('m<valor>')"); return; }
  double sII = 0, sAI = 0, sEI = 0;
  for (int i = 0; i < nPuntos; i++) {
    sII += (double)puntos[i].iMult * puntos[i].iMult;
    sAI += (double)puntos[i].nAds  * puntos[i].iMult;
    sEI += (double)puntos[i].nEsp  * puntos[i].iMult;
  }
  float sA = sAI / sII, sE = sEI / sII;
  Serial.println("# ---- Resumen ----");
  Serial.println("# I_mult[A]  I_ADS[A]  I_ESP[A]   (con la sensibilidad ajustada de cada vía)");
  for (int i = 0; i < nPuntos; i++)
    Serial.printf("# %8.3f  %8.3f  %8.3f\n", puntos[i].iMult, puntos[i].nAds / sA, puntos[i].nEsp / sE);
  Serial.printf("# Sensibilidad ajustada (a VCC = 5 V): ADS %.1f mV/A, ESP32 %.1f mV/A (nominal 66, hoja 63 a 69)\n",
                sA * 1000, sE * 1000);
  if (sA < 0.063f || sA > 0.069f)
    Serial.println("# OJO: la del ADS sale de la hoja. Revisar divisores (tolerancia de resistencias) y el multímetro.");
}

void atenderSerie() {
  static char buf[24];
  static int len = 0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') { if (len < (int)sizeof(buf) - 1) buf[len++] = c; continue; }
    if (len == 0) continue;
    buf[len] = 0;
    len = 0;
    switch (buf[0]) {
      case 't': tarar(); break;
      case 'm': {
        float i = atof(buf + 1);
        if (fabsf(i) < 0.05f) Serial.println("# Valor de multímetro inválido (ej. m2.35)");
        else anotarMultimetro(i);
        break;
      }
      case 'r': resumen(); break;
      case 'b': nPuntos = 0; Serial.println("# Puntos borrados"); break;
      default:  Serial.println("# Comandos: t, m<valor>, r, b");
    }
  }
}

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n# == C2b: ACS712 con ADS1115 y ADC del ESP32 (sin probar) ==");

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_SALIDA, ADC_11db);   // zona lineal hasta ~2,45 V
  analogSetPinAttenuation(PIN_VCC, ADC_11db);

  Wire.begin(PIN_SDA, PIN_SCL, I2C_HZ);
  Wire.setTimeOut(5);
  Wire.beginTransmission(ADS_ADDR);
  while (Wire.endTransmission() != 0) {
    Serial.println("# Sin ADS en 0x48: revisar 3,3 V, GND, SDA (21), SCL (22), ADDR a GND. Reintento en 1 s.");
    delay(1000);
    Wire.beginTransmission(ADS_ADDR);
  }
  tarar();
  Serial.println("# Comandos: t = tara, m<valor> = anotar multímetro, r = resumen, b = borrar");
}

void loop() {
  atenderSerie();

  static uint32_t tProx = 0;
  if ((int32_t)(millis() - tProx) < 0) return;
  tProx = millis() + PERIODO_MS;

  Lectura L;
  if (!medir(L)) {
    Serial.println("# ADS1115 sin respuesta en 0x48");
    tProx = millis() + 1000;
    return;
  }
  float iAds = corriente(L.vsAds, L.vcAds, ceroAds);
  float iEsp = corriente(L.vsEsp, L.vcEsp, ceroEsp);

  Serial.printf("I_ADS:%.3f,I_ESP:%.3f,DIF:%.3f,VCC_ADS:%.3f,VCC_ESP:%.3f\n",
                iAds, iEsp, iAds - iEsp, L.vcAds, L.vcEsp);

  if (L.vcAds < 4.5f || L.vcAds > 5.5f) Serial.println("# OJO: VCC del ACS fuera de 4,5..5,5 V");
  if (L.xEsp > V_ADC_LINEAL)            Serial.println("# OJO: GPIO34 > 2,45 V, ADC del ESP32 alineal (vale el ADS)");
  if (L.xAds > V_ADS_MAX)               Serial.println("# OJO: AIN1 > 3,3 V, al límite del ADS");
}
