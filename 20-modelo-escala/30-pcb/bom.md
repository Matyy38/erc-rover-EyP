---
titulo: BOM de la PCB del modelo
estado: vigente
fecha: 2026-09-16
fuente: pcb-modelo-escala.md, sección bom
---

# BOM

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
| J1..J19 | ver [mapa-conectores.md](mapa-conectores.md) | — | — |
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

Valores de fusible y pendientes de especificación en [especificacion.md](especificacion.md); conectores en [mapa-conectores.md](mapa-conectores.md).
