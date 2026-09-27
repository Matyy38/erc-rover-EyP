---
titulo: Dirección swerve y riel de servos
estado: vigente
fecha: 2026-09-27
fuente: pcb-modelo-escala.md, sección direccion-swerve-servos
---

# Dirección swerve y servos

El modelo a escala pasó a ser **swerve, con un servo de dirección por rueda** (ya no es skid-steer).
Con eso el modelo sí valida la cinemática swerve, que era su laguna principal.

### Interfaz

Cuatro conectores Molex KK-254 de 3 vías, formato servo estándar: J12 (Servo1), J14 (Servo2), J13
(Servo3) y J17 (Servo4). Cada uno lleva GND, riel de servos y señal.

Dos señales salen de un conmutador analógico y no del GPIO directo: **Servo1 vía U4 (IO0)** y
**Servo3 vía U5 (IO15)**, porque IO0 e IO15 son pines de strapping que arrancan en alto. Servo2 (IO2)
y Servo4 (IO12) van directo, con pull-down de 10 k. El razonamiento completo está en
[40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md); el pinout con huella y corriente, en
[30-pcb/mapa-conectores.md](../30-pcb/mapa-conectores.md).

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
osciloscopio en el riel de 5V/7,2V y en 3V3, y ver cuánto cae y si el ESP32 sobrevive. Sumar un shunt
de 0,1 Ω en serie con el riel de servos (el TMCS1126 no lo ve) y repetir con arranque simultáneo
contra escalonado. Ese ensayo convierte esto de hipótesis en dato.

### Mitigaciones

De dónde sale el pico: un servo de hobby consume cerca de su corriente de bloqueo cada vez que la
consigna queda lejos de su posición, porque su control interno satura. Hay dos formas de atacarlo:
que los picos de los cuatro no coincidan (firmware) o que el ESP32 no dependa del nodo que se hunde
(hardware). **Solo el hardware resuelve V3**; el firmware baja la frecuencia del pico pero no cubre
un módulo trabado contra una piedra.

| # | Estrategia | Tipo | Costo sobre la placa | ¿Resuelve V3? |
|---|---|---|---|---|
| 1 | Arranque escalonado | firmware | ninguno | solo en el encendido, que es el peor caso |
| 2 | Rampa sincronizada | firmware | ninguno | baja los picos en marcha |
| 3 | Límite de 90° por módulo | firmware | ninguno | acorta cada pico |
| 4 | Reorientar con tracción detenida | firmware | ninguno | parcial, con contra |
| 5 | Convertidor propio para el ESP32 | hardware | corte y puente + módulo | **sí** |
| 6 | Alimentación separada para los servos | hardware | corte y puente, o solo mazo | **sí** |
| 7 | C1 de 1000 a 4700 uF | hardware | reemplazo de componente | no: ~235 us en vez de 50 us |

**Firmware** (implementar 1, 2 y 3 en cualquier caso):

1. **Arranque escalonado.** En el primer pulso válido el servo no sabe dónde está y va a máxima
   velocidad hasta la consigna. Habilitar los canales LEDC de a uno, con 150 a 200 ms entre cada
   uno. En el arranque la tracción está cortada, así que no hay costo cinemático.
2. **Rampa sincronizada en marcha.** En swerve los módulos tienen que orientarse juntos: desfasar
   los comandos hace que las ruedas se peleen y arrastren. En su lugar, la consigna avanza en rampa
   por debajo de la velocidad máxima del servo, así el error interno se mantiene chico y la
   corriente también. La duración la fija el módulo con más recorrido y cada módulo ajusta su
   velocidad para terminar en ese mismo tiempo. Actualizar a 50 Hz, un paso por período de PWM.
3. **Límite de 90°.** Si el giro pedido supera 90°, girar el complemento e invertir el sentido del
   motor de tracción. Reduce a la mitad el recorrido máximo.
4. **Reorientar con tracción detenida**, de a un módulo, solo para giros grandes (más de 15° a 20°
   de error). Contra: girar una rueda que no rueda pide más torque que girarla en marcha, sobre todo
   en arena. Baja la simultaneidad pero sube el pico de cada servo.

**Temporizador de bloqueo: no implementable tal como estaba planteado.** Cortar el PWM "si el servo
no llegó a destino" requiere saber dónde está, y un servo de hobby no informa su posición. El
TMCS1126 tampoco ve el riel de servos. Alternativas reales:

- Corte por tiempo, a ciegas: no distingue un servo trabado de uno lento.
- Servos con realimentación (por ejemplo, de bus serie, que informan posición y carga): cuesta
  pines y un UART.
- Sensado de corriente en el riel de servos: hoy no queda ningún ADC1 libre.

**Hardware** (se decide con el resultado de V3):

5. **Convertidor propio para el ESP32**, alimentado desde el nodo A. Aísla la carga chica (~250 mA),
   así que el cableado es simple. Los servos pueden seguir hundiendo su riel y perder torque, pero
   el ESP32 sobrevive.
6. **Alimentación separada para los servos**: una 2S LiPo dedicada (7,4 V, ESR bajo del orden de
   25 mΩ) o un convertidor dedicado. Solo se comparte GND, en el punto estrella. Libera además al
   LM2596 para quedar en 5 V, lo que baja la disipación del AMS1117. Se puede hacer **sin tocar la
   placa** con un mazo en Y: señal y GND salen de J12 a J17 y el positivo viene del convertidor
   externo, con conectores con traba. En ese caso F2 deja de proteger ese riel y hace falta un
   fusible propio a la salida del convertidor.
7. Subir C1 a 4700 uF solo ayuda con los flancos de arranque; no sostiene un bloqueo.

Orden sugerido: firmware 1, 2 y 3 ya; ensayo de V3; si el ESP32 se resetea con los cuatro trabados,
opción 6 con mazo en Y.

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

El punto V3 y su medición están resumidos en [30-pcb/pendientes-de-verificacion.md](../30-pcb/pendientes-de-verificacion.md).
