---
titulo: Filosofía de diseño — reglas no negociables
estado: vigente
fecha: 2026-09-16
fuente: 00-contexto-maestro.md §3
---

# Filosofía de diseño

Las reglas que no se discuten en cada decisión porque ya se discutieron una vez. Versión corta en el
[contexto maestro](00-contexto-maestro.md) §2.

---

## 1. Reglas

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

## 2. Dónde se aplica cada regla

Ninguna de estas reglas es abstracta: todas tienen su caso concreto en la placa del modelo o en el
cableado del rover final. La tabla es el índice de esos casos.

| Regla | Caso concreto | Dónde se discute |
|---|---|---|
| COTS primero | Módulos BTS7960, LM2596 y ESP32-DEVKITC comprados hechos; la PCB propia es solo la distribución | [bom.md](../20-modelo-escala/30-pcb/bom.md) |
| Sobredimensionar por seguridad | Anchos de pista IPC-2221 con ΔT de 10 °C, planos en ambas caras reforzados con estaño | [especificacion.md](../20-modelo-escala/30-pcb/especificacion.md) |
| Star grounding estricto | Dos vertidos de masa unidos en un solo punto; el retorno del sensor va a masa lógica | [decisiones-layout.md](../20-modelo-escala/30-pcb/decisiones-layout.md) |
| Cable de silicona, norma AWG | Tabla de calibres por tramo, de 12–14 AWG en batería a 22–24 AWG en señal | [mapa-conectores.md](../20-modelo-escala/30-pcb/mapa-conectores.md) |
| Prohibido protoboard y Dupont | Conectores Molex KK-254 con traba en toda la placa; XT en el instrumento externo | [mapa-conectores.md](../20-modelo-escala/30-pcb/mapa-conectores.md) |
| E-Stop y fusibles | Seta NC sobre la bobina del relé, fusible por driver, PTC en el riel de servos | [protecciones-desacople.md](../20-modelo-escala/20-electronica/protecciones-desacople.md) |
| El fusible protege el cableado | La cadena conector-fusible-cable del rover final se dimensiona junta | [cableado-y-protecciones.md](../10-rover-final/20-potencia/cableado-y-protecciones.md) |
| Punteras tubulares | Obligatorias en todo multifilar que entre a bornera a tornillo | [decisiones-layout.md](../20-modelo-escala/30-pcb/decisiones-layout.md) |

---

## 3. Los tres escenarios de juicio

- **Vibración continua.** Es la que descarta el protoboard y el Dupont, la que obliga a sujetar los
  mazos pesados al chasis cerca de la placa, y la que hace que todo conector tenga traba.
- **Corriente de bloqueo.** Es el peor caso eléctrico: 22 A con las cuatro ruedas trabadas en el
  modelo. Define fusibles, conectores, anchos de pista y el límite del driver. No es condición de
  operación, es condición de falla, pero el hardware la tiene que sobrevivir.
- **Estrés térmico.** Es el que aparece tarde y sin aviso: la disipación de Q1 (V2), la temperatura
  del AMS1117 con el LM2596 en 7,2 V (V10), el margen térmico del motor en continuo.

Las reglas ganadas de estos escenarios que trascienden su caso original están en
[05-principios-transversales.md](05-principios-transversales.md).
