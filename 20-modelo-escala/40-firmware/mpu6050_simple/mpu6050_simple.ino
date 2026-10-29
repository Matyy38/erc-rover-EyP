/*
 * mpu6050_simple.ino — lectura mínima del MPU6050 (GY-521) en la placa del modelo a escala
 *
 * estado: cargado en placa (29-oct): bus OK, ADS1115 responde, MPU no aparece
 * fecha:  2026-10-29
 *
 * Pines: fuente única en 20-modelo-escala/40-firmware/mapa-gpio.md (SDA = IO21, SCL = IO22).
 * Sin bibliotecas externas, solo Wire. Monitor serie a 115200.
 *
 * Qué hace:
 *   1. Pone en bajo las entradas de los BTS7960 para que ningún motor arranque.
 *   2. Mira el nivel de reposo de SDA y SCL (sin pull-up tienen que leer 1).
 *   3. Escanea el bus y lee WHO_AM_I.
 *   4. Despierta el MPU y cada 200 ms imprime aceleración (g), giro (°/s) y temperatura (°C).
 */

#include <Wire.h>

const int PIN_SDA = 21;
const int PIN_SCL = 22;
const uint8_t MPU = 0x68;

// Entradas de los BTS7960 (mapa-gpio.md). IO5 maneja R_PWM_4 a través de U1.
const int PINES_MOTOR[] = {25, 26, 27, 14, 13, 4, 5, 23};

bool escribir(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}


bool leer(uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

bool iniciarMPU() {
  uint8_t who = 0;
  if (!leer(0x75, &who, 1)) {
    Serial.println("MPU no responde en 0x68");
    return false;
  }
  Serial.printf("WHO_AM_I = 0x%02X %s\n", who, who == 0x68 ? "(MPU6050 original)" : "(clon u otro chip, igual sirve)");
  escribir(0x6B, 0x01);  // PWR_MGMT_1: sale de sleep, reloj del PLL del giróscopo X
  delay(100);
  escribir(0x1A, 0x03);  // CONFIG: filtro interno 44 Hz
  escribir(0x1B, 0x00);  // GYRO_CONFIG: ±250 °/s -> 131 cuentas por °/s
  escribir(0x1C, 0x00);  // ACCEL_CONFIG: ±2 g -> 16384 cuentas por g
  return true;
}

void setup() {
  for (int p : PINES_MOTOR) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }

  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== MPU6050 simple ===");

  pinMode(PIN_SDA, INPUT);
  pinMode(PIN_SCL, INPUT);
  delay(5);
  Serial.printf("Reposo del bus: SDA=%d SCL=%d (tienen que ser 1 y 1)\n",
                digitalRead(PIN_SDA), digitalRead(PIN_SCL));

  Wire.begin(PIN_SDA, PIN_SCL, 100000);

  Serial.print("Escaneo I2C:");
  int n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.printf(" 0x%02X", a); n++; }
  }
  Serial.println(n ? "" : " nada");

  while (!iniciarMPU()) delay(1000);
}

void loop() {
  uint8_t b[14];
  if (!leer(0x3B, b, 14)) {
    Serial.println("error de lectura, reintento");
    delay(500);
    iniciarMPU();
    return;
  }
  int16_t ax = b[0] << 8 | b[1], ay = b[2] << 8 | b[3], az = b[4] << 8 | b[5];
  int16_t t  = b[6] << 8 | b[7];
  int16_t gx = b[8] << 8 | b[9], gy = b[10] << 8 | b[11], gz = b[12] << 8 | b[13];

  Serial.printf("a[g] %6.3f %6.3f %6.3f | w[dps] %7.2f %7.2f %7.2f | T %.1f\n",
                ax / 16384.0, ay / 16384.0, az / 16384.0,
                gx / 131.0, gy / 131.0, gz / 131.0,
                t / 340.0 + 36.53);
  delay(200);
}
