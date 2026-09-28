---
titulo: Pendientes de verificación de la PCB (V1 a V12)
estado: vigente
fecha: 2026-09-27
fuente: pcb-modelo-escala.md, sección pendientes-de-verificacion
---

# Pendientes de verificación (V1 a V12)

Puntos surgidos del análisis del esquemático que **quedan por confirmar en el banco**. No son fallas
comprobadas: varios pueden descartarse con una medición de dos minutos, y algunos ya podrían estar
descartados por las pruebas hechas.

**Cómo se cierra un punto:** se hace la medición de la columna del medio, se actualiza la fila con el
resultado y se lo mueve a resuelto. Un punto sin medición no se cierra por argumento.

Por consecuencia, los que conviene atacar primero son **V1** (si el análisis es correcto, el motor 4
gira solo hasta que el firmware toma el control) y **V3** (cuatro servos trabados pueden resetear el
ESP32 por brownout). Estado al 27-sep: V1 tiene una mitigación aplicada que falta verificar, V3 quedó
en pausa, y V2 dejó de aplicar.

| # | Qué revisar | Cómo se confirma o descarta | Sección |
|---|---|---|---|
| V1 | Nivel de R_PWM_4 durante el arranque del ESP32 (U1 con B1 a 3,3 V). **Mitigado el 27-sep** con un pull-up en L_PWM_4: las dos entradas arrancan en alto y el motor queda frenado. **Falta verificarlo** | osciloscopio disparado en EN, mirando R_PWM_4 y L_PWM_4, con el driver 4 sin potencia; confirmar que L_PWM_4 supera el umbral alto. Después, con el motor conectado, confirmar que no se mueve al encender | [drivers-bts7960](../20-electronica/drivers-bts7960.md) |
| V3 | Margen del riel de servos con el LM2596 compartido con el ESP32. **En pausa (27-sep):** se observa durante los ensayos; por ahora se depende del corte del PTC y de rampas por firmware para bajar los picos | el firmware de ensayo registra la causa de cada reinicio del ESP32; si aparecen reinicios por brownout: trabar los 4 servos y mirar el riel y 3V3 con osciloscopio; shunt de 0,1 Ω en el riel de servos, arranque simultáneo contra escalonado | [direccion-swerve-servos](../20-electronica/direccion-swerve-servos.md) |
| V4 | Márgenes de J1 (12 A nominal) y J4..J7 (3-4 A nominal) | tocar/medir temperatura de los conectores tras un minuto con una rueda trabada | [mapa-conectores](mapa-conectores.md) |
| V5 | Fuga del TVS SM6T15A con la batería recién cargada (13,8-14,4 V) | medir corriente o temperatura de D1 con el rover encendido | [protecciones-desacople](../20-electronica/protecciones-desacople.md) |
| V6 | Pico en el riel conmutado al abrir la seta a plena marcha | osciloscopio en el bus +12 V conmutado durante el corte | [protecciones-desacople](../20-electronica/protecciones-desacople.md) |
| V7 | Estado de los contactos de K1 tras varios ciclos de encendido | continuidad entre nodo A y riel conmutado con la seta apretada (debe dar abierto) | [protecciones-desacople](../20-electronica/protecciones-desacople.md) |
| V8 | Saturación del sensor por encima de 15,5 A | cargar la tracción y ver dónde se aplana la lectura | [sensado-corriente](../20-electronica/sensado-corriente.md) |
| V9 | Tensión real en VOC y utilidad del pin OC | medir VOC con el multímetro y comparar con el cálculo (3,0 V esperados) | [sensado-corriente](../20-electronica/sensado-corriente.md) |
| V10 | Temperatura del AMS1117 del devkit con el LM2596 en 7,2 V | medirla con todo el 3,3 V conectado y Wi-Fi activo | [arquitectura-electrica](../20-electronica/arquitectura-electrica.md) |
| V11 | Signo de la lectura de corriente (IN- a IN+) | carga conocida y ver si Vout sube o baja desde 1,65 V | [sensado-corriente](../20-electronica/sensado-corriente.md) |
| V12 | Lectura de V_bat con la seta apretada (el divisor está post-relé) | apretar la seta y leer el ADC | [arquitectura-electrica](../20-electronica/arquitectura-electrica.md) |

## Resueltos

| # | Qué era | Resultado |
|---|---|---|
| V2 | Disipación de Q1 (IRF9540N, 117 mΩ) sin disipador | **No aplica (27-sep-2026).** Q1 quedó puenteado y no se usa. La polaridad inversa se protege con un conector que solo entra en un sentido. Ver [protecciones-desacople](../20-electronica/protecciones-desacople.md) |

Fuentes de error posibles en este análisis, que conviene tener presentes al revisar cada punto:

- U1, U4 y U5 usan el símbolo `RF_Switch:MASWSS0136` renombrado a NLAS4157. La numeración de pines
  del símbolo coincide con la del NLAS4157 en SC-88A, pero **hay que verificar que la huella
  `SOT-363_SC-70-6` esté con la misma numeración**. Si no lo estuviera, todo el razonamiento sobre
  B0/B1 cambia.
- La asignación de pines de K1 (`Relay:RAYEX-L90S`) se dedujo resolviendo el espejado del símbolo en
  el esquemático. Conviene confirmar con el multímetro cuál par es la bobina y cuál el contacto NO.
- Los valores nominales de corriente de los conectores son de catálogo genérico; el fabricante real
  que se haya comprado puede diferir.

La advertencia de numeración de pines del símbolo del ESP32 —otro caso del mismo problema— está en
[40-firmware/mapa-gpio.md](../40-firmware/mapa-gpio.md).
