---
titulo: Contexto maestro — Rover ERC, subsistema de Electrónica y Potencia
owner: Matías Bélico
version: 2.0
estado: vigente
fecha: 2026-09-16
fuente: 00-contexto-maestro.md (fuente original en 90-archivo/fuentes-originales/), reducido a índice de estado
alcance: subsistema de potencia y electrónica
proposito: documento raíz del repo. Estado, conflictos abiertos y próximos pasos. El detalle técnico vive en los archivos que este índice referencia.
---

# Contexto maestro — Rover ERC

Este es el documento raíz. **No tiene datos técnicos propios, salvo los próximos pasos (§7).** Si un
número aparece acá y en un archivo de detalle, manda el detalle. Acá está el estado, lo que todavía no
cierra y dónde buscar.

---

## 1. Qué es el proyecto

**Competencia:** European Rover Challenge (ERC).
**Equipo:** IEEE ITBA I+D.
**Subsistema:** Electrónica y Potencia.

Dos frentes en paralelo:

| Frente | Propósito | Estado |
|---|---|---|
| **Rover final** | El vehículo que compite. 40 kg, bus de 24 V, swerve de 4 ruedas. | Arquitectura mecánica cerrada. Sin motor elegido. |
| **Modelo a escala** | Instrumento de medición, no maqueta. Mide torque real en piso duro y arena y extrapola al rover final. | PCB fabricada y soldada, con validación parcial. Sin ensayos. |

El modelo existe para cerrar la especificación de motores sin comprar motores de tamaño real a
ciegas. Esa es su única razón de ser, y todo lo que se decide sobre él se juzga contra eso.

---

## 2. Marco

| Tema | Resumen | Detalle |
|---|---|---|
| Alcance | Batería, potencia, control de motores, protecciones, sensado, firmware y telemetría. Mecánica entra solo como interfaz; λ entra como dato | [01-alcance-y-fronteras.md](01-alcance-y-fronteras.md) |
| Reglas no negociables | COTS primero, star grounding, silicona AWG, conectores con traba, E-Stop y fusibles | [02-filosofia-de-diseno.md](02-filosofia-de-diseno.md) |
| Descartados | Qué se evaluó y no quedó, con el motivo | [04-decisiones-descartadas.md](04-decisiones-descartadas.md) |
| Principios transversales | Trece reglas ganadas que aplican más allá de su caso | [05-principios-transversales.md](05-principios-transversales.md) |

---

## 3. Conflictos abiertos

Tres puntos bloquean decisiones hoy. Ninguno se resuelve escribiendo: se resuelven midiendo o
preguntando.

> **Resuelto (16-sep): qué mide el TMCS1126.** Mide **solo la rama de tracción**. La afirmación de
> que medía el total de la PDB quedó superada. El Ensayo D es válido con el sensor de a bordo.

| # | Conflicto | Qué bloquea | Cómo se cierra |
|---|---|---|---|
| 1 | Diámetro de rueda del rover real: circulan tres valores | Cálculos de torque y reimpresión de ruedas del modelo | Consulta a Mecánica |
| 2 | μ / pendiente: el μ declarado no alcanza para 30° | Elección del motor grande | Ensayo de ángulo de deslizamiento |
| 3 | Torque pedido inconsistente con el techo de tracción | Dimensionamiento continuo contra transitorio | Ya tiene lectura propuesta |

Desarrollo y consulta consolidada a Mecánica en
[03-conflictos-abiertos.md](03-conflictos-abiertos.md).

---

## 4. Rover final — estado

| Tema | Estado | Detalle |
|---|---|---|
| Parámetros y requisitos de motor | Vigentes pero sujetos a los conflictos 1 a 3 | [requisitos-motor.md](../10-rover-final/10-locomocion/requisitos-motor.md) |
| Motor | **Sin candidato.** El último quedó descartado | [candidatos-motores.md](../10-rover-final/10-locomocion/candidatos-motores.md) |
| Presupuesto de corriente | Tracción calculada. Falta brazo, cómputo, comunicaciones y ciencia | [presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md) |
| Cableado y protecciones | Preliminar, bloqueado por la elección del motor | [cableado-y-protecciones.md](../10-rover-final/20-potencia/cableado-y-protecciones.md) |

---

## 5. Modelo a escala — estado

| Tema | Estado | Detalle |
|---|---|---|
| Escala, motor y qué valida el modelo | λ recalculado por la masa real. Ruedas a reimprimir cuando responda Mecánica | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) |
| PCB | Fabricada y soldada. Validación parcial correcta. Testeo completo en curso | [00-estado-actual.md](../20-modelo-escala/00-estado-actual.md) |
| Puntos V1 a V12 | Doce hipótesis de escritorio pendientes de medir, no defectos. Prioridad: V1, V2, V3 | [pendientes-de-verificacion.md](../20-modelo-escala/30-pcb/pendientes-de-verificacion.md) |
| Pinout | Fuente única de verdad | [mapa-gpio.md](../20-modelo-escala/40-firmware/mapa-gpio.md) |

---

## 6. Instrumentación de ensayos — decisiones cerradas

- **ADS1115** para leer el TMCS1126: el conversor interno del ESP32 es el eslabón débil.
- **Tiro en barra como método primario**, corriente como canal redundante, en simultáneo.
- **Sin sensor por motor:** el error de ganancia se cancela con la misma cadena de medición.
- **Instrumento externo:** mejora opcional, ya no corrección obligatoria.

Detalle en [instrumentacion.md](../20-modelo-escala/50-ensayos/instrumentacion.md) y
[sensado-corriente.md](../20-modelo-escala/20-electronica/sensado-corriente.md).

---

## 7. Próximos pasos

En orden de valor por esfuerzo.

| # | Acción | Costo | Desbloquea | Bloqueado por |
|---|---|---|---|---|
| 1 | **Ensayo de ángulo de deslizamiento** en piso duro | 0 | El μ real, que hoy bloquea la elección del motor grande | nada |
| 2 | **Consulta única a Mecánica**: diámetro, μ, naturaleza de los 30° | 1 mensaje | λ, requisitos de torque, reimpresión de ruedas | conviene hacer #1 antes |
| 3 | **Pesar el modelo completo** con todo montado | 0 | λ definitivo | nada |
| 4 | **Ensayo A en banco**, fuera de la PDB, un motor, continua, multímetro en serie | 0 | K y calibración del sensor en la misma sesión | nada |
| 5 | Cerrar V1, V2, V3 con osciloscopio | 0 | Seguridad de la placa | nada |
| 6 | Verificar la sensibilidad real del TMCS montado con multímetro | 0 | Fondo de escala del sensor | nada |
| 7 | Comprar ADS1115 | 3 USD | Cadena de medición | nada |
| 8 | **Reimprimir ruedas** al diámetro correcto, y medirlas con calibre | — | Cierra la escala | **#2 y #3** |
| 9 | Medir altura del CM y comparar con 0,3464·λ | 0 | Validez de conclusiones de estabilidad | #8 |
| 10 | Decidir si hace falta instrumento externo | 0 a 20 USD | Ensayo D | #4 |
| 11 | Comprar balanza de gancho | 12 USD | Tiro en barra | nada |
| 12 | Buscar motor de reemplazo del 42RBL04A | — | Rover final | #1 y #2 |
| 13 | Presupuesto de corriente de los otros subsistemas | — | Batería y PDB del rover final | otros subsistemas |

Los siete primeros no dependen de nadie más y cuestan entre cero y tres dólares. Los pasos 8 y 12,
que son los que cuestan plata o tiempo de impresión, dependen de que Mecánica responda.

Plan de ensayos del modelo en
[plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md).

---

## 8. Mapa del repositorio

| Carpeta | Qué contiene |
|---|---|
| `00-contexto/` | Este documento, alcance, filosofía, conflictos, descartados, principios |
| `10-rover-final/` | Requisitos de motor, candidatos, presupuesto de corriente, cableado |
| `20-modelo-escala/` | Escala, electrónica, PCB, firmware, ensayos |
| `30-componentes/` | Ficha por componente y enlaces a hojas de datos |
| `40-proveedores-costos/` | Proveedores y costos de importación |
| `90-archivo/` | Versiones superadas y ramas muertas. **No usar como fuente** |

Índice archivo por archivo y convenciones en el [README del repo](../README.md).
