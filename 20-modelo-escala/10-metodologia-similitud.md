---
titulo: Metodología de similitud y escala del modelo
estado: vigente
fecha: 2026-09-16
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §4, riesgos de escala de §5 y anexo de constantes
---

# Revisión de escala por la masa real de 5,5 kg

**La masa real de 5,5 kg no saca al modelo de rango: mejora el margen.** Pero obliga a llevar el
diámetro de rueda del modelo de 102 a 111 mm.

> **Nota de estado:** los valores de escala de este archivo (λ = 0,371) reemplazan los del protocolo
> rev. 1.0 (λ = 0,34). El resto del protocolo (ensayos A, B, C, planillas, fallas típicas) sigue
> vigente. La cadena de medición basada en INA226 también quedó reemplazada: ver
> [50-ensayos/instrumentacion.md](50-ensayos/instrumentacion.md).

---

## 1. El número

$$\lambda = \sqrt{\frac{m_{modelo}}{m_{real}}} = \sqrt{\frac{5{,}5}{40}} = 0{,}371$$

La escala pasa de 0,34 a 0,371, un 9% más grande. Toda la geometría se corre con ella:

| Magnitud | λ = 0,34 (plan) | λ = 0,371 (real) | Δ |
|---|---|---|---|
| Diámetro de rueda | 102 mm | 111 mm | +9% |
| Largo | 272 mm | 297 mm | +9% |
| Ancho | 136 mm | 148 mm | +9% |
| Altura del centro de masa | 118 mm | 129 mm | +9% |
| Altura de obstáculo de ensayo | 34 mm | 37 mm | +9% |
| Factor 1/λ³ (término de peso) | 25,44 | 19,61 | −23% |
| Factor 1/λ² (compactación) | 8,65 | 7,27 | −16% |

## 2. El motor sigue sirviendo, y con más margen

| m (kg) | λ | D rueda (mm) | T requerido (N·m) | RPM requeridas | RPM disponibles | Margen | I por motor (A) |
|---|---|---|---|---|---|---|---|
| 4,6 | 0,339 | 102 | 0,203 | 109,3 | 112,2 | 2,7% | 0,54 |
| 5,5 | 0,371 | 111 | 0,266 | 104,5 | 109,8 | 5,1% | 0,65 |
| 6,0 | 0,387 | 116 | 0,303 | 102,3 | 108,4 | 6,0% | 0,71 |
| 6,5 | 0,403 | 121 | 0,342 | 100,3 | 106,9 | 6,6% | 0,78 |
| 7,0 | 0,418 | 126 | 0,382 | 98,4 | 105,4 | 7,1% | 0,85 |

**El margen mejora al engordar el modelo.** El requisito de velocidad angular cae con la masa a la
potencia −0,25: el modelo es más grande, la rueda es más grande, y hacen falta menos vueltas para
replicar la misma velocidad real. El torque requerido sube, pero desde el 6,5% del torque de bloqueo
hasta el 8,5%: la curva torque-velocidad casi no se entera.

**Verificación térmica:** 0,65 A contra un límite continuo conservador de 1,1 a 1,65 A. Al 40 a 60%.
El techo térmico recién se alcanza cerca de los 8,7 kg.

**Bonus:** el conflicto de empaquetado señalado en la lista de verificación del protocolo (dos
motores de 70 mm enfrentados necesitan 140 mm, y el ancho escalado eran 136 mm) se resuelve solo: a
λ = 0,371 el ancho pasa a 148 mm.

## 3. El problema real es la rueda, no el peso

**λ se define por el diámetro de rueda, no por la masa**, porque la resistencia a la compactación
depende del ancho de contacto y del hundimiento, que son geométricos.

Con 5,5 kg y ruedas de 102 mm conviven dos escalas distintas:

- λ geométrico = 102 / 300 = 0,34
- λ másico = raíz de (5,5/40) = 0,371

La masa correcta para ruedas de 102 mm sería 4,62 kg. Hay 0,88 kg de más, o sea **19% de exceso de
presión de contacto**. En arena el modelo se hunde más de lo representativo, el término de
compactación sale sobreestimado, y la predicción del rover real queda sesgada hacia arriba en
aproximadamente ese mismo porcentaje.

Es un error conservador (sobredimensiona el motor), pero es un error, y entra multiplicado en la
sensibilidad dominante del proyecto:

$$\frac{\Delta T}{T} \approx 3\,\frac{\Delta\lambda}{\lambda}$$

**Atención:** λ geométrico usa 300 mm como diámetro del rover real, que es uno de los tres valores en
circulación. El diámetro correcto del modelo según cada valor:

| Diámetro del rover real | Diámetro correcto del modelo (λ = 0,371) |
|---|---|
| 0,28 m | 104 mm |
| 0,30 m | 111 mm |
| 0,35 m | 130 mm |

Son 26 mm entre extremos: no reimprimir antes de que Mecánica confirme. Ver [00-contexto/03-conflictos-abiertos.md](../00-contexto/03-conflictos-abiertos.md).

## 4. Las tres salidas

| Opción | Qué implica | Recomendación |
|---|---|---|
| **A. Ruedas a 111 mm** | reimprimir cuatro ruedas, recalcular λ = 0,371, todo cierra | **adoptada**, es la más barata en tiempo |
| B. Bajar a 4,6 kg | sacar 0,9 kg de estructura, batería o electrónica ya montada | difícil, no vale la pena |
| C. Aceptar el desfasaje y corregir a mano | usar el cociente exacto en lugar de 1/λ³ para el término de peso | funciona en piso duro, no cierra en arena |

Aclaración sobre la opción C: el término de peso sí se puede corregir analíticamente, porque la
fuerza es proporcional a masa por radio y ambos se conocen. Pero el término de compactación depende
del ancho de contacto y del hundimiento con un exponente del modelo de Bekker, y ahí no hay
corrección exacta sin caracterizar la arena. Como ese es justamente el término que domina a escala
chica, se descarta.

## 5. Sobre el rango [1, 5] kg del informe de mecánica

Sí, se salió del rango. No importa.

Ese rango era un criterio de constructibilidad de mecánica, no un límite físico: "ni tan liviano que
sea poco manejable, ni tan pesado que encarezca el lastre". El criterio vinculante de verdad está en
la misma sección del informe: **que exista un motorreductor de catálogo que cumpla torque y velocidad
simultáneamente a esa masa**. A 5,5 kg el CQRobot cumple los dos, con más margen que a 4,6 kg.

Lo que sí cambia es que **se acabó el lastre.** El plan original era estructura liviana más lastre
para llegar a 4,6 kg, y el lastre era la variable de ajuste: permitía corregir λ después de construir
y ubicar masa para fijar la altura del centro de masa. Esa libertad ya no existe: no se puede sacar
peso, solo agregar. El diámetro de rueda queda como única variable de ajuste, lo que refuerza la
opción A.

## 6. Altura del centro de masa

El objetivo pasa a **129 mm**. Si la batería quedó baja en el chasis (lo normal por estabilidad),
probablemente esté por debajo de ese valor. Si está por debajo, el modelo vuelca a un ángulo mayor
que el rover real, o sea que el ensayo es optimista en estabilidad. No invalida los ensayos de
tracción, pero sí cualquier conclusión sobre vuelco.

## 7. Riesgos de escala

- **Si el peso sigue creciendo** (cableado, conectores, guardapolvos de arena), λ se mueve otra vez.
  Conviene congelar el peso antes de imprimir las ruedas definitivas, o directamente pesar el modelo
  completo con todo montado.
- **El margen de velocidad del 5% sigue siendo estructuralmente chico.** Una caída del 8% de tensión
  de batería se lo come entero. La alimentación por convertidor reductor regulado a 12,00 V no es
  opcional: es lo que hace comparables dos corridas.

Los riesgos de medición están en [50-ensayos/instrumentacion.md](50-ensayos/instrumentacion.md).

## 8. Motor y configuración del modelo

**Motor:** CQRobot CQR37D12V64EN-I ×4. 37D, 12 V, 90:1, 120 rpm sin carga, τ_stall 3,138 N·m,
I_stall 5,5 A, I₀ 0,2 A, encoder 64 CPR en el eje del motor → 5760 CPR en el eje de salida, ~22 USD.

**Configuración:** swerve, con un servo de dirección por rueda. Ya no es skid-steer. Con eso el
modelo sí valida la cinemática swerve, que era su laguna principal.

## 9. Qué valida el modelo y qué no

| Valida | No valida |
|---|---|
| Tracción y torque real en dos superficies | Cualquier magnitud eléctrica o térmica |
| Cinemática swerve | Rendimiento del tren motriz real |
| Control de velocidad por rueda | Constante térmica del motor grande |
| Firmware y telemetría | Corriente de arranque, ondulación, EMI |
| μ real por ángulo de deslizamiento | Cargas de rodamiento del cubo full-scale |

Regla práctica: **si la magnitud tiene unidades eléctricas o térmicas, el modelo no la dice.** El
detalle de por qué cada magnitud eléctrica o térmica no escala está en
[10-rover-final/20-potencia/presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md) §6.

---

## Anexo. Constantes recalculadas

| Símbolo | Valor rev. 1.0 | Valor vigente | Origen |
|---|---|---|---|
| λ | 0,34 | 0,371 | masa real del modelo, 5,5 kg |
| 1 / λ² | 8,65 | 7,27 | factor del término de compactación |
| 1 / λ³ | 25,44 | 19,61 | factor del término de peso |
| Masa del modelo | 4,6 kg (objetivo) | 5,5 kg (real) | pesado con electrónica y batería |
| Diámetro de rueda del modelo | 102 mm | 111 mm | derivado de λ |
| Altura del CM del modelo | 118 mm | 129 mm | derivado de λ |
| Torque de trabajo por rueda | 0,203 N·m | 0,266 N·m | criterio del informe a 5,5 kg |
| Corriente de trabajo por motor | 0,54 A | 0,65 A | derivada de K |

El techo de tracción del rover real y la potencia continua de tracción no dependen de λ: se calculan
en [10-rover-final/20-potencia/presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md).
