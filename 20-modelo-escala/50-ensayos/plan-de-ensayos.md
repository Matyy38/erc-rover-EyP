---
titulo: Plan de ensayos del modelo a escala
estado: vigente
fecha: 2026-09-16
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §6, §3.2 opción A y §2.7
---

# Plan de ensayos

Qué se ensaya, en qué orden y por qué. Los instrumentos y la cadena de medición están en
[instrumentacion.md](instrumentacion.md).

---

## Ensayo de ángulo de deslizamiento — primero, y gratis

El conflicto abierto con mecánica (μ = 0,4 contra 30°) se cierra empíricamente sin ningún instrumento: se sube la rampa gradualmente hasta que el modelo patina, y en ese ángulo

$$\mu = \tan\theta_{deslizamiento}$$

El coeficiente de fricción es **adimensional**: no escala, no tiene λ, no tiene error de extrapolación. El valor medido en el modelo es directamente el del rover real, siempre que rueda y superficie sean del mismo material.

Es el resultado de mayor valor por unidad de esfuerzo de todo el programa de ensayos, y es el dato que hoy bloquea la selección del motor grande.

Este ensayo cierra el conflicto 2 de
[00-contexto/03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md), y conviene
hacerlo **antes** de la consulta a Mecánica: se les lleva un número medido para confirmar o discutir,
en lugar de pedirles uno que quizá no midieron.

---

## Ensayo A fuera de la PDB — sigue vigente por su propio motivo

| Opción | Qué implica | Costo | Veredicto |
|---|---|---|---|
| **A. Ensayo A fuera de la PDB** | alimentar el motor directo desde la fuente de banco, con multímetro y sensor en serie | 0 | **hacer ya**, no requiere ningún cambio |

Esta opción nació dentro de un análisis que después quedó superado (se creía que el sensor de a bordo
medía el total de la PDB). **El ensayo sigue vigente igual, por un motivo propio e independiente:** la
calibración de K exige continua pura sin modulación, y eso solo se consigue alimentando el motor
directo desde la fuente de banco, fuera de la PDB. De a un motor por vez, con multímetro en serie
leyendo en simultáneo con el sensor de a bordo: salen K y dos puntos de calibración del sensor en la
misma sesión.

El resto de aquel análisis —y por qué se cayó— está en
[90-archivo/analisis-sensado-superado.md](../../90-archivo/analisis-sensado-superado.md).

---

## Próximos pasos

1. **Pesar el modelo completo en balanza**, con batería, electrónica y cableado montados. De ahí sale λ, y de λ salen las ruedas.
2. **Reimprimir las ruedas** al diámetro que corresponda a esa masa (ver [10-metodologia-similitud.md](../10-metodologia-similitud.md)). Medir el diámetro impreso con calibre y recalcular λ con ese valor, no con el nominal.
3. **Medir la altura del centro de masa** y comparar con 0,3464·λ. Corregir con la ubicación de la batería antes de fijar nada.
4. **Comprar el ADS1115** y verificar con multímetro la sensibilidad real del TMCS1126 montado.
5. **Ensayo A en banco, fuera de la PDB**, de a un motor, en continua, sin modulación, con multímetro en serie. Se obtienen K y la calibración del sensor en la misma sesión.
6. **Ensayo de ángulo de deslizamiento en piso duro**, antes que cualquier otra cosa. Es gratis, no necesita instrumentos, y da el μ real que hoy bloquea la elección del motor del rover real.
7. **Definir el instrumento externo** (sensor + ADS1115 + RP2040 + registro) o el puente sobre la placa, antes del Ensayo D.
8. **Comprar balanza de gancho** para el tiro en barra como canal redundante del Ensayo D.

Dos aclaraciones sobre esta lista, posteriores a su redacción:

- **El paso 2 está bloqueado.** La reimpresión de ruedas depende de la respuesta de Mecánica sobre el
  diámetro del rover real, que cambia el diámetro de rueda del modelo (tabla en [10-metodologia-similitud.md](../10-metodologia-similitud.md) §3). **No
  mandar a imprimir antes de tener la respuesta.**
- **El paso 7 dejó de ser obligatorio.** Ni el instrumento externo ni el puente sobre la placa hacen
  falta para que el Ensayo D sea válido: el sensor de a bordo mide solo la rama de tracción. El
  instrumento externo queda como mejora opcional, con una alternativa de 3 USD. Ver
  [instrumentacion.md](instrumentacion.md).

El resto (1, 3, 4, 5, 6 y 8) no depende de nadie más. El paso 6 es el de mayor valor por esfuerzo de
todo el programa y conviene hacerlo primero.

El orden completo de todo el subsistema, con costos y dependencias, está en el
[contexto maestro](../../00-contexto/00-contexto-maestro.md) §7.
