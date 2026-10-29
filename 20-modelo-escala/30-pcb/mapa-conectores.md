---
titulo: Mapa de conectores de la PCB del modelo
estado: vigente
fecha: 2026-10-26
fuente: pcb-modelo-escala.md, sección mapa-conectores; reuniones de laboratorio del 8-oct y del 23-oct-2026
---

# Mapa de conectores

Tabla completa de los 19 conectores de la placa, con huella, pinout, corriente nominal y corriente
esperada. Las columnas de pin nombran GPIO del ESP32 donde corresponde; la asignación completa y
su justificación están en [40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md), que es la fuente
única de verdad del pinout.

> **Cambios del 8-oct-2026.** J1 (batería) y J4..J7 (alimentación de drivers) **ya no se usan como
> conectores**: el cable de silicona va soldado directo a la placa, para no depender de la corriente
> nominal del conector. Del lado del driver, puntera en la bornera del módulo. El **ADS1115 quedó fijo
> a la placa con cables soldados**, sin pasar por J3. Las filas de la tabla describen las huellas del
> `.sch`.

### Tabla completa

| Ref | Etiqueta | Vías | Huella | Pin 1 | Pin 2 | Pin 3 | Pin 4 | Corriente nominal del conector | Corriente esperada |
|---|---|---|---|---|---|---|---|---|---|
| J1 | Alimentacion | 2 | Phoenix MSTBA 2,5 horizontal, 5,08 mm. **Cable soldado desde el 8-oct** | Batería + (a Q1.D) | GND | — | — | 12 A (ya no aplica) | hasta 22 A en bloqueo total |
| J2 | E-Stop | 2 | Molex KK-254 | del nodo A (post JP1) | a bobina K1.A2 | — | — | 3-4 A | ~150 mA (solo bobina) |
| J3 | MPU6050 (el ADS1115 está fijo a la placa desde el 8-oct, no pasa por J3) | 4 | Molex KK-254 | +3,3 V | GND | SCL | SDA | 3-4 A | pocos mA |
| J4 | 12v_driver_2 | 2 | Molex KK-254. **Cable soldado desde el 8-oct** | +12 V tras F3 | GND | — | — | ya no aplica | 5,5 A de pico |
| J5 | 12v_driver_3 | 2 | Molex KK-254. **Cable soldado desde el 8-oct** | +12 V tras F4 | GND | — | — | ya no aplica | 5,5 A de pico |
| J6 | 12v_driver_1 | 2 | Molex KK-254. **Cable soldado desde el 8-oct** | +12 V tras F5 | GND | — | — | ya no aplica | 5,5 A de pico |
| J7 | 12v_driver_4 | 2 | Molex KK-254. **Cable soldado desde el 8-oct** | +12 V tras F6 | GND | — | — | ya no aplica | 5,5 A de pico |
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

### Cable soldado en J1 y J4..J7: calibre y fusible (8-oct-2026)

Al sacar los conectores, el límite pasa a ser el cable, y **el fusible tiene que proteger al
cable**. Calibre recomendado, en silicona multifilar:

| Tramo | Corriente de peor caso | Calibre | Fusible que lo protege | Caída en el peor caso |
|---|---|---|---|---|
| Batería a placa (ex J1) | 22 A, las 4 ruedas trabadas | **14 AWG**; 16 AWG si no entra en el agujero | general: 20 A retardado con 14 AWG; 15 A con 16 AWG | 14 AWG, 1 m de ida y vuelta: 8,3 mΩ × 22 A ≈ 0,18 V |
| Placa a driver (ex J4..J7) | 5,5 A, una rueda trabada | **18 AWG**; 16 AWG si entra | F3..F6: 7,5 A retardado | 18 AWG, 2 m de ida y vuelta (drivers a 60-100 cm): 21 mΩ/m × 2 m × 5,5 A ≈ 0,23 V |

Criterio: capacidad de cable para cableado de chasis, del orden de 32 A para 14 AWG, 22 A para
16 AWG y 16 A para 18 AWG. Es una tabla conservadora y empírica, no un dato del fabricante: la
silicona aguanta más temperatura que el PVC, así que en la práctica hay más margen. Verificación
por un segundo camino: la caída de tensión queda por debajo del 2 % del bus en los dos tramos, así
que el calibre lo define el fusible y no la caída. Con 16 AWG en la batería y fusible general de
15 A, un bloqueo simultáneo de las 4 ruedas corta el fusible; es una falla, no una condición de
operación (en los ensayos se trabaja a ~2,6 A).

**Lo que limita en la práctica es el agujero de la huella.** Valor típico, a medir con calibre: del
orden de 1,2 mm en KK-254 y de 1,3 a 1,4 mm en el Phoenix de 5,08 mm. El 18 AWG multifilar entra
justo en un KK-254. El 16 y el 14 AWG multifilares no entran enteros. No hay que cortar hilos para
que entre: eso baja la sección que el fusible supone. Si no entra, conviene bajar un calibre y el
fusible con él, según la tabla.

**Lo que se usó (23-oct-2026).** Placa a drivers y la mayoría de los tramos: cable de **~1,5 mm**.
Entrada de batería: un cable más grueso que ya había, con **XT60 y el fusible general**. Falta
asentar si 1,5 mm es el diámetro del conductor, el diámetro exterior con la aislación, o una
sección de 1,5 mm². La diferencia es grande:

| Lectura de "1,5 mm" | Sección | Equivalente | ¿F3..F6 de 7,5 A lo protegen? |
|---|---|---|---|
| Diámetro del conductor | ~1,8 mm² | ~15 AWG | sí, con margen |
| Sección de 1,5 mm² | 1,5 mm² | ~16 AWG | sí, con margen |
| Diámetro exterior con aislación | ~0,3 mm² | ~22 AWG | **no**: ~7 A en cableado de chasis, por debajo del fusible. Habría que rehacer los tramos placa a driver con cable más grueso, porque bajar el fusible a 5 A lo haría cortar en cada bloqueo de 5,5 A |

Se resuelve midiendo con calibre el conductor pelado. Lo mismo para el cable de batería: con 14 AWG
o más, el fusible general va de 20 A retardado.

### Márgenes de los conectores de potencia (V4) — ya no aplica (8-oct-2026)

Se resolvió sacando los conectores (ver arriba). Se conserva el análisis original:

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
| Batería a PDB | recomendado 14 AWG silicona. Usado: cable grueso que ya había (23-oct) | XT60 con fusible del lado de la batería; soldado a la placa |
| PDB a drivers | recomendado 18 AWG silicona. Usado: ~1,5 mm (23-oct), a asentar | soldado a la placa; puntera en la bornera del driver |
| Driver a motor | 16-18 AWG silicona | terminales pala o soldadura |
| PDB a servos | 20-22 AWG | Molex KK / conector servo |
| Señales PWM | 22-24 AWG | Molex KK-254 |
| Encoders e I2C | 22 AWG, par trenzado | Molex KK-254 |
| Seta a PDB | 18-20 AWG | Molex KK-254 (la corriente es solo la de bobina) |

Prohibido protoboard y cables Dupont en cualquier tramo: el desgaste por vibración (*fretting*)
genera contactos intermitentes imposibles de diagnosticar después.

**Estado al 23-oct-2026:** los mazos de señal y potencia con conectores con traba están casi
terminados. Faltan los largos definitivos, las punteras y los cables a los motores, porque los
motores los tiene Mecánica. La correspondencia driver ↔ rueda ↔ servo ↔ encoder se fija y se
rotula cuando estén los motores.

---

El punto V4 está resumido en [pendientes-de-verificacion.md](pendientes-de-verificacion.md); los valores de fusible sin definir, en [bom.md](bom.md).
