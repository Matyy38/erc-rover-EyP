/*
  test-mpu6050.ino — prueba del MPU6050 (módulo GY-521) del modelo a escala
  estado: borrador, sin probar en placa
  fecha: 2026-10-08

  Qué hace:
    1. Escanea el bus I2C (en J3 tienen que aparecer 0x68 MPU6050 y 0x48 ADS1115).
    2. Lee WHO_AM_I, configura el sensor y relee los registros para confirmar la escritura.
    3. Prueba estática de 3 s (rover quieto, motores apagados): módulo de g, sesgo y ruido
       de cada eje, con criterio pasa / no pasa. El sesgo del giróscopo queda guardado.
    4. Lazo a 100 Hz: roll y pitch por filtro complementario, salida para el Serial Plotter.
       Si el sensor deja de responder, cuenta errores y reintenta inicializar.

  Comandos por el monitor serie (115200, sin fin de línea o con cualquiera):
    c  recalibrar (prueba estática)     z  tomar la posición actual como cero de ángulo
    r  alternar modo crudo / ángulos    s  escanear el bus             h  ayuda

  Pines: fuente única en 20-modelo-escala/40-firmware/mapa-gpio.md (IO21 SDA, IO22 SCL).
  Sin librerías externas: solo Wire, acceso directo a registros.
*/

#include <Wire.h>
#include <math.h>

// ---- Bus I2C (ver mapa-gpio.md) ----
constexpr int PIN_SDA = 21;
constexpr int PIN_SCL = 22;
constexpr uint32_t I2C_HZ = 400000;  // si hay errores de bus, bajar a 100000

uint8_t mpuAddr = 0x68;  // 0x69 si AD0 está en alto; se detecta en el escaneo

// ---- Registros (MPU-6000/6050 Register Map, rev. 4.2) ----
constexpr uint8_t REG_SMPLRT_DIV   = 0x19;
constexpr uint8_t REG_CONFIG       = 0x1A;
constexpr uint8_t REG_GYRO_CONFIG  = 0x1B;
constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t REG_PWR_MGMT_1   = 0x6B;
constexpr uint8_t REG_WHO_AM_I     = 0x75;

// ---- Configuración ----
constexpr uint8_t CFG_DLPF  = 0x03;  // filtro digital ~44 Hz: corta vibración de motores
constexpr uint8_t CFG_GYRO  = 0x08;  // ±500 °/s
constexpr uint8_t CFG_ACCEL = 0x08;  // ±4 g: con ±2 g los golpes saturan el acelerómetro
constexpr float GYR_LSB_DPS = 65.5f;
constexpr float ACC_LSB_G   = 8192.0f;

constexpr uint32_t PERIODO_US   = 10000;  // 100 Hz
constexpr uint8_t  DIEZMADO_OUT = 5;      // imprime a 20 Hz
constexpr float    ALFA         = 0.98f;  // complementario: tau ≈ 0,49 s a 100 Hz

// ---- Estado ----
struct Muestra { float ax, ay, az, gx, gy, gz, temp; };

float sesgoG[3] = {0, 0, 0};
float roll = 0, pitch = 0;
float rollCero = 0, pitchCero = 0;
bool modoCrudo = false;
bool mpuOk = false;
uint32_t erroresI2C = 0;
uint32_t erroresSeguidos = 0;
uint32_t tAnterior = 0;
uint8_t contadorOut = 0;

// ================= Acceso a registros =================

bool escribirReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(mpuAddr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

bool leerRegs(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(mpuAddr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(mpuAddr, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

bool leerReg(uint8_t reg, uint8_t &val) { return leerRegs(reg, &val, 1); }

// ================= Sensor =================

bool leerMuestra(Muestra &m) {
  uint8_t b[14];
  if (!leerRegs(REG_ACCEL_XOUT_H, b, 14)) {
    erroresI2C++;
    erroresSeguidos++;
    return false;
  }
  erroresSeguidos = 0;
  auto s16 = [&](int i) { return (int16_t)((b[i] << 8) | b[i + 1]); };
  m.ax = s16(0) / ACC_LSB_G;
  m.ay = s16(2) / ACC_LSB_G;
  m.az = s16(4) / ACC_LSB_G;
  m.temp = s16(6) / 340.0f + 36.53f;
  m.gx = s16(8) / GYR_LSB_DPS - sesgoG[0];
  m.gy = s16(10) / GYR_LSB_DPS - sesgoG[1];
  m.gz = s16(12) / GYR_LSB_DPS - sesgoG[2];
  return true;
}

bool iniciarMPU() {
  uint8_t who = 0;
  if (!leerReg(REG_WHO_AM_I, who)) {
    Serial.printf("# MPU en 0x%02X no responde\n", mpuAddr);
    return false;
  }
  if (who == 0x68) {
    Serial.println("# WHO_AM_I = 0x68: MPU6050 original");
  } else {
    Serial.printf("# WHO_AM_I = 0x%02X: no es un MPU6050 original (clon o MPU65xx).\n", who);
    Serial.println("#   Los registros base suelen coincidir, pero ruido y deriva no son los de la hoja de datos.");
  }

  escribirReg(REG_PWR_MGMT_1, 0x80);  // reset
  delay(100);
  bool ok = escribirReg(REG_PWR_MGMT_1, 0x01)        // despierta, reloj = PLL del giróscopo X
         && escribirReg(REG_SMPLRT_DIV, 9)            // 1 kHz / (1+9) = 100 Hz
         && escribirReg(REG_CONFIG, CFG_DLPF)
         && escribirReg(REG_GYRO_CONFIG, CFG_GYRO)
         && escribirReg(REG_ACCEL_CONFIG, CFG_ACCEL);
  delay(50);

  // Relectura: confirma que las escrituras llegaron
  uint8_t g = 0, a = 0, p = 0;
  ok = ok && leerReg(REG_GYRO_CONFIG, g) && leerReg(REG_ACCEL_CONFIG, a) && leerReg(REG_PWR_MGMT_1, p);
  if (!ok || (g & 0x18) != CFG_GYRO || (a & 0x18) != CFG_ACCEL || (p & 0x40)) {
    Serial.printf("# Configuración NO verificada (GYRO=0x%02X ACCEL=0x%02X PWR=0x%02X)\n", g, a, p);
    return false;
  }
  Serial.println("# Configuración verificada: ±500 °/s, ±4 g, filtro 44 Hz, 100 Hz");
  return true;
}

void escanearBus() {
  Serial.println("# Escaneo I2C:");
  bool hay68 = false, hay69 = false;
  int n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() != 0) continue;
    n++;
    const char *nombre = (a == 0x68) ? "MPU6050"
                       : (a == 0x69) ? "MPU6050 con AD0 en alto"
                       : (a == 0x48) ? "ADS1115"
                       : "desconocido";
    Serial.printf("#   0x%02X  %s\n", a, nombre);
    hay68 |= (a == 0x68);
    hay69 |= (a == 0x69);
  }
  if (n == 0) {
    Serial.println("#   Ningún dispositivo. Revisar J3 (orden 3V3-GND-SCL-SDA), 3,3 V en el módulo");
    Serial.println("#   y pull-ups: la placa no tiene, los ponen el GY-521 y el ADS1115.");
  }
  if (!hay68 && hay69) mpuAddr = 0x69;
}

// Prueba estática: media y desvío de cada eje con el rover quieto
void pruebaEstatica() {
  constexpr int N = 300;  // 3 s a 100 Hz
  Serial.println("# Prueba estática: no mover el rover, motores apagados (3 s)...");

  for (int i = 0; i < 3; i++) sesgoG[i] = 0;
  double suma[7] = {0}, suma2[7] = {0};
  int validas = 0;
  Muestra m;
  for (int i = 0; i < N; i++) {
    uint32_t t0 = micros();
    if (leerMuestra(m)) {
      float v[7] = {m.ax, m.ay, m.az, m.gx, m.gy, m.gz, m.temp};
      for (int k = 0; k < 7; k++) { suma[k] += v[k]; suma2[k] += (double)v[k] * v[k]; }
      validas++;
    }
    while (micros() - t0 < PERIODO_US) {}
  }
  if (validas < N * 9 / 10) {
    Serial.printf("# FALLA: solo %d de %d lecturas válidas. Problema de bus, no del sensor.\n", validas, N);
    return;
  }

  float media[7], desvio[7];
  for (int k = 0; k < 7; k++) {
    media[k] = suma[k] / validas;
    desvio[k] = sqrt(fmax(0.0, suma2[k] / validas - (double)media[k] * media[k]));
  }
  float modA = sqrtf(media[0] * media[0] + media[1] * media[1] + media[2] * media[2]);

  // Criterios: |a| y sesgo según hoja de datos (offset cero-g ±50/±80 mg, ZRO ±20 °/s);
  // los límites de ruido son empíricos para módulo quieto y motores apagados.
  bool okA     = modA > 0.90f && modA < 1.10f;
  bool okSesgo = fabsf(media[3]) < 20 && fabsf(media[4]) < 20 && fabsf(media[5]) < 20;
  bool okRuidoA = desvio[0] < 0.02f && desvio[1] < 0.02f && desvio[2] < 0.02f;
  bool okRuidoG = desvio[3] < 0.5f && desvio[4] < 0.5f && desvio[5] < 0.5f;
  bool okTemp  = media[6] > 0 && media[6] < 60;

  Serial.println("#            media        desvío");
  const char *ej[6] = {"ax [g]   ", "ay [g]   ", "az [g]   ", "gx [°/s] ", "gy [°/s] ", "gz [°/s] "};
  for (int k = 0; k < 6; k++) Serial.printf("#   %s %+9.4f   %8.4f\n", ej[k], media[k], desvio[k]);
  Serial.printf("#   temp [°C]  %6.1f\n", media[6]);
  Serial.printf("#   |a| = %.4f g  %s\n", modA, okA ? "OK" : "FUERA (0,90-1,10)");
  Serial.printf("#   sesgo giróscopo   %s\n", okSesgo ? "OK" : "FUERA (±20 °/s)");
  Serial.printf("#   ruido acelerómetro %s\n", okRuidoA ? "OK" : "ALTO (>0,02 g: ¿vibra algo?)");
  Serial.printf("#   ruido giróscopo    %s\n", okRuidoG ? "OK" : "ALTO (>0,5 °/s: ¿vibra algo?)");
  Serial.printf("#   temperatura        %s\n", okTemp ? "OK" : "IMPLAUSIBLE");
  Serial.printf("# RESULTADO: %s\n", (okA && okSesgo && okRuidoA && okRuidoG && okTemp) ? "PASA" : "NO PASA");

  for (int i = 0; i < 3; i++) sesgoG[i] = media[3 + i];

  // Arranca el filtro desde el ángulo del acelerómetro
  roll  = atan2f(media[1], media[2]) * RAD_TO_DEG;
  pitch = atan2f(-media[0], sqrtf(media[1] * media[1] + media[2] * media[2])) * RAD_TO_DEG;
  tAnterior = micros();
}

void ayuda() {
  Serial.println("# Comandos: c=recalibrar  z=cero de ángulo  r=crudo/ángulos  s=escanear  h=ayuda");
}

// ================= Arduino =================

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n# ===== Prueba MPU6050 - modelo a escala =====");

  Wire.begin(PIN_SDA, PIN_SCL, I2C_HZ);
  Wire.setTimeOut(10);  // ms; evita que un bus colgado trabe el lazo

  escanearBus();
  mpuOk = iniciarMPU();
  if (mpuOk) pruebaEstatica();
  ayuda();
  tAnterior = micros();
}

void loop() {
  // Comandos
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'c': if (mpuOk) pruebaEstatica(); break;
      case 'z': rollCero = roll; pitchCero = pitch; Serial.println("# Cero de ángulo tomado"); break;
      case 'r': modoCrudo = !modoCrudo; break;
      case 's': escanearBus(); break;
      case 'h': ayuda(); break;
    }
  }

  // Reintento si el sensor no está o se perdió
  if (!mpuOk || erroresSeguidos >= 10) {
    static uint32_t tReintento = 0;
    if (millis() - tReintento > 1000) {
      tReintento = millis();
      Serial.printf("# Sensor perdido (errores I2C totales: %lu). Reintentando...\n", (unsigned long)erroresI2C);
      mpuOk = iniciarMPU();
      if (mpuOk) { erroresSeguidos = 0; tAnterior = micros(); }
    }
    return;
  }

  // Lazo a 100 Hz
  uint32_t ahora = micros();
  if (ahora - tAnterior < PERIODO_US) return;
  float dt = (ahora - tAnterior) * 1e-6f;
  tAnterior = ahora;

  Muestra m;
  if (!leerMuestra(m)) return;

  float rollAcc  = atan2f(m.ay, m.az) * RAD_TO_DEG;
  float pitchAcc = atan2f(-m.ax, sqrtf(m.ay * m.ay + m.az * m.az)) * RAD_TO_DEG;
  roll  = ALFA * (roll + m.gx * dt) + (1 - ALFA) * rollAcc;
  pitch = ALFA * (pitch + m.gy * dt) + (1 - ALFA) * pitchAcc;

  if (++contadorOut < DIEZMADO_OUT) return;
  contadorOut = 0;

  // Formato etiqueta:valor para el Serial Plotter del IDE 2
  if (modoCrudo) {
    Serial.printf("ax:%.3f,ay:%.3f,az:%.3f,gx:%.2f,gy:%.2f,gz:%.2f,T:%.1f,err:%lu\n",
                  m.ax, m.ay, m.az, m.gx, m.gy, m.gz, m.temp, (unsigned long)erroresI2C);
  } else {
    float modA = sqrtf(m.ax * m.ax + m.ay * m.ay + m.az * m.az);
    Serial.printf("roll:%.2f,pitch:%.2f,rollAcc:%.2f,pitchAcc:%.2f,modA:%.3f\n",
                  roll - rollCero, pitch - pitchCero, rollAcc - rollCero, pitchAcc - pitchCero, modA);
  }
}
