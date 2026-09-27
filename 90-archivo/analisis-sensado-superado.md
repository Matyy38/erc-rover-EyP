---
titulo: Análisis superado — el sensor mediría el total de la PDB
estado: superado
fecha: 2026-09-16
fuente: metodologia-modelo-escala-y-presupuesto-pdb.md §3.1 y §3.2
---

# Análisis superado: la limitación del sensado que no era

> **Qué se creyó, qué se verificó y cuándo.**
>
> **Se creyó** (hasta el 15-sep-2026) que el TMCS1126 de la placa del modelo medía el **consumo total
> de la PDB** —tracción más servos más lógica— y que por eso el Ensayo D quedaba invalidado: los
> picos de los servos de dirección, de hasta 10 A, son del mismo orden que la señal de tracción que
> se quiere medir. De esa premisa salieron tres correcciones: un puente sobre la placa, un instrumento
> externo obligatorio, y el Ensayo A fuera de la PDB.
>
> **Se verificó** el 16-sep-2026, contra el esquemático de KiCad, que la premisa es **incorrecta**. El
> sensor está en la rama conmutada, entre el contacto NO del relé y el riel de +12 V de los cuatro
> drivers. El LM2596 —y con él el ESP32 y los servos— cuelga del nodo A, aguas arriba del relé, y no
> atraviesa el sensor. Lo confirma una consecuencia arquitectónica ya documentada: al apretar la seta
> muere el bus de los drivers pero los servos y el ESP32 siguen alimentados.
>
> **Consecuencia:** el sensor mide **solo la rama de tracción**, el Ensayo D es válido con el sensor
> de a bordo, el puente sobre la placa es innecesario y el instrumento externo pasa a mejora
> opcional. El único término parásito es el VCC lógico de los cuatro BTS7960 (10 a 30 mA), constante
> y restable como línea de base.
>
> **Vigente hoy:**
> [20-modelo-escala/20-electronica/sensado-corriente.md](../20-modelo-escala/20-electronica/sensado-corriente.md)
> e [20-modelo-escala/50-ensayos/instrumentacion.md](../20-modelo-escala/50-ensayos/instrumentacion.md).
>
> Se conserva el texto original porque sin esta nota el mismo razonamiento vuelve a aparecer desde
> cero dentro de tres meses.

---

## Texto original (superado)

### 3.1 El problema

El TMCS1126 **no está midiendo la rama de tracción: está midiendo el consumo total de la PDB.** Eso incluye tracción, servos de dirección, lógica y cualquier otra carga aguas abajo del relé.

Consecuencias por ensayo:

| Ensayo | ¿Afecta? | Por qué |
|---|---|---|
| A (calibración) | no, si se hace en banco | el motor se alimenta directo desde la fuente, fuera de la PDB |
| B (fcem) | no | ídem |
| C (encoder) | no | no usa corriente |
| **D (tracción)** | **sí, lo invalida** | los servos consumen durante el ensayo y su consumo no es constante |

El punto crítico es el Ensayo D. Restar una línea de base funciona para un consumo **constante** (lógica), pero los servos de dirección del swerve mantienen torque durante el ascenso de la rampa y su corriente varía de forma impredecible, con picos de varios amperes. Ese consumo entra sumado a la corriente de tracción y no hay forma de separarlo a posteriori.

Peor: los picos de servo son del mismo orden que la señal que se quiere medir (2,6 A de tracción contra picos de servo de hasta 10 A por la especificación de la rama), así que no es un error pequeño sino una contaminación total de la medición.

### 3.2 Opciones ordenadas

La opción A de esta tabla (Ensayo A fuera de la PDB) **no está acá**: sigue vigente por un motivo
propio, independiente de esta premisa, y se movió a
[20-modelo-escala/50-ensayos/plan-de-ensayos.md](../20-modelo-escala/50-ensayos/plan-de-ensayos.md).

| Opción | Qué implica | Costo | Veredicto |
|---|---|---|---|
| **B. Puente sobre la placa actual** | cortar la pista de entrada a la rama de tracción y reinsertar el sensor ahí; si hay portafusible o conector en esa rama, se hace sin cortar nada | 0 a 2 | **la corrección definitiva** para la placa del modelo |
| **C. Instrumento externo independiente** | módulo aparte con sensor Hall + ADS1115 + microcontrolador propio, intercalado en el cable de la rama de tracción con conectores XT | 15 a 20 | **la mejor opción para los ensayos** |
| D. Sensor por motor | cuatro sensores, cuatro canales | 60 | no se justifica (ver 2.1) |

**Plan adoptado: A para el Ensayo A, C para el Ensayo D, B como corrección definitiva de la placa.**

Ese plan adoptado ya no rige: la opción B es innecesaria y la C quedó degradada a opcional.
