---
titulo: Encoders en cuadratura y periférico PCNT
estado: vigente
fecha: 2026-09-16
fuente: pcb-modelo-escala.md, sección encoders-pcnt
---

# Encoders y PCNT

Validado: se probó un encoder y funcionó correctamente.

### Interfaz física

Cuatro conectores idénticos de 4 vías, Molex KK-254 vertical, uno por rueda: J9 (FR), J16 (FL),
J18 (RL) y J8 (RR). El pinout completo, con huella y corriente nominal, está en
[30-pcb/mapa-conectores.md](../30-pcb/mapa-conectores.md); la asignación de GPIO a cada canal, en
[40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md).

**Los encoders se alimentan con 3,3 V**, no con 5 V. Esto resuelve de raíz el riesgo de sobretensión
en IO34..IO39 que estaba abierto: como el encoder del CQR37D12V64EN-I acepta de 3,3 a 24 V, al
alimentarlo con 3,3 V la salida no puede superar 3,3 V, tenga o no pull-up interno el módulo.
Si algún módulo trae pull-up interno a VCC, queda en paralelo con el de la placa y el equivalente
baja (4,7 k // 10 k = 3,2 k): no es un problema, pero sí una diferencia de tiempos entre canales.

### Acondicionamiento

Cada canal tiene **un pull-up de 4,7 k a 3,3 V** (R1, R7, R8, R9, R10, R11, R12, R13) y nada más.
No hay resistencia serie ni capacitor. Esto es obligatorio, no opcional: IO34, IO35, IO36 e IO39
son de solo entrada y **no tienen pull-up ni pull-down internos**, así que sin estas resistencias
esas cuatro líneas quedarían flotando.

Verificación de que 4,7 k alcanza:

- Capacidad del cable: ~100 pF/m. Con 60 cm de cable más la entrada del ESP32, del orden de 70 pF.
- Constante de subida: 4,7 k x 70 pF = **0,33 us**.
- Frecuencia máxima por canal, con el motor CQRobot a 120 rpm de salida, 90:1 y 64 CPR en el eje
  del motor: 10800 rpm de motor = 180 rev/s, 16 pulsos por vuelta y por canal -> **2,9 kHz**.
  Período de 345 us, y el evento más corto (flanco de A a flanco de B en cuadratura) son **86 us**.

0,33 us contra 86 us: tres órdenes de magnitud de margen. El pull-up de 4,7 k está bien elegido para
velocidad. Lo que se pierde frente a un valor más bajo es inmunidad al ruido. Si aparecen cuentas
espurias con los motores girando, la secuencia de corrección es:

1. Activar el **filtro de glitch del periférico PCNT** (es gratis, no lleva componentes). Con 100 a
   200 ciclos de APB (1,25 a 2,5 us) se filtra ruido sin tocar los 86 us reales. El máximo son 1023
   ciclos = 12,8 us, así que hay lugar de sobra.
2. Recién si eso no alcanza, bajar los pull-ups a 2,2 k.
3. Como último recurso, un RC (2,2 k serie + 1 nF). **No usar 10 nF**: con esas impedancias destruye
   el flanco.

### Uso del periférico PCNT

El ESP32 tiene 8 unidades PCNT. Se usan 4, una por rueda, con los dos canales de cada unidad
configurados para decodificación en cuadratura x4 (una señal como pulso, la otra como control).

Cuentas por vuelta de rueda: 64 CPR en el eje del motor x 90 de reducción = **5760 cuentas por
vuelta de salida**.

Punto a resolver en firmware: **el contador del PCNT es de 16 bits con signo** (±32767). A 120 rpm
de salida son 11520 cuentas/s, o sea que **desborda cada 2,8 segundos**. Hay que usar los eventos de
límite del hardware y acumular en una variable de 32 o 64 bits. Si se lee el contador por sondeo sin
manejar el desborde, la medición de velocidad se rompe silenciosamente.

### Pendientes

- Repetir la prueba en los cuatro canales, no solo en el que ya se validó.
- Medir el consumo real de los cuatro encoders a 3,3 V (entra directo en el presupuesto del AMS1117).
- Definir el sentido positivo de cuenta de cada rueda y dejarlo escrito (las dos ruedas de un lado
  giran al revés que las del otro para avanzar).
- Cablear A/B como par trenzado con su propio GND de retorno, lejos de los cables de motor.

---

Conectores en [30-pcb/mapa-conectores.md](../30-pcb/mapa-conectores.md); GPIO en [40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md).
