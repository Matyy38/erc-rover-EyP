---
titulo: Protecciones y desacople de la PCB del modelo
estado: vigente
fecha: 2026-09-27
fuente: pcb-modelo-escala.md, sección protecciones-desacople
---

# Protecciones y desacople

> **Cambio (27-sep-2026): Q1 quedó puenteado y no se usa.** La protección contra polaridad inversa
> pasa a ser un conector que solo entra en un sentido: es la alternativa que el propio PDF mencionaba
> (ver más abajo). V2 deja de aplicar. Lo que se pierde es la protección contra el error de conectar
> con cables sueltos o pinzas: **nunca alimentar la placa si no es por ese conector**. La protección
> contra cortocircuito sigue siendo el fusible general. El análisis de Q1 queda abajo como registro.

### Capas de protección

| Capa | Componente | Qué protege |
|---|---|---|
| Fusible general | aéreo, off-board | cableado de batería |
| Polaridad inversa | ~~Q1 IRF9540N + R14 10 k + D4 zener 16 V~~ **Q1 puenteado.** Conector de un solo sentido | toda la placa |
| Sobretensión | D1 SM6T15A en el nodo A | toda la placa |
| Corte de emergencia | J2 (seta NC) -> bobina K1 -> contacto NO | rama de tracción |
| Antirretorno de bobina | D3 en antiparalelo con la bobina | contactos de la seta |
| Sobrecorriente por rama | F3..F6, uno por driver | cableado a cada driver |
| Sobrecorriente en servos | F2 (PTC) | rama de servos |
| Sobrecorriente electrónica | pin OC del TMCS1126 | hoy sin usar (ver V9) |
| Protección de ADC | D2 BAT54S en V_bat | entrada IO32 |
| Indicación | D5 + R16 2,2 k (~4,5 mA) | — |

### Circuito de polaridad inversa

El circuito de compuerta está bien resuelto: el drenador va a la batería y la fuente a la carga, de
manera que el diodo intrínseco conduce en el sentido normal de operación y bloquea con la batería
invertida. La compuerta se lleva a masa por R14 (10 k) y el zener D4 de 16 V limita V_GS, que en el
IRF9540N tiene un máximo de ±20 V. Correcto.

**Disipación en Q1 (V2).** El IRF9540N es canal P, -100 V, -23 A, con **R_DS(on) = 117 mΩ** típico a
VGS = -10 V, en TO-220 vertical. La aritmética de disipación:

| Corriente de bus | Disipación en Q1 | Salto térmico sin disipador (~62 °C/W) |
|---|---|---|
| 1 A | 0,12 W | 7 °C |
| 3 A | 1,05 W | 65 °C |
| 5 A | 2,9 W | 181 °C |
| 10 A | 11,7 W | fuera de rango |
| 22 A (bloqueo) | 56,6 W | fuera de rango |

En banco con poca carga no se nota; el problema aparecería con las cuatro ruedas cargadas. **La
medición que lo define es tocar/medir la temperatura de Q1 con el rover andando sobre terreno**, y
medir la caída de tensión entre drenador y fuente con corriente conocida (a 5 A deberían ser unos
0,6 V si el R_DS(on) es el del datasheet).

Si el número da alto, reemplazos que mantienen el mismo circuito de compuerta:

| Candidato | V_DS | I_D | R_DS(on) | Encapsulado | Disipación a 22 A |
|---|---|---|---|---|---|
| **IRF4905** | -55 V | -74 A | 20 mΩ | TO-220 | 9,7 W (con disipador) |
| AOD4185 / AOB4185 | -40 V | -40 A | ~10 mΩ | TO-252 / TO-263 | 4,8 W |

Alternativa que el propio PDF menciona: eliminar el circuito de polaridad inversa y confiar en un
conector polarizado (XT60). Pierde protección contra el error humano de conectar con caimanes, pero
elimina una fuente de calor y de caída.

### TVS: tensión de trabajo y ubicación (V5, V6)

**V5.** D1 es un **SM6T15A**: 600 W, tensión de trabajo (standoff) de aproximadamente 12,8 V,
ruptura mínima de 14,3 V. Una batería de plomo-ácido flota entre 13,5 y 13,8 V y en carga cíclica
llega a 14,4 V. Con el cargador conectado y el rover energizado, el TVS podría quedar cerca de su
ruptura y conducir. Medición: con la batería recién cargada, medir la corriente de fuga por D1 (o
simplemente su temperatura) con el rover encendido.

Si conduce, reemplazo: **SM6T18A** (standoff 15,3 V) o **SMBJ16A / SMCJ16A** (standoff 16 V). La
tensión de recorte resultante, del orden de 26 V, es segura: los BTS7960 admiten hasta 45 V.

**V6.** El TVS está en el nodo A, aguas arriba del relé. Cuando la seta abre los contactos con los
motores girando, la energía inductiva queda del lado de los drivers, donde no hay TVS: solo C2
(1000 uF), que absorbe la mayor parte, y los diodos intrínsecos de los puentes. Conviene ver con
osciloscopio qué pico aparece en el riel conmutado al abrir la seta a plena marcha. Si es alto, un
segundo TVS (SMBJ16A alcanza) entre el riel conmutado y GND, cerca de C2.

La ubicación del TVS que sí está es correcta en un aspecto: queda **después** del MOSFET de
polaridad inversa, que es donde tiene que ir.

### Inrush del contacto del relé (V7)

K1 es un relé automotriz de 12 V / 40 A. Al cerrar conecta el nodo A contra C2 (1000 uF) más el
desacople local de los drivers (4 x 940 uF si se monta lo recomendado), del orden de **4,8 mF**. La
energía que va al arco de cierre es del orden de `½CV² = 350 mJ` por encendido.

Un contacto de 40 A lo tolera, pero repetido erosiona y puede terminar soldando los contactos. El
síntoma es que la seta deja de cortar y nadie se entera hasta que hace falta. **Verificación
periódica: con la seta apretada, medir continuidad entre el nodo A y el riel conmutado.** Debe ser
circuito abierto.

Mitigación estándar si aparece: **resistencia de precarga de 22 Ω / 5 W en paralelo con el
contacto**. La constante de carga pasa a `22 × 4,8 mF = 106 ms`, o sea que los capacitores ya están
cargados cuando el contacto cierra. No requiere lógica. Para el arco de apertura, un snubber RC
(100 Ω + 100 nF en serie, en paralelo con el contacto).

### Desacople actual

| Riel / punto | Capacitores | Comentario |
|---|---|---|
| Nodo A (entrada del LM2596) | C7 1000 uF | correcto |
| +12 V conmutado | C2 1000 uF | correcto en la placa; el desacople remoto de los drivers es aparte |
| Salida del LM2596 ("+5V") | C9 100 uF + C10 1 uF | correcto |
| Riel de servos (detrás de F2) | C1 1000 uF | correcto en topología |
| +3,3 V | C15 100 uF + C11 1 uF | conviene sumar un 100 nF cerámico pegado al pin 3V3 |
| VS del TMCS1126 | C4 100 n + FB1 + C3 100 n | filtro en pi, muy bien |
| VOC del TMCS1126 | C5 1 uF | el fabricante sugiere 10 uF |
| VCC de cada NLAS4157 | C12, C13, C14 (100 n) | uno por chip, correcto |
| V_bat (ADC) | C6 100 n | con la impedancia de Thevenin del divisor (1,8 k) da un polo en 883 Hz |
| Vout_CS (ADC) | C8 100 n | con R15 de 1 k da un polo en 1,59 kHz |

El único agregado que falta es un 100 nF en 3V3: los electrolíticos de 100 uF tienen ESR e
inductancia que los vuelve poco efectivos por encima de unos cientos de kHz, que es el rango donde
el ESP32 mete escalones de corriente al transmitir por Wi-Fi.

---

Los puntos V2, V5, V6 y V7 y sus mediciones están resumidos en [30-pcb/pendientes-de-verificacion.md](../30-pcb/pendientes-de-verificacion.md).
