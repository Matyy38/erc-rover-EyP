/*
 * mpu6050_prueba.ino — Lectura del MPU6050 en la placa del modelo a escala
 *
 * estado: borrador, sin probar en placa
 * fecha:  2026-10-08
 *
 * Pines y direcciones: ver 20-modelo-escala/40-firmware/mapa-gpio.md (fuente única de verdad).
 *   Bus I2C por J3: SDA = IO21, SCL = IO22. MPU6050 en 0x68, ADS1115 en 0x48 (mismo bus).
 *   Pull-ups: solo los de los módulos (GY-521 + ADS1115 en paralelo).
 *
 * Qué hace:
 *   1. Pone en BAJO las entradas de los BTS7960 para que ningún motor arranque.
 *   2. Escanea el bus y verifica 0x68 (MPU) y 0x48 (ADS1115).
 *   3. Configura el MPU: 100 Hz, filtro interno 44 Hz, ±8 g, ±500 °/s.
 *   4. Calibra el offset del giróscopo en reposo (rechaza si detecta movimiento).
 *   5. Muestrea a 100 Hz con período fijo e imprime CSV por serie.
 *      Incluye inclinación (cabeceo / rolido) por filtro complementario.
 *
 * Comandos por serie:  c = recalibrar giróscopo   z = poner ángulos a cero (re-inicializa con el acelerómetro)
 *
 * Sin bibliotecas externas: solo Wire (núcleo Arduino-ESP32 2.x o 3.x).
 * Monitor serie a 921600 baudios.
 */

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// ---------------- Configuración ----------------
static const int      PIN_SDA     = 21;
static const int      PIN_SCL     = 22;
static const uint32_t I2C_HZ      = 400000;   // bajar a 100000 si aparecen errores (mazo en Y largo)
static const uint8_t  MPU_ADDR    = 0x68;     // 0x69 si AD0 del GY-521 queda en alto
static const uint8_t  ADS_ADDR    = 0x48;
static const uint32_t PERIODO_US  = 10000;    // 100 Hz
static const uint32_t BAUDIOS     = 921600;
static const int      N_CALIB     = 500;      // 5 s de reposo
static const float    ALFA        = 0.98f;    // filtro complementario (constante de tiempo ≈ 0,5 s a 100 Hz)

// Entradas de los BTS7960 (ver mapa-gpio.md). IO5 selecciona R_PWM_4 vía U1.
static const int PINES_MOTOR[] = {25, 26, 27, 14, 13, 4, 5, 23};

// Escalas según la configuración elegida
static const float LSB_POR_G   = 4096.0f;    // ±8 g
static const float LSB_POR_DPS = 65.5f;      // ±500 °/s
static const float G           = 9.80665f;

// Registros MPU6050
enum : uint8_t {
  R_SMPLRT_DIV   = 0x19,
  R_CONFIG       = 0x1A,
  R_GYRO_CONFIG  = 0x1B,
  R_ACCEL_CONFIG = 0x1C,
  R_ACCEL_XOUT_H = 0x3B,
  R_PWR_MGMT_1   = 0x6B,
  R_WHO_AM_I     = 0x75,
};

// ---------------- Estado ----------------
static float bias_gx = 0, bias_gy = 0, bias_gz = 0;
static float cabeceo = 0, rolido = 0;   // grados
static uint32_t errores_i2c = 0, sobrepasos = 0, n_muestra = 0;
static uint32_t errores_seguidos = 0;
static uint32_t t_proximo = 0;

struct Muestra { int16_t ax, ay, az, temp, gx, gy, gz; };

// ---------------- I2C ----------------
static bool escribirReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

static bool leerRegs(uint8_t reg, uint8_t *buf, size_t n) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;   // inicio repetido
  if (Wire.requestFrom((int)MPU_ADDR, (int)n) != (int)n) return false;
  for (size_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

static bool leerMuestra(Muestra &m) {
  uint8_t b[14];
  if (!leerRegs(R_ACCEL_XOUT_H, b, 14)) return false;
  m.ax   = (int16_t)(b[0]  << 8 | b[1]);
  m.ay   = (int16_t)(b[2]  << 8 | b[3]);
  m.az   = (int16_t)(b[4]  << 8 | b[5]);
  m.temp = (int16_t)(b[6]  << 8 | b[7]);
  m.gx   = (int16_t)(b[8]  << 8 | b[9]);
  m.gy   = (int16_t)(b[10] << 8 | b[11]);
  m.gz   = (int16_t)(b[12] << 8 | b[13]);
  return true;
}

static void iniciarBus() {
  Wire.begin(PIN_SDA, PIN_SCL, I2C_HZ);
  Wire.setTimeOut(5);   // ms; evita que un bus colgado frene el lazo
}

static bool presente(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

static void escanearBus() {
  Serial.println("# Escaneo I2C:");
  int n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    if (presente(a)) { Serial.printf("#   0x%02X\n", a); n++; }
  }
  if (n == 0) Serial.println("#   nada. Revisar J3, alimentacion 3,3 V y pull-ups.");
  Serial.printf("# MPU6050 (0x%02X): %s\n", MPU_ADDR, presente(MPU_ADDR) ? "OK" : "NO RESPONDE");
  Serial.printf("# ADS1115 (0x%02X): %s\n", ADS_ADDR, presente(ADS_ADDR) ? "OK" : "no responde");
}

// ---------------- MPU ----------------
static bool configurarMPU() {
  uint8_t who = 0;
  if (!leerRegs(R_WHO_AM_I, &who, 1)) return false;
  Serial.printf("# WHO_AM_I = 0x%02X%s\n", who,
                who == 0x68 ? " (MPU6050)" :
                " (no es 0x68: posible clon o MPU6500; los registros usados son compatibles, verificar escalas)");

  bool ok = true;
  ok &= escribirReg(R_PWR_MGMT_1, 0x80);   // reinicio del dispositivo
  delay(100);
  ok &= escribirReg(R_PWR_MGMT_1, 0x01);   // despierto, reloj = PLL del giróscopo X (más estable que el RC interno)
  delay(10);
  ok &= escribirReg(R_CONFIG,       0x03); // filtro interno: 44 Hz acel / 42 Hz giro, base 1 kHz
  ok &= escribirReg(R_SMPLRT_DIV,   9);    // 1 kHz / (1+9) = 100 Hz
  ok &= escribirReg(R_GYRO_CONFIG,  0x08); // ±500 °/s
  ok &= escribirReg(R_ACCEL_CONFIG, 0x10); // ±8 g
  return ok;
}

static void anguloDesdeAcel(const Muestra &m, float &p, float &r) {
  float ax = m.ax, ay = m.ay, az = m.az;
  p = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;
  r = atan2f(ay, az) * 180.0f / PI;
}

// Promedia el giróscopo en reposo. Rechaza si la dispersión indica movimiento o vibración.
static bool calibrarGiro() {
  Serial.printf("# Calibrando giroscopo: %d muestras, NO mover el modelo...\n", N_CALIB);
  double sx = 0, sy = 0, sz = 0, sxx = 0, syy = 0, szz = 0;
  int validas = 0;
  Muestra m;
  uint32_t t = micros();
  for (int i = 0; i < N_CALIB; i++) {
    while ((int32_t)(micros() - t) < (int32_t)PERIODO_US) {}
    t += PERIODO_US;
    if (!leerMuestra(m)) { errores_i2c++; continue; }
    sx += m.gx; sy += m.gy; sz += m.gz;
    sxx += (double)m.gx * m.gx; syy += (double)m.gy * m.gy; szz += (double)m.gz * m.gz;
    validas++;
  }
  if (validas < N_CALIB * 0.9) {
    Serial.printf("# Calibracion fallida: solo %d muestras validas\n", validas);
    return false;
  }
  double mx = sx / validas, my = sy / validas, mz = sz / validas;
  double dx = sqrt(sxx / validas - mx * mx) / LSB_POR_DPS;
  double dy = sqrt(syy / validas - my * my) / LSB_POR_DPS;
  double dz = sqrt(szz / validas - mz * mz) / LSB_POR_DPS;
  Serial.printf("# Offset [deg/s]: %.3f %.3f %.3f | desvio [deg/s]: %.3f %.3f %.3f\n",
                mx / LSB_POR_DPS, my / LSB_POR_DPS, mz / LSB_POR_DPS, dx, dy, dz);
  // Ruido típico del MPU6050 con filtro de 42 Hz: ~0,05 a 0,1 °/s eficaz. 0,5 °/s es criterio empírico de "se movió".
  if (dx > 0.5 || dy > 0.5 || dz > 0.5) {
    Serial.println("# Calibracion rechazada: hubo movimiento o vibracion. Enviar 'c' para reintentar.");
    return false;
  }
  bias_gx = mx; bias_gy = my; bias_gz = mz;
  return true;
}

static void reiniciarAngulos() {
  Muestra m;
  if (leerMuestra(m)) anguloDesdeAcel(m, cabeceo, rolido);
}

static void detenerMotores() {
  for (int p : PINES_MOTOR) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }
}

// ---------------- Arduino ----------------
void setup() {
  detenerMotores();   // primero, antes que cualquier otra cosa
  Serial.begin(BAUDIOS);
  delay(300);
  Serial.println("# mpu6050_prueba - modelo a escala ERC");

  iniciarBus();
  escanearBus();

  while (!configurarMPU()) {
    Serial.println("# Error configurando el MPU6050. Reintento en 1 s.");
    delay(1000);
  }
  delay(100);   // asentar el filtro interno
  calibrarGiro();
  reiniciarAngulos();

  Serial.println("t_ms,n,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp_C,cabeceo_deg,rolido_deg,err_i2c,sobrepasos");
  t_proximo = micros() + PERIODO_US;
}

void loop() {
  // Comandos
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') { calibrarGiro(); reiniciarAngulos(); t_proximo = micros() + PERIODO_US; }
    if (c == 'z') reiniciarAngulos();
  }

  // Período fijo. Si se llegó tarde, se cuenta y se resincroniza (no se acumula atraso).
  int32_t falta = (int32_t)(t_proximo - micros());
  if (falta > 0) return;
  if (falta < -(int32_t)PERIODO_US) { sobrepasos++; t_proximo = micros(); }
  t_proximo += PERIODO_US;

  Muestra m;
  if (!leerMuestra(m)) {
    errores_i2c++;
    if (++errores_seguidos >= 5) {   // bus posiblemente colgado: reiniciar periférico
      Wire.end();
      iniciarBus();
      configurarMPU();
      errores_seguidos = 0;
    }
    return;
  }
  errores_seguidos = 0;

  const float dt = PERIODO_US * 1e-6f;
  float ax = m.ax / LSB_POR_G, ay = m.ay / LSB_POR_G, az = m.az / LSB_POR_G;
  float gx = (m.gx - bias_gx) / LSB_POR_DPS;
  float gy = (m.gy - bias_gy) / LSB_POR_DPS;
  float gz = (m.gz - bias_gz) / LSB_POR_DPS;
  float temp = m.temp / 340.0f + 36.53f;   // fórmula de la hoja de registros, rev 4.2

  // Filtro complementario. Válido para inclinación cuasi estática (rampa);
  // en aceleraciones fuertes del chasis el término del acelerómetro se contamina.
  float p_acc, r_acc;
  anguloDesdeAcel(m, p_acc, r_acc);
  cabeceo = ALFA * (cabeceo + gy * dt) + (1 - ALFA) * p_acc;
  rolido  = ALFA * (rolido  + gx * dt) + (1 - ALFA) * r_acc;

  Serial.printf("%lu,%lu,%.4f,%.4f,%.4f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%lu,%lu\n",
                millis(), n_muestra++, ax, ay, az, gx, gy, gz, temp,
                cabeceo, rolido, errores_i2c, sobrepasos);
}
