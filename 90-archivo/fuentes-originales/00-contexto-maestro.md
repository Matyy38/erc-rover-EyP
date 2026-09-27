---
titulo: Contexto maestro — Rover ERC, subsistema de Electrónica y Potencia
owner: Matías Bélico
version: 1.0
fecha: 2026-09-16
estado: vigente
alcance: subsistema de potencia y electrónica
proposito: documento raíz del repo. Contexto, estado y conflictos abiertos. El detalle técnico vive en los archivos que este índice referencia.
---

# Contexto maestro — Rover ERC

Este es el documento raíz. No duplica el detalle técnico: lo ubica. Si buscás el valor de una
resistencia o el pinout de un conector, está en los archivos de `20-modelo-escala/`. Acá está el
porqué, el estado y lo que todavía no cierra.

---

## 1. Qué es el proyecto

**Competencia:** European Rover Challenge (ERC).
**Equipo:** IEEE ITBA I+D.
**Subsistema:** Electrónica y Potencia.

Dos frentes en paralelo:

| Frente | Propósito | Estado |
|---|---|---|
| **Rover final** | El vehículo que compite. 40 kg, bus de 24 V, swerve de 4 ruedas. | Arquitectura mecánica cerrada. Sin motor elegido. |
| **Modelo a escala** | Instrumento de medición, no maqueta. Mide torque real en piso duro y arena y extrapola al rover final. | PCB fabricada y soldada, con validación parcial. Sin ensayos. |

El modelo existe para cerrar la especificación de motores sin comprar motores de tamaño real a
ciegas. Esa es su única razón de ser, y todo lo que se decide sobre él se juzga contra eso.

---

## 2. Alcance

**Dentro:** batería y BMS, distribución de potencia, control de motores, protecciones, sensado,
firmware, telemetría.

**Fuera:** diseño estructural, cinemático y de transmisión (subequipo de Mecánica). Entra solo como
interfaz.

**Explícitamente fuera:** la derivación del factor de escala λ por parte de Mecánica. λ entra como
dato de entrada. Lo que sí es de este subsistema es la ecuación de extrapolación de torque y su
sensibilidad.

---

## 3. Reglas de diseño (no negociables)

- **COTS primero.** PCB custom solo si lo justifica espacio, peso o ruteo de EMI.
- **Sobredimensionar por seguridad, optimizar por peso y espacio.**
- **Star grounding estricto.** El retorno de motores no comparte cobre con el de lógica ni con el
  analógico en ningún tramo.
- **Cable de silicona en potencia.** Norma AWG.
- **Prohibido protoboard y cables Dupont.** Fretting bajo vibración, contactos intermitentes
  imposibles de diagnosticar. Siempre conectores con traba: XT60, Molex, Anderson, bornera.
- **E-Stop y fusibles innegociables.** El fusible protege el cableado, no la carga.
- **Punteras tubulares** en todo cable multifilar que entre a bornera.

Escenarios contra los que se juzga todo diseño: vibración continua, corriente de bloqueo, estrés
térmico.

---

## 4. Conflictos abiertos

Los tres puntos que hoy bloquean decisiones. Ninguno se resuelve escribiendo: se resuelven midiendo
o preguntando.

> **Resuelto (16-sep): qué mide el TMCS1126.** Hubo una contradicción entre documentos. Vale
> `pcb-modelo-escala`: el sensor mide **solo la rama de tracción**. La afirmación de
> `metodologia-modelo-escala` §3 de que medía el consumo total de la PDB es incorrecta y queda
> superada. Consecuencias en §7.3.

### 4.1 Diámetro de rueda del rover real — tres valores en circulación

| Fuente | Diámetro |
|---|---|
| Hoja 1 de cálculos de Mecánica | 0,35 m |
| Hoja 2 de cálculos de Mecánica | 0,28 m |
| Informe mecánico jul-2026 | 0,30 m (declarado definitivo) |

El torque escala lineal con R y la velocidad angular inversamente: entre 0,28 y 0,35 hay ~20% de
diferencia en ambos requisitos.

**Consecuencia que todavía no está contemplada:** λ se define por el cociente de diámetros de rueda.
El modelo va a reimprimir ruedas a 111 mm asumiendo que el rover real es de 300 mm. Con los otros dos
valores el resultado cambia:

| Diámetro del rover real | Diámetro correcto del modelo (λ = 0,371) |
|---|---|
| 0,28 m | 104 mm |
| 0,30 m | 111 mm |
| 0,35 m | 130 mm |

**Estado: pendiente de consulta a Mecánica.** Bloquea la reimpresión de ruedas. No mandar a imprimir
antes de tener la respuesta: son 26 mm de diferencia entre extremos y la impresión no es gratis en
tiempo.

### 4.2 Conflicto μ / pendiente

No deslizar en pendiente exige μ ≥ tan(θ). Para 30° hace falta μ ≥ 0,577; el informe declara μ = 0,4,
que topea la pendiente trepable en arctan(0,4) ≈ 21,8°.

Es estructural, no un error de cuenta: más torque de motor no lo resuelve.

**Se cierra empíricamente y gratis:** ensayo de ángulo de deslizamiento con el modelo. Se sube la
rampa hasta que patina, y μ = tan(θ_deslizamiento). El coeficiente es adimensional: no escala, no
tiene λ, no tiene error de extrapolación. El valor medido en el modelo es directamente el del rover
real, con la condición de que rueda y superficie sean del mismo material.

Es el ensayo de mayor valor por unidad de esfuerzo de todo el programa. Además vuelve la pregunta a
Mecánica mucho más fácil de responder: en vez de pedirles un número que quizá no midieron, se les
lleva uno medido para que lo confirmen o lo discutan.

### 4.3 Requisito de torque inconsistente con el techo de tracción

Los requisitos vigentes de Mecánica piden 11–17 N·m nominales por rueda. El techo de tracción en la
peor pendiente sostenible es 5,10 N·m. Están en dos órdenes distintos de la misma magnitud.

La lectura correcta: los 11–17 N·m son de trepado de obstáculo (evento de segundos, torque alto y
velocidad baja) y los 5,10 N·m son el continuo sostenible. No son el mismo requisito y no deben
dimensionar la misma cosa. Ver `20-potencia/presupuesto-corriente`.

### 4.4 Consulta pendiente a Mecánica

Los tres conflictos se responden con las mismas personas. Conviene una sola consulta y no tres
mensajes sueltos:

1. **¿Cuál es el diámetro de rueda definitivo?** Hay 0,28, 0,30 y 0,35 m circulando en tres
   documentos distintos. Afecta todos los cálculos de torque del subsistema y el diámetro de las
   ruedas del modelo a escala.
2. **¿Cuál es el μ real, medido o asumido, para el terreno de la competencia?** Con μ = 0,4 la
   pendiente trepable topea en 21,8°, no en 30°.
3. **¿Los 30° son una pendiente sostenida o un obstáculo puntual?** Si es puntual se supera con
   inercia y el criterio de no deslizamiento no aplica igual, lo que cambia el requisito de continuo.

Conviene hacer primero el ensayo de ángulo de deslizamiento (§4.2) y llevar ese número a la consulta.

---

## 5. Rover final — estado

### 5.1 Parámetros

| Parámetro | Valor |
|---|---|
| Tracción | Swerve de 4 ruedas. Por rueda: NEMA 23 para la dirección, motor de tracción a definir |
| Masa | 40 kg |
| Chasis | 0,8 × 0,4 m |
| Diámetro de rueda | **en conflicto** (ver 4.2) |
| Pendiente máxima | 30° declarados (ver 4.3) |
| μ | 0,4 declarado, pendiente de medición |
| Velocidad de rueda | ~63,7 rpm (~1 m/s) con R = 0,15 |
| Bus | 24 V DC |
| Presupuesto por motor | < 100 USD |

6WD y rocker-bogie descartados definitivamente.

### 5.2 Requisitos de motor vigentes

Por rueda, a la salida de la caja reductora: torque nominal 11–17 N·m, pico ≥ 18 N·m, velocidad
≥ 68 RPM, potencia eléctrica 83–120 W. Ningún motor de catálogo entrega 11 N·m directo: se compra
motorreductor o motor + reductora, relación típica 1/50 a 1/100.

Detalle completo en `10-locomocion/requisitos-motor`.

### 5.3 Presupuesto de corriente

| Escenario | Valor |
|---|---|
| Continuo de tracción al techo (4 ruedas, η 0,55) | 247 W eléctricos, **10,3 A a 24 V** |
| Cálculo viejo de la hoja de Mecánica | 480 W |

La diferencia: el cálculo viejo multiplicaba el torque de obstáculo por la velocidad máxima, y esas
dos condiciones nunca ocurren juntas. El obstáculo se trepa despacio.

**Falta:** consumo de brazo, cómputo, comunicaciones y ciencia. Sin eso no se cierra ni la batería
ni el PDB. Sin apuro.

### 5.4 Motor: sin candidato

El 42RBL04A quedó **descartado**: el proveedor nunca respondió y jamás se pudo confirmar la interfaz
del controlador integrado.

Queda huérfano el criterio de reducción 1/68 (~9,93 N·m continuos, motor al 71% del torque nominal),
que era específico de ese motor. Se conserva el **principio**, no el número: elegir la reducción que
deje el motor por debajo del 75% de su torque nominal en continuo, por margen térmico.

Criterio de búsqueda del reemplazo: proveedor con respuesta comprobada y hoja de datos que declare
**corriente de bloqueo a la tensión de bus**. Ese, y no la corriente nominal, es el dato que define el
driver.

---

## 6. Modelo a escala — estado

### 6.1 Escala vigente

| Símbolo | rev 1.0 | Vigente | Origen |
|---|---|---|---|
| λ | 0,34 | **0,371** | masa real de 5,5 kg |
| Masa del modelo | 4,6 kg (objetivo) | **5,5 kg** (real, pesado) | con electrónica y batería |
| Diámetro de rueda | 102 mm | **111 mm** | derivado de λ |
| Altura del CM | 118 mm | **129 mm** | derivado de λ |
| 1/λ² | 8,65 | 7,27 | término de compactación |
| 1/λ³ | 25,44 | 19,61 | término de peso |

λ = √(m_modelo / m_real). El margen **mejora** al engordar el modelo: el requisito de velocidad
angular cae con la masa a la −0,25, y el torque requerido pasa del 6,5% al 8,5% del torque de
bloqueo. Verificación térmica: 0,65 A contra un límite continuo conservador de 1,1–1,65 A.

**El problema no es el peso, es la rueda.** Con 5,5 kg y ruedas de 102 mm conviven dos escalas:
λ geométrico 0,34 contra λ másico 0,371. Son 19% de exceso de presión de contacto, que sesga el
término de compactación hacia arriba. Se corrige reimprimiendo las ruedas a 111 mm.

Y se acabó el lastre: el diámetro de rueda queda como única variable de ajuste.

### 6.2 Motor

CQRobot CQR37D12V64EN-I ×4. 37D, 12 V, 90:1, 120 rpm sin carga, τ_stall 3,138 N·m, I_stall 5,5 A,
I₀ 0,2 A, encoder 64 CPR en el eje del motor → 5760 CPR en el eje de salida, ~22 USD.

### 6.3 Configuración

**Swerve, con un servo de dirección por rueda.** Ya no es skid-steer. Con eso el modelo sí valida
la cinemática swerve, que era su laguna principal.

### 6.4 Qué valida y qué no

| Valida | No valida |
|---|---|
| Tracción y torque real en dos superficies | Cualquier magnitud eléctrica o térmica |
| Cinemática swerve | Rendimiento del tren motriz real |
| Control de velocidad por rueda | Constante térmica del motor grande |
| Firmware y telemetría | Corriente de arranque, ondulación, EMI |
| μ real por ángulo de deslizamiento | Cargas de rodamiento del cubo full-scale |

Regla práctica: **si la magnitud tiene unidades eléctricas o térmicas, el modelo no la dice.**

### 6.5 Estado de la PCB

**Fabricada y soldada.** Validación parcial con resultado correcto: se probó un encoder y un puente H
de cada unidad y funcionaron. El testeo completo está en curso.

Hay **12 puntos de verificación abiertos (V1 a V12)**, que son análisis de escritorio pendientes de
confirmar midiendo, no defectos comprobados. Viven en `30-pcb/pendientes-de-verificacion`.

Los tres de mayor consecuencia:

| # | Qué | Por qué importa |
|---|---|---|
| V1 | Nivel de R_PWM_4 durante el arranque del ESP32 | Si el análisis es correcto, el motor 4 gira solo hasta que el firmware toma el control |
| V2 | Disipación de Q1 (IRF9540N, 117 mΩ) | A 5 A son 2,9 W y 181 °C de salto sin disipador |
| V3 | Margen del riel de servos con el LM2596 compartido | Cuatro servos trabados pueden resetear el ESP32 por brownout |

### 6.6 Cambios confirmados respecto del PDF de planificación

- ACS712-30A cancelado → **TMCS1126B4** (Hall aislado, 100 mV/A, ±15,5 A).
- INA226 descartado (no cubre el pico de 5,5 A; no se consiguen módulos de más de 5 A en Argentina).
- Pines IS de los BTS7960 sin cablear: no hay medición por rueda.
- Pinout del ESP32 cambiado.
- Los pull-down de 10 k en las 8 líneas PWM no existen: se reemplazaron por conmutadores analógicos
  NLAS4157 en los pines de strapping.
- Cuatro PTC de servo → uno solo para todo el riel.
- MOSFET de polaridad inversa: IRF9540N, no IRF4905.

---

## 7. Instrumentación de ensayos

### 7.1 Decisiones cerradas

- **ADS1115** (16 bits, I2C) para leer el TMCS1126. El ADC interno del ESP32 es el eslabón débil de
  la cadena: 2–3% de error y **no lineal**, que es justamente lo único que no se cancela.
- **Tiro en barra como método primario**, corriente como canal redundante, medidos en simultáneo. Si
  coinciden dentro del 15%, hay validación cruzada que ningún ensayo individual da.
- **Sensor por motor: no hace falta.** El error de ganancia se cancela solo si calibración y ensayo
  usan la misma cadena de medición.

### 7.2 Lo que sí se cancela y lo que no

Se cancela: el error de ganancia. Si el sensor tiene 4% de error, K sale con 4% en sentido contrario
y los dos se anulan en el Ensayo D.

No se cancela: la **linealidad**, el **offset de cero** (deriva térmica, fuente de error dominante en
un Hall — tarar al empezar cada sesión) y el **punto de operación** (calibrar en el rango donde se va
a medir).

### 7.3 Qué mide realmente el sensor de a bordo

**Resuelto:** el TMCS1126 está en la rama conmutada, entre el contacto NO del relé y el riel de
+12 V que alimenta a los cuatro drivers. Lo que lee es **la corriente de tracción**: los cuatro
motores más el VCC lógico de los cuatro BTS7960. Los servos y el ESP32 cuelgan del nodo A, aguas
arriba del relé, y no lo atraviesan.

Lo confirma la consecuencia arquitectónica documentada: al apretar la seta muere el bus de los
drivers pero los servos y el ESP32 siguen alimentados. Si los servos estuvieran en la rama medida,
caerían con la seta.

**El único término parásito es el VCC lógico de los cuatro drivers**, estimado en 10–30 mA. Es
constante y se resta como línea de base: se lee con el bus energizado y los motores parados, en el
mismo tarado de cero que ya hay que hacer cada sesión por la deriva del Hall.

Consecuencias:

| Afirmación de `metodologia` §3 | Estado |
|---|---|
| El sensor mide el total de la PDB | **Incorrecta.** Mide solo tracción |
| El Ensayo D queda invalidado | **No.** El Ensayo D es válido con el sensor de a bordo |
| Opción B: cortar pista y reinsertar el sensor | **Innecesaria.** Ya está donde corresponde |
| Opción C: instrumento externo obligatorio | **Degradada a opcional.** Ver abajo |
| Opción A: Ensayo A fuera de la PDB | **Sigue vigente**, por su propio motivo: el motor se alimenta directo desde la fuente en continua pura |

### 7.4 Instrumento externo: sigue siendo buena idea, pero ya no es urgente

Deja de ser una corrección obligatoria y pasa a ser una mejora de calidad de ensayo. Las razones que
lo justifican por sí mismas siguen todas en pie:

- **Aísla el muestreo del lazo de control.** El ESP32 está ocupado con cuatro canales de conteo,
  la modulación y el lazo de velocidad. Sumarle el muestreo promediado compite por el mismo bus I2C
  y por tiempo de CPU, justo durante el ensayo.
- **Es equipo de banco reutilizable**, no parte del rover: sirve después para caracterizar la PDB del
  rover real o cualquier rama que se quiera instrumentar.
- **No compromete una placa ya fabricada y soldada.**

Si se hace: Hall aislado + ADS1115 + RP2040 + registro por USB, conectores XT, 100–500 muestras/s.
**El único punto no trivial es la sincronización** con los encoders del ESP32 principal: línea
digital compartida de arranque, resuelta antes de la primera corrida y no después.

Alternativa más barata que resuelve el mismo problema de contención: mantener el sensor de a bordo y
subir solo el conversor a ADS1115, aceptando que el muestreo comparta CPU con el lazo. Para un ensayo
de treinta segundos a 100 muestras por segundo probablemente alcance. Vale medirlo antes de gastar
15–20 USD.

### 7.5 Lo que no cambia

Independiente de dónde esté el sensor:

- **Satura alrededor de 15,5 A** contra 22 A de bloqueo de las cuatro ruedas. No sirve como
  protección por sobrecorriente. Son dos funciones distintas y el sensor cubre una.
- **Falta confirmar la sensibilidad de la variante montada.** La familia va de 50 a 400 mV/A, y eso
  cambia el fondo de escala por un factor 8.

---

## 8. Principios transversales

Reglas ganadas en el proyecto que aplican más allá del caso que las originó.

1. **La tracción es la restricción vinculante, no el par del motor.** Si el modelo predice más torque
   que el techo de tracción, no significa que hagan falta motores más grandes: significa que el rover
   patina antes de poder aplicarlo.
2. **El error en λ es cúbico:** ΔT/T ≈ 3·Δλ/λ.
3. **El pico de corriente de la PDB no ocurre trepando el obstáculo.** Trepar pide mucho torque a
   velocidad baja, o sea ciclo de trabajo bajo, o sea poca corriente de bus. El peor caso de bus es
   rueda trabada con el lazo pidiendo velocidad alta.
4. **I_bus ≈ D · I_motor, pero el cable driver-motor ve la corriente plena.** Dos dimensionamientos
   distintos sobre la misma rama.
5. **La corriente como proxy de torque exige continua sin modulación** en el banco de calibración.
6. **Los pull internos del ESP32 no sirven para nada crítico:** dispersión de 30–80 kΩ, ausentes
   durante los 100–300 ms del arranque, inexistentes en IO34–IO39.
7. **Los capacitores de bulk tienen que estar en la carga**, no en el PDB, si hay cable en el medio.
   La inductancia del cable (~1 µH/m) domina a frecuencia de conmutación.
8. **El standoff del TVS se elige contra la tensión máxima real de bus**, incluyendo carga.
9. **Los contactos de relé se sueldan por inrush.** El síntoma es que la seta deja de cortar y nadie
   se entera hasta que hace falta.
10. **El fusible protege el cableado, no la carga.** Y el conector también tiene un límite: la cadena
    conector-fusible-cable tiene que ser coherente.
11. **El dato que define el driver es la corriente de bloqueo a la tensión de bus**, no la nominal.
12. **Un pull-down del lado del GPIO no protege contra un cable desconectado.** Va del lado del
    driver.
13. **La repetibilidad del terreno domina sobre la precisión del instrumento.** El estado de
    compactación de la arena varía más entre corridas que cualquier error de medición.

---

## 9. Próximos pasos

En orden de valor por esfuerzo.

| # | Acción | Costo | Desbloquea | Bloqueado por |
|---|---|---|---|---|
| 1 | **Ensayo de ángulo de deslizamiento** en piso duro | 0 | El μ real, que hoy bloquea la elección del motor grande | nada |
| 2 | **Consulta única a Mecánica** (§4.4): diámetro, μ, naturaleza de los 30° | 1 mensaje | λ, requisitos de torque, reimpresión de ruedas | conviene hacer #1 antes |
| 3 | **Pesar el modelo completo** con todo montado | 0 | λ definitivo | nada |
| 4 | **Ensayo A en banco**, fuera de la PDB, un motor, continua, multímetro en serie | 0 | K y calibración del sensor en la misma sesión | nada |
| 5 | Cerrar V1, V2, V3 con osciloscopio | 0 | Seguridad de la placa | nada |
| 6 | Verificar la sensibilidad real del TMCS montado con multímetro | 0 | Fondo de escala del sensor | nada |
| 7 | Comprar ADS1115 | 3 USD | Cadena de medición | nada |
| 8 | **Reimprimir ruedas** al diámetro correcto, y medirlas con calibre | — | Cierra la escala | **#2 y #3** |
| 9 | Medir altura del CM y comparar con 0,3464·λ | 0 | Validez de conclusiones de estabilidad | #8 |
| 10 | Decidir si hace falta instrumento externo (§7.4) | 0 a 20 USD | Ensayo D | #4 |
| 11 | Comprar balanza de gancho | 12 USD | Tiro en barra | nada |
| 12 | Buscar motor de reemplazo del 42RBL04A | — | Rover final | #1 y #2 |
| 13 | Presupuesto de corriente de los otros subsistemas | — | Batería y PDB del rover final | otros subsistemas |

Los siete primeros no dependen de nadie más y cuestan entre cero y tres dólares. Los pasos 8 y 12,
que son los que cuestan plata o tiempo de impresión, dependen de que Mecánica responda.

---

## 10. Mapa del repositorio

| Carpeta | Qué contiene |
|---|---|
| `00-contexto/` | Este documento, alcance, filosofía, decisiones descartadas, conflictos |
| `10-rover-final/` | Requisitos de motor, presupuesto de corriente, potencia, control |
| `20-modelo-escala/` | Escala, electrónica, PCB, firmware, ensayos |
| `30-componentes/` | Ficha por componente y enlaces a hojas de datos |
| `40-proveedores-costos/` | Proveedores y costos de importación |
| `90-archivo/` | Versiones superadas y ramas muertas |

---

## 11. Convenciones

- Contenido en español. Nombres de archivo en minúscula, sin tildes ni espacios, con guiones.
- Carpetas numeradas para forzar orden de lectura.
- Frontmatter YAML con `titulo`, `estado`, `fecha` en cada archivo.
- Enlaces markdown relativos, no wikilinks.
- 150–400 líneas por archivo.
- **Los archivos de KiCad no se versionan.** Se procesan en chat y el resultado se documenta acá.
- Registro de decisiones = tabla de descartados, no ADR formales.
- Dentro de tablas, decimales en texto plano con coma. LaTeX solo fuera de tablas.
