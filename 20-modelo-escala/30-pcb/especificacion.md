---
titulo: Especificación de la PCB del modelo
estado: vigente
fecha: 2026-09-16
fuente: pcb-modelo-escala.md, sección especificacion
---

# Especificación de la PCB

### Datos generales

| Ítem | Valor |
|---|---|
| Herramienta | KiCad 9.0, eeschema |
| Hoja | A3, una sola hoja (diseño plano, sin jerarquía) |
| Componentes en el esquemático | 74 (sin contar símbolos de alimentación) |
| Redes | 33 con nombre + 22 locales |
| Montaje | mixto: SMD 0805/0603 para señal, THT para potencia y conectores |
| Fijación | 4 agujeros M3 (H1..H4) en las esquinas, huella `MountingHole_3.2mm_M3` |
| Cobre | 1 oz (35 um) asumido, capa externa |
| Capas | a definir (2 capas es viable; 4 mejoraría el retorno) |

### Corrientes de diseño

| Tramo | Continua esperada | Pico | Criterio |
|---|---|---|---|
| Batería a nodo A | 3 a 6 A | 22 A | limitado por el fusible general |
| Nodo A a relé a sensor a bus +12 V | 3 a 6 A | 22 A | pasa por el conductor interno del TMCS (0,7 mΩ) |
| Bus +12 V a cada fusible | 1 a 2 A | 5,5 A | por rama |
| Riel de servos | 1 a 3 A | 10 A | ver V3 |
| Riel de 3,3 V | 0,15 a 0,25 A | 0,4 A | ver presupuesto |

### Anchos de pista (IPC-2221, 1 oz, ΔT = 10 °C)

| Red | Corriente de diseño | Ancho | Implementación |
|---|---|---|---|
| Bus principal 12 V y su retorno | 22 a 25 A | 10,5 a 12 mm | plano de cobre en ambas caras, unido con vías, reforzado con estaño |
| Riel de servos | 10 A de pico | 4,5 a 5,5 mm | pista ancha o vertido |
| Salida a cada driver | 5,5 A | 2,5 a 3,0 mm | pista |
| Señales (PWM, encoders, I2C, ADC) | menos de 100 mA | 0,4 a 0,6 mm | estándar |

Estos anchos vienen del PDF y siguen siendo válidos. IPC-2221 es una guía empírica conservadora para
régimen continuo; para picos de milisegundos el cobre tiene mucha más capacidad de la que sugiere.

### Componentes en placa y fuera de placa

**En la PCB:** zócalo del ESP32, U2 (módulo LM2596), U3 (TMCS1126), U1/U4/U5 (NLAS4157), Q1, K1
(relé), F2 a F6, D1 a D5, todos los capacitores y resistencias, y los 19 conectores.

**Fuera de la PCB:** batería, fusible general aéreo, seta de emergencia, los 4 módulos BTS7960 (con
su desacople local), los 4 servos, los 4 motores con encoder, y el módulo GY-521 (MPU6050) montado
en el centro de masa.

El criterio de montar los BTS7960 fuera es correcto: son los que disipan y los que hay que poder
cambiar rápido. El costo es la inductancia del cableado, que se compensa con el desacople local.

### Pendientes de especificación

- Dimensiones de la placa (no constan en el esquemático).
- Número de capas y espesor.
- Valores de F2 a F6.
- Definir si el conector de batería sigue siendo Phoenix o pasa a XT60 (ver V4).

---

El punto V4 (márgenes de conectores) está en [pendientes-de-verificacion.md](pendientes-de-verificacion.md); las decisiones de ruteo, en [decisiones-layout.md](decisiones-layout.md).
