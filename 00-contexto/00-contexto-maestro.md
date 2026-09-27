---
titulo: Contexto maestro — Rover ERC, subsistema de Electrónica y Potencia
owner: Matías Bélico
version: 2.1
estado: vigente
fecha: 2026-09-27
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
| Escala, motor y qué valida el modelo | **Abierta:** las ruedas miden 150 mm y no corresponden a ~5,5 kg. Decidir reimprimir (recomendado) o lastrar | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) §0 |
| PCB | Fabricada y soldada. Validación parcial correcta. Q1 puenteado, pull-up en L_PWM_4. Testeo completo en curso | [00-estado-actual.md](../20-modelo-escala/00-estado-actual.md) |
| Puntos V1 a V12 | Hipótesis de escritorio pendientes de medir, no defectos. V1 mitigado sin verificar, V2 no aplica, V3 en pausa | [pendientes-de-verificacion.md](../20-modelo-escala/30-pcb/pendientes-de-verificacion.md) |
| Ensayos | Plan rev. 2.0 (reemplaza al protocolo rev. 1.0). Ninguno corrido | [plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md) |
| Pinout | Fuente única de verdad | [mapa-gpio.md](../20-modelo-escala/40-firmware/mapa-gpio.md) |

---

## 6. Instrumentación de ensayos — decisiones cerradas

- **ADS1115** para leer el TMCS1126: el conversor interno del ESP32 es el eslabón débil. **Comprado.**
- **ACS712 a bordo en la pata de motor de una rueda delantera**, leído por el mismo ADS1115: respaldo
  y validación. No es un sensor por motor para la extrapolación.
- **Corrección por ciclo de trabajo:** con PWM el TMCS mide corriente de batería (≈ D · I_motor). Se
  registra D por rueda y se corrige.
- **Tiro en barra como método primario**, corriente como canal redundante, en simultáneo. Coinciden
  en piso duro; en arena la diferencia es la resistencia de compactación. Balanza de gancho: la presta
  la facultad.
- **Instrumento externo:** no se construye por ahora.
- **Sin convertidor regulado:** batería de plomo directo al bus, lazo cerrado de velocidad y registro
  de tensión y D.

Detalle en [instrumentacion.md](../20-modelo-escala/50-ensayos/instrumentacion.md) y
[sensado-corriente.md](../20-modelo-escala/20-electronica/sensado-corriente.md).

---

## 7. Próximos pasos

En orden de valor por esfuerzo.

| # | Acción | Costo | Desbloquea | Bloqueado por |
|---|---|---|---|---|
| 1 | **Ensayo 0: ángulo de deslizamiento** en piso duro | 0 | El μ real, que hoy bloquea la elección del motor grande, y el ángulo del Ensayo D | superficie y rampa (Mecánica) |
| 2 | **Consulta única a Mecánica**: diámetro, μ, naturaleza de los 30°, más los pedidos de ensayo | 1 mensaje | λ, requisitos de torque, decisión de ruedas | conviene hacer #1 antes |
| 3 | **Decidir rueda o lastre** (ruedas de 150 mm) | 0 | Escala del modelo, Ensayo D | reunión de líderes, #2 |
| 4 | **Pesar el modelo completo** con todo montado | 0 | λ definitivo | nada |
| 5 | **Cablear ADS1115 y ACS712** y calibrar TMCS y ACS712 contra multímetro | ~2 USD | Cadena de medición; cierra V9, V11 y la sensibilidad del TMCS | nada |
| 6 | Verificar V1 con osciloscopio (pull-up ya soldado) | 0 | Seguridad de la placa | nada |
| 7 | **Ensayo A** en los 4 motores, con ACS712 y multímetro en serie | 0 | K, I₀ y R | #5 |
| 8 | **Ensayo A'** (cadena bajo modulación), B y C | 0 | Corrección por D validada, K verificada, encoders | #7 |
| 9 | **D0**, ensayo de método en piso duro | 0 | Método del Ensayo D depurado | #1, #5, #8 |
| 10 | **Reimprimir ruedas** al diámetro correcto, y medirlas con calibre | — | Cierra la escala | **#2, #3 y #4** |
| 11 | Medir altura del CM y comparar con 0,3464·λ | 0 | Validez de conclusiones de estabilidad | #10 |
| 12 | Buscar motor de reemplazo del 42RBL04A | — | Rover final | #1 y #2 |
| 13 | Presupuesto de corriente de los otros subsistemas | — | Batería y PDB del rover final | otros subsistemas |

Hechos desde la versión anterior: ADS1115 comprado; balanza de gancho conseguida (la presta la
facultad); Q1 puenteado (V2 no aplica); pull-up en L_PWM_4 para V1. Descartado: instrumento externo
por ahora.

Casi todo lo que va de #4 a #9 no depende de nadie más y cuesta menos de tres dólares. Los pasos que
cuestan plata o tiempo de impresión (#10 y #12) dependen de que Mecánica responda.

Plan de ensayos del modelo en
[plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md). Versión para compartir con
Mecánica: `Plan_Ensayos_Modelo_Rover_ERC_rev2.docx` en `Desktop\Rover` (fuera del repo).

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
