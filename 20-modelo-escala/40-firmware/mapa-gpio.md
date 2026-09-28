---
titulo: Mapa de GPIO del ESP32 y arranque seguro
estado: vigente
fecha: 2026-09-27
fuente: pcb-modelo-escala.md, sección arquitectura-electrica (extraído)
---

# Mapa de GPIO del ESP32

**Este archivo es la fuente única de verdad del pinout.** Si una tabla de pines aparece en otro
archivo del repo, ese archivo enlaza acá y no la copia: duplicar el pinout es la forma más probable
de romper algo.

Extraído del esquemático de KiCad (`PCB_modelo.kicad_sch`). El mapeo cambió respecto de la tabla del
PDF de planificación del 17-ago-2026.

**Advertencia de numeración:** el símbolo de KiCad numera los pines 1 a 38 arrancando en 3V3, y la
imagen de pinout de 38 pines que circula (myhomethings) numera arrancando en GND. En la columna
izquierda hay un desfasaje de 1 entre ambas. **Referirse siempre al nombre del GPIO**, nunca al
número de pin.

| GPIO | Pin del símbolo | Red | Función | Acondicionamiento en la PCB |
|---|---|---|---|---|
| IO36 | 3 | Encoder_FR_A | Entrada, solo entrada | pull-up 4,7 k (R1) a 3,3 V |
| IO39 | 4 | Encoder_FR_B | Entrada, solo entrada | pull-up 4,7 k (R7) |
| IO34 | 5 | Encoder_FL_A | Entrada, solo entrada | pull-up 4,7 k (R10) |
| IO35 | 6 | Encoder_FL_B | Entrada, solo entrada | pull-up 4,7 k (R11) |
| IO32 | 7 | V_bat | ADC1, medición de batería | divisor 10 k / 2,2 k + C6 100 n + clamp BAT54S |
| IO33 | 8 | Vout_CS | ADC1, corriente total | R15 1 k serie + C8 100 n |
| IO25 | 9 | R_PWM_1 | PWM driver 1 | nada |
| IO26 | 10 | L_PWM_1 | PWM driver 1 | nada |
| IO27 | 11 | R_PWM_2 | PWM driver 2 | nada |
| IO14 | 12 | L_PWM_2 | PWM driver 2 | nada |
| **IO12** | 13 | Servo4 | PWM servo 4 | **pull-down 10 k (R22)** - strapping, debe estar en BAJO |
| IO13 | 15 | R_PWM_3 | PWM driver 3 | nada |
| **IO15** | 23 | -> U5.S | selector del conmutador de Servo3 | strapping, arranca en ALTO por pull-up interno |
| **IO2** | 24 | Servo2 | PWM servo 2 | **pull-down 10 k (R21)** - strapping, debe estar en BAJO |
| **IO0** | 25 | -> U4.S | selector del conmutador de Servo1 | strapping, arranca en ALTO |
| IO4 | 26 | L_PWM_3 | PWM driver 3 | nada |
| IO16 | 27 | Encoder_RL_A | Entrada | pull-up 4,7 k (R12) |
| IO17 | 28 | Encoder_RL_B | Entrada | pull-up 4,7 k (R13) |
| **IO5** | 29 | -> U1.S | selector del conmutador de R_PWM_4 | strapping, arranca en ALTO |
| IO18 | 30 | Encoder_RR_A | Entrada | pull-up 4,7 k (R8) |
| IO19 | 31 | Encoder_RR_B | Entrada | pull-up 4,7 k (R9) |
| IO21 | 33 | SDA | I2C hardware: MPU6050 (0x68) y ADS1115 (0x48) | sin pull-up en la placa (los ponen los módulos GY-521 y ADS1115, quedan en paralelo) |
| IO22 | 36 | SCL | I2C hardware: ídem | sin pull-up en la placa |
| IO23 | 37 | L_PWM_4 | PWM driver 4 | **pull-up agregado** (27-sep, valor a asentar), para V1 |
| 3V3 | 1 | +3,3 V | alimentación | C15 100u + C11 1u |
| EXT_5V | 19 | +5V | alimentación (VIN) | C9 100u + C10 1u |
| GND | 14, 32, 38 | GND | — | — |
| EN | 2 | sin conexión | marcado no-connect | — |
| SD0..SD3, CMD, CLK | 16-18, 20-22 | sin conexión | flash interna, marcados no-connect | correcto |
| TXD0 / RXD0 | 35 / 34 | sin conexión | USB-UART del devkit | quedan libres para consola |

**No queda ningún GPIO libre.** IO1/IO3 están tomados por la consola serie. Cualquier función nueva
(por ejemplo leer el pin OC del sensor de corriente) requiere sacar algo o agregar un expansor I2C.
Por eso el ADS1115 (27-sep) se lee por sondeo, sin su línea ALERT/RDY, y cuelga del mismo bus I2C
que el MPU6050, a través de J3 con un mazo en Y.

## Estrategia de arranque seguro

El ESP32 tiene 5 pines de strapping: IO0, IO2, IO5, IO12 e IO15. Se leen en los primeros
milisegundos y definen el modo de arranque y la tensión de la flash. El diseño los resuelve de dos
maneras según qué nivel exigen:

- **Los que deben arrancar en BAJO (IO2 e IO12):** se usan directo, con pull-down de 10 k a masa
  (R21 y R22). Ese pull-down además garantiza que el servo correspondiente no reciba nada durante
  el boot.
- **Los que arrancan en ALTO (IO0, IO5 e IO15):** no se pueden cargar con un pull-down sin romper el
  arranque. Acá está la parte ingeniosa del diseño: cada uno maneja la **entrada de selección de un
  conmutador analógico NLAS4157** (U1, U4, U5), y es el conmutador el que maneja la carga real. La
  entrada de selección es CMOS pura, no carga el pull-up interno, así que el strapping queda intacto
  y la línea larga hacia el driver o el servo queda eléctricamente aislada del pin de arranque.

Tabla de verdad del NLAS4157: con S en bajo, A queda unido a B0; con S en alto, A queda unido a B1.
En la placa, B1 va a +3,3 V y B0 a GND en los tres, o sea que la salida A copia el estado de S.

| Conmutador | Selector | Carga | Estado esperado durante el boot |
|---|---|---|---|
| U4 | IO0 | Servo1 | señal en 3,3 V continuos. Sin pulso PWM válido el servo no se mueve |
| U5 | IO15 | Servo3 | ídem |
| U1 | IO5 | R_PWM_4 | entrada del BTS7960 en alto. Con el pull-up agregado en L_PWM_4 (IO23), las dos entradas quedan en alto y el motor 4 queda frenado. **A verificar con osciloscopio (V1)** |

---

## Relacionado

- Rieles, cadena de potencia y presupuesto del 3,3 V: [20-electronica/arquitectura-electrica.md](../20-electronica/arquitectura-electrica.md)
- Conectores por los que sale cada línea: [30-pcb/mapa-conectores.md](../30-pcb/mapa-conectores.md)
- Estado del canal 4 durante el arranque (V1): [20-electronica/drivers-bts7960.md](../20-electronica/drivers-bts7960.md)
- Estado de los servos durante el arranque: [20-electronica/direccion-swerve-servos.md](../20-electronica/direccion-swerve-servos.md)
