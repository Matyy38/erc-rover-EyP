---
titulo: Modelo a escala — estado actual de la placa
estado: vigente
fecha: 2026-11-15
fuente: pcb-modelo-escala.md (encabezado, estado de validación y cambios respecto del PDF); reuniones de laboratorio del 8-oct y del 23-oct-2026; avances reportados al 15-nov-2026
---

# PCB del modelo a escala (PDB + control) — estado actual

Documentación derivada del esquemático de KiCad, que es la fuente de verdad. Donde el PDF de
planificación del 17-ago-2026 dice otra cosa, manda el `.sch` y se aclara la diferencia.

- Fuente primaria: `PCB_modelo.kicad_sch` (KiCad 9.0, hoja A3).
- Fuente secundaria: `Modelo_a_escala_ROVER.pdf` (17-ago-2026) — **obsoleto donde contradiga al
  `.sch`**.
- Los archivos de KiCad están en [`pcb_V1/`](../pcb_V1/) para descargar. Lo que salió de analizarlos
  está en estos markdown.

---

## Estado de validación

Al 15-nov-2026: los cuatro motores andan con realimentación de posición por PCNT, el MPU6050 anda
en la placa, el Schottky resolvió la retroalimentación desde el USB, la seta corta, F3..F6 de 5 A
están colocados y todos los cables están terminados. La batería está comprada y el posicionamiento
de la electrónica en el modelo ya está definido. **Abierto:** servos a probar de nuevo con F2
reemplazado (V13), TMCS sin probar y ACS712 quemado, a reemplazar.

| Bloque | Estado | Detalle |
|---|---|---|
| Arquitectura eléctrica y rieles | dos LM2596 desde el 8-oct (5 V lógica, 7,2 V servos). **Schottky entre U2 y VIN colocado y funcionando (al 15-nov)**: resuelta la retroalimentación desde el USB. Batería comprada: Hemmel BSL-012-007, AGM 12 V / 7 Ah. Rieles sin medir con osciloscopio | [20-electronica/arquitectura-electrica.md](20-electronica/arquitectura-electrica.md) |
| Encoders y PCNT | **los 4 validados (al 15-nov)**: motores andando con realimentación de posición por PCNT | [20-electronica/encoders-pcnt.md](20-electronica/encoders-pcnt.md) |
| Drivers BTS7960 | **los 4 canales de la placa validados** (8-oct). El módulo que había fallado quedó en condiciones (al 15-nov) | [20-electronica/drivers-bts7960.md](20-electronica/drivers-bts7960.md) |
| Dirección swerve y servos | riel propio desde el 8-oct (V3 mitigado, sin verificar). **A probar de nuevo:** F2 es un X50 (~0,5 A), mal dimensionado; se reemplaza por un PTC de 3 A (V13) | [20-electronica/direccion-swerve-servos.md](20-electronica/direccion-swerve-servos.md) |
| Sensado de corriente | TMCS (B4) cableado a AIN0 del ADS1115 y a IO33, sin probar ni calibrar. **ACS712 quemado (al 15-nov)**: se evalúa reemplazarlo, a consultar | [20-electronica/sensado-corriente.md](20-electronica/sensado-corriente.md) |
| MPU6050 | **andando en la placa (al 15-nov)** | [40-firmware/mpu6050_simple/](40-firmware/mpu6050_simple/mpu6050_simple.ino) |
| Protecciones y desacople | montado, Q1 puenteado, cuatro puntos abiertos | [20-electronica/protecciones-desacople.md](20-electronica/protecciones-desacople.md) |
| Conectores de potencia | J1 y J4..J7 reemplazados por cable soldado; batería por XT60 con fusible. **Todos los cables terminados (al 15-nov)**, salvo el del ACS712. F3..F6 de 5 A colocados. La seta se probó y corta (V7) | [30-pcb/mapa-conectores.md](30-pcb/mapa-conectores.md) |
| Mapa de GPIO y arranque seguro | documentado, V1 mitigado con pull-up y sin verificar | [40-firmware/mapa-gpio.md](40-firmware/mapa-gpio.md) |

---

## Cambios ya confirmados respecto del PDF

- El **ACS712-30A quedó cancelado como sensor de placa** (vuelve como respaldo a bordo desde el
  27-sep, ver abajo). Se reemplaza por un **TMCS1126B4** midiendo **corriente total
  de tracción**, no por rama.
- **No se usa el sensado de corriente de los módulos BTS7960** (los pines `IS` quedan sin cablear).
- El **pinout del ESP32 cambió** respecto de la tabla del PDF (ver
  [40-firmware/mapa-gpio.md](40-firmware/mapa-gpio.md)).
- Los **pull-down de 10 k en las 8 líneas PWM no existen** en la placa. Se reemplazaron por una
  estrategia distinta con conmutadores analógicos NLAS4157.
- Los **4 PTC individuales de servo** se redujeron a **un solo PTC** para todo el riel.
- El MOSFET de polaridad inversa es **IRF9540N**, no IRF4905. **Desde el 27-sep está puenteado**: la
  polaridad se protege con un conector de un solo sentido.

## Cambios del 27-sep-2026, posteriores al `.sch`

- **Pull-up en L_PWM_4** (V1): componente agregado. Falta verificar con osciloscopio.
- **Q1 puenteado** (V2 no aplica).
- **ACS712-30A vuelve**, no como sensor de placa sino como segundo sensor a bordo en la pata de motor
  de una rueda delantera, leído por un **ADS1115** en el bus I2C junto al TMCS.
- **Ruedas del modelo: 150 mm de diámetro**, no 102 mm. Abre la decisión de escala en
  [10-metodologia-similitud.md](10-metodologia-similitud.md) §0.

## Cambios del 8-oct-2026 (reunión de laboratorio)

| Cambio | Tipo | Estado |
|---|---|---|
| Segundo LM2596 en 7,2 V solo para los servos; U2 a 5 V solo para el ESP32 (V3 mitigado; V10 no aplica) | corte y puente + módulo | hecho |
| J1 (batería) reemplazado por cable de silicona soldado | conector sacado | hecho |
| J4..J7 (alimentación de drivers) reemplazados por cable soldado; puntera del lado del driver (V4 no aplica) | conector sacado | a terminar |
| ADS1115 fijo a la placa, cables soldados (sin mazo en Y por J3) | componente agregado | hecho, sin probar |
| ACS712 cableado | agregado fuera de placa | hecho, sin probar |
| TMCS1126 confirmado como B4 por la marca | verificación | cerrado |

El segundo LM2596 toma la entrada del nodo A, igual que U2: con la seta apretada, servos y ESP32
siguen alimentados. Ver [20-electronica/arquitectura-electrica.md](20-electronica/arquitectura-electrica.md).

## Reunión del 23-oct-2026

**Hecho:** mazos de señal y potencia con conectores con traba, casi completos. Cable de ~1,5 mm en
la mayoría de los tramos; la entrada de batería, con XT60 y fusible, en un cable más grueso. Ver
[30-pcb/mapa-conectores.md](30-pcb/mapa-conectores.md).

**Sin hacer, por falta de tiempo:** módulo BTS7960 que falló, medición de rieles, seta, V1,
sensores de corriente y PCNT.

**Bloqueado por Mecánica:** cables a los motores y la correspondencia driver ↔ rueda ↔ servo ↔
encoder. Los motores no están todos disponibles.

**Problema encontrado:** al conectar el USB del ESP32 con la batería desconectada, los LM2596
calientan mucho. Se cree que calentaron los dos, pero no está confirmado. El ESP32 sobrevivió y no
se sabe si los módulos quedaron dañados. Causa probable y corrección en
[20-electronica/arquitectura-electrica.md](20-electronica/arquitectura-electrica.md).

| Cambio | Tipo | Estado |
|---|---|---|
| Schottky en serie entre la salida de U2 y VIN del ESP32 | corte y puente + diodo | planificado |
| U2 de 5 V a ~5,4 V, para compensar la caída del diodo | ajuste | planificado |
| LM2596 de repuesto | compra | a comprar |

Los motivos de cada descarte están en
[00-contexto/04-decisiones-descartadas.md](../00-contexto/04-decisiones-descartadas.md).

---

## Al 15-nov-2026

Avances reportados después de la reunión del 23-oct (labos del 29-oct al 13-nov):

| Tema | Resultado | Tipo |
|---|---|---|
| Schottky entre U2 y VIN | colocado; todo anda bien. Cierra la retroalimentación desde el USB | corte y puente + diodo, hecho |
| Motores y encoders | los 4 andan con realimentación de posición por PCNT | validación |
| Módulo BTS7960 que había fallado | en condiciones | reparado |
| MPU6050 | anda en la placa | validación |
| Seta | probada: corta el bus de drivers (V7) | validación |
| F3..F6 | 5 A colocados; tipo a asentar (V14) | fusibles |
| Cables | todos terminados, salvo el del ACS712 | mazos |
| Batería | comprada: Hemmel BSL-012-007, plomo AGM 12 V / 7 Ah | compra |
| Posicionamiento de la electrónica en el modelo | definido | mecánica de montaje |
| Servos | F2 es un X50 (~0,5 A), mal dimensionado: probable causa de V13. A reemplazar por PTC de 3 A y volver a probar | componente a reemplazar |
| TMCS1126 | cableado a AIN0 del ADS1115 y a IO33; falta probarlo | sin probar |
| ACS712 | **se quemó** por un error en el laboratorio. Se evalúa cambiarlo por otro sensor: **queda para consultar** | a decidir |

Sin novedades: medición de rieles con osciloscopio, V1 y revisión de los LM2596 en vacío y con carga.

## Los puntos V1 a V14

A lo largo de la documentación hay referencias del tipo (V1), (V2)... Son puntos de análisis de
escritorio que **quedan pendientes de confirmar midiendo, no defectos verificados**. Varios pueden
descartarse con una medición de dos minutos y algunos ya podrían estar descartados por las pruebas
hechas.

Están resumidos, con su método de confirmación y su sección de discusión, en
[30-pcb/pendientes-de-verificacion.md](30-pcb/pendientes-de-verificacion.md).

**Cualquier propuesta de cambio sobre esta placa tiene que decir si es un corte y un puente, un
componente a reemplazar o un rediseño.** La placa ya está fabricada y soldada: el costo de la
corrección importa tanto como su corrección técnica.
