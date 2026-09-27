---
titulo: Cableado y protecciones — rover real
estado: en revision
fecha: 2026-09-16
fuente: requisitos-motor-rover-real.md (Implicancias eléctricas) + 00-contexto-maestro.md §3 y §8
---

# Cableado y protecciones del rover real

Lo que se desprende de los requisitos de motor para el lado eléctrico: calibres, fusibles y el dato
que define el driver. Es dimensionamiento preliminar: se rehace cuando cierren el diámetro de rueda y
el μ, que mueven los requisitos ~20%.

---

## 1. Implicancias eléctricas de los requisitos de motor

- La potencia sostenida de tracción está calculada en [presupuesto-corriente.md](presupuesto-corriente.md).
  El número original de la hoja de Mecánica quedó corregido a menos de la mitad.
- A 24 V: ~5 A nominales por motor. Con stall de 3–5× la nominal, los picos transitorios del bus
  pueden llegar a 60–100 A si arrancan los cuatro juntos.
- El dato que define el driver **no** es la corriente nominal sino la corriente de stall a la tensión
  de bus. Pedirlo siempre al fabricante.
- Cableado estimado a 24 V: AWG 12–14 siliconado por rama, troncal más grueso. Los fusibles se
  dimensionan para proteger el cable, no el motor.

---

## 2. Dos dimensionamientos sobre la misma rama

El tramo driver-motor y el tramo PDB-driver no se dimensionan con la misma corriente:

| Tramo | Corriente que ve | Criterio |
|---|---|---|
| PDB a driver | I_bus ≈ D · I_motor | Ciclo de trabajo bajo en trepada, alto en crucero |
| Driver a motor | corriente plena del motor, sin factor D | El peor caso es rueda trabada |

Es la aplicación directa del principio 4 de
[00-contexto/05-principios-transversales.md](../../00-contexto/05-principios-transversales.md).
Dimensionar los dos tramos con el mismo número sobredimensiona uno y deja corto el otro.

---

## 3. La cadena conector-fusible-cable

**El fusible protege el cableado, no la carga.** Y el conector también tiene un límite: los tres
elementos se eligen juntos o la protección no sirve. La regla operativa es que el fusible corte antes
de que se dañe el elemento más débil de la cadena, que muchas veces es el conector y no el cable.

El caso testigo está en el modelo a escala: conectores Molex KK-254 de 3–4 A por contacto en ramas
que pueden llevar 5,5 A, con fusibles que tendrían que ser de 4 A para protegerlos y que entonces
cortarían en cada bloqueo de una rueda. La cadena coherente exigió cambiar el conector, no el
fusible. Está documentado en
[20-modelo-escala/30-pcb/mapa-conectores.md](../../20-modelo-escala/30-pcb/mapa-conectores.md) (V4),
y es el error a no repetir en el rover final, donde las corrientes son mayores.

---

## 4. Reglas que aplican al cableado del rover final

De las reglas no negociables de
[00-contexto/02-filosofia-de-diseno.md](../../00-contexto/02-filosofia-de-diseno.md), las que caen
directamente sobre este archivo:

- **Cable de silicona en potencia**, norma AWG.
- **Prohibido protoboard y cables Dupont.** Siempre conectores con traba: XT60, Molex, Anderson,
  bornera.
- **E-Stop y fusibles innegociables.**
- **Punteras tubulares** en todo cable multifilar que entre a bornera.
- **Star grounding estricto**: el retorno de motores no comparte cobre con el de lógica ni con el
  analógico en ningún tramo.

---

## 5. Pendiente

- Calibres y fusibles definitivos: bloqueados por el diámetro de rueda y el μ, que mueven los
  requisitos de torque ~20% y con ellos la corriente.
- Corriente de bloqueo a 24 V del motor que se elija: es el dato que define el driver, y hoy no hay
  motor elegido. Ver [10-locomocion/candidatos-motores.md](../10-locomocion/candidatos-motores.md).
- Consumo de brazo, cómputo, comunicaciones y ciencia: sin eso no se cierra el troncal ni la batería.
