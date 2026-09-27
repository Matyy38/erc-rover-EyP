---
titulo: Candidatos de motor — rover real
estado: en revision
fecha: 2026-09-16
fuente: requisitos-motor-rover-real.md (Estado de candidatos) + 00-contexto-maestro.md §5.4
---

# Candidatos de motor — rover real

**Hoy no hay candidato.** El único que había quedó descartado y el criterio de reducción que lo
acompañaba era específico de ese motor.

Los requisitos que cualquier candidato tiene que cumplir están en
[requisitos-motor.md](requisitos-motor.md).

---

## Estado de candidatos

| Candidato | Estado | Motivo |
|---|---|---|
| 42RBL04A (Changzhou Smart Automation) | descartado | el proveedor no respondió ningún mensaje; nunca se pudo confirmar la interfaz del controlador integrado |
| Reducción 1/68 | criterio vigente | ~9,93 N·m continuos, motor al 71% del torque nominal (margen térmico real) |
| Reducción 1/49 | descartada | 98% del torque nominal en continuo, sin margen térmico |

Presupuesto objetivo: < 100 USD por motor, bus 24 V, brushed DC con reductora o BLDC con reductora
(sin definir).

La lista completa de lo descartado en todo el proyecto, motores incluidos, está en
[00-contexto/04-decisiones-descartadas.md](../../00-contexto/04-decisiones-descartadas.md).

---

## Qué quedó huérfano con el descarte del 42RBL04A

El criterio de reducción 1/68 (~9,93 N·m continuos, motor al 71% del torque nominal) era específico
de ese motor. Se conserva el **principio**, no el número:

> Elegir la reducción que deje el motor por debajo del **75% de su torque nominal en continuo**, por
> margen térmico.

Ese es el criterio que descartó la reducción 1/49 (98% del nominal) y el que hay que aplicar a
cualquier combinación motor + reductora que se evalúe de acá en adelante.

---

## Criterio de búsqueda del reemplazo

Dos filtros, en este orden:

1. **Proveedor con respuesta comprobada.** El 42RBL04A no se cayó por un problema técnico: se cayó
   porque nunca contestaron. Un motor que no se puede consultar no se puede especificar.
2. **Hoja de datos que declare corriente de bloqueo a la tensión de bus.** Ese, y no la corriente
   nominal, es el dato que define el driver. Si el fabricante no lo publica, se pide; si no lo
   contesta, vuelve al filtro 1.

Los dos filtros son previos a cualquier comparación de torque, velocidad o precio: sin proveedor que
responda y sin corriente de bloqueo declarada, el resto de los números no se puede usar para decidir.

---

## Qué destraba la búsqueda

La selección del motor grande está bloqueada por los conflictos abiertos con Mecánica: el diámetro de
rueda mueve los requisitos de torque y velocidad ~20%, y el μ real define el techo de tracción. Ver
[00-contexto/03-conflictos-abiertos.md](../../00-contexto/03-conflictos-abiertos.md).

Por eso la búsqueda de reemplazo figura como paso 12 en los próximos pasos del maestro, después del
ensayo de ángulo de deslizamiento y de la consulta a Mecánica.
