---
titulo: Requisitos de motor de tracción — rover real
estado: en revision
fecha: 2026-09-16
fuente: requisitos-motor-rover-real.md (cálculos manuales del equipo de mecánica, 2 hojas, + informe mecánico jul-2026)
---

# Requisitos de motor de tracción — rover real

## TL;DR

Por rueda, **a la salida de la caja reductora**: torque nominal 11–17 N·m, pico ≥ 18 N·m, velocidad
≥ 68 RPM, potencia eléctrica 83–120 W. Bus objetivo 24 V. Hay dos temas abiertos con mecánica
(diámetro de rueda y conflicto μ/pendiente) que pueden mover estos números ~20%.

## Parámetros del rover final

| Parámetro | Valor |
|---|---|
| Tracción | Swerve de 4 ruedas. Por rueda: un NEMA 23 para la dirección y un motor de tracción a definir (el que especifica este documento) |
| Masa | 40 kg |
| Chasis | 0,8 × 0,4 m |
| Diámetro de rueda | **en conflicto** (ver abajo) |
| Pendiente máxima | 30° declarados, **en conflicto** con μ |
| μ | 0,4 declarado, pendiente de medición |
| Velocidad de rueda | ~63,7 rpm (~1 m/s) con R = 0,15 |
| Bus | 24 V DC |
| Presupuesto por motor | < 100 USD |

6WD y rocker-bogie descartados definitivamente.

## Requisitos vigentes (hoja 2 de mecánica)

| Magnitud | Requisito | Origen |
|---|---|---|
| Velocidad angular nominal | ≥ 68,21 RPM | 1 m/s con D = 0,28 m |
| Torque nominal | ≥ 11–17 N·m | trepado de obstáculo |
| Torque máximo | ≥ 18 N·m | margen sobre nominal |
| Potencia mecánica | 78,57 W | T·ω con T = 11 N·m |
| Potencia eléctrica | 83–120 W | según η asumido (0,95 a 0,65) |

Son valores **por rueda**, en el eje de salida. Ningún motor DC o BLDC de catálogo entrega 11 N·m
directo: se compra motorreductor o motor + reductora (relación típica 1/50 a 1/100).

## Cálculos de respaldo (hoja 1 de mecánica)

- Tracción máxima disponible: F = μ·M·g = 0,4 · 40 kg · 9,81 m/s² = 156,96 N (repartida en 4 ruedas).
- Torque de deslizamiento con R = 0,175 m: 6,867 N·m por rueda. **No es requisito**: es el techo útil
  en plano, más torque que eso solo hace patinar la rueda.
- Torque para trepar obstáculo: ~11,4 N·m (calculado sin bielas ni suspensión, probablemente algo
  menor en la práctica).
- ω máxima: 5,7 rad/s para 1 m/s con R = 0,175 m.

## Discrepancias detectadas (pendiente de cerrar)

Las hojas de Mecánica y el informe usan tres diámetros de rueda distintos. La tabla y su impacto están
consolidados en [00-contexto/03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md)
(conflicto 1): mueven ambos requisitos ~20%. **Todos los cálculos del subsistema de potencia deben rehacerse con
el diámetro final una vez confirmado.**

## Conflicto de torque μ / pendiente

No deslizar en pendiente exige μ ≥ tan(θ). Para 30° hace falta μ ≥ 0,577, pero el informe declara
μ = 0,4, que topea la pendiente trepable en arctan(0,4) ≈ 21,8°. Es un conflicto estructural, no un
error de cuenta: **más torque de motor no lo resuelve**.

El tratamiento completo, con las preguntas para Mecánica y el ensayo que lo cierra, está consolidado
en [00-contexto/03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md). Hasta que se
responda, el requisito de 11–17 N·m se toma como válido, pero el límite sin deslizar del informe
(5,214 N·m) queda inconsistente con él.

## Implicancias eléctricas

- 120 W por motor × 4 motores ≈ 480 W de tracción sostenida.

  > **Nota (16-sep):** este número quedó corregido a **247 W eléctricos (10,3 A a 24 V)**. El cálculo
  > de 480 W multiplicaba el torque de obstáculo por la velocidad máxima, y esas dos condiciones
  > nunca ocurren juntas: el obstáculo se trepa despacio. El cálculo corregido está en
  > [20-potencia/presupuesto-corriente.md](../20-potencia/presupuesto-corriente.md). El 480 W queda
  > acá a propósito: el contraste entre los dos números es informativo.

El resto de las implicancias eléctricas (corriente nominal y picos a 24 V, dato que define el driver,
calibres y fusibles) está en
[20-potencia/cableado-y-protecciones.md](../20-potencia/cableado-y-protecciones.md).

## Estado de candidatos

Ningún candidato vigente. El estado por motor evaluado, el criterio de reducción y el criterio de
búsqueda del reemplazo están en [candidatos-motores.md](candidatos-motores.md).

## Próximos pasos

1. Confirmar con mecánica el diámetro de rueda definitivo y rehacer todos los cálculos de torque del
   subsistema.
2. Elevar a mecánica las dos preguntas de μ y pendiente.
3. Buscar candidato de motor de reemplazo, priorizando proveedores con respuesta comprobada y hoja de
   datos que declare corriente de stall.
