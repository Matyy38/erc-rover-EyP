---
titulo: Plan de migración — fragmentar los documentos fuente en el árbol del repo
fecha: 2026-09-16
destinatario: Claude Code
estado: listo para ejecutar
---

# Plan de migración

Instrucciones para construir el repositorio. Ejecutar en orden. No inventar contenido: todo sale de
los cuatro documentos fuente listados abajo.

## Documentos fuente

| Archivo | Rol |
|---|---|
| `00-contexto-maestro.md` | Documento raíz. Va casi entero a `00-contexto/`. |
| `pcb-modelo-escala.md` | Detalle de la placa. **Ya viene fragmentado**: cada `##` es un archivo destino. |
| `metodologia-modelo-escala-y-presupuesto-pdb.md` | Escala, instrumentación y presupuesto de corriente. Se reparte entre tres carpetas. |
| `requisitos-motor-rover-real.md` | Requisitos del motor grande. Va casi entero a un archivo. |

## Reglas de fragmentación

1. **No reescribir.** Mover el texto tal cual, salvo lo que estas instrucciones indiquen
   explícitamente. Es contenido ya revisado.
2. **Frontmatter en cada archivo:** `titulo`, `estado`, `fecha`, `fuente` (de qué documento salió).
3. **Los enlaces cruzados son markdown relativo**, nunca wikilinks.
4. **Si una sección queda por debajo de 40 líneas**, fusionarla con la contigua en lugar de crear un
   archivo anémico. Los casos candidatos están marcados abajo.
5. **No duplicar tablas entre archivos.** Si dos archivos necesitan la misma tabla, uno la tiene y el
   otro enlaza.
6. **Al final, generar `README.md`** con el árbol y una línea de descripción por archivo.

---

## Árbol a crear

```
rover-erc/
├── README.md
├── CLAUDE.md
├── 00-contexto/
│   ├── 00-contexto-maestro.md
│   ├── 01-alcance-y-fronteras.md
│   ├── 02-filosofia-de-diseno.md
│   ├── 03-conflictos-abiertos.md
│   ├── 04-decisiones-descartadas.md
│   └── 05-principios-transversales.md
├── 10-rover-final/
│   ├── 10-locomocion/
│   │   ├── requisitos-motor.md
│   │   └── candidatos-motores.md
│   └── 20-potencia/
│       ├── presupuesto-corriente.md
│       └── cableado-y-protecciones.md
├── 20-modelo-escala/
│   ├── 00-estado-actual.md
│   ├── 10-metodologia-similitud.md
│   ├── 20-electronica/
│   │   ├── arquitectura-electrica.md
│   │   ├── encoders-pcnt.md
│   │   ├── drivers-bts7960.md
│   │   ├── direccion-swerve-servos.md
│   │   ├── sensado-corriente.md
│   │   └── protecciones-desacople.md
│   ├── 30-pcb/
│   │   ├── especificacion.md
│   │   ├── decisiones-layout.md
│   │   ├── mapa-conectores.md
│   │   ├── bom.md
│   │   └── pendientes-de-verificacion.md
│   ├── 40-firmware/
│   │   └── mapa-gpio.md
│   └── 50-ensayos/
│       ├── instrumentacion.md
│       └── plan-de-ensayos.md
├── 30-componentes/
├── 40-proveedores-costos/
└── 90-archivo/
```

---

## Mapeo detallado

### Desde `00-contexto-maestro.md`

| Sección origen | Destino |
|---|---|
| Completo | `00-contexto/00-contexto-maestro.md` (queda como raíz, sin recortar) |
| §2 Alcance | también `00-contexto/01-alcance-y-fronteras.md`, ampliado con la tabla de interfaces con Mecánica |
| §3 Reglas de diseño | `00-contexto/02-filosofia-de-diseno.md` |
| §4 Conflictos abiertos | `00-contexto/03-conflictos-abiertos.md` |
| §8 Principios transversales | `00-contexto/05-principios-transversales.md` |

El maestro conserva versiones resumidas de §2, §3, §4 y §8 con enlace al archivo detallado. Es el
único caso donde se permite duplicación parcial, porque el maestro tiene que poder leerse solo.

### Desde `pcb-modelo-escala.md`

Este documento **ya está fragmentado en origen**. Cada encabezado `##` corresponde a un archivo:

| Sección origen | Destino |
|---|---|
| Encabezado + estado de validación + lista de cambios respecto del PDF | `20-modelo-escala/00-estado-actual.md` |
| `## arquitectura-electrica` | `20-modelo-escala/20-electronica/arquitectura-electrica.md` |
| `## encoders-pcnt` | `20-modelo-escala/20-electronica/encoders-pcnt.md` |
| `## drivers-bts7960` | `20-modelo-escala/20-electronica/drivers-bts7960.md` |
| `## direccion-swerve-servos` | `20-modelo-escala/20-electronica/direccion-swerve-servos.md` |
| `## sensado-corriente` | `20-modelo-escala/20-electronica/sensado-corriente.md` |
| `## protecciones-desacople` | `20-modelo-escala/20-electronica/protecciones-desacople.md` |
| `## especificacion` | `20-modelo-escala/30-pcb/especificacion.md` |
| `## decisiones-layout` | `20-modelo-escala/30-pcb/decisiones-layout.md` |
| `## mapa-conectores` | `20-modelo-escala/30-pcb/mapa-conectores.md` |
| `## bom` | `20-modelo-escala/30-pcb/bom.md` |
| `## pendientes-de-verificacion` | `20-modelo-escala/30-pcb/pendientes-de-verificacion.md` |

**Dos operaciones especiales sobre este documento:**

**(a) Extraer el mapa de GPIO.** La tabla "Mapeo de pines del ESP32" y la subsección "Estrategia de
arranque seguro" que están dentro de `## arquitectura-electrica` se **mueven** a
`20-modelo-escala/40-firmware/mapa-gpio.md`. En `arquitectura-electrica.md` queda un enlace, no una
copia. Junto con ellas va la advertencia de numeración de pines del símbolo de KiCad, que es
información que se pierde fácil y cuesta caro.

**(b) Confirmar la ubicación del sensor.** Este documento es el correcto: el TMCS1126 mide **solo la
rama de tracción**. En `sensado-corriente.md`, en la subsección "Dónde está conectado", agregar una
nota que diga que hubo una contradicción con `metodologia-modelo-escala` §3, que se resolvió a favor
de este documento el 16-sep, y que el análisis superado quedó en `90-archivo/`. Agregar también el
único término parásito real: el VCC lógico de los cuatro BTS7960 (10–30 mA), constante y restable
como línea de base en el tarado de cero de cada sesión.

### Desde `metodologia-modelo-escala-y-presupuesto-pdb.md`

Este es el que más se reparte:

| Sección origen | Destino |
|---|---|
| §1 completo (qué aporta el modelo, torque a corriente, escenarios, techo de tracción) | `10-rover-final/20-potencia/presupuesto-corriente.md` |
| §2 (medición sin INA226, ADS1115, tiro en barra, instrumentos) | `20-modelo-escala/50-ensayos/instrumentacion.md` |
| §3 | **parcialmente superada, ver operación especial abajo** |
| §4 (revisión de escala por masa real, las tres salidas, CM) | `20-modelo-escala/10-metodologia-similitud.md` |
| §5 Riesgos y trade-offs | repartir: los de escala a `10-metodologia-similitud.md`, los de medición a `instrumentacion.md` |
| §6 Próximos pasos | `20-modelo-escala/50-ensayos/plan-de-ensayos.md` |
| Anexo de constantes recalculadas | `20-modelo-escala/10-metodologia-similitud.md`, al final |
| §2.7 (ensayo de ángulo de deslizamiento) | `plan-de-ensayos.md` **y** referenciado desde `03-conflictos-abiertos.md` |

**Operación especial: §3 está parcialmente superada.** Su premisa es incorrecta — el sensor mide solo
la rama de tracción, no el total de la PDB. Pero no toda la sección se cae con la premisa. Dividir
así:

| Subsección | Qué dice | Destino |
|---|---|---|
| §3.1 El problema | El sensor mide el total, los servos contaminan el Ensayo D | **`90-archivo/analisis-sensado-superado.md`** |
| §3.2 Opciones ordenadas | Plan A/C/B | `90-archivo/`, salvo la opción A |
| §3.2 opción A | Ensayo A fuera de la PDB, con el motor directo a la fuente | `50-ensayos/plan-de-ensayos.md` — sigue vigente por su propio motivo |
| §3.3 Por qué el instrumento externo es buena idea | Aísla el muestreo, es reutilizable, no toca la placa | `50-ensayos/instrumentacion.md` — **íntegra, sigue vigente** |
| §3.4 Requisitos del instrumento externo | Sensor, conversor, MCU, sincronización | `50-ensayos/instrumentacion.md` — **íntegra** |

En `instrumentacion.md`, encabezar la parte del instrumento externo aclarando que pasó de corrección
obligatoria a mejora opcional, y sumar la alternativa barata: mantener el sensor de a bordo y subir
solo el conversor a ADS1115, aceptando que el muestreo comparta CPU con el lazo de control. Para
treinta segundos a 100 muestras por segundo probablemente alcance, y conviene medirlo antes de gastar
15–20 USD.

El archivo de `90-archivo/analisis-sensado-superado.md` lleva frontmatter con `estado: superado` y
una línea arriba explicando qué se creyó, qué se verificó y cuándo. La trazabilidad importa: sin esa
nota, dentro de tres meses el razonamiento vuelve a aparecer desde cero.

### Desde `requisitos-motor-rover-real.md`

| Sección origen | Destino |
|---|---|
| Todo salvo "Estado de candidatos" | `10-rover-final/10-locomocion/requisitos-motor.md` |
| "Estado de candidatos" | `10-rover-final/10-locomocion/candidatos-motores.md` |
| "Implicancias eléctricas" (cableado, fusibles, calibres) | también `10-rover-final/20-potencia/cableado-y-protecciones.md` |
| "Conflicto de torque μ / pendiente" | consolidar en `00-contexto/03-conflictos-abiertos.md`, dejando enlace en el archivo de requisitos |

**Operación especial:** el archivo de requisitos declara 480 W de tracción sostenida. El documento de
metodología corrige ese número a 247 W con el razonamiento de que torque de obstáculo y velocidad
máxima nunca ocurren juntos. En `requisitos-motor.md`, dejar el 480 W con una nota que enlace al
cálculo corregido en `presupuesto-corriente.md`. No borrar el número viejo: el contraste es
informativo.

### `00-contexto/04-decisiones-descartadas.md`

Consolidar en una sola tabla, desde los cuatro documentos:

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

Dos filas tienen el motivo pendiente. Dejarlas marcadas como tales, no inventar la razón.

---

## Carpetas que se crean vacías

`30-componentes/`, `40-proveedores-costos/` y `90-archivo/` se crean con un `README.md` de una línea
explicando para qué son. Se llenan después.

---

## Verificación final

Antes de dar por terminada la migración, comprobar:

- [ ] Ningún archivo supera 400 líneas ni baja de 40 (salvo los README de carpeta).
- [ ] La tabla de GPIO aparece **una sola vez** en todo el repo.
- [ ] Los 12 puntos V1–V12 están completos en `pendientes-de-verificacion.md` y cada uno enlaza a la
      sección donde se discute.
- [ ] El análisis superado del sensado está en `90-archivo/` con `estado: superado` y su nota de
      trazabilidad, y `sensado-corriente.md` lo referencia.
- [ ] En todo el repo, la única afirmación vigente sobre el sensor es que mide **solo la rama de
      tracción**. Ninguna copia activa dice lo contrario.
- [ ] `03-conflictos-abiertos.md` tiene tres conflictos, no cuatro, y la consulta consolidada a
      Mecánica al final.
- [ ] Todo archivo tiene frontmatter con `titulo`, `estado`, `fecha`, `fuente`.
- [ ] `README.md` lista el árbol completo con una línea por archivo.
- [ ] Ningún enlace roto.
