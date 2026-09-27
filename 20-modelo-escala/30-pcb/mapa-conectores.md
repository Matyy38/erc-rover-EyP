---
titulo: Mapa de conectores de la PCB del modelo
estado: vigente
fecha: 2026-09-16
fuente: pcb-modelo-escala.md, sección mapa-conectores
---

# Mapa de conectores

Tabla completa de los 19 conectores de la placa, con huella, pinout, corriente nominal y corriente
esperada. Las columnas de pin nombran GPIO del ESP32 donde corresponde; la asignación completa y
su justificación están en [40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md), que es la fuente
única de verdad del pinout.

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

El punto V4 está resumido en [pendientes-de-verificacion.md](pendientes-de-verificacion.md); los valores de fusible sin definir, en [bom.md](bom.md).
