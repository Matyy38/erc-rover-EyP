---
titulo: Del modelo a escala al presupuesto de corriente de la PDB
estado: vigente
fecha: 2026-09-16
reemplaza_parcialmente: Protocolo_Ensayos_Motor_Rover_ERC rev. 1.0 (secciones 4, 5 y 9)
alcance: subsistema de potencia y electronica
---

# Del modelo a escala al presupuesto de corriente de la PDB

## Resumen ejecutivo

1. El modelo a escala entrega **torque**, no corriente. La corriente de la PDB se calcula después, con la hoja de datos del motor grande que se termine eligiendo. El modelo aporta el dato de entrada (torque real por rueda) y el perfil de uso, que hoy están estimados a ojo.
2. **El sensor de corriente por motor no es necesario.** El sensor general alcanza, siempre que la calibración y el ensayo se hagan con la misma cadena de medición: el error de ganancia se cancela solo.
3. **La masa real de 5,5 kg no saca al modelo de rango: mejora el margen.** Pero obliga a llevar el diámetro de rueda del modelo de 102 a 111 mm.
4. **Limitación nueva detectada:** el sensor actual no mide la rama de tracción sino el consumo total de la PDB. Eso lo invalida para el Ensayo D tal como está montado. Ver sección 5.

> **Nota de estado:** este documento reemplaza los valores de escala (λ = 0,34) y la cadena de medición basada en INA226 del protocolo rev. 1.0. El resto del protocolo (ensayos A, B, C, planillas, fallas típicas) sigue vigente.

---

## 1. Qué aporta el modelo al motor grande y qué no

### 1.1 La cadena completa

El torque es la única magnitud de la cadena que escala con una ley limpia. La corriente depende de la constante de torque, la tensión de bus y el rendimiento de la reductora del motor real, que no tienen ninguna relación con los del motor chico.

| Paso | De dónde sale | Resultado |
|---|---|---|
| 1 | Ensayo D en piso duro y arena | T_duro y T_arena del modelo |
| 2 | Reconstrucción con dos exponentes | torque por rueda del rover real (N·m) |
| 3 | T × ω_real (6,667 rad/s) | potencia mecánica por rueda (W) |
| 4 | Hoja de datos del motor elegido | corriente por motor (A) |
| 5 | Suma + ciclo de trabajo | presupuesto de corriente de la PDB |

Los pasos 1 y 2 son los que el modelo resuelve. Los pasos 3 a 5 son cálculo de escritorio una vez elegido el motor.

### 1.2 Del torque a la corriente

No hace falta conocer la constante de torque del motor grande. Para cualquier motor de continua con curva torque-velocidad lineal, la corriente sale de tres datos de catálogo:

$$I_{motor} = I_0 + (I_{stall} - I_0)\cdot\frac{\tau}{\tau_{stall}}$$

Es la expresión del anexo del protocolo, usada al revés. Cada escenario de torque que devuelva el modelo se convierte en corriente sin supuestos adicionales.

### 1.3 Corriente de motor contra corriente de bus

Con puente completo y modulación por ancho de pulso, la corriente que ve la PDB es aproximadamente el ciclo de trabajo por la corriente del motor:

$$I_{bus} \approx D \cdot I_{motor}$$

Consecuencia poco intuitiva: **el pico de corriente de la PDB no ocurre trepando el obstáculo.** Trepar exige mucho torque (mucha corriente de motor) pero a velocidad baja, o sea ciclo de trabajo bajo, o sea poca corriente de bus. El peor caso de bus es rueda trabada con el lazo pidiendo velocidad alta, que es condición de falla y la acota el límite de corriente del driver, no el terreno.

En cambio, **el cableado entre driver y motor sí ve la corriente plena del motor**, sin el factor D. Son dos dimensionamientos distintos sobre la misma rama.

### 1.4 Escenarios a presupuestar

| Escenario | Torque por rueda (N·m) | D aprox. | Duración | Qué dimensiona |
|---|---|---|---|---|
| Plano, crucero | 1,5 a 2 (rodadura) | 0,9 a 1,0 | continuo | autonomía de batería |
| Pendiente al límite de tracción | 5,1 | 0,9 a 1,0 | minutos | corriente continua de PDB, fusibles, secciones |
| Trepado de obstáculo | 11 a 14 | 0,2 a 0,4 | segundos | corriente eficaz, térmica del driver |
| Rueda trabada | torque de bloqueo del motor | según lazo | ms a s | límite del driver, TVS, capacitores locales |

Las filas 1 a 3 son las que el modelo convierte de estimadas a medidas.

### 1.5 El techo de tracción acota el presupuesto continuo

$$\tau = \frac{m\,g}{n}\cos\theta \cdot \mu \cdot R = \frac{40 \cdot 9{,}81}{4}\cdot\cos 30° \cdot 0{,}4 \cdot 0{,}15 = 5{,}10\ \text{N·m}$$

Si el modelo predice un torque mayor a este valor, no significa que hagan falta motores más grandes: significa que **el rover patina antes de poder aplicar ese torque**. La restricción vinculante es el suelo, no el motor.

Potencia continua de tracción en la peor pendiente sostenible:

$$P_{mec} = 4 \cdot 5{,}10 \cdot 6{,}667 = 136\ \text{W}$$

Con rendimiento de tren motriz 0,55: **247 W eléctricos, 10,3 A en bus de 24 V**.

Contra los 480 W que salían de la hoja de mecánica. La diferencia es que aquel cálculo multiplicaba el torque de obstáculo (11 N·m) por la velocidad máxima, y esas dos condiciones nunca ocurren juntas: el obstáculo se trepa despacio. Ese solo razonamiento baja el presupuesto continuo de tracción a menos de la mitad.

### 1.6 Lo que el modelo no dice

Regla práctica: **si la magnitud tiene unidades eléctricas o térmicas, el modelo no la dice.**

- Rendimiento del tren motriz real: no escala, sale del fabricante.
- Comportamiento térmico del motor grande: la constante térmica depende de la masa del bobinado y del montaje, que no escalan.
- Corriente de arranque, ondulación de conmutación, compatibilidad electromagnética.
- Cargas de rodamiento del cubo full-scale.

---

## 2. Medición de corriente sin el INA226

### 2.1 Qué necesita realmente cada ensayo

| Ensayo | Qué mide | ¿Alcanza el sensor general? | Por qué |
|---|---|---|---|
| A (constante K) | corriente de un motor bloqueado | sí | se ensaya de a un motor, en continua pura, sin modulación |
| B (verificación por fcem) | corriente en vacío | sí | también de a un motor |
| C (encoder) | no usa corriente | — | — |
| D (tracción) | corriente de los cuatro | sí, con la rama aislada | el total dividido cuatro es el promedio, que es lo que pide la extrapolación |

El sensor por motor nunca fue necesario para la calibración: en el Ensayo A no hay otro motor conectado.

### 2.2 El argumento fuerte: el error de ganancia se cancela

En el Ensayo A, K sale como la pendiente de torque contra corriente, donde la corriente es **lo que lee la cadena de medición**, no la corriente verdadera. Si el sensor tiene 4% de error de ganancia, K sale con 4% de error en sentido contrario. En el Ensayo D se convierte corriente a torque con esa misma K, leyendo con el mismo sensor: los dos errores se cancelan exactamente.

$$\tau = K_{medido}\cdot(I_{medido} - I_0), \qquad K_{medido} = \frac{\tau_{real}}{I_{medido} - I_0}$$

Lo que **no** se cancela:

- **La linealidad.** Si el sensor no es lineal, la pendiente no es constante. Los Hall aislados como el TMCS son buenos en esto.
- **El offset de cero.** Deriva con temperatura y es la fuente de error dominante en un Hall. Solución: tarar el cero al empezar cada sesión, con el bus energizado y los motores parados.
- **El punto de operación.** Calibrar en el rango donde después se va a medir.

### 2.3 El eslabón débil es el conversor, no el sensor

El TMCS1126 entrega tensión analógica. Leerlo con el conversor interno del ESP32 mete el peor eslabón de la cadena: 2 a 3% de error, y **no lineal**, que es justamente lo único que no se cancela.

| Opción | Costo aprox. (USD) | Resolución a 100 mV/A | Nota |
|---|---|---|---|
| ADC interno ESP32, 12 bits | 0 | 8 mA por cuenta | no lineal, eslabón débil |
| ADS1115, 16 bits, I2C | 3 | 1,25 mA por cuenta | **adoptado** |
| ADS1015, 12 bits, I2C | 2 | 20 mA por cuenta | lineal, alcanza si se promedia |

**Decisión: se adopta el ADS1115.** Con él, la limitación de precisión pasa a ser el montaje mecánico del brazo de calibración, que es lo correcto.

### 2.4 Verificar el fondo de escala

Con 100 mV/A y alimentación de 3,3 V con referencia en la mitad, el rango queda en ±16 A aproximadamente. El bloqueo simultáneo de los cuatro motores son 22 A: **el sensor satura ahí**. Para los ensayos no molesta (se trabaja a 2,6 A), pero implica que no sirve para caracterizar el bloqueo ni como base de protección por sobrecorriente.

Pendiente: confirmar la sensibilidad de la variante montada. La familia va de 50 a 400 mV/A, y eso cambia el fondo de escala por un factor 8.

### 2.5 Instrumentos externos, por relación valor-costo

| Instrumento | Costo (USD) | Para qué | Veredicto |
|---|---|---|---|
| Lectura de corriente de la fuente de banco | 0 | Ensayo A, referencia independiente | hacerlo, es gratis |
| Multímetro en serie, escala 10 A | 0 | Ensayo A, referencia de calibración | hacerlo |
| Balanza de gancho digital | 12 | tiro en barra (ver 2.6) | muy recomendado |
| Pinza amperométrica de continua | 40 a 70 | diagnosticar un motor sin desarmar | útil, no imprescindible |
| Shunt 0,1 Ω + osciloscopio | ya disponible | ver la forma de onda real bajo modulación | mejor diagnóstico, solo banco |
| Sensor por motor (ACS758, INA226) | 15 | corriente individual continua | no necesario |

Jugada eficiente: durante el Ensayo A, multímetro en serie leyendo en simultáneo con el sensor de a bordo. Se obtienen dos cosas en la misma sesión: la constante K, y la calibración de dos puntos del sensor contra una referencia.

### 2.6 Alternativa sin corriente: tiro en barra

Método estándar en terramecánica. Se ata el modelo a un punto fijo a través de una balanza de gancho, se comandan los motores y se mide la fuerza hasta que las ruedas patinan:

$$T_{total} = F_{medida} \cdot R_{rueda}$$

Ventajas: mide fuerza, no un proxy. Cero cadena de conversión, cero K, cero I₀, cero rendimiento de reductora. No le afecta la modulación, ni la temperatura del bobinado, ni la dispersión entre motores.

Desventajas: es un ensayo estático, no captura comportamiento dinámico ni trepado con impulso. Requiere anclaje rígido y alineado con el eje de avance.

**Recomendación: tiro en barra como método primario y corriente como canal redundante, medidos en simultáneo.** Si coinciden dentro del 15%, se obtiene una validación cruzada que ningún ensayo individual da. Además el tiro en barra calibra el método por corriente en condiciones reales de tracción, no en banco bloqueado.

### 2.7 El dato que el modelo resuelve sin instrumentos

El conflicto abierto con mecánica (μ = 0,4 contra 30°) se cierra empíricamente sin ningún instrumento: se sube la rampa gradualmente hasta que el modelo patina, y en ese ángulo

$$\mu = \tan\theta_{deslizamiento}$$

El coeficiente de fricción es **adimensional**: no escala, no tiene λ, no tiene error de extrapolación. El valor medido en el modelo es directamente el del rover real, siempre que rueda y superficie sean del mismo material.

Es el resultado de mayor valor por unidad de esfuerzo de todo el programa de ensayos, y es el dato que hoy bloquea la selección del motor grande.

---

## 3. Limitación del sensado actual (nuevo)

### 3.1 El problema

El TMCS1126 **no está midiendo la rama de tracción: está midiendo el consumo total de la PDB.** Eso incluye tracción, servos de dirección, lógica y cualquier otra carga aguas abajo del relé.

Consecuencias por ensayo:

| Ensayo | ¿Afecta? | Por qué |
|---|---|---|
| A (calibración) | no, si se hace en banco | el motor se alimenta directo desde la fuente, fuera de la PDB |
| B (fcem) | no | ídem |
| C (encoder) | no | no usa corriente |
| **D (tracción)** | **sí, lo invalida** | los servos consumen durante el ensayo y su consumo no es constante |

El punto crítico es el Ensayo D. Restar una línea de base funciona para un consumo **constante** (lógica), pero los servos de dirección del swerve mantienen torque durante el ascenso de la rampa y su corriente varía de forma impredecible, con picos de varios amperes. Ese consumo entra sumado a la corriente de tracción y no hay forma de separarlo a posteriori.

Peor: los picos de servo son del mismo orden que la señal que se quiere medir (2,6 A de tracción contra picos de servo de hasta 10 A por la especificación de la rama), así que no es un error pequeño sino una contaminación total de la medición.

### 3.2 Opciones ordenadas

| Opción | Qué implica | Costo | Veredicto |
|---|---|---|---|
| **A. Ensayo A fuera de la PDB** | alimentar el motor directo desde la fuente de banco, con multímetro y sensor en serie | 0 | **hacer ya**, no requiere ningún cambio |
| **B. Puente sobre la placa actual** | cortar la pista de entrada a la rama de tracción y reinsertar el sensor ahí; si hay portafusible o conector en esa rama, se hace sin cortar nada | 0 a 2 | **la corrección definitiva** para la placa del modelo |
| **C. Instrumento externo independiente** | módulo aparte con sensor Hall + ADS1115 + microcontrolador propio, intercalado en el cable de la rama de tracción con conectores XT | 15 a 20 | **la mejor opción para los ensayos** |
| D. Sensor por motor | cuatro sensores, cuatro canales | 60 | no se justifica (ver 2.1) |

**Plan adoptado: A para el Ensayo A, C para el Ensayo D, B como corrección definitiva de la placa.**

### 3.3 Por qué el instrumento externo es buena idea más allá de la limitación

No es un parche. Separar la instrumentación del control es buena práctica y tiene ventajas propias:

- **Aísla el muestreo del lazo de control.** El ESP32 principal está ocupado con cuatro canales de conteo de encoder, la modulación por ancho de pulso y el lazo de velocidad. Meterle además el muestreo promediado del ADS1115 compite por el mismo bus I2C y por tiempo de CPU, justo en el momento del ensayo.
- **Es reutilizable.** El mismo instrumento sirve después para caracterizar la PDB del rover real, para medir consumo de servos por separado, o para cualquier rama que se quiera instrumentar. Es equipo de banco, no parte del rover.
- **Se puede intercalar donde haga falta** sin tocar la placa: basta con abrir el mazo con conectores XT en el punto de interés.
- **No compromete la placa del modelo**, que ya está fabricada y soldada, y cuyos puntos de verificación de banco todavía no se terminaron.

### 3.4 Requisitos del instrumento externo

| Ítem | Especificación | Nota |
|---|---|---|
| Sensor | Hall aislado (TMCS1126 o ACS758-50B) | el ACS758 no satura en bloqueo, el TMCS sí |
| Conversor | ADS1115, 16 bits, I2C | promediado por configuración interna |
| Microcontrolador | RP2040 (Pico) o segundo ESP32 | el Pico alcanza y sobra, y sale menos |
| Registro | tarjeta SD o USB serie a la notebook | USB es más simple para ensayos de banco |
| Conectores | XT30 o XT60 en línea | prohibido DuPont, misma regla que el resto del proyecto |
| Frecuencia de muestreo | 100 a 500 muestras por segundo | suficiente: la constante mecánica es de decenas de ms |
| Sincronización con encoders | **línea digital compartida de arranque** | ver abajo |

**La sincronización es el único punto no trivial.** El Ensayo D identifica el torque umbral como el torque en el instante en que el deslizamiento supera el 20%, o sea que hay que correlacionar la corriente (instrumento externo) con las cuentas de encoder (ESP32 principal). Si los dos registros tienen relojes independientes, esa correlación no se puede hacer con precisión.

Solución mínima: una línea digital común entre los dos microcontroladores que marque el arranque del ensayo. Ambos ponen su marca de tiempo a cero en ese flanco y después se alinean los dos archivos por tiempo transcurrido. Con eventos de decenas de milisegundos y muestreo de 100 por segundo, la deriva de reloj entre dos cristales en un ensayo de treinta segundos es despreciable.

---

## 4. Revisión de escala por la masa real de 5,5 kg

### 4.1 El número

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

### 4.2 El motor sigue sirviendo, y con más margen

| m (kg) | λ | D rueda (mm) | T requerido (N·m) | RPM requeridas | RPM disponibles | Margen | I por motor (A) |
|---|---|---|---|---|---|---|---|
| 4,6 | 0,339 | 102 | 0,203 | 109,3 | 112,2 | 2,7% | 0,54 |
| 5,5 | 0,371 | 111 | 0,266 | 104,5 | 109,8 | 5,1% | 0,65 |
| 6,0 | 0,387 | 116 | 0,303 | 102,3 | 108,4 | 6,0% | 0,71 |
| 6,5 | 0,403 | 121 | 0,342 | 100,3 | 106,9 | 6,6% | 0,78 |
| 7,0 | 0,418 | 126 | 0,382 | 98,4 | 105,4 | 7,1% | 0,85 |

**El margen mejora al engordar el modelo.** El requisito de velocidad angular cae con la masa a la potencia −0,25: el modelo es más grande, la rueda es más grande, y hacen falta menos vueltas para replicar la misma velocidad real. El torque requerido sube, pero desde el 6,5% del torque de bloqueo hasta el 8,5%: la curva torque-velocidad casi no se entera.

**Verificación térmica:** 0,65 A contra un límite continuo conservador de 1,1 a 1,65 A. Al 40 a 60%. El techo térmico recién se alcanza cerca de los 8,7 kg.

**Bonus:** el conflicto de empaquetado señalado en la lista de verificación del protocolo (dos motores de 70 mm enfrentados necesitan 140 mm, y el ancho escalado eran 136 mm) se resuelve solo: a λ = 0,371 el ancho pasa a 148 mm.

### 4.3 El problema real es la rueda, no el peso

**λ se define por el diámetro de rueda, no por la masa**, porque la resistencia a la compactación depende del ancho de contacto y del hundimiento, que son geométricos.

Con 5,5 kg y ruedas de 102 mm conviven dos escalas distintas:

- λ geométrico = 102 / 300 = 0,34
- λ másico = raíz de (5,5/40) = 0,371

La masa correcta para ruedas de 102 mm sería 4,62 kg. Hay 0,88 kg de más, o sea **19% de exceso de presión de contacto**. En arena el modelo se hunde más de lo representativo, el término de compactación sale sobreestimado, y la predicción del rover real queda sesgada hacia arriba en aproximadamente ese mismo porcentaje.

Es un error conservador (sobredimensiona el motor), pero es un error, y entra multiplicado en la sensibilidad dominante del proyecto:

$$\frac{\Delta T}{T} \approx 3\,\frac{\Delta\lambda}{\lambda}$$

### 4.4 Las tres salidas

| Opción | Qué implica | Recomendación |
|---|---|---|
| **A. Ruedas a 111 mm** | reimprimir cuatro ruedas, recalcular λ = 0,371, todo cierra | **adoptada**, es la más barata en tiempo |
| B. Bajar a 4,6 kg | sacar 0,9 kg de estructura, batería o electrónica ya montada | difícil, no vale la pena |
| C. Aceptar el desfasaje y corregir a mano | usar el cociente exacto en lugar de 1/λ³ para el término de peso | funciona en piso duro, no cierra en arena |

Aclaración sobre la opción C: el término de peso sí se puede corregir analíticamente, porque la fuerza es proporcional a masa por radio y ambos se conocen. Pero el término de compactación depende del ancho de contacto y del hundimiento con un exponente del modelo de Bekker, y ahí no hay corrección exacta sin caracterizar la arena. Como ese es justamente el término que domina a escala chica, se descarta.

### 4.5 Sobre el rango [1, 5] kg del informe de mecánica

Sí, se salió del rango. No importa.

Ese rango era un criterio de constructibilidad de mecánica, no un límite físico: "ni tan liviano que sea poco manejable, ni tan pesado que encarezca el lastre". El criterio vinculante de verdad está en la misma sección del informe: **que exista un motorreductor de catálogo que cumpla torque y velocidad simultáneamente a esa masa**. A 5,5 kg el CQRobot cumple los dos, con más margen que a 4,6 kg.

Lo que sí cambia es que **se acabó el lastre.** El plan original era estructura liviana más lastre para llegar a 4,6 kg, y el lastre era la variable de ajuste: permitía corregir λ después de construir y ubicar masa para fijar la altura del centro de masa. Esa libertad ya no existe: no se puede sacar peso, solo agregar. El diámetro de rueda queda como única variable de ajuste, lo que refuerza la opción A.

### 4.6 Altura del centro de masa

El objetivo pasa a **129 mm**. Si la batería quedó baja en el chasis (lo normal por estabilidad), probablemente esté por debajo de ese valor. Si está por debajo, el modelo vuelca a un ángulo mayor que el rover real, o sea que el ensayo es optimista en estabilidad. No invalida los ensayos de tracción, pero sí cualquier conclusión sobre vuelco.

---

## 5. Riesgos y trade-offs

- **Si el peso sigue creciendo** (cableado, conectores, guardapolvos de arena), λ se mueve otra vez. Conviene congelar el peso antes de imprimir las ruedas definitivas, o directamente pesar el modelo completo con todo montado.
- **El margen de velocidad del 5% sigue siendo estructuralmente chico.** Una caída del 8% de tensión de batería se lo come entero. La alimentación por convertidor reductor regulado a 12,00 V no es opcional: es lo que hace comparables dos corridas.
- **El sensor satura en bloqueo** (22 A contra ±16 A de fondo de escala). No sirve como protección por sobrecorriente. Son dos funciones distintas y el sensor solo cubre una.
- **El ensayo de arena necesita repetibilidad del terreno.** El estado de compactación de la arena varía más entre corridas que cualquier error de instrumentación: rastrillar y nivelar con el mismo procedimiento antes de cada corrida importa más que la precisión del sensor.
- **Si el instrumento externo se hace con un segundo microcontrolador**, la sincronización con los encoders es el único punto que puede arruinar el ensayo. Resolverlo con la línea digital compartida antes de la primera corrida, no después.

---

## 6. Próximos pasos

1. **Pesar el modelo completo en balanza**, con batería, electrónica y cableado montados. De ahí sale λ, y de λ salen las ruedas.
2. **Reimprimir las ruedas** al diámetro que corresponda a esa masa (111 mm si el peso final son 5,5 kg). Medir el diámetro impreso con calibre y recalcular λ con ese valor, no con el nominal.
3. **Medir la altura del centro de masa** y comparar con 0,3464·λ. Corregir con la ubicación de la batería antes de fijar nada.
4. **Comprar el ADS1115** y verificar con multímetro la sensibilidad real del TMCS1126 montado.
5. **Ensayo A en banco, fuera de la PDB**, de a un motor, en continua, sin modulación, con multímetro en serie. Se obtienen K y la calibración del sensor en la misma sesión.
6. **Ensayo de ángulo de deslizamiento en piso duro**, antes que cualquier otra cosa. Es gratis, no necesita instrumentos, y da el μ real que hoy bloquea la elección del motor del rover real.
7. **Definir el instrumento externo** (sensor + ADS1115 + RP2040 + registro) o el puente sobre la placa, antes del Ensayo D.
8. **Comprar balanza de gancho** para el tiro en barra como canal redundante del Ensayo D.

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
| Techo de tracción del rover real | 5,10 N·m | 5,10 N·m | sin cambios, no depende de λ |
| Potencia continua de tracción | — | 247 W (10,3 A a 24 V) | 4 ruedas al techo de tracción, η 0,55 |
