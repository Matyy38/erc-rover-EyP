---
titulo: Requisitos de motor de tracción — rover real
estado: en revision
fecha: 2026-09-15
fuente: calculos manuales del equipo de mecanica (2 hojas) + informe mecanico jul-2026
alcance: subsistema de potencia y electronica
---

# Requisitos de motor de tracción — rover real

## TL;DR

Por rueda, **a la salida de la caja reductora**: torque nominal 11–17 N·m, pico ≥ 18 N·m, velocidad ≥ 68 RPM, potencia eléctrica 83–120 W. Bus objetivo 24 V. Hay dos temas abiertos con mecánica (diámetro de rueda y conflicto μ/pendiente) que pueden mover estos números ~20%.

## Requisitos vigentes (hoja 2 de mecánica)

| Magnitud | Requisito | Origen |
|---|---|---|
| Velocidad angular nominal | ≥ 68,21 RPM | 1 m/s con D = 0,28 m |
| Torque nominal | ≥ 11–17 N·m | trepado de obstáculo |
| Torque máximo | ≥ 18 N·m | margen sobre nominal |
| Potencia mecánica | 78,57 W | T·ω con T = 11 N·m |
| Potencia eléctrica | 83–120 W | según η asumido (0,95 a 0,65) |

Son valores **por rueda**, en el eje de salida. Ningún motor DC o BLDC de catálogo entrega 11 N·m directo: se compra motorreductor o motor + reductora (relación típica 1/50 a 1/100).

## Cálculos de respaldo (hoja 1 de mecánica)

- Tracción máxima disponible: F = μ·M·g = 0,4 · 40 kg · 9,81 m/s² = 156,96 N (repartida en 4 ruedas).
- Torque de deslizamiento con R = 0,175 m: 6,867 N·m por rueda. **No es requisito**: es el techo útil en plano, más torque que eso solo hace patinar la rueda.
- Torque para trepar obstáculo: ~11,4 N·m (calculado sin bielas ni suspensión, probablemente algo menor en la práctica).
- ω máxima: 5,7 rad/s para 1 m/s con R = 0,175 m.

## Discrepancias detectadas (pendiente de cerrar)

| Fuente | Diámetro de rueda | Impacto |
|---|---|---|
| Hoja 1 de cálculos | 0,35 m | ω y T calculados con R = 0,175 |
| Hoja 2 de cálculos | 0,28 m | ω y T calculados con R = 0,14 |
| Informe mecánico jul-2026 | 0,30 m | valor declarado como definitivo |

El torque escala lineal con R y la velocidad angular inversamente: entre 0,28 y 0,35 m hay ~20% de diferencia en ambos requisitos. **Todos los cálculos del subsistema de potencia deben rehacerse con el diámetro final una vez confirmado.**

## Conflicto de torque μ / pendiente (archivado, abierto)

No deslizar en pendiente exige μ ≥ tan(θ). Para 30° hace falta μ ≥ 0,577, pero el informe declara μ = 0,4, que topea la pendiente trepable en arctan(0,4) ≈ 21,8°.

Es un conflicto estructural, no un error de cuenta: **más torque de motor no lo resuelve**. Preguntas archivadas para mecánica:

1. ¿Cuál es el μ real medido o asumido para el terreno de la competencia?
2. ¿Los 30° son una pendiente sostenida o un obstáculo puntual (donde se supera con inercia y el criterio de no-deslizamiento no aplica igual)?

Hasta que se responda, el requisito de 11–17 N·m se toma como válido, pero el límite sin deslizar del informe (5,214 N·m) queda inconsistente con él.

## Implicancias eléctricas

- 120 W por motor × 4 motores ≈ 480 W de tracción sostenida.
- A 24 V: ~5 A nominales por motor. Con stall de 3–5× la nominal, los picos transitorios del bus pueden llegar a 60–100 A si arrancan los cuatro juntos.
- El dato que define el driver **no** es la corriente nominal sino la corriente de stall a la tensión de bus. Pedirlo siempre al fabricante.
- Cableado estimado a 24 V: AWG 12–14 siliconado por rama, troncal más grueso. Los fusibles se dimensionan para proteger el cable, no el motor.

## Estado de candidatos

| Candidato | Estado | Motivo |
|---|---|---|
| 42RBL04A (Changzhou Smart Automation) | descartado | el proveedor no respondió ningún mensaje; nunca se pudo confirmar la interfaz del controlador integrado |
| Reducción 1/68 | criterio vigente | ~9,93 N·m continuos, motor al 71% del torque nominal (margen térmico real) |
| Reducción 1/49 | descartada | 98% del torque nominal en continuo, sin margen térmico |

Presupuesto objetivo: < 100 USD por motor, bus 24 V, brushed DC con reductora o BLDC con reductora (sin definir).

## Próximos pasos

1. Confirmar con mecánica el diámetro de rueda definitivo y rehacer todos los cálculos de torque del subsistema.
2. Elevar a mecánica las dos preguntas de μ y pendiente.
3. Buscar candidato de motor de reemplazo, priorizando proveedores con respuesta comprobada y hoja de datos que declare corriente de stall.
