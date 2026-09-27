---
titulo: Decisiones de layout de la PCB del modelo
estado: vigente
fecha: 2026-09-16
fuente: pcb-modelo-escala.md, sección decisiones-layout
---

# Decisiones de layout

### Masas

Topología en **estrella**, con un único punto de unión entre el retorno de potencia y el retorno
lógico. El retorno de los motores no debe compartir cobre con el retorno de la lógica ni con el
analógico en ningún tramo. Sobre esta placa:

- Un vertido de masa de potencia que abarque J1, Q1, K1, U3, C2, C7 y los cuatro conectores de driver.
- Un vertido de masa lógica que abarque el ESP32, los conectores de señal, los encoders y el I2C.
- Unión en un solo punto, cerca del capacitor de entrada (C7) o del borne de la batería.
- El retorno del sensor (U3 pin 9) va a masa **lógica**, no de potencia: es el lado aislado del
  sensor y su salida es la que lee el ADC.

### El TMCS1126 en el layout

Es el componente que más condiciona el ruteo:

- Los pines 1 y 2 (IN+ / IN-) son el lado de alta corriente y están **galvánicamente aislados** del
  resto. El fabricante especifica 8 mm de línea de fuga y de aire, y la huella los respeta. **No
  meter ninguna pista, vertido ni vía en ese canal.**
- El conductor interno son 0,7 mΩ: a 10 A disipa 70 mW y a 22 A, 340 mW. Poco, pero la capacidad de
  corriente del encapsulado depende del cobre que lo rodea. Vertidos grandes en ambas caras para
  IN+ e IN-, con vías térmicas.
- Alejarlo de conductores de alta corriente que no sean los suyos.
- C3 (100 n de VS) pegado al pin 4, con su vía a masa lo más corta posible.
- R15 y C8 pegados al pin 6, y el ruteo hacia IO33 corto y lejos del bus.

### Potencia

- Ventanas de máscara antisoldante sobre las pistas de alta corriente, para reforzar con estaño.
- Vías de costura uniendo los planos de masa top y bottom, densas cerca del relé, de Q1 y de los
  conectores de driver.
- C2 pegado a los bornes de salida hacia los drivers, no en un rincón.
- C1 pegado al conector de servos y detrás del PTC.
- Q1 en TO-220 vertical: prever espacio y agujero para disipador.
- K1 y sus pistas de contacto son las que llevan los 22 A. Si se agrega precarga, reservarle lugar
  en paralelo con el contacto.

### Analógico y señal

- Las dos entradas de ADC (IO32 e IO33) son las líneas más sensibles de la placa. Cortas, con su
  retorno acompañándolas, y nunca en paralelo con las líneas PWM.
- El bus I2C hacia el MPU6050 va cableado (el módulo está en el centro de masa): par trenzado, SDA
  con GND y SCL con 3,3 V, lejos de los cables de motor. Como en la placa no hay pull-ups (los pone
  el módulo GY-521), conviene quedarse en 100 kHz.
- Las 8 líneas de encoder y las 8 de PWM salen por conectores distintos; separarlas físicamente en
  el layout y en el mazo.

### Mecánica

- 4 agujeros M3 en las esquinas, libres de pistas y de vertido, con separadores de bronce.
- Los mazos pesados que salen hacia los drivers deben tener sujeción al chasis cerca de la placa,
  para que la vibración no trabaje sobre las soldaduras de los conectores.
- Punteras tubulares obligatorias en todo cable multifilar que entre a una bornera a tornillo.
- Los fusibles F3..F6 usan portafusibles a clip para vidrio de 5x20 mm: dejar acceso físico para
  cambiarlos sin desmontar la placa.

---

Componentes y valores en [bom.md](bom.md); conectores en [mapa-conectores.md](mapa-conectores.md).
