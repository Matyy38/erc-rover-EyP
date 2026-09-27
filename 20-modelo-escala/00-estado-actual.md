---
titulo: Modelo a escala — estado actual de la placa
estado: vigente
fecha: 2026-09-16
fuente: pcb-modelo-escala.md (encabezado, estado de validación y cambios respecto del PDF)
---

# PCB del modelo a escala (PDB + control) — estado actual

Documentación derivada del esquemático de KiCad, que es la fuente de verdad. Donde el PDF de
planificación del 17-ago-2026 dice otra cosa, manda el `.sch` y se aclara la diferencia.

- Fuente primaria: `PCB_modelo.kicad_sch` (KiCad 9.0, hoja A3).
- Fuente secundaria: `Modelo_a_escala_ROVER.pdf` (17-ago-2026) — **obsoleto donde contradiga al
  `.sch`**.
- Los archivos de KiCad no viven en el repo: se procesan en una conversación y el resultado se
  asienta en markdown.

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
| Sensado de corriente | montado, sin calibrar | [20-electronica/sensado-corriente.md](20-electronica/sensado-corriente.md) |
| Protecciones y desacople | montado, con cinco puntos abiertos | [20-electronica/protecciones-desacople.md](20-electronica/protecciones-desacople.md) |
| Mapa de GPIO y arranque seguro | documentado, V1 abierto | [40-firmware/mapa-gpio.md](40-firmware/mapa-gpio.md) |

---

## Cambios ya confirmados respecto del PDF

- El **ACS712-30A quedó cancelado**. Se reemplaza por un **TMCS1126B4** midiendo **corriente total
  de tracción**, no por rama.
- **No se usa el sensado de corriente de los módulos BTS7960** (los pines `IS` quedan sin cablear).
- El **pinout del ESP32 cambió** respecto de la tabla del PDF (ver
  [40-firmware/mapa-gpio.md](40-firmware/mapa-gpio.md)).
- Los **pull-down de 10 k en las 8 líneas PWM no existen** en la placa. Se reemplazaron por una
  estrategia distinta con conmutadores analógicos NLAS4157.
- Los **4 PTC individuales de servo** se redujeron a **un solo PTC** para todo el riel.
- El MOSFET de polaridad inversa es **IRF9540N**, no IRF4905.

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
