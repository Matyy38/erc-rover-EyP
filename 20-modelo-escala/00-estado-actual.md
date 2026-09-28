---
titulo: Modelo a escala — estado actual de la placa
estado: vigente
fecha: 2026-09-28
fuente: pcb-modelo-escala.md (encabezado, estado de validación y cambios respecto del PDF)
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

La placa ya tuvo pruebas parciales con resultado correcto. Se probó un encoder y un puente H de cada
unidad, y funcionaron. El testeo completo está en curso.

| Bloque | Estado | Detalle |
|---|---|---|
| Arquitectura eléctrica y rieles | documentado, sin medir | [20-electronica/arquitectura-electrica.md](20-electronica/arquitectura-electrica.md) |
| Encoders y PCNT | **un canal validado**, faltan tres | [20-electronica/encoders-pcnt.md](20-electronica/encoders-pcnt.md) |
| Drivers BTS7960 | **un puente H de cada unidad validado** | [20-electronica/drivers-bts7960.md](20-electronica/drivers-bts7960.md) |
| Dirección swerve y servos | sin ensayar (V3 abierto) | [20-electronica/direccion-swerve-servos.md](20-electronica/direccion-swerve-servos.md) |
| Sensado de corriente | TMCS montado, sin calibrar. ADS1115 comprado y ACS712 a sumar en una rueda | [20-electronica/sensado-corriente.md](20-electronica/sensado-corriente.md) |
| Protecciones y desacople | montado, Q1 puenteado, cuatro puntos abiertos | [20-electronica/protecciones-desacople.md](20-electronica/protecciones-desacople.md) |
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
