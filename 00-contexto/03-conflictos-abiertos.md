---
titulo: Conflictos abiertos con Mecánica
estado: vigente
fecha: 2026-09-16
fuente: 00-contexto-maestro.md §4 + requisitos-motor-rover-real.md (conflicto μ/pendiente)
---

# Conflictos abiertos

Los tres puntos que hoy bloquean decisiones. Ninguno se resuelve escribiendo: se resuelven midiendo
o preguntando.

> **Resuelto (16-sep): qué mide el TMCS1126.** Hubo una contradicción entre documentos. Vale
> `pcb-modelo-escala`: el sensor mide **solo la rama de tracción**. La afirmación de
> `metodologia-modelo-escala` §3 de que medía el consumo total de la PDB es incorrecta y queda
> superada. El análisis superado está en
> [90-archivo/analisis-sensado-superado.md](../90-archivo/analisis-sensado-superado.md) y el montaje
> real en
> [sensado-corriente.md](../20-modelo-escala/20-electronica/sensado-corriente.md).

---

## 1. Diámetro de rueda del rover real — tres valores en circulación

| Fuente | Diámetro |
|---|---|
| Hoja 1 de cálculos de Mecánica | 0,35 m |
| Hoja 2 de cálculos de Mecánica | 0,28 m |
| Informe mecánico jul-2026 | 0,30 m (declarado definitivo) |

El torque escala lineal con R y la velocidad angular inversamente: entre 0,28 y 0,35 hay ~20% de
diferencia en ambos requisitos. **Todos los cálculos del subsistema de potencia deben rehacerse con
el diámetro final una vez confirmado.**

Cada valor arrastra su propio par de cálculos en los documentos de Mecánica: la hoja 1 calculó ω y T
con R = 0,175 m, y la hoja 2 con R = 0,14 m. No es que haya un cálculo con tres resultados, son tres
cálculos consistentes cada uno con su hipótesis.

**Consecuencia que todavía no está contemplada:** λ se define por el cociente de diámetros de rueda.
El modelo va a reimprimir ruedas al diámetro que fija λ, asumiendo que el rover real es de 300 mm. Con
los otros dos valores el diámetro correcto del modelo cambia; la tabla de equivalencias está en
[10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) §3.

**Estado: pendiente de consulta a Mecánica.** Bloquea la reimpresión de ruedas. No mandar a imprimir
antes de tener la respuesta: son 26 mm de diferencia entre extremos y la impresión no es gratis en
tiempo.

---

## 2. Conflicto μ / pendiente

No deslizar en pendiente exige μ ≥ tan(θ). Para 30° hace falta μ ≥ 0,577; el informe declara μ = 0,4,
que topea la pendiente trepable en arctan(0,4) ≈ 21,8°.

Es estructural, no un error de cuenta: más torque de motor no lo resuelve.

Mientras no se responda, el requisito de 11–17 N·m se toma como válido, pero el límite sin deslizar
del informe (5,214 N·m) queda inconsistente con él. Es el mismo choque que aparece en el conflicto 3,
visto desde la hoja de Mecánica en lugar de desde el techo de tracción.

**Se cierra empíricamente y gratis:** ensayo de ángulo de deslizamiento con el modelo. Se sube la
rampa hasta que patina, y μ = tan(θ_deslizamiento). El coeficiente es adimensional: no escala, no
tiene λ, no tiene error de extrapolación. El valor medido en el modelo es directamente el del rover
real, con la condición de que rueda y superficie sean del mismo material.

Es el ensayo de mayor valor por unidad de esfuerzo de todo el programa. Además vuelve la pregunta a
Mecánica mucho más fácil de responder: en vez de pedirles un número que quizá no midieron, se les
lleva uno medido para que lo confirmen o lo discutan.

Procedimiento en
[50-ensayos/plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md).

---

## 3. Requisito de torque inconsistente con el techo de tracción

Los requisitos vigentes de Mecánica piden 11–17 N·m nominales por rueda. El techo de tracción en la
peor pendiente sostenible, calculado en
[presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md), es menos de la
mitad. Están en dos órdenes distintos de la misma magnitud.

La lectura correcta: los 11–17 N·m son de trepado de obstáculo (evento de segundos, torque alto y
velocidad baja) y el techo de tracción es el continuo sostenible. No son el mismo requisito y no deben
dimensionar la misma cosa. Ver
[20-potencia/presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md).

El requisito completo, con la nota del cálculo corregido de potencia, está en
[10-locomocion/requisitos-motor.md](../10-rover-final/10-locomocion/requisitos-motor.md).

---

## 4. Consulta pendiente a Mecánica

Los tres conflictos se responden con las mismas personas. Conviene una sola consulta y no tres
mensajes sueltos:

1. **¿Cuál es el diámetro de rueda definitivo?** Hay 0,28, 0,30 y 0,35 m circulando en tres
   documentos distintos. Afecta todos los cálculos de torque del subsistema y el diámetro de las
   ruedas del modelo a escala.
2. **¿Cuál es el μ real, medido o asumido, para el terreno de la competencia?** Con μ = 0,4 la
   pendiente trepable topea en 21,8°, no en 30°.
3. **¿Los 30° son una pendiente sostenida o un obstáculo puntual?** Si es puntual se supera con
   inercia y el criterio de no deslizamiento no aplica igual, lo que cambia el requisito de continuo.

Conviene hacer primero el ensayo de ángulo de deslizamiento (conflicto 2) y llevar ese número a la
consulta.
