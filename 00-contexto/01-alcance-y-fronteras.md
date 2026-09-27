---
titulo: Alcance y fronteras del subsistema de Electrónica y Potencia
estado: vigente
fecha: 2026-09-16
fuente: 00-contexto-maestro.md §2, con la tabla de interfaces armada desde §4 y §5
---

# Alcance y fronteras

Qué entra en este subsistema, qué no, y por dónde pasa exactamente la costura con Mecánica. La
versión corta vive en el [contexto maestro](00-contexto-maestro.md) §2; acá está el detalle y la
tabla de interfaces.

---

## 1. Dentro del alcance

Batería y BMS, distribución de potencia, control de motores, protecciones, sensado, firmware,
telemetría.

## 2. Fuera del alcance

Diseño estructural, cinemático y de transmisión (subequipo de Mecánica). Entra solo como interfaz.

**Explícitamente fuera:** la derivación del factor de escala λ por parte de Mecánica. λ entra como
dato de entrada. Lo que sí es de este subsistema es la ecuación de extrapolación de torque y su
sensibilidad.

La distinción no es burocrática: es la que determina qué se discute y qué se acepta. Un número que
viene de Mecánica no se recalcula acá, se usa o se cuestiona con una medición. Un número que sale de
acá no se delega.

---

## 3. Interfaces con Mecánica

Cada fila es un dato que cruza la frontera. La columna de estado dice si el dato está cerrado o si
hoy bloquea algo de este lado.

| Dato | Dirección | Valor o dónde vive | Estado | Qué depende de él |
|---|---|---|---|---|
| Masa del rover real | Mecánica → Electrónica | 40 kg | cerrado | λ, techo de tracción, presupuesto de corriente |
| Chasis | Mecánica → Electrónica | 0,8 × 0,4 m | cerrado | Ubicación de PDB y batería |
| Configuración de tracción | Mecánica → Electrónica | Swerve de 4 ruedas. Por rueda: NEMA 23 para la dirección, motor de tracción a definir | cerrado | Cantidad de drivers, arquitectura de control |
| Diámetro de rueda del rover real | Mecánica → Electrónica | tres valores en circulación, ver [03-conflictos-abiertos.md](03-conflictos-abiertos.md) | **en conflicto** | Torque y velocidad requeridos, λ, diámetro de rueda del modelo |
| μ del terreno | Mecánica → Electrónica | ver [03-conflictos-abiertos.md](03-conflictos-abiertos.md) | **en conflicto**, se mide de este lado | Techo de tracción, elección del motor |
| Pendiente máxima | Mecánica → Electrónica | ver [03-conflictos-abiertos.md](03-conflictos-abiertos.md) | **en conflicto**: falta saber si es sostenida o puntual | Requisito de torque continuo |
| Requisitos de torque y velocidad por rueda | Mecánica → Electrónica | [requisitos-motor.md](../10-rover-final/10-locomocion/requisitos-motor.md) | vigente, inconsistente con el techo de tracción | Selección de motor y driver |
| Factor de escala λ | Mecánica → Electrónica | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md), derivado de la masa real del modelo | vigente | Toda la extrapolación del modelo |
| Ecuación de extrapolación de torque y su sensibilidad | Electrónica | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) | vigente | Validez de los ensayos |
| μ medido por ángulo de deslizamiento | Electrónica → Mecánica | pendiente de ensayo | pendiente | Cierra el conflicto 2 con un dato, no con una pregunta |
| Presupuesto de corriente y batería | Electrónica | [presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md): tracción calculada | parcial: falta brazo, cómputo, comunicaciones y ciencia | Batería y PDB del rover final |

Los tres conflictos abiertos de la tabla se tratan en
[03-conflictos-abiertos.md](03-conflictos-abiertos.md), que incluye la consulta consolidada a
Mecánica.

---

## 4. Frontera con el modelo a escala

El modelo a escala es un instrumento de este subsistema, no una entrega de Mecánica. Su única razón
de ser es cerrar la especificación de motores sin comprar motores de tamaño real a ciegas.

Lo que el modelo aporta y lo que no está en
[20-modelo-escala/10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md).
Regla práctica: si la magnitud tiene unidades eléctricas o térmicas, el modelo no la dice.
