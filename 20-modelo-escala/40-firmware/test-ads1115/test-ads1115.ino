/*
  test-ads1115.ino — prueba simple del ADS1115 del modelo a escala
  estado: borrador, sin probar en placa
  fecha: 2026-10-21

  Qué hace:
    1. Escanea el bus I2C: tienen que aparecer 0x48 (ADS1115) y 0x68 (MPU6050).
    2. Cada 0,5 s lee los cuatro canales (AIN0..AIN3 contra GND) y los imprime en voltios.
       El formato etiqueta:valor sirve para leerlo en el monitor serie o verlo en el Serial Plotter.

  Qué esperar con todo en reposo (sin corriente). Canales según el diseño, la asignación real se
  confirma con esta misma prueba:
    AIN0  TMCS1126B4      ~1,65 V   100 mV/A; con corriente de tracción la tensión BAJA
    AIN1  ACS712-30A      ~2,5 V    mitad de su alimentación; 66 mV/A, con signo
    AIN2  5 V del ACS / 2 ~2,5 V    si da otra cosa, revisar el regulador del ACS712
    AIN3  libre           cualquier valor: una entrada al aire flota, es normal

  Rango: ±4,096 V en los cuatro canales, 125 uV por cuenta (1,25 mA por cuenta en el TMCS).
  Ninguna entrada puede pasar de 3,3 V: el ADS1115 está alimentado a 3,3 V.

  Pines: fuente única en 20-modelo-escala/40-firmware/mapa-gpio.md (IO21 SDA, IO22 SCL).
  Sin librerías externas: solo Wire, acceso directo a registros.
*/

#include <Wire.h>

constexpr int PIN_SDA = 21;
constexpr int PIN_SCL = 22;
constexpr uint8_t ADS_ADDR = 0x48;

// Registros del ADS1115
constexpr uint8_t REG_CONVERSION = 0x00;
constexpr uint8_t REG_CONFIG     = 0x01;

constexpr float VOLTIOS_POR_CUENTA = 4.096f / 32768.0f;  // PGA ±4,096 V

// Arma la palabra de configuración para una conversión única en el canal pedido:
//   bit 15     OS = 1        arranca una conversión
//   bits 14:12 MUX = 1xx     canal xx contra GND
//   bits 11:9  PGA = 001     ±4,096 V
//   bit 8      MODE = 1      conversión única
//   bits 7:5   DR = 100      128 muestras/s (7,8 ms por conversión)
//   bits 1:0   COMP = 11     comparador apagado
uint16_t configCanal(uint8_t canal) {
  return 0x8000 | ((0x4 + canal) << 12) | 0x0200 | 0x0100 | 0x0080 | 0x0003;
}

// Lee un canal y devuelve la tensión en voltios. Devuelve false si el ADS no responde.
bool leerCanal(uint8_t canal, float &voltios) {
  uint16_t cfg = configCanal(canal);

  // 1. Escribir la configuración: esto dispara la conversión
  Wire.beginTransmission(ADS_ADDR);
  Wire.write(REG_CONFIG);
  Wire.write(cfg >> 8);
  Wire.write(cfg & 0xFF);
  if (Wire.endTransmission() != 0) return false;

  // 2. Esperar a que termine (7,8 ms a 128 muestras/s, con margen)
  delay(9);

  // 3. Leer el resultado: 16 bits con signo, primero el byte alto
  Wire.beginTransmission(ADS_ADDR);
  Wire.write(REG_CONVERSION);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(ADS_ADDR, (uint8_t)2) != 2) return false;
  uint8_t alto = Wire.read();
  uint8_t bajo = Wire.read();
  int16_t cuentas = (int16_t)((alto << 8) | bajo);

  voltios = cuentas * VOLTIOS_POR_CUENTA;
  return true;
}

void escanearBus() {
  Serial.println("# Escaneo I2C:");
  int encontrados = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      const char *nombre = (a == 0x48) ? "ADS1115" : (a == 0x68) ? "MPU6050" : "desconocido";
      Serial.printf("#   0x%02X  %s\n", a, nombre);
      encontrados++;
    }
  }
  if (encontrados == 0) Serial.println("#   Ningún dispositivo: revisar 3,3 V, GND, SDA y SCL.");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n# ===== Prueba ADS1115 - modelo a escala =====");

  Wire.begin(PIN_SDA, PIN_SCL, 100000);  // 100 kHz: lento pero robusto, sobra para esta prueba
  escanearBus();
}

void loop() {
  float v[4];
  for (uint8_t canal = 0; canal < 4; canal++) {
    if (!leerCanal(canal, v[canal])) {
      Serial.println("# ADS1115 sin respuesta en 0x48");
      delay(1000);
      return;
    }
  }

  Serial.printf("A0:%.4f,A1:%.4f,A2:%.4f,A3:%.4f\n", v[0], v[1], v[2], v[3]);
  delay(500);
}
