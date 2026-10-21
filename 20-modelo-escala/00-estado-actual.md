---
titulo: Modelo a escala — estado actual de la placa
estado: vigente
fecha: 2026-10-21
fuente: pcb-modelo-escala.md (encabezado, estado de validación y cambios respecto del PDF); reunión de laboratorio del 8-oct-2026
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

La placa ya tuvo pruebas parciales con resultado correcto. Al 8-oct: los cuatro canales de los
puentes H funcionan, un encoder validado y el MPU6050 probado en banco. La electrónica todavía no
está montada en el modelo (el rover lo tiene Mecánica).

| Bloque | Estado | Detalle |
|---|---|---|
| Arquitectura eléctrica y rieles | **dos LM2596 desde el 8-oct** (5 V lógica, 7,2 V servos), sin medir | [20-electronica/arquitectura-electrica.md](20-electronica/arquitectura-electrica.md) |
| Encoders y PCNT | **un canal validado**, faltan tres (se prueban con PCNT directo) | [20-electronica/encoders-pcnt.md](20-electronica/encoders-pcnt.md) |
| Drivers BTS7960 | **los 4 canales de la placa validados** (8-oct). Un módulo BTS7960 falló: a verificar | [20-electronica/drivers-bts7960.md](20-electronica/drivers-bts7960.md) |
| Dirección swerve y servos | riel propio desde el 8-oct (V3 mitigado, sin verificar). Servos sin ensayar | [20-electronica/direccion-swerve-servos.md](20-electronica/direccion-swerve-servos.md) |
| Sensado de corriente | TMCS confirmado B4, sin calibrar. ADS1115 fijo a la placa y ACS712 cableado (8-oct), sin probar | [20-electronica/sensado-corriente.md](20-electronica/sensado-corriente.md) |
| MPU6050 | **funciona** en una prueba de banco (8-oct), todavía no en la placa | [40-firmware/test-mpu6050/](40-firmware/test-mpu6050/test-mpu6050.ino) |
| Protecciones y desacople | montado, Q1 puenteado, cuatro puntos abiertos | [20-electronica/protecciones-desacople.md](20-electronica/protecciones-desacople.md) |
| Conectores de potencia | J1 y J4..J7 reemplazados por cable soldado (8-oct; J4..J7 a terminar) | [30-pcb/mapa-conectores.md](30-pcb/mapa-conectores.md) |
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

**A verificar:** el equipo reporta que el riel de servos cae con la seta, pero si el segundo LM2596
toma la entrada del mismo punto que U2, está aguas arriba del relé y no debería caer. Ver
[20-electronica/arquitectura-electrica.md](20-electronica/arquitectura-electrica.md).

Los motivos de cada descarte están en
[00-contexto/04-decisiones-descartadas.md](../00-contexto/04-decisiones-descartadas.md).

---

## Los puntos V1 a V12

A lo largo de la documentación hay referencias del tipo (V1), (V2)... Son puntos de análisis de
escritorio que **quedan pendientes de confirmar midiendo, no defectos verificados**. Varios pueden
descartarse con una medición de dos minutos y algunos ya podrían estar descartados por las pruebas
hechas.

Están resumidos, con su método de confirmación y su sección de discusión, en
[30-pcb/pendientes-de-verificacion.md](30-pcb/pendientes-de-verificacion.md).

**Cualquier propuesta de cambio sobre esta placa tiene que decir si es un corte y un puente, un
componente a reemplazar o un rediseño.** La placa ya está fabricada y soldada: el costo de la
corrección importa tanto como su corrección técnica.
