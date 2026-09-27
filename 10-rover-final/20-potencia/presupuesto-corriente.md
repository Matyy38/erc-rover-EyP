---
titulo: Presupuesto de corriente de la PDB — rover real
estado: vigente
fecha: 2026-09-16
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §1
---

# Del torque medido al presupuesto de corriente de la PDB

El modelo a escala entrega **torque**, no corriente. La corriente de la PDB se calcula después, con
la hoja de datos del motor grande que se termine eligiendo. El modelo aporta el dato de entrada
(torque real por rueda) y el perfil de uso, que hoy están estimados a ojo.

---

## 1. La cadena completa

El torque es la única magnitud de la cadena que escala con una ley limpia. La corriente depende de la
constante de torque, la tensión de bus y el rendimiento de la reductora del motor real, que no tienen
ninguna relación con los del motor chico.

| Paso | De dónde sale | Resultado |
|---|---|---|
| 1 | Ensayo D en piso duro y arena | T_duro y T_arena del modelo |
| 2 | Reconstrucción con dos exponentes | torque por rueda del rover real (N·m) |
| 3 | T × ω_real (6,667 rad/s) | potencia mecánica por rueda (W) |
| 4 | Hoja de datos del motor elegido | corriente por motor (A) |
| 5 | Suma + ciclo de trabajo | presupuesto de corriente de la PDB |

Los pasos 1 y 2 son los que el modelo resuelve. Los pasos 3 a 5 son cálculo de escritorio una vez
elegido el motor.

## 2. Del torque a la corriente

No hace falta conocer la constante de torque del motor grande. Para cualquier motor de continua con
curva torque-velocidad lineal, la corriente sale de tres datos de catálogo:

$$I_{motor} = I_0 + (I_{stall} - I_0)\cdot\frac{\tau}{\tau_{stall}}$$

Es la expresión del anexo del protocolo, usada al revés. Cada escenario de torque que devuelva el
modelo se convierte en corriente sin supuestos adicionales.

## 3. Corriente de motor contra corriente de bus

Con puente completo y modulación por ancho de pulso, la corriente que ve la PDB es aproximadamente el
ciclo de trabajo por la corriente del motor:

$$I_{bus} \approx D \cdot I_{motor}$$

Consecuencia poco intuitiva: **el pico de corriente de la PDB no ocurre trepando el obstáculo.**
Trepar exige mucho torque (mucha corriente de motor) pero a velocidad baja, o sea ciclo de trabajo
bajo, o sea poca corriente de bus. El peor caso de bus es rueda trabada con el lazo pidiendo
velocidad alta, que es condición de falla y la acota el límite de corriente del driver, no el
terreno.

En cambio, **el cableado entre driver y motor sí ve la corriente plena del motor**, sin el factor D.
Son dos dimensionamientos distintos sobre la misma rama. Ver
[cableado-y-protecciones.md](cableado-y-protecciones.md).

## 4. Escenarios a presupuestar

| Escenario | Torque por rueda (N·m) | D aprox. | Duración | Qué dimensiona |
|---|---|---|---|---|
| Plano, crucero | 1,5 a 2 (rodadura) | 0,9 a 1,0 | continuo | autonomía de batería |
| Pendiente al límite de tracción | 5,1 | 0,9 a 1,0 | minutos | corriente continua de PDB, fusibles, secciones |
| Trepado de obstáculo | 11 a 14 | 0,2 a 0,4 | segundos | corriente eficaz, térmica del driver |
| Rueda trabada | torque de bloqueo del motor | según lazo | ms a s | límite del driver, TVS, capacitores locales |

Las filas 1 a 3 son las que el modelo convierte de estimadas a medidas.

## 5. El techo de tracción acota el presupuesto continuo

$$\tau = \frac{m\,g}{n}\cos\theta \cdot \mu \cdot R = \frac{40 \cdot 9{,}81}{4}\cdot\cos 30° \cdot 0{,}4 \cdot 0{,}15 = 5{,}10\ \text{N·m}$$

Si el modelo predice un torque mayor a este valor, no significa que hagan falta motores más grandes:
significa que **el rover patina antes de poder aplicar ese torque**. La restricción vinculante es el
suelo, no el motor.

Potencia continua de tracción en la peor pendiente sostenible:

$$P_{mec} = 4 \cdot 5{,}10 \cdot 6{,}667 = 136\ \text{W}$$

Con rendimiento de tren motriz 0,55: **247 W eléctricos, 10,3 A en bus de 24 V**.

Contra los 480 W que salían de la hoja de mecánica. La diferencia es que aquel cálculo multiplicaba
el torque de obstáculo (11 N·m) por la velocidad máxima, y esas dos condiciones nunca ocurren juntas:
el obstáculo se trepa despacio. Ese solo razonamiento baja el presupuesto continuo de tracción a
menos de la mitad.

El número viejo se conserva a propósito en
[10-locomocion/requisitos-motor.md](../10-locomocion/requisitos-motor.md), donde el contraste entre
los dos cálculos es informativo. El choque entre los 11–17 N·m pedidos y los 5,10 N·m de techo está
tratado como conflicto 3 en
[00-contexto/03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md).

## 6. Lo que el modelo no dice

Regla práctica: **si la magnitud tiene unidades eléctricas o térmicas, el modelo no la dice.**

- Rendimiento del tren motriz real: no escala, sale del fabricante.
- Comportamiento térmico del motor grande: la constante térmica depende de la masa del bobinado y del
  montaje, que no escalan.
- Corriente de arranque, ondulación de conmutación, compatibilidad electromagnética.
- Cargas de rodamiento del cubo full-scale.

## 7. Lo que falta para cerrar la batería

El presupuesto de arriba es solo tracción. Falta el consumo de brazo, cómputo, comunicaciones y
ciencia. Sin eso no se cierra ni la batería ni el PDB. Sin apuro: depende de otros subsistemas.
