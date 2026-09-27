---
titulo: Instrumentación de ensayos del modelo a escala
estado: vigente
fecha: 2026-09-16
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §2, §3.3, §3.4 y riesgos de medición de §5
---

# Instrumentación de ensayos

**El sensor de corriente por motor no es necesario.** El sensor general alcanza, siempre que la
calibración y el ensayo se hagan con la misma cadena de medición: el error de ganancia se cancela
solo.

El montaje del sensor de a bordo y qué mide exactamente están en
[20-electronica/sensado-corriente.md](../20-electronica/sensado-corriente.md).

---

## Medición de corriente sin el INA226

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
