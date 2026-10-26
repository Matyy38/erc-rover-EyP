---
titulo: Arquitectura eléctrica de la PCB del modelo
estado: vigente
fecha: 2026-10-26
fuente: pcb-modelo-escala.md, sección arquitectura-electrica; reuniones de laboratorio del 8-oct y del 23-oct-2026
---

# Arquitectura eléctrica

Cadena de potencia, rieles y presupuesto del 3,3 V de la placa del modelo a escala. El mapeo de pines
del ESP32 y la estrategia de arranque seguro **no están acá**: viven en
[40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md), que es la fuente única de verdad del pinout.

### Cadena de potencia

Diagrama al 26-oct-2026: ya muestra los dos LM2596 con rieles separados (8-oct) y el Schottky
planificado (23-oct). Q1 sigue dibujado, pero está puenteado desde el 27-sep (ver los cambios
debajo del diagrama).

```
                         NODO A (12 V protegido, permanente)
 J1          Q1                 |
[Bat +]--->[D  S]-------+-------+--- D1 (TVS SM6T15A) --- GND
 12V/7Ah    IRF9540N    |       |
                        |       +--- D5+R16 (LED de presencia)
                   R14 10k      |
                   D4 16V       +--- JP1 ---+--- C7 1000u
                    (gate)      |           |
                                |           +--- U2 LM2596 (5 V) --+--> [Schottky, planificado] --> ESP1.19 (VIN del devkit)
                                |           |                      +--- C9 100u + C10 1u
                                |           |
                                |           +--- LM2596 de servos (7,2 V, sin ref.) --> F2 (PTC) --> C1 1000u --> J12/J13/J14/J17 (4 servos)
                                |           |
                                |           +--- J2 (SETA NC)
                                |                  |
                                |                  v
                                |            K1 bobina A2
                                |            D3 en antiparalelo
                                |            K1 bobina A1 --> GND
                                |
                                +--- K1 contacto 11 (comun)
                                          |
                                     (NO) 14
                                          |
                                     U3 IN- [TMCS1126B4] IN+
                                                          |
                                                    RIEL +12V CONMUTADO
                                                          |
                                          +------+--------+--------+------+
                                          F5     F3       F4       F6     C2 1000u + R6 (divisor)
                                          |      |        |        |
                                         J6     J4       J5       J7
                                      driver1 driver2  driver3  driver4
```

> **Cambios del 27-sep-2026 sobre este diagrama:**
> - **Q1 está puenteado.** El nodo A queda unido directo a la batería; la polaridad se protege con un
>   conector que solo entra en un sentido. Ver [protecciones-desacople.md](protecciones-desacople.md).
> - **ACS712 en la pata de motor de una rueda delantera**, entre driver y motor, fuera de la placa.
>   Se alimenta con un regulador lineal de 5 V propio desde el riel "+5V" y se lee con un ADS1115 en
>   el bus I2C. Ver [sensado-corriente.md](sensado-corriente.md).
> - La batería de plomo alimenta el bus de tracción **sin regular** (10,5 a 14,4 V). Los ensayos lo
>   compensan con lazo cerrado de velocidad y registro de tensión y ciclo de trabajo, no con un
>   convertidor. Ver [50-ensayos/plan-de-ensayos.md](../50-ensayos/plan-de-ensayos.md).

> **Cambios del 8-oct-2026 sobre este diagrama:**
> - **Segundo LM2596, solo para los servos, en 7,2 V.** Mismo módulo que U2, sin referencia en el
>   `.sch`. Toma la entrada del mismo punto que U2 y su salida alimenta F2 → C1 → J12/J13/J14/J17.
>   Se hizo con **corte y puente en la placa**. F2 (PTC) sigue en serie.
> - **U2 queda en 5 V** y alimenta solo VIN del ESP32 (y por el AMS1117, todo el 3,3 V).
> - **J1 y J4..J7 ya no existen como conectores:** batería y alimentación de drivers van con cable
>   de silicona soldado directo a la placa. Del lado del driver, puntera en la bornera del módulo.
>   Ver [30-pcb/mapa-conectores.md](../30-pcb/mapa-conectores.md).
> - **La seta no corta los servos.** El segundo LM2596 toma la entrada del nodo A, después de JP1,
>   igual que U2: queda **aguas arriba del relé**. Con la seta apretada, servos y ESP32 siguen
>   alimentados (confirmado por el equipo el 21-oct).

> **Problema encontrado el 23-oct-2026: retroalimentación desde el USB del ESP32.** Al enchufar el
> USB con la batería desconectada, los LM2596 calientan mucho.
>
> ```
> USB (5 V) → pin 19 del devkit (EXT_5V/VIN) → salida de U2 → [conducción inversa] → entrada de U2 = nodo A
>                                                                       ├── bobina de K1, LED D5, C7
>                                                                       └── entrada del LM2596 de servos → riel de servos
> ```
>
> El pin 19 está unido directo a la salida de U2, sin nada que bloquee el retorno. El LM2596 no está
> hecho para recibir tensión en la salida con la entrada sin alimentar: su hoja de datos no garantiza
> nada en esa condición, y en la placa conduce hacia la entrada. Con el nodo A en ~4 V, el LM2596 de
> servos queda con la llave siempre cerrada, porque su salida está ajustada en 7,2 V. Así, el USB
> termina alimentando toda la rama permanente a través de dos módulos que trabajan fuera de su forma
> normal.
>
> **Cómo se confirma:** con la batería desconectada y el USB enchufado, medir el nodo A durante no
> más de 5 s. Si hay varios voltios, el camino es este.
>
> **Corrección planificada (corte y puente + diodo):** un Schottky en serie entre la salida de U2 y
> el pin 19, con el ánodo del lado de U2. Puede ser 1N5819 (THT, más fácil para un puente) o SS34
> (SMA). U2 se sube a ~5,4 V para que VIN quede en ~5,0 V, porque el diodo cae 0,3 a 0,4 V a la
> corriente del ESP32. Sin ese ajuste, el AMS1117 queda con 1,3 V de margen, justo en su caída
> mínima. Con la batería conectada, el USB y U2 alimentan VIN cada uno por su diodo, y alimenta el
> que tenga más tensión.
>
> **Pendiente:** no se sabe si el devkit es oficial o un clon. Hay que medir en modo diodo entre el
> VBUS del conector USB y el pin 5V. Si no hay diodo, U2 puede empujar corriente hacia el puerto de
> la notebook cuando están enchufados la batería y el USB a la vez.
>
> **Verificación de la corrección:** USB enchufado y batería desconectada. El nodo A tiene que quedar
> en 0 V y los módulos fríos.

### Rieles

| Riel | Origen | Tensión real | Cargas | Protección propia |
|---|---|---|---|---|
| Batería | cable soldado en lugar de J1 (8-oct), XT60 del lado de la batería | 10,5 - 14,4 V (plomo-ácido 12 V / 7 Ah) | todo | Fusible aéreo (off-board) |
| Nodo A | salida de Q1 (**Q1 puenteado**: nodo A = batería) | Vbat | los dos LM2596, bobina del relé, LED | D1 (TVS); polaridad por conector de un solo sentido |
| +12V conmutado | contacto NO de K1, vía U3 | Vbat - caída del relé - 0,7 mΩ del sensor | 4 drivers, C2, divisor de batería | F3..F6 (uno por driver) |
| "+5V" (nombre de red) | U2 LM2596 | **5 V** (8-oct); ~5,4 V con el Schottky | solo ESP32 VIN | ninguna. Schottky hacia VIN planificado (23-oct) |
| Riel de servos | segundo LM2596 (8-oct) detrás de F2 | 7,2 V menos la caída del PTC | 4 servos | F2 (PTC rearmable) |
| +3,3 V | AMS1117 del devkit ESP32 | 3,3 V | ESP32, TMCS, MPU, encoders, pull-ups, enables de los BTS, 3 NLAS | ninguna |

Dos consecuencias arquitectónicas que conviene tener presentes:

1. **La seta corta solamente la tracción.** El LM2596 y la bobina cuelgan del nodo A, aguas arriba
   del relé. Al apretar la seta se desenergiza la bobina, se abren los contactos y muere el bus de
   los drivers, pero **los servos de dirección y el ESP32 siguen alimentados**. Para el modelo a
   escala eso es deseable (la telemetría sobrevive al corte). Si en algún ensayo se necesita que los
   servos también caigan, hay que pasar el LM2596 aguas abajo del relé, y ahí se pierde la
   telemetría post-corte. Desde el 8-oct vale igual para el LM2596 de servos, que cuelga del mismo
   punto.
2. **V_bat se mide después del relé.** El divisor R6/R5 cuelga del riel +12 V conmutado. Con la seta
   apretada el ESP32 lee 0 V, y el corte por software a 10,5 V se dispararía como falso positivo.
   Se contempla en firmware (enmascarar el corte por baja tensión si la tensión cae de golpe a cero
   con corriente nula) o se mueve el divisor al nodo A.

### Presupuesto del riel de 3,3 V

Todo el 3,3 V de la placa sale del AMS1117 que trae el módulo ESP32. No hay regulador de 3,3 V
propio en la PDB.

| Carga | Consumo estimado |
|---|---|
| ESP32 con Wi-Fi activo | 80 - 120 mA promedio, picos de 300 mA |
| U3 TMCS1126 (Iq) | 11 - 14,5 mA |
| MPU6050 | ~4 mA |
| ADS1115 (27-sep) | <1 mA |
| 4 encoders Hall | ≤ 40 mA (hoja del fabricante: máximo 10 mA cada uno; confirmar midiendo) |
| 8 pull-ups de 4,7 k, peor caso todos en bajo | 5,6 mA |
| VCC lógico de los 4 BTS7960 + enables | 10 - 30 mA (**medir**) |
| 3 NLAS4157 | despreciable |
| **Total estimado** | **110 - 220 mA, picos de 400 mA** |

Las cuentas de disipación de abajo usan 0,25 A, que queda como cota conservadora.

Con el LM2596 en 5 V: el AMS1117 disipa (5 - 3,3) x 0,25 = 0,43 W. Se banca. **Es el caso real
desde el 8-oct.**

Con el LM2596 en 7,2 V (la idea anotada en el esquemático para los servos): disipa
(7,2 - 3,3) x 0,25 = 0,98 W. En SOT-223 sobre el cobre mínimo de un devkit, con resistencia térmica
del orden de 60 a 110 °C/W, son 60 a 110 °C de salto sobre ambiente. Ese era el motivo de V10.

De las tres salidas que se habían planteado, se eligió la **(a), dos convertidores**: U2 a 5 V para
el ESP32 y un segundo LM2596 a 7,2 V solo para los servos (8-oct). Las descartadas eran un
regulador de 3,3 V propio en la PDB, y dejar el único LM2596 en 5 V aceptando menos torque en los
servos.


### Mapeo de pines del ESP32 y arranque seguro

Movidos a [40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md). Ahí están la tabla completa de
GPIO extraída del `.sch`, la advertencia de numeración de pines del símbolo de KiCad, y la estrategia
de arranque seguro con los conmutadores NLAS4157. **No copiar esa tabla acá ni en ningún otro
archivo:** duplicar el pinout es la forma más probable de romper algo.

De ahí salen dos datos que se usan en este archivo: no queda ningún GPIO libre, y los puntos V1 y
V10 tocan directamente esta arquitectura.

### Notas de arquitectura

- El número de driver (1..4) **no está asociado a una rueda** en ningún lado del esquemático. Los
  encoders sí (FR, FL, RL, RR) y los servos están numerados 1..4. Conviene fijar la correspondencia
  driver N <-> rueda <-> servo N y rotularla en el mazo y en el firmware.
- No hay watchdog externo ni forma de resetear la etapa de potencia sin la seta.
- No hay CAN en esta etapa (decisión explícita del PDF, sigue vigente).

---

## Relacionado

- Protecciones del nodo A, TVS, relé y polaridad inversa: [protecciones-desacople.md](protecciones-desacople.md)
- Sensor de corriente en la rama conmutada: [sensado-corriente.md](sensado-corriente.md)
- Riel de servos y margen del LM2596 (V3): [direccion-swerve-servos.md](direccion-swerve-servos.md)
- Puntos V10 (temperatura del AMS1117, no aplica desde el 8-oct) y V12 (lectura de V_bat con la seta apretada): [30-pcb/pendientes-de-verificacion.md](../30-pcb/pendientes-de-verificacion.md)
