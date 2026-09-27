---
titulo: PCB del modelo a escala - documentacion completa
subsistema: Electronica y Potencia
estado: vigente
fecha: 2026-09-09
fuente_primaria: PCB_modelo.kicad_sch (KiCad 9.0, hoja A3)
fuente_secundaria: Modelo_a_escala_ROVER.pdf (17-ago-2026) - OBSOLETO donde contradiga al .sch
---

# PCB del modelo a escala (PDB + control)

Documentación derivada del esquemático de KiCad, que es la fuente de verdad. Donde el PDF de
planificación del 17-ago-2026 dice otra cosa, manda el `.sch` y se aclara la diferencia.

**Estado de validación:** la placa ya tuvo pruebas parciales con resultado correcto. Se probó un
encoder y un puente H de cada unidad, y funcionaron. El testeo completo está en curso.

Cambios ya confirmados respecto del PDF:
- El **ACS712-30A quedó cancelado**. Se reemplaza por un **TMCS1126B4** midiendo **corriente total
  de tracción**, no por rama.
- **No se usa el sensado de corriente de los módulos BTS7960** (los pines `IS` quedan sin cablear).
- El **pinout del ESP32 cambió** respecto de la tabla del PDF (ver `arquitectura-electrica`).
- Los **pull-down de 10 k en las 8 líneas PWM no existen** en la placa. Se reemplazaron por una
  estrategia distinta con conmutadores analógicos NLAS4157.
- Los **4 PTC individuales de servo** se redujeron a **un solo PTC** para todo el riel.
- El MOSFET de polaridad inversa es **IRF9540N**, no IRF4905.

A lo largo del texto hay referencias del tipo (V1), (V2)... Son puntos de análisis de escritorio
que **quedan pendientes de confirmar midiendo**, no defectos verificados. Están resumidos al final,
en `pendientes-de-verificacion`.

---

# 20-electronica

## arquitectura-electrica

### Cadena de potencia

```
                         NODO A (12 V protegido, permanente)
 J1          Q1                 |
[Bat +]--->[D  S]-------+-------+--- D1 (TVS SM6T15A) --- GND
 12V/7Ah    IRF9540N    |       |
                        |       +--- D5+R16 (LED de presencia)
                   R14 10k      |
                   D4 16V       +--- JP1 ---+--- C7 1000u
                    (gate)      |           |
                                |           +--- U2 LM2596  --> RIEL "+5V" (a setear en 7,2 V)
                                |           |                     |
                                |           +--- J2 (SETA NC)     +--- ESP1.19 (VIN del devkit)
                                |                  |              +--- C9 100u + C10 1u
                                |                  v              +--- F2 (PTC) --> C1 1000u --> J12/J13/J14/J17 (4 servos)
                                |            K1 bobina A2
                                |            D3 en antiparalelo
                                |            K1 bobina A1 --> GND
                                |
                                +--- K1 contacto 11 (comun)
                                          |
                                     (NO) 14
                                          |
                                     U3 IN- [TMCS1126B4] IN+
                                                          |
                                                    RIEL +12V CONMUTADO
                                                          |
                                          +------+--------+--------+------+
                                          F5     F3       F4       F6     C2 1000u + R6 (divisor)
                                          |      |        |        |
                                         J6     J4       J5       J7
                                      driver1 driver2  driver3  driver4
```

### Rieles

| Riel | Origen | Tensión real | Cargas | Protección propia |
|---|---|---|---|---|
| Batería | J1 | 10,5 - 14,4 V (plomo-ácido 12 V / 7 Ah) | todo | Fusible aéreo (off-board) |
| Nodo A | salida de Q1 | Vbat - I·R_DS(on) | LM2596, bobina del relé, LED | D1 (TVS), Q1 (polaridad) |
| +12V conmutado | contacto NO de K1, vía U3 | Vbat - caída del relé - 0,7 mΩ del sensor | 4 drivers, C2, divisor de batería | F3..F6 (uno por driver) |
| "+5V" (nombre de red) | U2 LM2596 | **a setear en 7,2 V**, no 5 V | ESP32 VIN + 4 servos | ninguna en la entrada del ESP32 |
| Riel de servos | +5V detrás de F2 | 7,2 V menos la caída del PTC | 4 servos | F2 (PTC rearmable) |
| +3,3 V | AMS1117 del devkit ESP32 | 3,3 V | ESP32, TMCS, MPU, encoders, pull-ups, enables de los BTS, 3 NLAS | ninguna |

Dos consecuencias arquitectónicas que conviene tener presentes:

1. **La seta corta solamente la tracción.** El LM2596 y la bobina cuelgan del nodo A, aguas arriba
   del relé. Al apretar la seta se desenergiza la bobina, se abren los contactos y muere el bus de
   los drivers, pero **los servos de dirección y el ESP32 siguen alimentados**. Para el modelo a
   escala eso es deseable (la telemetría sobrevive al corte). Si en algún ensayo se necesita que los
   servos también caigan, hay que pasar el LM2596 aguas abajo del relé, y ahí se pierde la
   telemetría post-corte.
2. **V_bat se mide después del relé.** El divisor R6/R5 cuelga del riel +12 V conmutado. Con la seta
   apretada el ESP32 lee 0 V, y el corte por software a 10,5 V se dispararía como falso positivo.
   Se contempla en firmware (enmascarar el corte por baja tensión si la tensión cae de golpe a cero
   con corriente nula) o se mueve el divisor al nodo A.

### Presupuesto del riel de 3,3 V

Todo el 3,3 V de la placa sale del AMS1117 que trae el módulo ESP32. No hay regulador de 3,3 V
propio en la PDB.

| Carga | Consumo estimado |
|---|---|
| ESP32 con Wi-Fi activo | 80 - 120 mA promedio, picos de 300 mA |
| U3 TMCS1126 (Iq) | 11 - 14,5 mA |
| MPU6050 | ~4 mA |
| 4 encoders Hall | 40 - 80 mA (**medir**, es la incógnita grande) |
| 8 pull-ups de 4,7 k, peor caso todos en bajo | 5,6 mA |
| VCC lógico de los 4 BTS7960 + enables | 10 - 30 mA (**medir**) |
| 3 NLAS4157 | despreciable |
| **Total estimado** | **150 - 250 mA, picos de 400 mA** |

Con el LM2596 en 5 V: el AMS1117 disipa (5 - 3,3) x 0,25 = 0,43 W. Se banca.

Con el LM2596 en 7,2 V (la idea anotada en el esquemático para los servos): disipa
(7,2 - 3,3) x 0,25 = 0,98 W. En SOT-223 sobre el cobre mínimo de un devkit, con resistencia térmica
del orden de 60 a 110 °C/W, son 60 a 110 °C de salto sobre ambiente. Vale la pena medir la
temperatura del regulador antes de dejar el sistema andando mucho rato a 7,2 V (V10).

Si el número da alto, tres salidas:
- (a) Dos convertidores: uno a 5 V para el ESP32 y otro a 7,2 V solo para los servos.
- (b) Un regulador de 3,3 V propio en la PDB desde el nodo A, alimentando el ESP32 por 3V3 directo.
  Ojo con no alimentar simultáneamente 5V y 3V3.
- (c) Dejar el LM2596 en 5 V y aceptar menos torque en los servos.

### Mapeo de pines del ESP32 (extraído del `.sch`)

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
| IO21 | 33 | SDA | I2C hardware | sin pull-up en la placa (los pone el módulo GY-521) |
| IO22 | 36 | SCL | I2C hardware | sin pull-up en la placa |
| IO23 | 37 | L_PWM_4 | PWM driver 4 | nada |
| 3V3 | 1 | +3,3 V | alimentación | C15 100u + C11 1u |
| EXT_5V | 19 | +5V | alimentación (VIN) | C9 100u + C10 1u |
| GND | 14, 32, 38 | GND | — | — |
| EN | 2 | sin conexión | marcado no-connect | — |
| SD0..SD3, CMD, CLK | 16-18, 20-22 | sin conexión | flash interna, marcados no-connect | correcto |
| TXD0 / RXD0 | 35 / 34 | sin conexión | USB-UART del devkit | quedan libres para consola |

**No queda ningún GPIO libre.** IO1/IO3 están tomados por la consola serie. Cualquier función nueva
(por ejemplo leer el pin OC del sensor de corriente) requiere sacar algo o agregar un expansor I2C.

### Estrategia de arranque seguro

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
| U1 | IO5 | R_PWM_4 | entrada del BTS7960 en alto. **A verificar con osciloscopio (V1)** |

### Notas de arquitectura

- El número de driver (1..4) **no está asociado a una rueda** en ningún lado del esquemático. Los
  encoders sí (FR, FL, RL, RR) y los servos están numerados 1..4. Conviene fijar la correspondencia
  driver N <-> rueda <-> servo N y rotularla en el mazo y en el firmware.
- No hay watchdog externo ni forma de resetear la etapa de potencia sin la seta.
- No hay CAN en esta etapa (decisión explícita del PDF, sigue vigente).

---

## encoders-pcnt

Validado: se probó un encoder y funcionó correctamente.

### Interfaz física

Cuatro conectores idénticos de 4 vías, Molex KK-254 vertical:

| Conector | Rueda | Pin 1 | Pin 2 | Pin 3 | Pin 4 |
|---|---|---|---|---|---|
| J9 | Delantera derecha (FR) | GND | +3,3 V | Canal A -> IO36 | Canal B -> IO39 |
| J16 | Delantera izquierda (FL) | GND | +3,3 V | Canal A -> IO34 | Canal B -> IO35 |
| J18 | Trasera izquierda (RL) | GND | +3,3 V | Canal A -> IO16 | Canal B -> IO17 |
| J8 | Trasera derecha (RR) | GND | +3,3 V | Canal A -> IO18 | Canal B -> IO19 |

**Los encoders se alimentan con 3,3 V**, no con 5 V. Esto resuelve de raíz el riesgo de sobretensión
en IO34..IO39 que estaba abierto: como el encoder del CQR37D12V64EN-I acepta de 3,3 a 24 V, al
alimentarlo con 3,3 V la salida no puede superar 3,3 V, tenga o no pull-up interno el módulo.
Si algún módulo trae pull-up interno a VCC, queda en paralelo con el de la placa y el equivalente
baja (4,7 k // 10 k = 3,2 k): no es un problema, pero sí una diferencia de tiempos entre canales.

### Acondicionamiento

Cada canal tiene **un pull-up de 4,7 k a 3,3 V** (R1, R7, R8, R9, R10, R11, R12, R13) y nada más.
No hay resistencia serie ni capacitor. Esto es obligatorio, no opcional: IO34, IO35, IO36 e IO39
son de solo entrada y **no tienen pull-up ni pull-down internos**, así que sin estas resistencias
esas cuatro líneas quedarían flotando.

Verificación de que 4,7 k alcanza:

- Capacidad del cable: ~100 pF/m. Con 60 cm de cable más la entrada del ESP32, del orden de 70 pF.
- Constante de subida: 4,7 k x 70 pF = **0,33 us**.
- Frecuencia máxima por canal, con el motor CQRobot a 120 rpm de salida, 90:1 y 64 CPR en el eje
  del motor: 10800 rpm de motor = 180 rev/s, 16 pulsos por vuelta y por canal -> **2,9 kHz**.
  Período de 345 us, y el evento más corto (flanco de A a flanco de B en cuadratura) son **86 us**.

0,33 us contra 86 us: tres órdenes de magnitud de margen. El pull-up de 4,7 k está bien elegido para
velocidad. Lo que se pierde frente a un valor más bajo es inmunidad al ruido. Si aparecen cuentas
espurias con los motores girando, la secuencia de corrección es:

1. Activar el **filtro de glitch del periférico PCNT** (es gratis, no lleva componentes). Con 100 a
   200 ciclos de APB (1,25 a 2,5 us) se filtra ruido sin tocar los 86 us reales. El máximo son 1023
   ciclos = 12,8 us, así que hay lugar de sobra.
2. Recién si eso no alcanza, bajar los pull-ups a 2,2 k.
3. Como último recurso, un RC (2,2 k serie + 1 nF). **No usar 10 nF**: con esas impedancias destruye
   el flanco.

### Uso del periférico PCNT

El ESP32 tiene 8 unidades PCNT. Se usan 4, una por rueda, con los dos canales de cada unidad
configurados para decodificación en cuadratura x4 (una señal como pulso, la otra como control).

Cuentas por vuelta de rueda: 64 CPR en el eje del motor x 90 de reducción = **5760 cuentas por
vuelta de salida**.

Punto a resolver en firmware: **el contador del PCNT es de 16 bits con signo** (±32767). A 120 rpm
de salida son 11520 cuentas/s, o sea que **desborda cada 2,8 segundos**. Hay que usar los eventos de
límite del hardware y acumular en una variable de 32 o 64 bits. Si se lee el contador por sondeo sin
manejar el desborde, la medición de velocidad se rompe silenciosamente.

### Pendientes

- Repetir la prueba en los cuatro canales, no solo en el que ya se validó.
- Medir el consumo real de los cuatro encoders a 3,3 V (entra directo en el presupuesto del AMS1117).
- Definir el sentido positivo de cuenta de cada rueda y dejarlo escrito (las dos ruedas de un lado
  giran al revés que las del otro para avanzar).
- Cablear A/B como par trenzado con su propio GND de retorno, lejos de los cables de motor.

---

## drivers-bts7960

Validado: se probó un puente H de cada unidad y funcionaron.

### Interfaz

Cada driver usa **dos conectores separados**: uno de señal (4 vías) y uno de potencia (2 vías).

Conectores de señal, Molex KK-254 de 4 vías:

| Conector | Pin 1 | Pin 2 | Pin 3 | Pin 4 |
|---|---|---|---|---|
| J10 (BTS7960_1) | L_PWM_1 <- IO26 | R_PWM_1 <- IO25 | +3,3 V | GND |
| J11 (BTS7960_2) | L_PWM_2 <- IO14 | R_PWM_2 <- IO27 | +3,3 V | GND |
| J15 (BTS7960_3) | L_PWM_3 <- IO4 | R_PWM_3 <- IO13 | +3,3 V | GND |
| J19 (BTS7960_4) | L_PWM_4 <- IO23 | **R_PWM_4 <- U1 (no directo del ESP32)** | +3,3 V | GND |

Conectores de potencia, Molex KK-254 de 2 vías, cada uno detrás de su fusible:

| Conector | Etiqueta | Fusible | Pin 1 | Pin 2 |
|---|---|---|---|---|
| J6 | 12v_driver_1 | F5 | +12 V fusionado | GND |
| J4 | 12v_driver_2 | F3 | +12 V fusionado | GND |
| J5 | 12v_driver_3 | F4 | +12 V fusionado | GND |
| J7 | 12v_driver_4 | F6 | +12 V fusionado | GND |

La correspondencia fusible-driver **no es correlativa** (F3 va al driver 2, F5 al driver 1).
Conviene dejarlo serigrafiado.

### Habilitaciones

Los `R_EN` y `L_EN` de los cuatro módulos se alimentan desde el pin 3 de cada conector de señal, que
va a +3,3 V permanente. Es decir: **los puentes H están habilitados desde que hay 3,3 V, y el estado
de reposo se delega enteramente a las líneas PWM**. Esto ahorra 4 GPIO, con dos consecuencias:

1. No hay forma de apagar los puentes por hardware sin cortar el bus de 12 V con el relé.
2. Queda libre el único mecanismo de apagado rápido disponible. Ver la propuesta en
   `sensado-corriente` para engancharle el pin OC del TMCS1126.

### Estado del canal 4 durante el arranque (V1)

De la tabla de verdad del NLAS4157 se desprende que, durante la ventana de arranque del ESP32,
`R_PWM_4` quedaría en 3,3 V:

- IO5 arranca en ALTO (pull-up interno, latcheado por el strapping).
- U1 tiene B1 a +3,3 V y B0 a GND, así que su salida A copia a S.
- L_PWM_4 (IO23) es un GPIO común: al reset queda como entrada sin pull. El BTS7960 tiene pull-down
  interno en IN, lo cual cubre a L_PWM_4 pero no a R_PWM_4.
- Los enables están activos de forma permanente.

Si el análisis es correcto, el driver 4 recibiría R_PWM alto y L_PWM bajo, y el motor giraría hasta
que el firmware tome el control. **Esto hay que confirmarlo o descartarlo con osciloscopio**, porque
hay al menos dos cosas que podrían hacer que no pase: el mapeo de números de pin del símbolo
reutilizado (U1 usa el símbolo `RF_Switch:MASWSS0136` renombrado a NLAS4157; si la numeración del
símbolo o de la huella no coincidiera con el NLAS4157 real, la conclusión cambia), y el
comportamiento real de la entrada del módulo BTS7960 a 3,3 V de VCC.

Medición concreta: disparar el osciloscopio en el flanco de EN del ESP32 y mirar R_PWM_4 y L_PWM_4
durante todo el arranque, con el conector de potencia del driver 4 desconectado.

Si se confirma, la corrección de menor costo es **invertir B0/B1 en U1** (B1 a GND, B0 a +3,3 V) e
invertir esa salida PWM por hardware en la matriz de GPIO del LEDC, que no cuesta nada. Si la placa
ya está fabricada son dos cortes y dos puentes. Conviene aplicar el mismo criterio a U4 y U5 aunque
ahí no sea crítico, para que los tres canales queden con la misma convención.

### Otros puntos de los drivers

- **VCC lógico a 3,3 V.** Los módulos BTS7960 comerciales están especificados con VCC de 5 V. Con
  3,3 V andan (el IN es un Schmitt trigger compatible TTL/CMOS) y ya se comprobó en banco, pero se
  está fuera de la especificación nominal del módulo. Conviene verificar la conmutación en todo el
  rango de ciclo de trabajo, no solo en un punto.
- **Los pines IS quedan sin usar.** Decisión confirmada. Consecuencia: no hay medición de corriente
  por rueda, solo la total.
- **No hay pull-down en las 8 líneas PWM.** Dependen del pull-down interno del BTS7960 durante el
  boot. Mejora barata: un pull-down de 10 k **del lado del driver** (soldado en el módulo o en el
  conector). Del lado del GPIO no protege contra un cable desconectado.
- **Terminación serie.** Con 60 a 100 cm de cable y flancos rápidos, 220 Ω en serie en el origen de
  cada línea PWM ayuda. No están en la placa; se pueden agregar en el conector.
- **Desacople local.** El PDB tiene C2 de 1000 uF, pero los drivers están a 60-100 cm. La
  inductancia de ese cable (del orden de 1 uH/m) domina a frecuencias de conmutación. **Cada
  BTS7960 necesita su propio desacople físicamente en el módulo**: 2 x 470 uF / 25 V / 105 °C de
  bajo ESR más 100 nF cerámico en los bornes. Es hardware fuera de la PCB pero es parte del diseño.

---

## direccion-swerve-servos

El modelo a escala pasó a ser **swerve, con un servo de dirección por rueda** (ya no es skid-steer).
Con eso el modelo sí valida la cinemática swerve, que era su laguna principal.

### Interfaz

Cuatro conectores Molex KK-254 de 3 vías, formato servo estándar:

| Conector | Nombre | Pin 1 | Pin 2 | Pin 3 (señal) | Origen de la señal |
|---|---|---|---|---|---|
| J12 | Servo1 | GND | riel de servos | Servo1 | **U4** <- IO0 |
| J14 | Servo2 | GND | riel de servos | Servo2 | IO2 directo, con pull-down R21 |
| J13 | Servo3 | GND | riel de servos | Servo3 | **U5** <- IO15 |
| J17 | Servo4 | GND | riel de servos | Servo4 | IO12 directo, con pull-down R22 |

Igual que con los drivers: la asociación servo N <-> rueda no está en el esquemático y hay que
fijarla.

### Margen del riel de servos (V3)

| Dato | Valor |
|---|---|
| Servos | 4, alimentados a 7,2 V según la nota del esquemático |
| Corriente de bloqueo por servo | 2,5 A (el PDF cita MG996R; confirmar el modelo real) |
| Demanda con los 4 bloqueados | 10 A |
| Demanda con los 4 moviéndose sin carga | 2 a 3,6 A |
| Capacidad del LM2596 (módulo comercial) | 3 A de pico nominal, 1,5 a 2 A continuos reales sin disipador |
| Reserva local | C1 = 1000 uF |

Cuánto aguanta el capacitor: para 10 A y aceptando 0,5 V de caída, `dt = C·dV/I = 1000u x 0,5 / 10`
= 50 microsegundos. O sea, sirve para los flancos de conmutación, no para sostener un bloqueo.

Como el ESP32 se alimenta del **mismo** nodo (pin 19, VIN, aguas arriba del PTC), en el caso de
bloqueo simultáneo el riel podría hundirse lo suficiente como para que el AMS1117 se quede sin
margen de caída (necesita del orden de 1,1 a 1,3 V por encima de 3,3 V) y el ESP32 se resetee. El
PTC no lo evita: actúa en segundos, el brownout en milisegundos.

**Ensayo que lo define:** trabar los cuatro servos a mano (o contra tope mecánico) con el
osciloscopio en el riel de 5V/7,2V y en 3V3, y ver cuánto cae y si el ESP32 sobrevive. Ese ensayo
convierte esto de hipótesis en dato.

Mitigaciones, de más a menos efectiva:

1. **Alimentación separada para los servos**: una 2S LiPo dedicada (7,4 V, ESR bajo del orden de
   25 mΩ) o un segundo convertidor. Solo se comparte GND, en el punto estrella. Libera además al
   LM2596 para quedar en 5 V.
2. **Convertidor propio y chico para el ESP32** desde el nodo A.
3. **Firmware, en cualquier caso**: escalonar los comandos de los 4 servos con 50 a 100 ms de
   desfasaje, limitar la velocidad de cambio de la consigna, y poner un temporizador de bloqueo que
   corte el PWM si el servo no llegó a destino. Un servo sin pulso consume mucho menos que uno
   trabado.
4. Subir C1 a 4700 uF si se decide seguir compartiendo. Ayuda con los picos de arranque.

### F2: un solo PTC para los cuatro servos

El PDF planificaba 4 PTC individuales; la placa tiene uno solo (F2) en serie con todo el riel.

Trade-off explícito:
- **A favor**: menos componentes, menos área, menos caída acumulada.
- **En contra**: se pierde selectividad. Un servo trabado corta el riel de los cuatro. Con 4 PTC, el
  que se traba se aísla y los otros tres siguen funcionando.

Además, no existen PTC radiales chicos que sostengan 10 A: los valores prácticos llegan a 3 a 5 A de
corriente de mantenimiento. Con uno de 3 A, un movimiento simultáneo de los cuatro servos queda
cerca del disparo; con 5 A, no protege contra dos servos trabados.

**El valor de F2 no está definido en el esquemático.** La huella asignada es de varistor de disco de
9 mm: un PTC radial entra físicamente, pero conviene corregir la huella para que el BOM no confunda.

### Estado en el arranque

| Servo | Pin | Estado durante el boot | Riesgo |
|---|---|---|---|
| Servo1 | IO0 vía U4 | 3,3 V continuos | Sin pulso PWM válido el servo no se mueve |
| Servo2 | IO2 con pull-down | 0 V | Ninguno |
| Servo3 | IO15 vía U5 | 3,3 V continuos | Ídem Servo1 |
| Servo4 | IO12 con pull-down | 0 V | Ninguno |

Los cuatro casos son seguros. La única salvedad es que un nivel continuo en alto puede producir una
contracción mecánica leve en algunos servos digitales al energizar.

---

## sensado-corriente

### Qué se eligió y por qué

El ACS712-30A del PDF quedó cancelado. En su lugar hay **un TMCS1126B4** (Texas Instruments), sensor
Hall con aislación reforzada, en encapsulado SOIC-10 ancho.

| Parámetro | Valor |
|---|---|
| Sensibilidad | **100 mV/A** |
| Tensión de salida a corriente cero (VREF interno) | **1,65 V** (variante B, bidireccional) |
| Rango lineal con VS = 3,3 V | **±15,5 A** |
| Resistencia del conductor interno | 0,7 mΩ (a 10 A disipa 70 mW) |
| Ancho de banda | 500 kHz |
| Consumo | 11 a 14,5 mA |
| Tiempo de encendido hasta salida válida | 34 ms |
| Aislación | reforzada, 8 mm de línea de fuga y de aire |

La ventaja frente a un shunt es doble: no hay modo común (la salida ya está referida a masa lógica)
y el aislamiento galvánico lo hace inmune a los flancos del bus conmutado, que es exactamente el
problema que tiene un shunt en el lado alto con un puente H al lado.

### Dónde está conectado

`contacto NO del relé (K1.14) -> U3.IN- -> [conductor interno] -> U3.IN+ -> riel +12 V -> F3..F6 -> drivers`

Tres consecuencias:

1. **Mide únicamente la rama de tracción.** El LM2596 (y con él el ESP32 y los servos) cuelga del
   nodo A, aguas arriba del relé y del sensor. Lo que se lee es la suma de los cuatro motores más el
   consumo lógico de los cuatro drivers. Para el modelo a escala eso es incluso mejor que medir el
   total: el número que sale es directamente la corriente de tracción.
2. **El signo queda invertido.** La corriente entra por IN- y sale por IN+. La función de
   transferencia es `VOUT = IIN·S + VREF` con IIN positiva de IN+ a IN-. Con este cableado,
   descargar la batería hace **bajar** la salida desde 1,65 V:
   `I_traccion = (1,65 - Vout) / 0,1`
   Conviene confirmarlo con una carga conocida en el primer ensayo. Dar vuelta IN+/IN- no gana
   rango, porque el dispositivo es simétrico alrededor de 1,65 V.
3. **No hay medición por rama.** Con los pines IS de los BTS7960 sin usar, no se puede estimar el
   torque rueda por rueda desde el rover. Para el ensayo de caracterización `τ = K·(I - I0)` no
   molesta (ese ensayo va en banco con un motor por vez), pero sí impide validar el reparto de carga
   entre ruedas durante una trepada.

### Rango de medición contra corriente de bloqueo (V8)

| Escenario | Corriente | Salida del sensor | ¿Entra en rango? |
|---|---|---|---|
| Marcha normal, 4 motores | 2 a 6 A | 1,45 a 1,05 V | Sí |
| Un motor bloqueado, tres girando | ~8 A | 0,85 V | Sí |
| Cuatro motores bloqueados | 22 A | saturaría a los 15,5 A | No |

El sensor satura alrededor de 15,5 A. Más que un error, es una decisión de diseño que conviene
asumir explícitamente: **el canal analógico cubre la operación normal y el canal digital OC cubre la
zona de falla**. Para eso hay que ajustar el umbral (V9). Alimentar el sensor con 5 V no ayuda,
porque el techo real lo pone la entrada del ADC del ESP32 en 3,3 V.

Resolución alcanzable: el ADC de 12 bits sobre 3,3 V da 0,806 mV por escalón, o sea **8 mA por
escalón**. El ruido de entrada del sensor (150 uA/√Hz) filtrado a 1,6 kHz da unos 6 mA rms, del
mismo orden. El límite práctico va a ser la no linealidad conocida del ADC del ESP32, que hay que
calibrar por puntos.

### Cadena de acondicionamiento

| Bloque | Componentes | Función |
|---|---|---|
| Alimentación del sensor | +3,3 V -> C4 100 n -> FB1 (ferrita) -> C3 100 n -> VS | filtro en pi, aísla el sensor del ruido del riel lógico. Es la topología que recomienda el fabricante |
| Umbral de sobrecorriente | R3 4,7 k (de VS a VOC) + R4 47 k (de VOC a GND) + C5 1 u | divisor que fija el umbral |
| Salida al ADC | R15 1 k serie + C8 100 n a masa -> IO33 | pasabajos de **1,59 kHz** (τ = 100 us). Atenúa 22 dB a 20 kHz de PWM |
| VREF (pin 7) | libre, solo con una etiqueta local `Vref_CS` | ver nota |
| OC (pin 3) | R2 4,7 k a masa | ver V9 |

Sobre el R15 de 1 k: además de filtrar, aísla el capacitor de 100 nF de la salida del sensor. El
TMCS1126 admite como máximo 4,7 nF directamente en VOUT; sin esa resistencia serie el amplificador
de salida podría oscilar. Está bien resuelto.

Sobre VREF libre: es una **salida**, no una entrada, así que dejarla al aire es legal. Es una
oportunidad disponible: llevándola a un segundo canal de ADC y midiendo `Vout - Vref` en vez de
`Vout - 1,65 V` se cancelan el error de referencia, su deriva térmica y buena parte de la deriva del
propio ADC. Hoy no hay ningún ADC1 libre; queda como mejora.

### Umbral de sobrecorriente y pin OC (V9)

**Umbral.** La ecuación del fabricante es `VOC = S · IOC / 2,5`. Con el divisor actual:
`VOC = 3,3 × 47 / (47 + 4,7) = 3,0 V`, lo que da

`IOC = 2,5 × VOC / S = 2,5 × 3,0 / 0,1 = 75 A`

75 A no los alcanza este sistema, así que la protección no llegaría a dispararse nunca. Además el
fabricante pide que la resistencia inferior del divisor sea **menor a 10 k** (la impedancia de
entrada del pin VOC es 120 k y carga el divisor); R4 = 47 k queda fuera de esa guía y agrega un
error de umbral del orden del 30 %. Conviene medir la tensión real en VOC antes de dar el cálculo
por válido.

Ajuste propuesto, para un umbral de 25 A (por encima del bloqueo de 22 A, así no hay falsos
positivos):

`VOC = 0,1 × 25 / 2,5 = 1,0 V`

| Componente | Actual | Propuesto |
|---|---|---|
| R3 (VS a VOC) | 4,7 k | 11 k |
| R4 (VOC a GND) | 47 k | 4,7 k |
| VOC resultante | 3,0 V | 0,987 V |
| IOC resultante | 75 A | 24,7 A |

La histéresis de la variante de 100 mV/A es de 1,4 A, así que rearmaría a los 23,3 A. También
conviene subir C5 de 1 uF a 10 uF: el fabricante lo pide para estabilizar el umbral del comparador
en ambientes con flancos de modo común rápidos, que es justo el caso de un puente H.

**Pin OC.** Es una salida de colector abierto activa en bajo, conectada a masa a través de R2 =
4,7 k. Con un pull-down no puede subir nunca, así que la señal queda siempre baja y no va a ningún
lado. El fabricante dice que si no se usa se conecte a masa, así que no rompe nada.

**Mejora de alto valor:** hoy los `R_EN`/`L_EN` de los cuatro BTS7960 están atados permanentemente a
+3,3 V. Si en vez de eso ese nodo de enables se lleva a 3,3 V a través de una resistencia de pull-up
de 4,7 k y se le conecta el pin OC del TMCS1126, se obtiene:

- Corte de los cuatro puentes en 100 ns ante sobrecorriente, sin intervención del firmware.
- Rearme automático con histéresis.
- Cero GPIO consumidos.
- Y si además ese nodo se lleva a una entrada del ESP32 (cuando se libere una), se puede contar
  cuántas veces disparó.

Es una resistencia y una pista, y convierte una protección inactiva en la más rápida de la placa.

### Calibración

El TMCS1126 viene calibrado de fábrica (error de sensibilidad de 0,3 % en grado B), así que el error
dominante no va a ser el sensor sino el ADC del ESP32. Procedimiento mínimo:

1. Con el bus energizado y sin carga, leer `Vout` y guardarlo como cero real. No asumir 1,65 V.
2. Poner una carga conocida (lámpara de auto, resistor de potencia) medida con pinza amperométrica
   en 3 o 4 puntos entre 1 y 10 A, y ajustar una recta.
3. Repetir con los motores girando y PWM activo, para ver cuánto ripple queda después del filtro y
   decidir el promediado por software.
4. Verificar el cero a distintas temperaturas: la deriva de offset de esta variante son 30 uV/°C
   típicos, o sea 0,3 mA/°C referido a la entrada. Despreciable.

---

## protecciones-desacople

### Capas de protección

| Capa | Componente | Qué protege |
|---|---|---|
| Fusible general | aéreo, off-board | cableado de batería |
| Polaridad inversa | Q1 IRF9540N + R14 10 k + D4 zener 16 V | toda la placa |
| Sobretensión | D1 SM6T15A en el nodo A | toda la placa |
| Corte de emergencia | J2 (seta NC) -> bobina K1 -> contacto NO | rama de tracción |
| Antirretorno de bobina | D3 en antiparalelo con la bobina | contactos de la seta |
| Sobrecorriente por rama | F3..F6, uno por driver | cableado a cada driver |
| Sobrecorriente en servos | F2 (PTC) | rama de servos |
| Sobrecorriente electrónica | pin OC del TMCS1126 | hoy sin usar (ver V9) |
| Protección de ADC | D2 BAT54S en V_bat | entrada IO32 |
| Indicación | D5 + R16 2,2 k (~4,5 mA) | — |

### Circuito de polaridad inversa

El circuito de compuerta está bien resuelto: el drenador va a la batería y la fuente a la carga, de
manera que el diodo intrínseco conduce en el sentido normal de operación y bloquea con la batería
invertida. La compuerta se lleva a masa por R14 (10 k) y el zener D4 de 16 V limita V_GS, que en el
IRF9540N tiene un máximo de ±20 V. Correcto.

**Disipación en Q1 (V2).** El IRF9540N es canal P, -100 V, -23 A, con **R_DS(on) = 117 mΩ** típico a
VGS = -10 V, en TO-220 vertical. La aritmética de disipación:

| Corriente de bus | Disipación en Q1 | Salto térmico sin disipador (~62 °C/W) |
|---|---|---|
| 1 A | 0,12 W | 7 °C |
| 3 A | 1,05 W | 65 °C |
| 5 A | 2,9 W | 181 °C |
| 10 A | 11,7 W | fuera de rango |
| 22 A (bloqueo) | 56,6 W | fuera de rango |

En banco con poca carga no se nota; el problema aparecería con las cuatro ruedas cargadas. **La
medición que lo define es tocar/medir la temperatura de Q1 con el rover andando sobre terreno**, y
medir la caída de tensión entre drenador y fuente con corriente conocida (a 5 A deberían ser unos
0,6 V si el R_DS(on) es el del datasheet).

Si el número da alto, reemplazos que mantienen el mismo circuito de compuerta:

| Candidato | V_DS | I_D | R_DS(on) | Encapsulado | Disipación a 22 A |
|---|---|---|---|---|---|
| **IRF4905** | -55 V | -74 A | 20 mΩ | TO-220 | 9,7 W (con disipador) |
| AOD4185 / AOB4185 | -40 V | -40 A | ~10 mΩ | TO-252 / TO-263 | 4,8 W |

Alternativa que el propio PDF menciona: eliminar el circuito de polaridad inversa y confiar en un
conector polarizado (XT60). Pierde protección contra el error humano de conectar con caimanes, pero
elimina una fuente de calor y de caída.

### TVS: tensión de trabajo y ubicación (V5, V6)

**V5.** D1 es un **SM6T15A**: 600 W, tensión de trabajo (standoff) de aproximadamente 12,8 V,
ruptura mínima de 14,3 V. Una batería de plomo-ácido flota entre 13,5 y 13,8 V y en carga cíclica
llega a 14,4 V. Con el cargador conectado y el rover energizado, el TVS podría quedar cerca de su
ruptura y conducir. Medición: con la batería recién cargada, medir la corriente de fuga por D1 (o
simplemente su temperatura) con el rover encendido.

Si conduce, reemplazo: **SM6T18A** (standoff 15,3 V) o **SMBJ16A / SMCJ16A** (standoff 16 V). La
tensión de recorte resultante, del orden de 26 V, es segura: los BTS7960 admiten hasta 45 V.

**V6.** El TVS está en el nodo A, aguas arriba del relé. Cuando la seta abre los contactos con los
motores girando, la energía inductiva queda del lado de los drivers, donde no hay TVS: solo C2
(1000 uF), que absorbe la mayor parte, y los diodos intrínsecos de los puentes. Conviene ver con
osciloscopio qué pico aparece en el riel conmutado al abrir la seta a plena marcha. Si es alto, un
segundo TVS (SMBJ16A alcanza) entre el riel conmutado y GND, cerca de C2.

La ubicación del TVS que sí está es correcta en un aspecto: queda **después** del MOSFET de
polaridad inversa, que es donde tiene que ir.

### Inrush del contacto del relé (V7)

K1 es un relé automotriz de 12 V / 40 A. Al cerrar conecta el nodo A contra C2 (1000 uF) más el
desacople local de los drivers (4 x 940 uF si se monta lo recomendado), del orden de **4,8 mF**. La
energía que va al arco de cierre es del orden de `½CV² = 350 mJ` por encendido.

Un contacto de 40 A lo tolera, pero repetido erosiona y puede terminar soldando los contactos. El
síntoma es que la seta deja de cortar y nadie se entera hasta que hace falta. **Verificación
periódica: con la seta apretada, medir continuidad entre el nodo A y el riel conmutado.** Debe ser
circuito abierto.

Mitigación estándar si aparece: **resistencia de precarga de 22 Ω / 5 W en paralelo con el
contacto**. La constante de carga pasa a `22 × 4,8 mF = 106 ms`, o sea que los capacitores ya están
cargados cuando el contacto cierra. No requiere lógica. Para el arco de apertura, un snubber RC
(100 Ω + 100 nF en serie, en paralelo con el contacto).

### Desacople actual

| Riel / punto | Capacitores | Comentario |
|---|---|---|
| Nodo A (entrada del LM2596) | C7 1000 uF | correcto |
| +12 V conmutado | C2 1000 uF | correcto en la placa; el desacople remoto de los drivers es aparte |
| Salida del LM2596 ("+5V") | C9 100 uF + C10 1 uF | correcto |
| Riel de servos (detrás de F2) | C1 1000 uF | correcto en topología |
| +3,3 V | C15 100 uF + C11 1 uF | conviene sumar un 100 nF cerámico pegado al pin 3V3 |
| VS del TMCS1126 | C4 100 n + FB1 + C3 100 n | filtro en pi, muy bien |
| VOC del TMCS1126 | C5 1 uF | el fabricante sugiere 10 uF |
| VCC de cada NLAS4157 | C12, C13, C14 (100 n) | uno por chip, correcto |
| V_bat (ADC) | C6 100 n | con la impedancia de Thevenin del divisor (1,8 k) da un polo en 883 Hz |
| Vout_CS (ADC) | C8 100 n | con R15 de 1 k da un polo en 1,59 kHz |

El único agregado que falta es un 100 nF en 3V3: los electrolíticos de 100 uF tienen ESR e
inductancia que los vuelve poco efectivos por encima de unos cientos de kHz, que es el rango donde
el ESP32 mete escalones de corriente al transmitir por Wi-Fi.

---

# 30-pcb

## especificacion

### Datos generales

| Ítem | Valor |
|---|---|
| Herramienta | KiCad 9.0, eeschema |
| Hoja | A3, una sola hoja (diseño plano, sin jerarquía) |
| Componentes en el esquemático | 74 (sin contar símbolos de alimentación) |
| Redes | 33 con nombre + 22 locales |
| Montaje | mixto: SMD 0805/0603 para señal, THT para potencia y conectores |
| Fijación | 4 agujeros M3 (H1..H4) en las esquinas, huella `MountingHole_3.2mm_M3` |
| Cobre | 1 oz (35 um) asumido, capa externa |
| Capas | a definir (2 capas es viable; 4 mejoraría el retorno) |

### Corrientes de diseño

| Tramo | Continua esperada | Pico | Criterio |
|---|---|---|---|
| Batería a nodo A | 3 a 6 A | 22 A | limitado por el fusible general |
| Nodo A a relé a sensor a bus +12 V | 3 a 6 A | 22 A | pasa por el conductor interno del TMCS (0,7 mΩ) |
| Bus +12 V a cada fusible | 1 a 2 A | 5,5 A | por rama |
| Riel de servos | 1 a 3 A | 10 A | ver V3 |
| Riel de 3,3 V | 0,15 a 0,25 A | 0,4 A | ver presupuesto |

### Anchos de pista (IPC-2221, 1 oz, ΔT = 10 °C)

| Red | Corriente de diseño | Ancho | Implementación |
|---|---|---|---|
| Bus principal 12 V y su retorno | 22 a 25 A | 10,5 a 12 mm | plano de cobre en ambas caras, unido con vías, reforzado con estaño |
| Riel de servos | 10 A de pico | 4,5 a 5,5 mm | pista ancha o vertido |
| Salida a cada driver | 5,5 A | 2,5 a 3,0 mm | pista |
| Señales (PWM, encoders, I2C, ADC) | menos de 100 mA | 0,4 a 0,6 mm | estándar |

Estos anchos vienen del PDF y siguen siendo válidos. IPC-2221 es una guía empírica conservadora para
régimen continuo; para picos de milisegundos el cobre tiene mucha más capacidad de la que sugiere.

### Componentes en placa y fuera de placa

**En la PCB:** zócalo del ESP32, U2 (módulo LM2596), U3 (TMCS1126), U1/U4/U5 (NLAS4157), Q1, K1
(relé), F2 a F6, D1 a D5, todos los capacitores y resistencias, y los 19 conectores.

**Fuera de la PCB:** batería, fusible general aéreo, seta de emergencia, los 4 módulos BTS7960 (con
su desacople local), los 4 servos, los 4 motores con encoder, y el módulo GY-521 (MPU6050) montado
en el centro de masa.

El criterio de montar los BTS7960 fuera es correcto: son los que disipan y los que hay que poder
cambiar rápido. El costo es la inductancia del cableado, que se compensa con el desacople local.

### Pendientes de especificación

- Dimensiones de la placa (no constan en el esquemático).
- Número de capas y espesor.
- Valores de F2 a F6.
- Definir si el conector de batería sigue siendo Phoenix o pasa a XT60 (ver V4).

---

## decisiones-layout

### Masas

Topología en **estrella**, con un único punto de unión entre el retorno de potencia y el retorno
lógico. El retorno de los motores no debe compartir cobre con el retorno de la lógica ni con el
analógico en ningún tramo. Sobre esta placa:

- Un vertido de masa de potencia que abarque J1, Q1, K1, U3, C2, C7 y los cuatro conectores de driver.
- Un vertido de masa lógica que abarque el ESP32, los conectores de señal, los encoders y el I2C.
- Unión en un solo punto, cerca del capacitor de entrada (C7) o del borne de la batería.
- El retorno del sensor (U3 pin 9) va a masa **lógica**, no de potencia: es el lado aislado del
  sensor y su salida es la que lee el ADC.

### El TMCS1126 en el layout

Es el componente que más condiciona el ruteo:

- Los pines 1 y 2 (IN+ / IN-) son el lado de alta corriente y están **galvánicamente aislados** del
  resto. El fabricante especifica 8 mm de línea de fuga y de aire, y la huella los respeta. **No
  meter ninguna pista, vertido ni vía en ese canal.**
- El conductor interno son 0,7 mΩ: a 10 A disipa 70 mW y a 22 A, 340 mW. Poco, pero la capacidad de
  corriente del encapsulado depende del cobre que lo rodea. Vertidos grandes en ambas caras para
  IN+ e IN-, con vías térmicas.
- Alejarlo de conductores de alta corriente que no sean los suyos.
- C3 (100 n de VS) pegado al pin 4, con su vía a masa lo más corta posible.
- R15 y C8 pegados al pin 6, y el ruteo hacia IO33 corto y lejos del bus.

### Potencia

- Ventanas de máscara antisoldante sobre las pistas de alta corriente, para reforzar con estaño.
- Vías de costura uniendo los planos de masa top y bottom, densas cerca del relé, de Q1 y de los
  conectores de driver.
- C2 pegado a los bornes de salida hacia los drivers, no en un rincón.
- C1 pegado al conector de servos y detrás del PTC.
- Q1 en TO-220 vertical: prever espacio y agujero para disipador.
- K1 y sus pistas de contacto son las que llevan los 22 A. Si se agrega precarga, reservarle lugar
  en paralelo con el contacto.

### Analógico y señal

- Las dos entradas de ADC (IO32 e IO33) son las líneas más sensibles de la placa. Cortas, con su
  retorno acompañándolas, y nunca en paralelo con las líneas PWM.
- El bus I2C hacia el MPU6050 va cableado (el módulo está en el centro de masa): par trenzado, SDA
  con GND y SCL con 3,3 V, lejos de los cables de motor. Como en la placa no hay pull-ups (los pone
  el módulo GY-521), conviene quedarse en 100 kHz.
- Las 8 líneas de encoder y las 8 de PWM salen por conectores distintos; separarlas físicamente en
  el layout y en el mazo.

### Mecánica

- 4 agujeros M3 en las esquinas, libres de pistas y de vertido, con separadores de bronce.
- Los mazos pesados que salen hacia los drivers deben tener sujeción al chasis cerca de la placa,
  para que la vibración no trabaje sobre las soldaduras de los conectores.
- Punteras tubulares obligatorias en todo cable multifilar que entre a una bornera a tornillo.
- Los fusibles F3..F6 usan portafusibles a clip para vidrio de 5x20 mm: dejar acceso físico para
  cambiarlos sin desmontar la placa.

---

## mapa-conectores

### Tabla completa

| Ref | Etiqueta | Vías | Huella | Pin 1 | Pin 2 | Pin 3 | Pin 4 | Corriente nominal del conector | Corriente esperada |
|---|---|---|---|---|---|---|---|---|---|
| J1 | Alimentacion | 2 | Phoenix MSTBA 2,5 horizontal, 5,08 mm | Batería + (a Q1.D) | GND | — | — | 12 A | hasta 22 A en bloqueo total. Ver V4 |
| J2 | E-Stop | 2 | Molex KK-254 | del nodo A (post JP1) | a bobina K1.A2 | — | — | 3-4 A | ~150 mA (solo bobina) |
| J3 | MPU6050 | 4 | Molex KK-254 | +3,3 V | GND | SCL | SDA | 3-4 A | pocos mA |
| J4 | 12v_driver_2 | 2 | Molex KK-254 | +12 V tras F3 | GND | — | — | 3-4 A | 5,5 A de pico. Ver V4 |
| J5 | 12v_driver_3 | 2 | Molex KK-254 | +12 V tras F4 | GND | — | — | 3-4 A | 5,5 A de pico |
| J6 | 12v_driver_1 | 2 | Molex KK-254 | +12 V tras F5 | GND | — | — | 3-4 A | 5,5 A de pico |
| J7 | 12v_driver_4 | 2 | Molex KK-254 | +12 V tras F6 | GND | — | — | 3-4 A | 5,5 A de pico |
| J8 | Encoder RR | 4 | Molex KK-254 | GND | +3,3 V | A -> IO18 | B -> IO19 | 3-4 A | pocos mA |
| J9 | Encoder FR | 4 | Molex KK-254 | GND | +3,3 V | A -> IO36 | B -> IO39 | 3-4 A | pocos mA |
| J10 | BTS7960_1 | 4 | Molex KK-254 | L_PWM_1 | R_PWM_1 | +3,3 V | GND | 3-4 A | pocos mA |
| J11 | BTS7960_2 | 4 | Molex KK-254 | L_PWM_2 | R_PWM_2 | +3,3 V | GND | 3-4 A | pocos mA |
| J12 | Servo1 | 3 | Molex KK-254 | GND | riel de servos | señal (de U4) | — | 3-4 A | 2,5 A de bloqueo |
| J13 | Servo3 | 3 | Molex KK-254 | GND | riel de servos | señal (de U5) | — | 3-4 A | 2,5 A de bloqueo |
| J14 | Servo2 | 3 | Molex KK-254 | GND | riel de servos | señal (IO2) | — | 3-4 A | 2,5 A de bloqueo |
| J15 | BTS7960_3 | 4 | Molex KK-254 | L_PWM_3 | R_PWM_3 | +3,3 V | GND | 3-4 A | pocos mA |
| J16 | Encoder FL | 4 | Molex KK-254 | GND | +3,3 V | A -> IO34 | B -> IO35 | 3-4 A | pocos mA |
| J17 | Servo4 | 3 | Molex KK-254 | GND | riel de servos | señal (IO12) | — | 3-4 A | 2,5 A de bloqueo |
| J18 | Encoder RL | 4 | Molex KK-254 | GND | +3,3 V | A -> IO16 | B -> IO17 | 3-4 A | pocos mA |
| J19 | BTS7960_4 | 4 | Molex KK-254 | L_PWM_4 | **R_PWM_4 (de U1)** | +3,3 V | GND | 3-4 A | pocos mA |
| JP1 | puente | 2 | TestPoint 2 pads, 2,54 mm | nodo A | rama permanente (LM2596 + seta + bobina) | — | — | — | 2 a 3 A |

### Márgenes de los conectores de potencia (V4)

Los valores nominales de catálogo contra las corrientes de peor caso:

- **J1, entrada de batería.** La familia Phoenix MSTB / MSTBA de 5,08 mm está especificada en 12 A
  por contacto. El bus puede pedir 22 A con las cuatro ruedas trabadas. Si el fusible general es de
  25 A, no protege al conector. Opciones: pasar J1 a **XT60** (60 A, polarizado, con traba), que era
  lo del plan original, o bajar el fusible general a 12-15 A y aceptar que un bloqueo simultáneo de
  las 4 ruedas lo corte (que es una falla, no una condición de operación).
- **J4 a J7, alimentación de los drivers.** Molex KK-254 está en el orden de 3 a 4 A por contacto
  según calibre y circuitos adyacentes. Cada rama puede llevar 5,5 A en bloqueo.

Esto además condiciona el valor de los fusibles: si el conector aguanta 4 A, el fusible tendría que
ser de 4 A o menos, y cortaría en cada bloqueo de una sola rueda (5,5 A). La cadena coherente es
cambiar J4..J7 por bornera a tornillo de 5,08 mm (lo del plan original) o XT30, y recién ahí poner
fusibles de 7,5 a 8 A retardados en F3..F6.

**Medición que lo define:** con el rover trepando o con una rueda trabada a propósito, tocar los
conectores después de un minuto. Si calientan, el margen no alcanza.

### Convenciones de cableado

Del PDF, siguen vigentes:

| Tramo | Calibre | Conector |
|---|---|---|
| Batería a PDB | 12-14 AWG silicona | XT60 (recomendado en lugar del Phoenix actual) |
| PDB a drivers | 14-16 AWG silicona | bornera 5,08 mm (recomendado en lugar del KK-254) |
| Driver a motor | 16-18 AWG silicona | terminales pala o soldadura |
| PDB a servos | 20-22 AWG | Molex KK / conector servo |
| Señales PWM | 22-24 AWG | Molex KK-254 |
| Encoders e I2C | 22 AWG, par trenzado | Molex KK-254 |
| Seta a PDB | 18-20 AWG | Molex KK-254 (la corriente es solo la de bobina) |

Prohibido protoboard y cables Dupont en cualquier tramo: el desgaste por vibración (*fretting*)
genera contactos intermitentes imposibles de diagnosticar después.

---

## bom

### Lista completa por referencia

| Ref | Valor | Huella | Función |
|---|---|---|---|
| ESP1 | ESP32-DEVKITC (38 pines) | zócalo | control, PCNT, PWM, ADC, I2C |
| U1 | NLAS4157 | SOT-363 / SC-70-6 | conmutador de R_PWM_4, aísla el strapping IO5 |
| U2 | LM2596 (módulo step-down) | módulo | 12 V a 5 V o 7,2 V |
| U3 | **TMCS1126B4** | SOIC-10 DVG | sensor de corriente total de tracción, 100 mV/A |
| U4 | NLAS4157 | SOT-363 | conmutador de Servo1, aísla IO0 |
| U5 | NLAS4157 | SOT-363 | conmutador de Servo3, aísla IO15 |
| Q1 | IRF9540N | TO-220 vertical | polaridad inversa (ver V2) |
| K1 | RAYEX-L90S (relé automotriz 12 V) | THT SPDT | corte de la rama de tracción |
| D1 | SM6T15A | SMB | TVS en el nodo A (ver V5) |
| D2 | BAT54S | SOT-23 | clamp doble Schottky en la entrada de ADC de batería |
| D3 | diodo estándar | SMA | antirretorno de la bobina del relé |
| D4 | zener 16 V | MiniMELF | protección de compuerta de Q1 |
| D5 | LED | 3 mm THT | indicación de bus energizado (~4,5 mA) |
| F2 | PTC rearmable | huella de disco 9 mm (corregir) | protección del riel de servos. Valor sin definir |
| F3, F4, F5, F6 | fusible 5x20 mm a clip | FuseClip 5x20 | uno por driver. Valor sin definir |
| FB1 | ferrita (1 k a 100 MHz) | 0805 | filtro de alimentación del sensor |
| C1 | 1000 uF | radial D10, P5 | reserva del riel de servos |
| C2 | 1000 uF | radial D10, P5 | reserva del bus +12 V conmutado |
| C7 | 1000 uF | radial D10, P5 | entrada del LM2596 |
| C9, C15 | 100 uF | radial D6,3 | salida de 5 V y riel de 3,3 V |
| C10, C11 | 1 uF | 0805 | acompañan a los de 100 uF |
| C5 | 1 uF | 0805 | estabilidad de VOC (sugerido 10 uF) |
| C3, C4 | 100 nF | 0805 | filtro en pi de la alimentación del sensor |
| C6 | 100 nF | 0805 | filtro del divisor de batería |
| C8 | 100 nF | 0805 | filtro de la salida del sensor |
| C12, C13, C14 | 100 nF | 0603 | desacople de los NLAS4157 |
| R1, R7, R8, R9, R10, R11, R12, R13 | 4,7 k | 0805 | pull-ups de los 8 canales de encoder |
| R2 | 4,7 k | 0805 | pin OC del sensor a masa (ver V9) |
| R3 | 4,7 k | 0805 | divisor de VOC, rama superior (ver V9) |
| R4 | 47 k | 0805 | divisor de VOC, rama inferior (ver V9) |
| R5 | 2,2 k | 0805 | divisor de batería, rama inferior |
| R6 | 10 k | 0805 | divisor de batería, rama superior |
| R14 | 10 k | 0805 | pull-down de compuerta de Q1 |
| R15 | 1 k | 0805 | serie del filtro de salida del sensor |
| R16 | 2,2 k | 0805 | limitadora del LED |
| R21, R22 | 10 k | 0805 | pull-down de IO2 e IO12 (strapping) |
| J1..J19 | ver `mapa-conectores` | — | — |
| JP1 | puente de 2 pads | TestPoint 2,54 mm | habilita la rama permanente |
| H1..H4 | agujero M3 | MountingHole 3,2 mm | fijación al chasis |

Referencias no usadas en el esquemático: R17 a R20 (saltadas en la numeración).

### Ítems sin valor definido

| Ref | Falta | Propuesta |
|---|---|---|
| F3..F6 | valor | 7,5 a 8 A retardado (T), 5x20 mm, una vez resuelto V4 |
| F2 | valor | depende de si se separa el riel de servos. Con riel compartido: 3 A de mantenimiento |
| Fusible general | valor y ubicación | 15 A si J1 sigue siendo Phoenix; 20-25 A si se pasa a XT60 |
| D3 | número de parte | cualquier rectificador de 1 A / 100 V (1N4007, S1M) |
| D4 | número de parte | zener 16 V / 500 mW |

### Compras adicionales que no están en el BOM de la placa

Parte del diseño aunque se monten fuera de la PCB:

- 8 x capacitor 470 uF / 25 V / 105 °C de bajo ESR (2 por driver BTS7960).
- 4 x capacitor 100 nF cerámico (uno por driver).
- 4 x resistencia 10 k para pull-down de PWM, montadas del lado del driver.
- 8 x resistencia 220 Ω de terminación serie (opcional, en los conectores de señal).
- 1 x portafusible aéreo + fusible general.
- 1 x seta de emergencia NC de 40 mm con retención.
- Punteras tubulares para todos los cables de potencia multifilares.

---

## pendientes-de-verificacion

Puntos surgidos del análisis del esquemático que **quedan por confirmar en el banco**. No son fallas
comprobadas: varios pueden descartarse con una medición de dos minutos, y algunos ya podrían estar
descartados por las pruebas hechas.

| # | Qué revisar | Cómo se confirma o descarta | Sección |
|---|---|---|---|
| V1 | Nivel de R_PWM_4 durante el arranque del ESP32 (U1 con B1 a 3,3 V) | osciloscopio disparado en EN, mirando R_PWM_4 y L_PWM_4, con el driver 4 sin potencia | drivers-bts7960 |
| V2 | Disipación de Q1 (IRF9540N, 117 mΩ) | medir V entre drenador y fuente con corriente conocida, y temperatura con el rover cargado | protecciones-desacople |
| V3 | Margen del riel de servos con el LM2596 compartido con el ESP32 | trabar los 4 servos y mirar el riel y 3V3 con osciloscopio | direccion-swerve-servos |
| V4 | Márgenes de J1 (12 A nominal) y J4..J7 (3-4 A nominal) | tocar/medir temperatura de los conectores tras un minuto con una rueda trabada | mapa-conectores |
| V5 | Fuga del TVS SM6T15A con la batería recién cargada (13,8-14,4 V) | medir corriente o temperatura de D1 con el rover encendido | protecciones-desacople |
| V6 | Pico en el riel conmutado al abrir la seta a plena marcha | osciloscopio en el bus +12 V conmutado durante el corte | protecciones-desacople |
| V7 | Estado de los contactos de K1 tras varios ciclos de encendido | continuidad entre nodo A y riel conmutado con la seta apretada (debe dar abierto) | protecciones-desacople |
| V8 | Saturación del sensor por encima de 15,5 A | cargar la tracción y ver dónde se aplana la lectura | sensado-corriente |
| V9 | Tensión real en VOC y utilidad del pin OC | medir VOC con el multímetro y comparar con el cálculo (3,0 V esperados) | sensado-corriente |
| V10 | Temperatura del AMS1117 del devkit con el LM2596 en 7,2 V | medirla con todo el 3,3 V conectado y Wi-Fi activo | arquitectura-electrica |
| V11 | Signo de la lectura de corriente (IN- a IN+) | carga conocida y ver si Vout sube o baja desde 1,65 V | sensado-corriente |
| V12 | Lectura de V_bat con la seta apretada (el divisor está post-relé) | apretar la seta y leer el ADC | arquitectura-electrica |

Fuentes de error posibles en este análisis, que conviene tener presentes al revisar cada punto:

- U1, U4 y U5 usan el símbolo `RF_Switch:MASWSS0136` renombrado a NLAS4157. La numeración de pines
  del símbolo coincide con la del NLAS4157 en SC-88A, pero **hay que verificar que la huella
  `SOT-363_SC-70-6` esté con la misma numeración**. Si no lo estuviera, todo el razonamiento sobre
  B0/B1 cambia.
- La asignación de pines de K1 (`Relay:RAYEX-L90S`) se dedujo resolviendo el espejado del símbolo en
  el esquemático. Conviene confirmar con el multímetro cuál par es la bobina y cuál el contacto NO.
- Los valores nominales de corriente de los conectores son de catálogo genérico; el fabricante real
  que se haya comprado puede diferir.
