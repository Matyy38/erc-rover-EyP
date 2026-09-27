---
titulo: Principios transversales
estado: vigente
fecha: 2026-09-16
fuente: 00-contexto-maestro.md §8
---

# Principios transversales

Reglas ganadas en el proyecto que aplican más allá del caso que las originó. El [contexto maestro](00-contexto-maestro.md) §2
solo los enlaza; acá está el enunciado completo y el
lugar donde cada una se puso a prueba.

---

## Los trece

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

## De dónde salió cada uno

| # | Caso que lo originó | Dónde se discute |
|---|---|---|
| 1 | Techo de tracción contra el torque pedido por Mecánica | [presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md) |
| 2 | Desfasaje entre λ geométrico y λ másico del modelo | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) |
| 3, 4 | Presupuesto de corriente de la PDB del rover final | [presupuesto-corriente.md](../10-rover-final/20-potencia/presupuesto-corriente.md) |
| 5 | Ensayo A de calibración de K, en banco y sin modulación | [instrumentacion.md](../20-modelo-escala/50-ensayos/instrumentacion.md) |
| 6, 12 | Pines de strapping, NLAS4157 y arranque del ESP32 | [mapa-gpio.md](../20-modelo-escala/40-firmware/mapa-gpio.md) y [drivers-bts7960.md](../20-modelo-escala/20-electronica/drivers-bts7960.md) |
| 7 | Desacople local de los BTS7960 a 60–100 cm del PDB | [drivers-bts7960.md](../20-modelo-escala/20-electronica/drivers-bts7960.md) |
| 8, 9 | TVS SM6T15A (V5) e inrush del contacto de K1 (V7) | [protecciones-desacople.md](../20-modelo-escala/20-electronica/protecciones-desacople.md) |
| 10 | Márgenes de J1 y J4..J7 contra el valor de los fusibles (V4) | [mapa-conectores.md](../20-modelo-escala/30-pcb/mapa-conectores.md) |
| 11 | Criterio de búsqueda del motor de reemplazo del 42RBL04A | [candidatos-motores.md](../10-rover-final/10-locomocion/candidatos-motores.md) |
| 13 | Ensayo de arena y repetibilidad del terreno | [plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md) |
