---
titulo: Drivers BTS7960 del modelo a escala
estado: vigente
fecha: 2026-09-27
fuente: pcb-modelo-escala.md, sección drivers-bts7960
---

# Drivers BTS7960

Validado: se probó un puente H de cada unidad y funcionaron.

### Interfaz

Cada driver usa **dos conectores separados**: uno de señal (4 vías) y uno de potencia (2 vías).

- **Señal:** J10 (BTS7960_1), J11 (BTS7960_2), J15 (BTS7960_3) y J19 (BTS7960_4), Molex KK-254 de 4
  vías. Llevan L_PWM, R_PWM, +3,3 V y GND. En J19, **R_PWM_4 no viene directo del ESP32 sino de U1**.
- **Potencia:** J6 (driver 1), J4 (driver 2), J5 (driver 3) y J7 (driver 4), Molex KK-254 de 2 vías,
  cada uno detrás de su fusible.

Pinout completo con huella y corriente nominal en
[30-pcb/mapa-conectores.md](../30-pcb/mapa-conectores.md). GPIO de origen de cada línea PWM en
[40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md).

La correspondencia fusible-driver **no es correlativa** (F3 va al driver 2, F5 al driver 1).
Conviene dejarlo serigrafiado.

### Habilitaciones

Los `R_EN` y `L_EN` de los cuatro módulos se alimentan desde el pin 3 de cada conector de señal, que
va a +3,3 V permanente. Es decir: **los puentes H están habilitados desde que hay 3,3 V, y el estado
de reposo se delega enteramente a las líneas PWM**. Esto ahorra 4 GPIO, con dos consecuencias:

1. No hay forma de apagar los puentes por hardware sin cortar el bus de 12 V con el relé.
2. Queda libre el único mecanismo de apagado rápido disponible. Ver la propuesta en
   [sensado-corriente.md](sensado-corriente.md) para engancharle el pin OC del TMCS1126.

### Estado del canal 4 durante el arranque (V1)

De la tabla de verdad del NLAS4157 se desprende que, durante la ventana de arranque del ESP32,
`R_PWM_4` quedaría en 3,3 V:

- IO5 arranca en ALTO (pull-up interno, latcheado por el strapping).
- U1 tiene B1 a +3,3 V y B0 a GND, así que su salida A copia a S.
- L_PWM_4 (IO23) es un GPIO común: al reset queda como entrada sin pull. El BTS7960 tiene pull-down
  interno en IN, lo cual cubre a L_PWM_4 pero no a R_PWM_4.
- Los enables están activos de forma permanente.

Si el análisis es correcto, el driver 4 recibiría R_PWM alto y L_PWM bajo, y el motor giraría hasta
que el firmware tome el control. **Esto hay que confirmarlo o descartarlo con osciloscopio**, porque
hay al menos dos cosas que podrían hacer que no pase: el mapeo de números de pin del símbolo
reutilizado (U1 usa el símbolo `RF_Switch:MASWSS0136` renombrado a NLAS4157; si la numeración del
símbolo o de la huella no coincidiera con el NLAS4157 real, la conclusión cambia), y el
comportamiento real de la entrada del módulo BTS7960 a 3,3 V de VCC.

Medición concreta: disparar el osciloscopio en el flanco de EN del ESP32 y mirar R_PWM_4 y L_PWM_4
durante todo el arranque, con el conector de potencia del driver 4 desconectado.

> **Mitigación aplicada (27-sep-2026).** Se soldó una **resistencia de pull-up en L_PWM_4** (valor
> a asentar). Durante el arranque las dos entradas del driver 4 quedan en alto: las dos llaves
> superiores encendidas, los dos bornes del motor al mismo potencial, motor frenado y sin corriente.
> Es un agregado de componente, sin cortes. **V1 sigue abierto hasta verificarlo:** osciloscopio en
> R_PWM_4 y L_PWM_4 durante el arranque, confirmando que L_PWM_4 supera el umbral alto frente al
> pull-down interno del driver, y después, con el motor conectado, que no se mueve al encender.
> Una consecuencia a tener en cuenta en firmware: si el ESP32 se cuelga con IO23 en alta impedancia,
> L_PWM_4 queda en alto.

La alternativa que se había propuesto era **invertir B0/B1 en U1** (B1 a GND, B0 a +3,3 V) e
invertir esa salida PWM por hardware en la matriz de GPIO del LEDC, que no cuesta nada. Si la placa
ya está fabricada son dos cortes y dos puentes. Conviene aplicar el mismo criterio a U4 y U5 aunque
ahí no sea crítico, para que los tres canales queden con la misma convención.

### Otros puntos de los drivers

- **VCC lógico a 3,3 V.** Los módulos BTS7960 comerciales están especificados con VCC de 5 V. Con
  3,3 V andan (el IN es un Schmitt trigger compatible TTL/CMOS) y ya se comprobó en banco, pero se
  está fuera de la especificación nominal del módulo. Conviene verificar la conmutación en todo el
  rango de ciclo de trabajo, no solo en un punto.
- **Los pines IS quedan sin usar.** Consecuencia: no hay medición de corriente por rueda desde la
  placa, solo la total (más la rueda del ACS712, ver
  [sensado-corriente.md](sensado-corriente.md)). **En revisión desde el 27-sep:** con el ADS1115
  sobra un canal. Propuesta: no usarlo como canal de medición en esta campaña (la relación de IS
  tiene mucha dispersión, a menos de ~1 A la domina el error de cero, y con PWM también necesita la
  corrección por D), y a lo sumo cablear un canal experimental en la rueda del ACS712 para medir su
  relación real. Si en falla IS entrega varios mA, sobre 1 k supera los 3,3 V del ADS: resistencia de
  carga ≤ 680 Ω y 10 k en serie. Valores a confirmar en la hoja de datos. Si la relación resulta
  estable, un segundo ADS1115 (0x49) daría las cuatro ruedas.
- **No hay pull-down en las 8 líneas PWM.** Dependen del pull-down interno del BTS7960 durante el
  boot. Mejora barata: un pull-down de 10 k **del lado del driver** (soldado en el módulo o en el
  conector). Del lado del GPIO no protege contra un cable desconectado.
- **Terminación serie.** Con 60 a 100 cm de cable y flancos rápidos, 220 Ω en serie en el origen de
  cada línea PWM ayuda. No están en la placa; se pueden agregar en el conector.
- **Desacople local.** El PDB tiene C2 de 1000 uF, pero los drivers están a 60-100 cm. La
  inductancia de ese cable (del orden de 1 uH/m) domina a frecuencias de conmutación. **Cada
  BTS7960 necesita su propio desacople físicamente en el módulo**: 2 x 470 uF / 25 V / 105 °C de
  bajo ESR más 100 nF cerámico en los bornes. Es hardware fuera de la PCB pero es parte del diseño.

---

El punto V1 y su medición están resumidos en [30-pcb/pendientes-de-verificacion.md](../30-pcb/pendientes-de-verificacion.md).
