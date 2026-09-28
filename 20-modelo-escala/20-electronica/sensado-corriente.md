---
titulo: Sensado de corriente — TMCS1126B4 y ACS712 de respaldo
estado: vigente
fecha: 2026-09-27
fuente: pcb-modelo-escala.md, sección sensado-corriente; decisiones del 27-sep-2026 (ADS1115 y ACS712)
---

# Sensado de corriente

### Qué se eligió y por qué

El ACS712-30A del PDF quedó cancelado **como sensor de placa**. En su lugar hay **un TMCS1126B4**
(Texas Instruments), sensor Hall con aislación reforzada, en encapsulado SOIC-10 ancho.

> **27-sep-2026:** el ACS712-30A vuelve, con otro rol: segundo sensor a bordo, en la pata de motor de
> una rueda, leído por el ADS1115. Ver [ACS712 de respaldo](#acs712-de-respaldo-27-sep-2026) al final
> de este archivo.

| Parámetro | Valor |
|---|---|
| Sensibilidad | **100 mV/A** |
| Tensión de salida a corriente cero (VREF interno) | **1,65 V** (variante B, bidireccional) |
| Rango lineal con VS = 3,3 V | **±15,5 A** |
| Resistencia del conductor interno | 0,7 mΩ (a 10 A disipa 70 mW) |
| Ancho de banda | 500 kHz |
| Consumo | 11 a 14,5 mA |
| Tiempo de encendido hasta salida válida | 34 ms |
| Aislación | reforzada, 8 mm de línea de fuga y de aire |

La ventaja frente a un shunt es doble: no hay modo común (la salida ya está referida a masa lógica)
y el aislamiento galvánico lo hace inmune a los flancos del bus conmutado, que es exactamente el
problema que tiene un shunt en el lado alto con un puente H al lado.

### Dónde está conectado

`contacto NO del relé (K1.14) -> U3.IN- -> [conductor interno] -> U3.IN+ -> riel +12 V -> F3..F6 -> drivers`

Tres consecuencias:

1. **Mide únicamente la rama de tracción.** El LM2596 (y con él el ESP32 y los servos) cuelga del
   nodo A, aguas arriba del relé y del sensor. Lo que se lee es la suma de los cuatro motores más el
   consumo lógico de los cuatro drivers. Para el modelo a escala eso es incluso mejor que medir el
   total: el número que sale es directamente la corriente de tracción.
2. **El signo queda invertido.** La corriente entra por IN- y sale por IN+. La función de
   transferencia es `VOUT = IIN·S + VREF` con IIN positiva de IN+ a IN-. Con este cableado,
   descargar la batería hace **bajar** la salida desde 1,65 V:
   `I_traccion = (1,65 - Vout) / 0,1`
   Conviene confirmarlo con una carga conocida en el primer ensayo. Dar vuelta IN+/IN- no gana
   rango, porque el dispositivo es simétrico alrededor de 1,65 V.
3. **No hay medición por rama en la placa.** Con los pines IS de los BTS7960 sin usar, no se puede
   estimar el torque rueda por rueda desde el rover (desde el 27-sep hay una rueda medida con el
   ACS712, ver al final). Para el ensayo de caracterización `τ = K·(I - I0)` no
   molesta (ese ensayo va en banco con un motor por vez), pero sí impide validar el reparto de carga
   entre ruedas durante una trepada.

> **Nota de trazabilidad (16-sep-2026).** Hubo una contradicción entre documentos:
> `metodologia-modelo-escala` §3 afirmaba que el sensor medía el **consumo total de la PDB** y que
> por eso el Ensayo D quedaba invalidado por la contaminación de los servos. **Se resolvió a favor de
> este documento:** el TMCS1126 está en la rama conmutada, aguas abajo del relé, y los servos y el
> ESP32 cuelgan del nodo A, aguas arriba. Lo confirma la consecuencia arquitectónica ya documentada:
> al apretar la seta muere el bus de los drivers pero los servos y el ESP32 siguen alimentados; si
> los servos estuvieran en la rama medida, caerían con la seta. El análisis superado quedó archivado
> en [90-archivo/analisis-sensado-superado.md](../../90-archivo/analisis-sensado-superado.md).
>
> **Consecuencia práctica:** el Ensayo D es válido con el sensor de a bordo. El instrumento externo
> pasó de corrección obligatoria a mejora opcional, y el puente sobre la placa (cortar pista y
> reinsertar el sensor) es innecesario: ya está donde corresponde. Ver
> [50-ensayos/instrumentacion.md](../50-ensayos/instrumentacion.md).

Estado de cada afirmación del análisis superado:

| Afirmación de `metodologia` §3 | Estado |
|---|---|
| El sensor mide el total de la PDB | **Incorrecta.** Mide solo tracción |
| El Ensayo D queda invalidado | **No.** El Ensayo D es válido con el sensor de a bordo |
| Opción B: cortar pista y reinsertar el sensor | **Innecesaria.** Ya está donde corresponde |
| Opción C: instrumento externo obligatorio | **Degradada a opcional** |
| Opción A: Ensayo A fuera de la PDB | **Sigue vigente**, por su propio motivo: el motor se alimenta directo desde la fuente en continua pura |

**El único término parásito real** es el VCC lógico de los cuatro BTS7960, estimado en **10 a 30 mA**
(ver el presupuesto del riel de 3,3 V en
[arquitectura-electrica.md](arquitectura-electrica.md)). Es constante, así que se resta como línea de
base: se lee con el bus energizado y los motores parados, en el mismo tarado de cero que ya hay que
hacer al empezar cada sesión por la deriva térmica del Hall. No hay ningún otro consumo en la rama
medida.

### Rango de medición contra corriente de bloqueo (V8)

| Escenario | Corriente | Salida del sensor | ¿Entra en rango? |
|---|---|---|---|
| Marcha normal, 4 motores | 2 a 6 A | 1,45 a 1,05 V | Sí |
| Un motor bloqueado, tres girando | ~8 A | 0,85 V | Sí |
| Cuatro motores bloqueados | 22 A | saturaría a los 15,5 A | No |

El sensor satura alrededor de 15,5 A. Más que un error, es una decisión de diseño que conviene
asumir explícitamente: **el canal analógico cubre la operación normal y el canal digital OC cubre la
zona de falla**. Para eso hay que ajustar el umbral (V9). Alimentar el sensor con 5 V no ayuda,
porque el techo real lo pone la entrada del ADC del ESP32 en 3,3 V.

Resolución alcanzable con el ADC interno (ya reemplazado por el ADS1115 para ensayos: 0,63 mA por
cuenta con PGA ±2,048 V): el ADC de 12 bits sobre 3,3 V da 0,806 mV por escalón, o sea **8 mA por
escalón**. El ruido de entrada del sensor (150 uA/√Hz) filtrado a 1,6 kHz da unos 6 mA rms, del
mismo orden. El límite práctico va a ser la no linealidad conocida del ADC del ESP32, que hay que
calibrar por puntos.

### Cadena de acondicionamiento

| Bloque | Componentes | Función |
|---|---|---|
| Alimentación del sensor | +3,3 V -> C4 100 n -> FB1 (ferrita) -> C3 100 n -> VS | filtro en pi, aísla el sensor del ruido del riel lógico. Es la topología que recomienda el fabricante |
| Umbral de sobrecorriente | R3 4,7 k (de VS a VOC) + R4 47 k (de VOC a GND) + C5 1 u | divisor que fija el umbral |
| Salida al ADC | R15 1 k serie + C8 100 n a masa -> IO33 | pasabajos de **1,59 kHz** (τ = 100 us). Atenúa 22 dB a 20 kHz de PWM |
| VREF (pin 7) | libre, solo con una etiqueta local `Vref_CS` | ver nota |
| OC (pin 3) | R2 4,7 k a masa | ver V9 |

Sobre el R15 de 1 k: además de filtrar, aísla el capacitor de 100 nF de la salida del sensor. El
TMCS1126 admite como máximo 4,7 nF directamente en VOUT; sin esa resistencia serie el amplificador
de salida podría oscilar. Está bien resuelto.

Sobre VREF libre: es una **salida**, no una entrada, así que dejarla al aire es legal. Es una
oportunidad disponible: llevándola a un segundo canal de ADC y midiendo `Vout - Vref` en vez de
`Vout - 1,65 V` se cancelan el error de referencia, su deriva térmica y buena parte de la deriva del
propio ADC. Hoy no hay ningún ADC1 libre; queda como mejora.

### Umbral de sobrecorriente y pin OC (V9)

**Umbral.** La ecuación del fabricante es `VOC = S · IOC / 2,5`. Con el divisor actual:
`VOC = 3,3 × 47 / (47 + 4,7) = 3,0 V`, lo que da

`IOC = 2,5 × VOC / S = 2,5 × 3,0 / 0,1 = 75 A`

75 A no los alcanza este sistema, así que la protección no llegaría a dispararse nunca. Además el
fabricante pide que la resistencia inferior del divisor sea **menor a 10 k** (la impedancia de
entrada del pin VOC es 120 k y carga el divisor); R4 = 47 k queda fuera de esa guía y agrega un
error de umbral del orden del 30 %. Conviene medir la tensión real en VOC antes de dar el cálculo
por válido.

Ajuste propuesto, para un umbral de 25 A (por encima del bloqueo de 22 A, así no hay falsos
positivos):

`VOC = 0,1 × 25 / 2,5 = 1,0 V`

| Componente | Actual | Propuesto |
|---|---|---|
| R3 (VS a VOC) | 4,7 k | 11 k |
| R4 (VOC a GND) | 47 k | 4,7 k |
| VOC resultante | 3,0 V | 0,987 V |
| IOC resultante | 75 A | 24,7 A |

La histéresis de la variante de 100 mV/A es de 1,4 A, así que rearmaría a los 23,3 A. También
conviene subir C5 de 1 uF a 10 uF: el fabricante lo pide para estabilizar el umbral del comparador
en ambientes con flancos de modo común rápidos, que es justo el caso de un puente H.

**Pin OC.** Es una salida de colector abierto activa en bajo, conectada a masa a través de R2 =
4,7 k. Con un pull-down no puede subir nunca, así que la señal queda siempre baja y no va a ningún
lado. El fabricante dice que si no se usa se conecte a masa, así que no rompe nada.

**Mejora de alto valor:** hoy los `R_EN`/`L_EN` de los cuatro BTS7960 están atados permanentemente a
+3,3 V. Si en vez de eso ese nodo de enables se lleva a 3,3 V a través de una resistencia de pull-up
de 4,7 k y se le conecta el pin OC del TMCS1126, se obtiene:

- Corte de los cuatro puentes en 100 ns ante sobrecorriente, sin intervención del firmware.
- Rearme automático con histéresis.
- Cero GPIO consumidos.
- Y si además ese nodo se lleva a una entrada del ESP32 (cuando se libere una), se puede contar
  cuántas veces disparó.

Es una resistencia y una pista, y convierte una protección inactiva en la más rápida de la placa.

### Calibración

El TMCS1126 viene calibrado de fábrica (error de sensibilidad de 0,3 % en grado B), así que el error
dominante no va a ser el sensor sino el ADC del ESP32. Procedimiento mínimo:

1. Con el bus energizado y sin carga, leer `Vout` y guardarlo como cero real. No asumir 1,65 V.
2. Poner una carga conocida (lámpara de auto, resistor de potencia) medida con pinza amperométrica
   en 3 o 4 puntos entre 1 y 10 A, y ajustar una recta.
3. Repetir con los motores girando y PWM activo, para ver cuánto ripple queda después del filtro y
   decidir el promediado por software.
4. Verificar el cero a distintas temperaturas: la deriva de offset de esta variante son 30 uV/°C
   típicos, o sea 0,3 mA/°C referido a la entrada. Despreciable.

---

## ACS712 de respaldo (27-sep-2026)

Con el ADS1115 comprado quedan canales libres, y se suma el ACS712-30A que ya estaba disponible. Va
**a bordo**, no como instrumento externo, y se lee por el mismo bus I2C.

### Dónde va

En la **pata de motor de una rueda delantera**, entre la salida del BTS7960 y el motor. Ahí mide la
corriente real del motor, con signo, que es la que produce torque. Las dos razones:

1. El TMCS está del lado de la batería y con PWM lee `D · I_motor`, no `I_motor`. El ACS712 en la
   pata del motor valida la corrección por ciclo de trabajo. Ver
   [50-ensayos/instrumentacion.md](../50-ensayos/instrumentacion.md).
2. Puede quedar en la misma posición en el Ensayo A (pata del motor alimentada desde la fuente de
   banco) y en el Ensayo D, así su error de ganancia se cancela para esa rueda. El TMCS no puede
   estar en el Ensayo A.

Delantera porque en subida pierde carga normal y desliza primero. La alternativa, en la entrada de
batería, daba respaldo del total y consumo de servos por diferencia (útil para V3), pero con el mismo
sesgo por D que el TMCS.

### Conexión

| Punto | Detalle |
|---|---|
| Alimentación | 5 V propios: regulador lineal pequeño (78L05 o AMS1117-5.0) desde el riel "+5V" de la placa. Aunque ese riel esté en 5 V conviene el regulador: el LM2596 conmuta y el ACS712 es proporcional a su alimentación |
| Salida | a AIN1 del ADS1115, con 10 k en serie (limita la corriente si el ACS712 queda alimentado antes que el ADS) |
| Referencia de alimentación | divisor 1:2 de su 5 V a AIN2, para corregir cero y ganancia |
| Rango útil | con el ADS a 3,3 V, de −30 A a +12 A según el sentido. En la pata de un motor (±5,5 A en bloqueo) la salida va de 2,14 a 2,86 V |
| Sensibilidad | 66 mV/A, cero en la mitad de la alimentación |

El canal del TMCS va a AIN0 con un **cable desde el nodo Vout_CS** (pad de C8 o pin IO33): es un
agregado sin cortes, y el ESP32 sigue leyendo por IO33. Mapa completo de canales en
[50-ensayos/instrumentacion.md](../50-ensayos/instrumentacion.md). El ADS1115 comparte el I2C de J3
con el MPU6050 ([40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md)).

### Qué no hace

- No mide el total: es una rueda. La extrapolación usa el total del TMCS corregido por D.
- No reemplaza a la protección por sobrecorriente.
- Su cero deriva más que el del TMCS: tarar al inicio de cada sesión y leer AIN2.

---

Los puntos V8, V9 y V11 y sus mediciones están resumidos en [30-pcb/pendientes-de-verificacion.md](../30-pcb/pendientes-de-verificacion.md). El uso de este sensor en los ensayos está en [50-ensayos/instrumentacion.md](../50-ensayos/instrumentacion.md).
