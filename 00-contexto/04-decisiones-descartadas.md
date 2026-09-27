---
titulo: Decisiones descartadas
estado: vigente
fecha: 2026-09-16
fuente: consolidado de los cuatro documentos (contexto maestro, pcb-modelo-escala, metodologia-modelo-escala, requisitos-motor)
---

# Decisiones descartadas

Registro de lo que se evaluó y no quedó, con el motivo. Sirve para no volver a proponer lo mismo
dentro de tres meses. No son ADR formales: es una tabla.

Dos filas tienen el motivo pendiente de registrar. Están marcadas como tales y no se inventa la
razón.

| Qué | Motivo |
|---|---|
| 6WD | (motivo pendiente de registrar) |
| Rocker-bogie | (motivo pendiente de registrar) |
| Skid-steer en el modelo | Se pasó a swerve, con un servo de dirección por rueda |
| N20 100 rpm | RPM y torque insuficientes |
| GA25-370 1/46.8 | Hoja de datos inconsistente, ventana de masa vacía |
| NEMA 17 + AS5600 | Corriente de fase constante: la corriente deja de ser proxy de torque |
| GoBILDA Saturn 5304-8002-0188 | Térmicamente válido pero sobredimensionado |
| 42RBL04A | El proveedor nunca respondió; interfaz del controlador nunca confirmada |
| Reducción 1/49 | 98% del torque nominal en continuo, sin margen térmico |
| λ = 0,34 / 4,6 kg | Superado por la masa real de 5,5 kg |
| INA226 | No cubre el pico de 5,5 A; no se consiguen módulos de más de 5 A en Argentina |
| ACS712-30A | Reemplazado por TMCS1126B4 (Hall aislado) |
| Sensor de corriente por motor | El error de ganancia se cancela con un solo sensor |
| Pines IS de los BTS7960 | Sin cablear: sin medición por rueda |
| 4 PTC de servo | Reducidos a uno solo para todo el riel |
| IRF4905 | Reemplazado por IRF9540N en la placa (queda como candidato si V2 da alto) |
| Capacitor de 10 nF en encoder | Destruye el flanco con esas impedancias |
| TVS de 12,8 V de standoff | Avalancha continua con la batería a 13,8–14,4 V |
| Bajar el modelo a 4,6 kg | Difícil, no vale la pena: se ajusta el diámetro de rueda |
| Corregir el desfasaje de λ a mano | El término de compactación no tiene corrección exacta sin caracterizar la arena |

---

## Dónde se discute cada descarte

| Fila | Discusión completa |
|---|---|
| Motores del rover final (42RBL04A, reducciones, candidatos) | [candidatos-motores.md](../10-rover-final/10-locomocion/candidatos-motores.md) |
| Escala (λ = 0,34, bajar a 4,6 kg, corregir a mano) | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) |
| Cadena de medición (INA226, sensor por motor, ACS712) | [instrumentacion.md](../20-modelo-escala/50-ensayos/instrumentacion.md) y [sensado-corriente.md](../20-modelo-escala/20-electronica/sensado-corriente.md) |
| Pines IS, pull-down de PWM | [drivers-bts7960.md](../20-modelo-escala/20-electronica/drivers-bts7960.md) |
| 4 PTC de servo | [direccion-swerve-servos.md](../20-modelo-escala/20-electronica/direccion-swerve-servos.md) |
| IRF4905 y TVS de 12,8 V | [protecciones-desacople.md](../20-modelo-escala/20-electronica/protecciones-desacople.md) |
| Capacitor de 10 nF en encoder | [encoders-pcnt.md](../20-modelo-escala/20-electronica/encoders-pcnt.md) |
| Skid-steer en el modelo | [10-metodologia-similitud.md](../20-modelo-escala/10-metodologia-similitud.md) §8 |
