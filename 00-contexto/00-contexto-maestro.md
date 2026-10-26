---
titulo: Contexto maestro — Rover ERC, subsistema de Electrónica y Potencia
owner: Matías Bélico
version: 2.3
estado: vigente
fecha: 2026-10-26
fuente: 00-contexto-maestro.md (fuente original en 90-archivo/fuentes-originales/), reducido a índice de estado
alcance: subsistema de potencia y electrónica
proposito: documento raíz del repo. Estado, conflictos abiertos y próximos pasos. El detalle técnico vive en los archivos que este índice referencia.
---

# Contexto maestro — Rover ERC

Este es el documento raíz. **No tiene datos técnicos propios, salvo los próximos pasos (§7).** Si un
número aparece acá y en un archivo de detalle, manda el detalle. Acá está el estado, lo que todavía no
cierra y dónde buscar.

---

## 1. Qué es el proyecto

**Competencia:** European Rover Challenge (ERC).
**Equipo:** IEEE ITBA I+D.
**Subsistema:** Electrónica y Potencia.

Dos frentes en paralelo:

| Frente | Propósito | Estado |
|---|---|---|
| **Rover final** | El vehículo que compite. 40 kg, bus de 24 V, swerve de 4 ruedas. | Arquitectura mecánica cerrada. Sin motor elegido. |
| **Modelo a escala** | Instrumento de medición, no maqueta. Mide torque real en piso duro y arena y extrapola al rover final. | PCB fabricada y soldada, con validación parcial. Electrónica sin montar en el modelo. Sin ensayos. |

El modelo existe para cerrar la especificación de motores sin comprar motores de tamaño real a
ciegas. Esa es su única razón de ser, y todo lo que se decide sobre él se juzga contra eso.

---

## 2. Marco

| Tema | Resumen | Detalle |
|---|---|---|
| Alcance | Batería, potencia, control de motores, protecciones, sensado, firmware y telemetría. Mecánica entra solo como interfaz; λ entra como dato | [01-alcance-y-fronteras.md](01-alcance-y-fronteras.md) |
| Reglas no negociables | COTS primero, star grounding, silicona AWG, conectores con traba, E-Stop y fusibles | [02-filosofia-de-diseno.md](02-filosofia-de-diseno.md) |
| Descartados | Qué se evaluó y no quedó, con el motivo | [04-decisiones-descartadas.md](04-decisiones-descartadas.md) |
| Principios transversales | Catorce reglas ganadas que aplican más allá de su caso | [05-principios-transversales.md](05-principios-transversales.md) |

---

## 3. Conflictos abiertos

Tres puntos bloquean decisiones hoy. Ninguno se resuelve escribiendo: se resuelven midiendo o
preguntando.

> **Resuelto (16-sep): qué mide el TMCS1126.** Mide **solo la rama de tracción**. La afirmación de
> que medía el total de la PDB quedó superada. El Ensayo D es válido con el sensor de a bordo.

| # | Conflicto | Qué bloquea | Cómo se cierra |
|---|---|---|---|
| 1 | Diámetro de rueda del rover real: circulan tres valores | Cálculos de torque y reimpresión de ruedas del modelo | Consulta a Mecánica |
| 2 | μ / pendiente: el μ declarado no alcanza para 30° | Elección del motor grande | Ensayo de ángulo de deslizamiento |
| 3 | Torque pedido inconsistente con el techo de tracción | Dimensionamiento continuo contra transitorio | Ya tiene lectura propuesta |

Desarrollo y consulta consolidada a Mecánica en
[03-conflictos-abiertos.md](03-conflictos-abiertos.md).

---

## 4. Rover final — estado

| Tema | Estado | Detalle |
|---|---|---|
| Parámetros y requisitos de motor | Vigentes pero sujetos a los conflictos 1 a 3 | [requisitos-motor.md](../10-rover-final/10-locomocion/requisitos-motor.md) |
| Motor | **Sin candidato.** El último quedó descartado | [candidatos-motores.md](../10-rover-final/10-locomocion/candidatos-motores.md) |
| Presupuesto de corriente | Tracción calculada. Falta brazo, cómputo, comunicaciones y ciencia | [presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md) |
| Cableado y protecciones | Preliminar, bloqueado por la elección del motor | [cableado-y-protecciones.md](../10-rover-final/20-potencia/cableado-y-protecciones.md) |

---

## 5. Modelo a escala — estado

| Tema | Estado | Detalle |
|---|---|---|
| Escala, motor y qué valida el modelo | **Abierta:** las ruedas miden 150 mm y no corresponden a ~5,5 kg. Decidir reimprimir (recomendado) o lastrar | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) §0 |
| PCB | Fabricada y soldada. Los 4 canales de puente H validados; un módulo BTS7960 falló y está sin revisar. Dos LM2596, cables de potencia soldados, mazos con traba casi terminados, ADS1115 y ACS712 montados sin probar. **Retroalimentación desde el USB del ESP32 (23-oct)**: corrección con Schottky planificada | [00-estado-actual.md](../20-modelo-escala/00-estado-actual.md) |
| Puntos V1 a V12 | Hipótesis de escritorio pendientes de medir, no defectos. V1 y V3 mitigados sin verificar; V2, V4 y V10 no aplican | [pendientes-de-verificacion.md](../20-modelo-escala/30-pcb/pendientes-de-verificacion.md) |
| Ensayos | Plan rev. 2.0 (reemplaza al protocolo rev. 1.0). Ninguno corrido | [plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md) |
| Pinout | Fuente única de verdad | [mapa-gpio.md](../20-modelo-escala/40-firmware/mapa-gpio.md) |

---

## 6. Instrumentación de ensayos — decisiones cerradas

- **ADS1115** para leer el TMCS1126: el conversor interno del ESP32 es el eslabón débil. **Montado
  fijo a la placa (8-oct), sin probar.** El TMCS quedó confirmado como B4 (100 mV/A).
- **ACS712 a bordo en la pata de motor de una rueda delantera**, leído por el mismo ADS1115: respaldo
  y validación. No es un sensor por motor para la extrapolación. **Cableado (8-oct), sin probar.**
- **Corrección por ciclo de trabajo:** con PWM el TMCS mide corriente de batería (≈ D · I_motor). Se
  registra D por rueda y se corrige.
- **Tiro en barra como método primario**, corriente como canal redundante, en simultáneo. Coinciden
  en piso duro; en arena la diferencia es la resistencia de compactación. Balanza de gancho: la presta
  la facultad.
- **Instrumento externo:** no se construye por ahora.
- **Sin convertidor regulado:** batería de plomo directo al bus, lazo cerrado de velocidad y registro
  de tensión y D.

Detalle en [instrumentacion.md](../20-modelo-escala/50-ensayos/instrumentacion.md) y
[sensado-corriente.md](../20-modelo-escala/20-electronica/sensado-corriente.md).

---

## 7. Próximos pasos

**Prioridad actual (8-oct-2026): terminar la electrónica del modelo.** El modelo lo tiene Mecánica,
que le está agregando cosas. Todo lo que pide medir sobre el modelo (peso, ensayos, ruedas) queda en
prioridad mínima hasta que vuelva.

### 7.1 Próxima reunión de laboratorio

El orden importa: nada se conecta a un riel sin medirlo antes, y ningún motor se mueve antes de V1,
la seta y los fusibles. Del #6 en adelante se puede trabajar en paralelo.

| # | Acción | Cómo | Cierra o desbloquea |
|---|---|---|---|
| 1 | **Revisar los dos LM2596** | Fuente de banco limitada a ~300 mA, con la salida desconectada. Sin carga: pocos mA de consumo y la salida en lo ajustado. Con carga (~22 Ω en el de 5 V): regulación y rizado. **Si la salida copia la entrada, el módulo está muerto y no se conecta.** Comprar un repuesto igual | Que no lleguen 12 V al ESP32 ni a los servos |
| 2 | **Corregir la retroalimentación del USB** | Confirmar el camino midiendo el nodo A (≤ 5 s, USB enchufado y sin batería). Schottky entre U2 y VIN; U2 a ~5,4 V; VIN en ~5,0 V. Medir en modo diodo si el devkit tiene diodo en VBUS. Verificar: USB sin batería, nodo A en 0 V y módulos fríos | Programar sin riesgo |
| 3 | **V1** | Osciloscopio disparado en EN, R_PWM_4 y L_PWM_4 en J19 sin potencia en el driver. Después, con el motor conectado, confirmar que no se mueve al encender | V1 |
| 4 | **Seta** | Apretarla y confirmar que muere el bus de drivers | V7 |
| 5 | **Fusibles** | F3..F6 de 7,5 A retardados y fusible general junto al XT60. Antes, medir con calibre el conductor del cable de ~1,5 mm y el de batería: los valores dependen de eso ([mapa-conectores](../20-modelo-escala/30-pcb/mapa-conectores.md)) | Condición para energizar la tracción |
| 6 | **Rieles con motores y servos andando** | Osciloscopio con masa de resorte corta y disparo en el arranque. Bus de 12 V en los drivers, riel de servos, 5 V y 3,3 V. Sobrepicos y caídas en el arranque y la inversión, rizado de conmutación | V3; validación de los rieles |
| 7 | **ACS712 por separado** | Puente H y motor en banco. Cero y respuesta contra el multímetro en serie. Cuando funcione, sumar el ADS1115 con [test-ads1115.ino](../20-modelo-escala/40-firmware/test-ads1115/test-ads1115.ino). Ninguna entrada del ADS por encima de 3,3 V. El protoboard sirve solo para esta prueba | Cadena de medición |
| 8 | **TMCS1126: código de lectura** | I = (V_reposo − V) / 0,1; tarar el cero en cada sesión. Después se prueba todo junto en la placa | V9, V11, calibración |
| 9 | **Cables** | Largos y punteras. Los cables a los motores, la correspondencia y la rotulación quedan para cuando Mecánica devuelva los motores | Montaje en el modelo |
| 10 | **Módulo BTS7960 que falló** | En un canal que ya anduvo, con fuente limitada en corriente y cada sentido por separado | Reparar o comprar repuesto |
| 11 | **Encoders con PCNT, por separado** | Los tres que faltan; el riel ya está probado. Girar a mano: 5760 cuentas por vuelta de rueda y sentido de cuenta. Con el motor girando, cuentas falsas | Encoders validados |

Códigos: [test-ads1115.ino](../20-modelo-escala/40-firmware/test-ads1115/test-ads1115.ino) para el #7 y
[test-mpu6050.ino](../20-modelo-escala/40-firmware/test-mpu6050/test-mpu6050.ino). Faltan el de
lectura del TMCS (#8) y el de prueba del PCNT (#11).

### 7.2 Después, en prioridad baja

| # | Acción | Desbloquea | Bloqueado por |
|---|---|---|---|
| 1 | **Ensayo 0: ángulo de deslizamiento** en piso duro | El μ real, que bloquea el motor grande, y el ángulo del Ensayo D | superficie y rampa (Mecánica) |
| 2 | **Consulta única a Mecánica**: diámetro, μ, si los 30° son sostenidos, pedidos de ensayo | λ, requisitos de torque, decisión de ruedas | conviene hacer #1 antes |
| 3 | **Decidir rueda o lastre** (ruedas de 150 mm) | Escala del modelo, Ensayo D | reunión de líderes, #2 |
| 4 | **Pesar el modelo completo** | λ definitivo | que Mecánica devuelva el modelo |
| 5 | **Ensayo A** en los 4 motores, con ACS712 y multímetro en serie | K, I₀ y R | 7.1 #7 y #8 |
| 6 | **Ensayo A'**, B y C | Corrección por D, K verificada, encoders | #5 |
| 7 | **D0**, ensayo de método en piso duro | Método del Ensayo D | #1, #6 |
| 8 | **Reimprimir ruedas** y medirlas con calibre | Cierra la escala | **#2, #3 y #4** |
| 9 | Altura del CM contra 0,3464·λ | Conclusiones de estabilidad | #8 |
| 10 | Motor de reemplazo del 42RBL04A | Rover final | #1 y #2 |
| 11 | Presupuesto de corriente de los otros subsistemas | Batería y PDB del rover final | otros subsistemas |

Hechos desde la versión anterior (8-oct → 23-oct): mazos con traba casi terminados, con cable de
~1,5 mm y XT60 con fusible en la entrada de batería. Problema encontrado: retroalimentación desde el
USB del ESP32, con los LM2596 calentando; corrección planificada. Sin hacer por falta de tiempo:
BTS7960, rieles, seta, V1, sensores de corriente y PCNT.

Plan de ensayos del modelo en
[plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md). Versión para compartir con
Mecánica: `Plan_Ensayos_Modelo_Rover_ERC_rev2.docx` en `Desktop\Rover` (fuera del repo).

---

## 8. Mapa del repositorio

| Carpeta | Qué contiene |
|---|---|
| `00-contexto/` | Este documento, alcance, filosofía, conflictos, descartados, principios |
| `10-rover-final/` | Requisitos de motor, candidatos, presupuesto de corriente, cableado |
| `20-modelo-escala/` | Escala, electrónica, PCB, firmware, ensayos |
| `30-componentes/` | Ficha por componente y enlaces a hojas de datos |
| `40-proveedores-costos/` | Proveedores y costos de importación |
| `90-archivo/` | Versiones superadas y ramas muertas. **No usar como fuente** |

Índice archivo por archivo y convenciones en el [README del repo](../README.md).
