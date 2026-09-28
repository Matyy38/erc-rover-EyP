---
titulo: Rover ERC — subsistema de Electrónica y Potencia
estado: vigente
fecha: 2026-09-27
fuente: migracion.md (árbol y mapeo de los cuatro documentos fuente)
---

# Rover ERC — Electrónica y Potencia

Documentación del subsistema de Electrónica y Potencia del rover del European Rover Challenge
(IEEE ITBA I+D). Dos frentes: el **rover final** (40 kg, bus de 24 V, swerve de 4 ruedas) y el **modelo a
escala**, que es un instrumento de medición para cerrar la especificación de motores sin comprar
motores de tamaño real a ciegas.

**Empezá por [00-contexto/00-contexto-maestro.md](00-contexto/00-contexto-maestro.md).** Tiene el
estado, los conflictos abiertos y el mapa.

---

## Árbol

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
    ├── analisis-sensado-superado.md
    ├── migracion-2026-09-16.md
    └── fuentes-originales/
```

---

## Índice

### 00-contexto

| Archivo | Qué tiene |
|---|---|
| [00-contexto-maestro.md](00-contexto/00-contexto-maestro.md) | Documento raíz: estado de los dos frentes, conflictos abiertos, instrumentación, próximos pasos y mapa del repo. |
| [01-alcance-y-fronteras.md](00-contexto/01-alcance-y-fronteras.md) | Qué entra y qué no en el subsistema, con la tabla de interfaces con Mecánica. |
| [02-filosofia-de-diseno.md](00-contexto/02-filosofia-de-diseno.md) | Las reglas no negociables y dónde se aplica cada una. |
| [03-conflictos-abiertos.md](00-contexto/03-conflictos-abiertos.md) | Los tres conflictos que bloquean decisiones y la consulta consolidada a Mecánica. |
| [04-decisiones-descartadas.md](00-contexto/04-decisiones-descartadas.md) | Tabla única de lo que se evaluó y no quedó, con el motivo. |
| [05-principios-transversales.md](00-contexto/05-principios-transversales.md) | Las trece reglas ganadas en el proyecto que aplican más allá de su caso. |

### 10-rover-final

| Archivo | Qué tiene |
|---|---|
| [10-locomocion/requisitos-motor.md](10-rover-final/10-locomocion/requisitos-motor.md) | Torque, velocidad y potencia por rueda; discrepancias de diámetro; implicancias eléctricas. |
| [10-locomocion/candidatos-motores.md](10-rover-final/10-locomocion/candidatos-motores.md) | Estado por candidato, criterio de reducción y criterio de búsqueda del reemplazo. |
| [20-potencia/presupuesto-corriente.md](10-rover-final/20-potencia/presupuesto-corriente.md) | Del torque medido a la corriente de la PDB: escenarios, techo de tracción, potencia continua. |
| [20-potencia/cableado-y-protecciones.md](10-rover-final/20-potencia/cableado-y-protecciones.md) | Calibres, fusibles y la cadena conector-fusible-cable del rover final. |

### 20-modelo-escala

| Archivo | Qué tiene |
|---|---|
| [00-estado-actual.md](20-modelo-escala/00-estado-actual.md) | Estado de validación de la placa por bloque y cambios confirmados respecto del PDF. |
| [10-metodologia-similitud.md](20-modelo-escala/10-metodologia-similitud.md) | λ vigente, revisión de escala por la masa real, altura del CM y constantes recalculadas. |
| [20-electronica/arquitectura-electrica.md](20-modelo-escala/20-electronica/arquitectura-electrica.md) | Cadena de potencia, rieles y presupuesto del riel de 3,3 V. |
| [20-electronica/encoders-pcnt.md](20-modelo-escala/20-electronica/encoders-pcnt.md) | Acondicionamiento de los ocho canales y uso del periférico PCNT. |
| [20-electronica/drivers-bts7960.md](20-modelo-escala/20-electronica/drivers-bts7960.md) | Habilitaciones, estado del canal 4 en el arranque (V1) y desacople local. |
| [20-electronica/direccion-swerve-servos.md](20-modelo-escala/20-electronica/direccion-swerve-servos.md) | Margen del riel de servos (V3), el PTC único y el estado en el arranque. |
| [20-electronica/sensado-corriente.md](20-modelo-escala/20-electronica/sensado-corriente.md) | TMCS1126B4: qué mide, rango, acondicionamiento, umbral de OC (V9) y calibración. |
| [20-electronica/protecciones-desacople.md](20-modelo-escala/20-electronica/protecciones-desacople.md) | Capas de protección, polaridad inversa (V2), TVS (V5, V6), inrush del relé (V7). |
| [30-pcb/especificacion.md](20-modelo-escala/30-pcb/especificacion.md) | Datos generales, corrientes de diseño y anchos de pista. |
| [30-pcb/decisiones-layout.md](20-modelo-escala/30-pcb/decisiones-layout.md) | Masas en estrella, ruteo del TMCS1126, potencia, analógico y mecánica. |
| [30-pcb/mapa-conectores.md](20-modelo-escala/30-pcb/mapa-conectores.md) | Tabla completa de J1 a J19, márgenes de los conectores (V4) y convenciones de cableado. |
| [30-pcb/bom.md](20-modelo-escala/30-pcb/bom.md) | Lista por referencia, ítems sin valor definido y compras fuera de placa. |
| [30-pcb/pendientes-de-verificacion.md](20-modelo-escala/30-pcb/pendientes-de-verificacion.md) | Los doce puntos V1 a V12, cómo se confirma cada uno y dónde se discute. |
| [40-firmware/mapa-gpio.md](20-modelo-escala/40-firmware/mapa-gpio.md) | **Fuente única de verdad del pinout**, con la estrategia de arranque seguro. |
| [50-ensayos/instrumentacion.md](20-modelo-escala/50-ensayos/instrumentacion.md) | Cadena de medición, tiro en barra, instrumento externo y riesgos de medición. |
| [50-ensayos/plan-de-ensayos.md](20-modelo-escala/50-ensayos/plan-de-ensayos.md) | Ensayo de ángulo de deslizamiento, Ensayo A fuera de la PDB y próximos pasos. |

### Carpetas por llenar y archivo

| Archivo | Qué tiene |
|---|---|
| [30-componentes/README.md](30-componentes/README.md) | Ficha por componente y enlaces a hojas de datos. Vacía. |
| [40-proveedores-costos/README.md](40-proveedores-costos/README.md) | Proveedores y costos de importación. Vacía. |
| [90-archivo/README.md](90-archivo/README.md) | Versiones superadas y ramas muertas. **No usar como fuente.** |
| [90-archivo/analisis-sensado-superado.md](90-archivo/analisis-sensado-superado.md) | El análisis que suponía que el TMCS1126 medía el total de la PDB, con su nota de trazabilidad. |
| [90-archivo/migracion-2026-09-16.md](90-archivo/migracion-2026-09-16.md) | Plan con el que se armó este árbol a partir de los cuatro documentos fuente. |
| [90-archivo/fuentes-originales/README.md](90-archivo/fuentes-originales/README.md) | Los cuatro documentos previos a la migración, sin tocar. |

---

## Reglas de trabajo sobre el repo

- **El mapa de GPIO es fuente única de verdad.** Si una tabla de pines aparece en otro archivo, ese
  archivo enlaza al mapa, no lo copia.
- **Los puntos V1 a V12 son hipótesis, no defectos.** Al cerrar uno, se actualiza su fila con el
  resultado de la medición.
- **La placa ya está fabricada y soldada.** Toda propuesta de cambio dice si es un corte y un puente,
  un componente a reemplazar, o un rediseño.
- **Los archivos de KiCad no viven acá.** Se procesan en una conversación y el resultado se asienta
  en markdown.
- **Cada dato técnico tiene un solo dueño.** El contexto maestro es un índice de estado: resume y
  enlaza, no copia números. Si hay diferencia, manda el archivo de detalle.
- **`90-archivo/` es historia.** No se usa como fuente.

## Convenciones

- Contenido en español. Nombres de archivo en minúscula, sin tildes ni espacios, con guiones.
- Carpetas numeradas para forzar orden de lectura.
- Frontmatter YAML con `titulo`, `estado`, `fecha` y `fuente` en cada archivo.
- Enlaces markdown relativos, nunca wikilinks.
- Entre 40 y 400 líneas por archivo de contenido. Por debajo de 40, fusionar con el contiguo.
- **Los archivos de KiCad no se versionan.** Se procesan en una conversación y el resultado se
  documenta acá.
- Registro de decisiones = tabla de descartados, no ADR formales.
- Dentro de tablas, decimales en texto plano con coma. LaTeX solo fuera de tablas.
