---
titulo: Reglas para escribir código del proyecto
estado: vigente
fecha: 2026-11-13
fuente: pedido de Matías Bélico (13-nov-2026), a partir de la prueba de servos
---

# Reglas para escribir código

Aplican a todo código nuevo del proyecto y a todo código que se modifique, lo escriba una persona o
Claude. Los sketches anteriores a esta regla no se reescriben solo por esto. Si se tocan, la parte tocada sí la cumple.

**Por qué existe.** El equipo sabe Arduino básico, no programación avanzada. Un código que el equipo no
entiende no lo puede corregir cuando falla en el banco, y en el banco es donde falla. Entender la
prueba de servos, que tiene apenas cuatro servos y una secuencia, ya lleva tiempo. Esa es la vara.

## Las cinco reglas

### 1. Bien comentado y detallado

- **Encabezado en cada archivo** con: qué prueba, qué hardware necesita, versión del núcleo
  Arduino-ESP32 para la que está escrito, qué se espera ver en el monitor serie y cómo se sabe si
  pasa o falla. El encabezado de
  [test-ads1115.ino](test-ads1115/test-ads1115.ino) es un buen modelo.
- Los comentarios explican **el porqué**, no repiten la línea. `// 300 ms entre servos: el primer
  pulso va a velocidad máxima` sirve; `// espera 300 ms` no.
- **Toda magnitud lleva su unidad**, en el nombre o en el comentario: `PULSO_MIN_US`, `PASO_MS`,
  `// grados por segundo`.
- Las cuentas se muestran: `// 0,5 ms / 20 ms · 16384 = 410 cuentas`.
- Los pines se definen como constantes con el nombre de la red y una referencia a
  [mapa-gpio.md](mapa-gpio.md). No se copia la tabla de pines en un comentario: el mapa es la fuente
  única de verdad.

### 2. Simple antes que corto

Es mejor un código más largo que se lee de arriba abajo que uno corto que hay que descifrar.

- Usar solo lo de Arduino básico: variables, `if`, `for`, `while`, funciones, arreglos, `Serial`,
  `millis()` y `delay()`.
- **Evitar**, salvo que el hardware lo exija y quede explicado en un comentario: clases propias,
  punteros, plantillas (`template`), funciones lambda, macros con lógica, operador `? :` anidado,
  tareas de FreeRTOS, interrupciones y secciones críticas.
- Una cosa por línea. Nombres en español que digan qué son (`anguloObjetivo`, no `a2`).
- Los números de configuración van como constantes con nombre al principio del archivo, nunca
  sueltos en el medio.
- Escribir para **una sola versión del núcleo**, la que está instalada (hoy la 3.x), y decirlo en el
  encabezado. Nada de `#if` para soportar varias versiones.
- `delay()` está bien en una prueba donde no hay nada más que hacer. `millis()` solo cuando hay que
  hacer dos cosas a la vez (por ejemplo, una rampa mientras se lee el monitor serie).

### 3. Sin sobreingeniería

Antes de entregar un código, revisar:

- [ ] ¿Cada función y cada variable se usa?
- [ ] ¿Hay opciones de configuración para casos que hoy no existen? Sacarlas.
- [ ] ¿Se podría hacer lo mismo con menos piezas sin volverlo críptico?
- [ ] ¿La prueba prueba **una sola cosa**? Un sketch para servos, otro para motores, otro para el
  ADS1115. Se juntan recién cuando cada uno anda por separado.
- [ ] Librerías: usar una si oculta algo que no hace falta entender (el protocolo I2C del MPU6050).
  No usarla si oculta algo que vamos a tener que diagnosticar (cómo se reparten los temporizadores
  del LEDC entre servos y motores).

### 4. Escrito para el nivel del equipo

- Suponer que quien lo lee sabe Arduino básico y nada más. Todo lo que pase de eso se explica en un
  comentario o se evita.
- Al entregar un código nuevo, acompañarlo con una explicación bloque por bloque: qué hace cada
  parte y por qué está ahí.
- Si una solución necesita un concepto nuevo (rampa de consigna, periférico LEDC, PCNT), explicar
  el concepto antes de usarlo.

### 5. Modular y fácil de diagnosticar

- **Funciones cortas con una sola tarea** y un nombre que diga cuál es: `leerCorrienteTraccion()`,
  `moverServoConRampa()`.
- **Interruptores de configuración** para apagar partes enteras sin borrar código, por ejemplo
  un `#define USAR_SERVOS 1` al principio del archivo. Si algo falla, se apaga de a una hasta encontrar al
  culpable.
- **Al arrancar, imprimir por serie:**
  - la configuración usada (pines, frecuencias, límites);
  - el resultado de cada inicialización, con `OK` o `FALLA`;
  - la causa del último reinicio (`esp_reset_reason()`). Un reinicio por falta de tensión
    (brownout) apunta directo a la alimentación.
- **Mensajes de error que digan qué revisar:** `ADS1115 no responde en 0x48: revisar SDA, SCL y
  alimentación del módulo`, no `error 2`.
- **Verificar lo que devuelven las funciones** que pueden fallar (`Wire.endTransmission()`,
  `ledcAttach()`) e imprimir el resultado.
- **Imprimir el valor crudo además del convertido** (cuentas del ADC y voltios, duty y
  microsegundos). Así se separa una falla de hardware de un error de cuenta.
- Todo valor impreso lleva su unidad: `Vbat = 12.41 V`, no `12.41`. En el monitor serie va
  punto decimal, no coma: así lo lee el Serial Plotter.
