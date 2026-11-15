---
titulo: Contexto maestro — Rover ERC, subsistema de Electrónica y Potencia
owner: Matías Bélico
version: 2.4
estado: vigente
fecha: 2026-11-15
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
| **Modelo a escala** | Instrumento de medición, no maqueta. Mide torque real en piso duro y arena y extrapola al rover final. | PCB fabricada y soldada; motores, encoders, MPU y seta andando. Posicionamiento de la electrónica definido, sin montar en el modelo. Sin ensayos. |

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
| **Reglas de código** (13-nov) | Todo código nuevo o modificado: bien comentado y detallado; simple antes que corto, sin estructuras difíciles de leer; sin sobreingeniería; escrito para el nivel del equipo (Arduino básico); modular y fácil de diagnosticar por el monitor serie | [reglas-de-codigo.md](../20-modelo-escala/40-firmware/reglas-de-codigo.md) |

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
| PCB | Fabricada y soldada. **Al 15-nov:** los 4 motores andan con realimentación por PCNT; el BTS7960 que falló quedó en condiciones; MPU6050 andando en la placa; Schottky colocado (resuelta la retroalimentación del USB); seta probada; F3..F6 de 5 A; cables terminados; batería comprada (AGM 12 V / 7 Ah). **Abierto:** servos con F2 mal dimensionado, TMCS sin probar, ACS712 quemado | [00-estado-actual.md](../20-modelo-escala/00-estado-actual.md) |
| Puntos V1 a V14 | Hipótesis de escritorio pendientes de medir, no defectos. V7 verificado (seta). V1 y V3 mitigados sin verificar; V2, V4 y V10 no aplican. V13: F2 es un X50 (~0,5 A), causa probable, se reemplaza por 3 A. V14: tipo de F3..F6 | [pendientes-de-verificacion.md](../20-modelo-escala/30-pcb/pendientes-de-verificacion.md) |
| Ensayos | Plan rev. 2.0 (reemplaza al protocolo rev. 1.0). Ninguno corrido | [plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md) |
| Pinout | Fuente única de verdad | [mapa-gpio.md](../20-modelo-escala/40-firmware/mapa-gpio.md) |

---

## 6. Instrumentación de ensayos — decisiones cerradas

- **ADS1115** para leer el TMCS1126: el conversor interno del ESP32 es el eslabón débil. Montado
  fijo a la placa (8-oct); responde en el bus (29-oct). **TMCS cableado a AIN0 y a IO33, sin probar.**
  El TMCS quedó confirmado como B4 (100 mV/A).
- **ACS712 a bordo en la pata de motor de una rueda delantera**, leído por el mismo ADS1115: respaldo
  y validación. No es un sensor por motor para la extrapolación. **Se quemó en el labo (al 15-nov):
  se evalúa reemplazarlo, a consultar.**
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

Hecho al 15-nov: Schottky, seta (V7), fusibles F3..F6 de 5 A, cables, encoders con PCNT, módulo
BTS7960 que falló y MPU6050 en la placa. Detalle en
[00-estado-actual.md](../20-modelo-escala/00-estado-actual.md).

| # | Acción | Cómo | Cierra o desbloquea |
|---|---|---|---|
| 1 | **Reemplazar F2 y volver a probar los servos** | PTC de 3 A de mantenimiento en lugar del X50 (~0,5 A). Mismo barrido de los 4 servos: caída sobre F2 al arrancar y a los 3 min. Sumar consigna en rampa y arranque escalonado | V13; V3 |
| 2 | **TMCS1126: probar y calibrar** | Ya cableado a AIN0 e IO33. I = (V_reposo − V) / 0,1; tarar el cero en cada sesión. Comparar ADS contra ADC del ESP32 y contra multímetro en serie | V9, V11, calibración |
| 3 | **V1** | Osciloscopio disparado en EN, R_PWM_4 y L_PWM_4 en J19 sin potencia en el driver. Después, con el motor conectado, confirmar que no se mueve al encender | V1 |
| 4 | **Rieles con motores y servos andando** | Osciloscopio con masa de resorte corta y disparo en el arranque. Bus de 12 V en los drivers, riel de servos, 5 V y 3,3 V. Sobrepicos y caídas en el arranque y la inversión, rizado de conmutación | V3; validación de los rieles |
| 5 | **Asentar el tipo de F3..F6** | Leer el rótulo (5x20 rápido o retardado, o de cuchilla) | V14 |
| 6 | **Reemplazo del ACS712 (quemado)** | **A consultar:** qué sensor y si se mantiene el respaldo en la pata de una rueda. AIN2 y AIN3 del ADS1115 quedan libres | Validación de la corrección por D |

### 7.2 Después, en prioridad baja

| # | Acción | Desbloquea | Bloqueado por |
|---|---|---|---|
| 1 | **Ensayo 0: ángulo de deslizamiento** en piso duro | El μ real, que bloquea el motor grande, y el ángulo del Ensayo D | superficie y rampa (Mecánica) |
| 2 | **Consulta única a Mecánica**: diámetro, μ, si los 30° son sostenidos, pedidos de ensayo | λ, requisitos de torque, decisión de ruedas | conviene hacer #1 antes |
| 3 | **Decidir rueda o lastre** (ruedas de 150 mm) | Escala del modelo, Ensayo D | reunión de líderes, #2 |
| 4 | **Pesar el modelo completo** | λ definitivo | que Mecánica devuelva el modelo |
| 5 | **Ensayo A** en los 4 motores, con el reemplazo del ACS712 y multímetro en serie | K, I₀ y R | 7.1 #2 y #6 |
| 6 | **Ensayo A'**, B y C | Corrección por D, K verificada, encoders | #5 |
| 7 | **D0**, ensayo de método en piso duro | Método del Ensayo D | #1, #6 |
| 8 | **Reimprimir ruedas** y medirlas con calibre | Cierra la escala | **#2, #3 y #4** |
| 9 | Altura del CM contra 0,3464·λ | Conclusiones de estabilidad | #8 |
| 10 | Motor de reemplazo del 42RBL04A | Rover final | #1 y #2 |
| 11 | Presupuesto de corriente de los otros subsistemas | Batería y PDB del rover final | otros subsistemas |

Hechos desde la versión anterior (23-oct → 15-nov): Schottky colocado y funcionando, seta probada,
F3..F6 de 5 A, cables terminados, los 4 motores con encoders por PCNT, BTS7960 que falló en
condiciones, MPU6050 en la placa, batería comprada y posicionamiento de la electrónica definido.
Problemas: F2 mal dimensionado (servos) y ACS712 quemado. Los LM2596 andan con el Schottky puesto;
su medición con carga y rizado queda dentro de 7.1 #4.

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
