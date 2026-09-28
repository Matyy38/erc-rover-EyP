---
titulo: Plan de ensayos del modelo a escala
estado: vigente
fecha: 2026-09-27
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §6, §3.2 opción A y §2.7; protocolo de ensayos rev. 1.0 (jul-2026), corregido en rev. 2.0
---

# Plan de ensayos

Qué se ensaya, en qué orden y por qué. Los instrumentos y la cadena de medición están en
[instrumentacion.md](instrumentacion.md).

**Versión para compartir:** `Plan_Ensayos_Modelo_Rover_ERC_rev2.docx` (rev. 2.0, 28-sep-2026),
en la carpeta `Desktop\Rover`, fuera del repo. Tiene los procedimientos completos, las planillas de
registro y la tabla de fallas típicas. Reemplaza al protocolo rev. 1.0 (jul-2026). **Si el docx y
este archivo no coinciden, manda este archivo.**

---

## Secuencia

| Fase | Ensayo | Qué entrega | Depende de |
|---|---|---|---|
| 0 | Preparación y seguridad | Placa segura para energizar la tracción. Firmware de registro | nada |
| 1 | **Ensayo 0: ángulo de deslizamiento** | μ real. Fija el ángulo del Ensayo D | superficie representativa y rampa (Mecánica) |
| 2 | Calibración de sensores | Ganancia y cero de TMCS y ACS712 contra multímetro. Cierra V11 y V9 | fase 0 |
| 3 | **Ensayo A**: constante de torque | K, I₀ y R de cada motor | fase 2 |
| 4 | **Ensayo A'**: cadena bajo modulación | Valida I_batería ≈ D · I_motor | fase 3 |
| 5 | Ensayo B: fuerza contraelectromotriz | Verificación cruzada de K | fase 3 |
| 6 | Ensayo C: encoders | Los 4 canales validados (hoy hay uno) | fase 0 |
| 7 | **D0**: ensayo de método en piso duro | Método del D depurado. **No se usa para extrapolar** | fases 1 a 4 |
| 8 | **Ensayo D definitivo** | Torque del rover real | escala cerrada, modelo pesado, respuesta de Mecánica |

Las fases 0 a 7 no dependen de la escala. Solo el D definitivo queda bloqueado por la decisión de
rueda o lastre ([10-metodologia-similitud.md](../10-metodologia-similitud.md) §0) y por la consulta a
Mecánica.

---

## Fase 0 — preparación y seguridad

Condiciones para energizar la tracción:

- Seta verificada: con la seta apretada, circuito abierto entre nodo A y riel conmutado (V7).
- Fusibles F3..F6 y general instalados con valor asentado. Hoy no tienen valor en el
  [BOM](../30-pcb/bom.md); depende de V4.
- **V1 verificado con osciloscopio** tras el pull-up en L_PWM_4: las dos entradas del driver 4 en
  alto durante todo el arranque, y el motor 4 quieto al encender.
- Correspondencia driver ↔ rueda ↔ servo ↔ encoder rotulada en mazo y firmware.
- Dupont de fábrica reemplazados, 3 × 100 nF soldados en cada motor, mazos de potencia y señal
  separados.
- Firmware de registro: marca de tiempo, cuentas de encoder acumuladas en 32 bits (el PCNT desborda
  cada 2,8 s), **D comandado por rueda**, V_bat, canales del ADS1115, ángulo del MPU6050, causa del
  último reinicio (para observar V3). Salida por Wi-Fi: el cable USB tira del modelo en el tiro en
  barra.

## Ensayo 0 — ángulo de deslizamiento, primero y gratis

El conflicto abierto con Mecánica (μ = 0,4 contra 30°) se cierra empíricamente: se sube la rampa
hasta que el modelo desliza, y en ese ángulo

$$\mu = \tan\theta_{deslizamiento}$$

El coeficiente de fricción es **adimensional**: no escala, no tiene λ, no tiene error de
extrapolación. El valor medido en el modelo es directamente el del rover real, siempre que rueda y
superficie sean del mismo material.

Procedimiento: ruedas **trabadas mecánicamente** (el motor en cortocircuito solo frena en
movimiento), servos al centro, rampa subida a ~1°/s, ángulo con inclinómetro y con el MPU6050,
cinco repeticiones por orientación (trompa arriba y cola arriba).

Lectura: por debajo de 21,8° el μ es peor que el 0,4 de Mecánica; entre 21,8° y 30° el rover no se
sostiene en 30° por fricción con ningún motor; desde 30° la pendiente es sostenible. ±1° de error da
±4 a 6 % en μ.

**El resultado fija el ángulo del Ensayo D:** se ensaya unos 5° por debajo del ángulo de
deslizamiento (criterio empírico).

Cierra el conflicto 2 de
[00-contexto/03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md). Conviene
hacerlo **antes** de la consulta a Mecánica: se les lleva un número medido para confirmar o
discutir.

## Calibración de sensores

- **TMCS1126:** bus a tensión nominal, carga resistiva conocida (lámparas de auto de 12 V o
  resistencias de potencia) en la salida de un driver al 100 % de ciclo, que es continua pura.
  Multímetro en serie, 5 puntos entre 0 y ~9 A. Confirma 100 mV/A, el signo (V11) y la tensión de
  VOC (V9). Se lee el mismo punto por IO33 y por el ADS1115 para cuantificar la mejora.
- **ACS712:** fuente de banco, carga resistiva, multímetro en serie, 5 puntos entre 0 y 5 A **en los
  dos sentidos**. Esperado 66 mV/A y cero en la mitad de su alimentación.
- Aceptación: apartamiento de la recta < 1 % del fondo usado, cero estable ±20 mA en 10 minutos.

## Ensayo A — constante de torque, fuera de la PDB

La calibración de K exige continua pura sin modulación, y eso solo se consigue alimentando el motor
directo desde la fuente de banco, fuera de la PDB. Brazo sobre el eje de salida apoyado en balanza,
de 1 a 8 V, pulsos de 3 s como máximo y 60 s de enfriamiento. Se obtienen K, I₀ y **R** (pendiente
de tensión contra corriente, que se usa en el Ensayo B).

**El TMCS no participa de este ensayo.** Con la fuente entre 1 y 8 V no arrancan ni el LM2596 ni la
bobina del relé, así que el sensor de placa no ve el motor. En serie con el motor van el
**multímetro** (referencia) y el **ACS712**, que queda en la misma posición en el Ensayo D: su error
de ganancia se cancela para esa rueda. La pata del motor se desconecta del driver y se alimenta
desde la fuente; con la seta apretada la tracción queda muerta y la lógica viva para registrar.

Aceptación: K = 0,592 N·m/A ±20 %, I₀ bloqueado entre 0,2 y 0,5 A, R = 2,18 Ω ±20 %, R² > 0,99,
dispersión de K entre motores < 15 %.

El análisis superado del que nació esta opción está en
[90-archivo/analisis-sensado-superado.md](../../90-archivo/analisis-sensado-superado.md).

## Ensayo A' — cadena bajo modulación (nuevo)

Mismo montaje del A, pero con el motor conectado a su driver y el bus en batería. Ciclos de 10, 20,
30 y 40 % con el motor bloqueado. Se comparan ACS712 (corriente de motor), TMCS (corriente de
batería) y balanza.

| Comparación | Criterio |
|---|---|
| I_TMCS / D contra ACS712 | dentro del 5 % |
| Torque de la balanza contra K · (I_ACS − I₀) | dentro del 5 % |

Si falla la primera, la corrección por D no alcanza y el canal de torque por rueda pasa a ser el
ACS712. Si falla la segunda, la calibración en continua no vale bajo PWM.

## Ensayo B — fuerza contraelectromotriz

Motor libre a 6, 9 y 12 V; E = V − I·R con **la R medida en el Ensayo A**, no la de hoja. η
despejado entre 0,55 y 0,70 da la calibración por coherente.

## Ensayo C — encoders

Diez vueltas a mano = 57 600 cuentas ±20, en los cuatro canales, después con el motor girando con
carga. Si aparecen cuentas espurias, primero el filtro de glitch del PCNT, después pull-ups de 2,2 k,
último recurso RC de 2,2 k + 1 nF. **No usar 10 nF** ([encoders-pcnt.md](../20-electronica/encoders-pcnt.md)).

## D0 — ensayo de método en piso duro

Con las ruedas actuales, antes de cerrar la escala. Sus números **no** se extrapolan. Verifica:

| Verificación | Criterio |
|---|---|
| Torque por barra contra torque por corriente corregido por D | dentro del 15 % |
| TMCS / (4 · D) contra ACS712, en plano | dentro del 10 % |
| Umbral a dos velocidades (por ejemplo 60 y 90 rpm) | cambia menos del 5 % |
| Repetibilidad, 3 corridas | dispersión < 10 % |
| Muestras perdidas, reinicios, caídas de 3,3 V | ninguno (observación de V3) |

El criterio de dos velocidades es el que justifica correr con la batería de plomo sin regular:
con lazo cerrado de velocidad el firmware compensa la caída subiendo D mientras quede margen. Se
arranca cada serie con la batería en reposo ≥ 12,6 V y se descartan corridas con D saturado al
100 %. Es una convención de ensayo, no un dato: se valida en el D0.

## Ensayo D definitivo

Piso duro y arena el mismo día, misma batería, mismo ángulo θ_D, tres corridas por superficie
como mínimo. Tiro en barra (anclaje abajo, T = (F + m·g·sen θ)·R) y corriente en simultáneo.
Torque umbral: el torque cuando el deslizamiento supera el 20 %, convertido con el K de cada
motor y corregido por D. La definición operativa se confirma con Mecánica (su informe §1.8).

En arena la barra y la corriente **no** tienen que coincidir: la diferencia es la resistencia de
compactación (ver [instrumentacion.md](instrumentacion.md)).

$$T_{real} = \frac{T_{duro}}{\lambda^3} + \frac{T_{arena} - T_{duro}}{\lambda^2}$$

Condiciones previas: escala cerrada, modelo pesado con todo montado, diámetro de rueda medido con
calibre, altura del CM medida contra 0,346 m · λ, Ensayos 0, A, A' y D0 aprobados.

Corriente esperada en la subida en piso duro (casi estático, sin rodadura), por rueda y total:

| θ | 150 mm, 5,5 kg (actual) | 111 mm, 5,5 kg (reimpresión) | 150 mm, 10 kg (lastre) |
|---|---|---|---|
| 15° | 0,64 A · 2,6 A | 0,53 A · 2,1 A | 1,00 A · 4,0 A |
| 20° | 0,78 A · 3,1 A | 0,63 A · 2,5 A | 1,26 A · 5,1 A |
| 25° | 0,92 A · 3,7 A | 0,73 A · 2,9 A | 1,51 A · 6,1 A |
| 30° | 1,05 A · 4,2 A | 0,83 A · 3,3 A | 1,75 A · 7,0 A |

Todo dentro del rango lineal del TMCS (±15,5 A).

---

## Qué se le pide a Mecánica

Además de las tres preguntas de
[03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md) §4:

1. Decisión sobre las ruedas del modelo: reimprimir o lastrar
   ([10-metodologia-similitud.md](../10-metodologia-similitud.md) §0).
2. Material y dibujo de la rueda real, y que el modelo use el mismo.
3. Superficie de piso duro representativa del terreno ERC.
4. Rampa inclinable de 0 a 40° con traba de ángulo.
5. Anclaje rígido para el tiro en barra.
6. Lecho de arena: al menos tres largos del modelo; ~10 cm de profundidad como punto de partida
   (criterio empírico).
7. Definición operativa del «torque umbral» de su informe §1.8.
8. Dónde se puede agregar o mover masa en el modelo.

---

## Próximos pasos

1. **Decidir rueda o lastre** en la reunión de líderes (recomendado: reimprimir). Bloquea el D
   definitivo.
2. **Pesar el modelo completo**, con batería, electrónica, cableado y sensores nuevos montados.
3. **Ensayo 0** en piso duro, en cuanto haya superficie y rampa. Es el de mayor valor por esfuerzo.
4. **Cablear el ADS1115 y el ACS712** ([instrumentacion.md](instrumentacion.md)) y calibrar.
5. **Verificar V1** con osciloscopio.
6. **Ensayo A** en los cuatro motores, después A', B y C.
7. **D0** en piso duro.
8. **Reimprimir ruedas** cuando Mecánica confirme el diámetro real y el modelo esté pesado. Medirlas
   con calibre y recalcular λ con el valor medido. **No mandar a imprimir antes.**

El orden completo del subsistema, con costos y dependencias, está en el
[contexto maestro](../../00-contexto/00-contexto-maestro.md) §7.
