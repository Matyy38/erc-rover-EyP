---
titulo: Instrumentación de ensayos del modelo a escala
estado: vigente
fecha: 2026-09-27
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §2, §3.3, §3.4 y riesgos de medición de §5; decisiones del 27-sep-2026 (ADS1115 comprado, ACS712 a bordo)
---

# Instrumentación de ensayos

**El sensor de corriente por motor no es necesario para la extrapolación.** El sensor general
alcanza, siempre que la calibración y el ensayo se hagan con la misma cadena de medición: el error de
ganancia se cancela solo. Desde el 27-sep se suma un **ACS712 a bordo en una rueda**, no porque haga
falta un sensor por motor, sino como respaldo y para validar la corrección por ciclo de trabajo
(ver abajo).

El montaje del sensor de a bordo, qué mide exactamente y el cableado del ACS712 están en
[20-electronica/sensado-corriente.md](../20-electronica/sensado-corriente.md).

---

## Cadena adoptada (27-sep-2026)

Todo se lee por I2C con un solo ADS1115 a bordo. No hay instrumento externo con un segundo
microcontrolador.

| Instrumento | Dónde | Qué mide | Rol |
|---|---|---|---|
| TMCS1126B4 (placa) | entre relé y fusibles de drivers | corriente de batería de los 4 drivers | canal principal del Ensayo D |
| ACS712-30A (a bordo) | pata de motor de **una rueda delantera**, entre driver y motor | corriente real de ese motor, con signo | respaldo; misma cadena en A y D; valida la corrección por D |
| ADS1115 (a bordo) | bus I2C de J3, junto al MPU6050 | convierte ambos sensores a 16 bits | reemplaza al ADC del ESP32 |
| Balanza de gancho | entre modelo y anclaje | fuerza de tracción | método primario en piso duro. **La presta la facultad** |
| Multímetro 10 A | en serie, en banco | referencia | calibración |
| Brazo + balanza de cocina | eje de salida | torque en bloqueo | Ensayo A |

**Canales del ADS1115** (dirección 0x48; el MPU6050 está en 0x68):

| Canal | Señal | PGA | Resolución |
|---|---|---|---|
| AIN0 | Vout_CS del TMCS (cable desde el pad de C8 o el pin IO33) | ±2,048 V | 0,63 mA por cuenta |
| AIN1 | salida del ACS712, con 10 k en serie | ±4,096 V | 1,9 mA por cuenta |
| AIN2 | alimentación del ACS712 dividida por 2 (corrige que es proporcional a su alimentación) | ±4,096 V | — |
| AIN3 | reservado: IS del BTS7960 de la rueda del ACS712, solo si se aprueba | ±4,096 V | — |

- **ADS1115 a 3,3 V.** A 5 V no reconoce el alto de 3,3 V del ESP32. Ninguna entrada puede pasar de
  3,3 V: el ACS712 (cero en 2,5 V, 66 mV/A) en la pata de un motor va de 2,14 a 2,86 V con ±5,5 A.
  El límite estaría en +12 A.
- **Muestreo:** 860 muestras/s de a una conversión, alternando 3 canales → ~200 por canal. Cada
  conversión integra ~1,2 ms (unos 23 períodos de PWM a 20 kHz): el ADS1115 promedia la modulación
  por sí solo, cosa que el ADC del ESP32 no hace.
- **Sin GPIO libre para ALERT/RDY:** se lee por sondeo.
- **Por qué el ACS712 va en la pata de un motor y no en la entrada de batería:** en la pata mide la
  corriente que produce torque, sin corrección por D, y puede quedar en la misma posición en el
  Ensayo A y en el D. En la entrada de batería daría respaldo del total y, restando el TMCS, el
  consumo de servos (útil para V3), pero con el mismo sesgo por D que el TMCS: no lo validaría.
- **Delantera** porque en subida las delanteras pierden carga normal y deslizan primero.

### Corrección por ciclo de trabajo

Con PWM, cada driver toma de la batería aproximadamente `D · I_motor`: en el tiempo de apagado la
corriente del motor recircula por el puente sin pasar por la batería. El TMCS está del lado de la
batería, así que lee

$$I_{TMCS} \approx \sum_i D_i \cdot I_{motor,i} \quad\Rightarrow\quad \bar I_{motor} \approx \frac{I_{TMCS}}{4\,D}\ \text{(mismo D en las 4)}$$

Sin corregir, con D ≈ 0,9 hay ~10 % de sesgo, y **no se cancela con K**, porque K se mide en
continua pura. El firmware registra el D comandado de cada rueda en cada muestra. El Ensayo A'
confirma la corrección contra el ACS712.

> **Corrección (27-sep-2026).** La versión anterior de este archivo decía que en el Ensayo D "el
> total dividido cuatro es el promedio, que es lo que pide la extrapolación". Es el promedio de la
> corriente de batería, no de motor: falta dividir por D.

### El TMCS no puede estar en el Ensayo A

El Ensayo A alimenta el motor desde la fuente de banco entre 1 y 8 V. Con esa tensión en el bus no
arrancan ni el LM2596 ni la bobina del relé: el TMCS no ve el motor. El TMCS se calibra aparte, con
carga resistiva a tensión nominal contra el multímetro. El argumento de cancelación del error de
ganancia (2.2) aplica entonces **al ACS712**, que sí está en la cadena del A y del D. Para el TMCS,
la trazabilidad pasa por el multímetro, que es la misma referencia con la que se mide K.

---

## Medición de corriente sin el INA226

### 2.1 Qué necesita realmente cada ensayo

| Ensayo | Qué mide | ¿Alcanza el sensor general? | Por qué |
|---|---|---|---|
| A (constante K) | corriente de un motor bloqueado | sí | se ensaya de a un motor, en continua pura, sin modulación |
| B (verificación por fcem) | corriente en vacío | sí | también de a un motor |
| C (encoder) | no usa corriente | — | — |
| D (tracción) | corriente de los cuatro | sí, con la rama aislada | el total dividido cuatro es el promedio de corriente **de batería**; dividido además por D da el de motor, que es lo que pide la extrapolación |

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
| Balanza de gancho digital | 0 (la presta la facultad) | tiro en barra (ver 2.6) | conseguida |
| Pinza amperométrica de continua | 40 a 70 | diagnosticar un motor sin desarmar | útil, no imprescindible |
| Shunt 0,1 Ω + osciloscopio | ya disponible | ver la forma de onda real bajo modulación | mejor diagnóstico, solo banco |
| Sensor por motor (ACS758, INA226) | 15 | corriente individual continua | no necesario |
| ACS712-30A en una rueda | 0 (ya disponible) | respaldo y validación de la corrección por D | **adoptado** (ver arriba) |

Jugada eficiente: durante el Ensayo A, multímetro en serie leyendo en simultáneo con el ACS712. Se obtienen dos cosas en la misma sesión: la constante K, y la calibración del ACS712 contra una referencia. (La versión anterior decía "con el sensor de a bordo": el TMCS no puede estar en el Ensayo A, ver arriba.)

### 2.6 Alternativa sin corriente: tiro en barra

Método estándar en terramecánica. Se ata el modelo a un punto fijo a través de una balanza de gancho, se comandan los motores y se mide la fuerza hasta que las ruedas patinan:

$$T_{total} = F_{medida} \cdot R_{rueda}$$

Ventajas: mide fuerza, no un proxy. Cero cadena de conversión, cero K, cero I₀, cero rendimiento de reductora. No le afecta la modulación, ni la temperatura del bobinado, ni la dispersión entre motores.

Desventajas: es un ensayo estático, no captura comportamiento dinámico ni trepado con impulso. Requiere anclaje rígido y alineado con el eje de avance.

**Recomendación: tiro en barra como método primario y corriente como canal redundante, medidos en simultáneo.** Si coinciden dentro del 15%, se obtiene una validación cruzada que ningún ensayo individual da. Además el tiro en barra calibra el método por corriente en condiciones reales de tracción, no en banco bloqueado.

En rampa, con el anclaje abajo: $T_{total} = (F_{medida} + m\,g\,\sin\theta)\cdot R_{rueda}$.

**El criterio del 15% vale en piso duro, no en arena** (aclaración del 27-sep). La balanza mide la
fuerza que sobra después de vencer la resistencia de compactación; la corriente mide el torque total
que entregan las ruedas. En piso duro la diferencia es la resistencia a la rodadura, chica. En arena
la diferencia **es** la resistencia de compactación, y es un dato, no un error:

$$F_{compactación} \approx \frac{T_{corriente}}{R} - F_{barra} - m\,g\,\sin\theta$$

---

## Instrumento externo: de corrección obligatoria a mejora opcional

> **Cambio de estado (16-sep-2026).** El instrumento externo nació como corrección obligatoria, para
> salvar el Ensayo D de una contaminación que se creía existente: se pensaba que el sensor de a bordo
> medía el consumo total de la PDB y que los servos ensuciaban la medición. **Eso quedó descartado:**
> el TMCS1126 mide solo la rama de tracción y el Ensayo D es válido con el sensor de a bordo. Ver
> [90-archivo/analisis-sensado-superado.md](../../90-archivo/analisis-sensado-superado.md).
>
> El instrumento externo **pasa a ser una mejora de calidad de ensayo**, no una corrección. Las
> razones que lo justifican por sí mismas, abajo, siguen todas en pie.
>
> **Decisión (27-sep-2026): no se construye por ahora.** Se adoptó la alternativa barata (ADS1115 a
> bordo) y el ACS712 también va a bordo, leído por el mismo ADS1115. Si el D0 muestra muestras
> perdidas o degradación del lazo de control, se retoma esta sección.

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

### La alternativa barata

Si lo único que preocupa es la contención de recursos en el ESP32, hay una salida de 3 USD en lugar
de 15 a 20: **mantener el sensor de a bordo y subir solo el conversor a ADS1115**, aceptando que el
muestreo comparta CPU y bus I2C con el lazo de control.

Para un ensayo de treinta segundos a 100 muestras por segundo probablemente alcance. Conviene
medirlo —registrar y ver si se pierden muestras o si el lazo se degrada— antes de gastar en el
instrumento externo completo.

| Opción | Costo | Qué resuelve | Qué no resuelve |
|---|---|---|---|
| Sensor de a bordo + ADC del ESP32 | 0 | Nada nuevo | No linealidad del ADC, contención de CPU |
| Sensor de a bordo + ADS1115 | 3 USD | La no linealidad del conversor | La contención de CPU y de bus I2C |
| Instrumento externo completo | 15 a 20 USD | Contención, reutilización, no tocar la placa | Agrega el problema de sincronización |

---

## Riesgos de medición

- **El sensor satura en bloqueo** (22 A contra ±16 A de fondo de escala). No sirve como protección
  por sobrecorriente. Son dos funciones distintas y el sensor solo cubre una.
- **El ensayo de arena necesita repetibilidad del terreno.** El estado de compactación de la arena
  varía más entre corridas que cualquier error de instrumentación: rastrillar y nivelar con el mismo
  procedimiento antes de cada corrida importa más que la precisión del sensor.
- **Si el instrumento externo se hace con un segundo microcontrolador**, la sincronización con los
  encoders es el único punto que puede arruinar el ensayo. Resolverlo con la línea digital compartida
  antes de la primera corrida, no después.

Los riesgos de escala están en
[10-metodologia-similitud.md](../10-metodologia-similitud.md). El orden en que se usan estos
instrumentos está en [plan-de-ensayos.md](plan-de-ensayos.md).
