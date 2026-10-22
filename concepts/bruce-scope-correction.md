## Korrektur des AXIOM-Scopes

**Festgelegt:** AXIOM-AI ist ausschließlich ein **Softwareprojekt**.

Nicht Bestandteil von AXIOM-AI sind:

- Hosting-Architekturen,
- Service-Zellen für Kundenhosting,
- Rechenzentren,
- physische Reaktoren,
- chemische oder mechanische Konzepte,
- spezielle Hardwareprodukte,
- physikalische „Paralleluniversen“ oder Quantensysteme.

CPU-RAM-NPU, NanoCell, NanoNeuron und ExecutionCell sind innerhalb von AXIOM-AI **Software- und Runtime-Architekturen**. „NPU“ beschreibt hier zunächst die Art der Ausführung, nicht zwingend einen herzustellenden Chip.

Hardware wird nur abstrakt betrachtet:

- **CPU:** deterministisches Routing, Regeln, Bitoperationen und Runtime-Ausführung.
- **GPU:** optionales SLM-, Embedding- oder Wissenstraining sowie stark parallele Batch-Verarbeitung.
- **Cache:** unmittelbar benötigte NanoNeuron-Projektionen und Operatoren.
- **RAM:** aktive ExecutionCells, Hot Skills, Indizes und AnswerPoints.
- **VRAM:** nur bei GPU-Ausführung oder Training.
- **SSD:** kanonisches Wissen, Erfahrungen, Traces, Trainingsdaten und Modellstände.

Die aktuelle Nutzerkorrektur ersetzt damit ältere oder zu breit gefasste Verbindungen. Die hochgeladene Schlagwortsammlung bestätigt den AXIOM-Kern aus NanoCell, CPU-RAM-NPU, ExecutionCell, NanoNeuron, Universe Engine, AnswerPoints, Experience-System, PeacemakerLLM, AXIOM-Math und Semantic Routing; nicht zum Softwarekern passende Begriffe werden aus dem aktiven Scope entfernt.

## Korrigierte AXIOM-AI-Hierarchie

```text
AXIOM-AI
│
├─ AXIOM-Core
│  ├─ Eingabeverständnis
│  ├─ Semantic Routing
│  ├─ Cascade-Filterung
│  ├─ Wissensweg-Berechnung
│  ├─ Aussagenbildung
│  ├─ Verifikation
│  └─ Traceability
│
├─ PeacemakerLLM
│  ├─ PostgreSQL Knowledge Repository
│  ├─ Language Engine
│  ├─ Rule Engine
│  ├─ Direct Relation Engine
│  ├─ Relation Profiles
│  ├─ dynamische Lücken-/Baustein-Templates
│  ├─ statische Lücken-/Baustein-Templates
│  └─ nachvollziehbare Antworten
│
├─ AXIOM-Math
│  ├─ exakte Formelauswertung
│  ├─ BigInt
│  ├─ rationale Arithmetik
│  ├─ modulare Arithmetik
│  ├─ Benchmarks
│  ├─ Domain Enforcement
│  └─ Safe Shutdown
│
├─ NanoCell Runtime
│  ├─ CPU-RAM-NPU
│  ├─ ExecutionCell
│  ├─ NanoNeuron
│  ├─ Active Projection
│  ├─ Universe Engine
│  ├─ Pattern Engine
│  ├─ Technique Engine
│  ├─ AnswerPoint-System
│  └─ Experience-System
│
└─ Sprachrekonstruktion
   ├─ strukturierter Renderer
   ├─ themenorientiertes SLM
   ├─ Embedding-Modell
   └─ Semantic Roundtrip Validator
```

**Zusätzliche Korrektur:** PeacemakerLLM verwendet nach deiner aktuellen Festlegung **PostgreSQL**, nicht SQLite.

---

# Wie viele Prozesse entstehen tatsächlich aus einem Byte?

## 1. Der unveränderliche Ausgangspunkt

Ein Byte enthält:

\[
8\ \text{Bits}
\]

Es kann darstellen:

\[
2^8=256
\]

unterschiedliche Zustände.

Beispiel:

```text
00000000
00000001
00000010
...
11111111
```

Das bedeutet:

- **8 Bitpositionen**
- **256 mögliche Bytewerte**
- aber zunächst nur **ein aktuell geladener Bytewert**

Ein konkretes NanoNeuron könnte beispielsweise aktuell enthalten:

```text
10110100
```

AXIOM speichert diesen Ausgangswert nicht tausendfach. Stattdessen führt AXIOM verschiedene Berechnungsrezepte auf demselben Wert aus.

---

# 2. Dimension, Prozesspunkt und Operation trennen

Diese drei Begriffe dürfen nicht vermischt werden.

## Prozessdimension

Eine Dimension ist eine auswählbare Achse, beispielsweise:

```text
Shift-Position
Rotation
Gate
Maske
Kontext
Technik
```

## Prozesspunkt

Ein Prozesspunkt ist eine konkrete Auswahl auf allen Achsen:

```text
Shift links 2
+ Rotation rechts 3
+ XOR
+ Maske 10110000
+ Kontext Fahrzeug
```

## Operation

Eine Operation ist die tatsächlich ausgeführte Berechnung dieses Punktes.

Damit gilt:

```text
Dimensionen
→ bilden einen theoretischen Koordinatenraum

Prozesspunkte
→ sind mögliche Koordinaten

Operationen
→ sind tatsächlich berechnete Koordinaten

AnswerPoints
→ sind wenige behaltene Ergebnisse
```

---

# 3. Dein einfaches 8er-Dimensionsmodell

Wenn jede zusätzliche Prozessachse genau acht Möglichkeiten besitzt, gilt:

\[
P(D)=8^D
\]

Dabei ist \(D\) die Anzahl der Achtfachachsen.

| Prozessachsen | Theoretische Prozesspunkte | Speicher bei 1 Byte je gespeichertem Punkt |
|---:|---:|---:|
| 1 | 8 | 8 Byte |
| 2 | 64 | 64 Byte |
| 3 | 512 | 512 Byte |
| 4 | 4.096 | 4 KiB |
| 5 | 32.768 | 32 KiB |
| 6 | 262.144 | 256 KiB |
| 7 | 2.097.152 | 2 MiB |
| 8 | 16.777.216 | 16 MiB |
| 9 | 134.217.728 | 128 MiB |
| 10 | 1.073.741.824 | 1 GiB |
| 11 | 8.589.934.592 | 8 GiB |
| 12 | 68.719.476.736 | 64 GiB |
| 13 | 549.755.813.888 | 512 GiB |
| 14 | 4.398.046.511.104 | 4 TiB |
| 15 | 35.184.372.088.832 | 32 TiB |
| 16 | 281.474.976.710.656 | 256 TiB |

Der wichtige Unterschied:

> AXIOM kann einen Raum mit theoretisch 281 Billionen Prozesspunkten **beschreiben**, ohne 256 TiB an Resultaten anzulegen.

AXIOM muss trotzdem die tatsächlich besuchten Punkte berechnen. Der virtuelle Raum beseitigt den Speicherbedarf, aber nicht automatisch den Rechenaufwand.

---

# 4. Korrektur der bisherigen Layerzählung

Die saubere Folge lautet:

```text
eine Achse:
8¹ = 8

zwei Achsen:
8² = 64

drei Achsen:
8³ = 512

vier Achsen:
8⁴ = 4.096

fünf Achsen:
8⁵ = 32.768

sechs Achsen:
8⁶ = 262.144
```

Dein früheres:

```text
512 xyz × 512 xyz
```

ergibt:

\[
512\cdot512
=
262.144
=
8^6
\]

Das ist kein gewöhnlicher vierdimensionaler Raum, sondern ein **Vergleich zweier dreiachsiger Prozesswürfel**:

```text
Cube A:
8 × 8 × 8 = 512

Cube B:
8 × 8 × 8 = 512

Cube-to-Cube:
512 × 512 = 262.144 Beziehungen
```

Innerhalb von AXIOM kann das trotzdem eine eigene logische Ebene heißen, beispielsweise:

```text
Relation Layer
Cube-to-Cube Layer
Parallel-Universe Comparison Layer
```

Mathematisch enthält sie jedoch sechs Achtfachachsen.

---

# 5. Konkreter Operatorraum eines Bytes

Neben dem abstrakten \(8^D\)-Modell können wir die tatsächlich vorgesehenen Operatoren zählen.

## Logische Shifts

Für ein Byte:

```text
Identität:          1
Shift links 1–7:    7
Shift rechts 1–7:   7
```

Damit entstehen:

\[
1+7+7=15
\]

beschriftete Shift-Zustände.

Shift um acht oder mehr würde bei klassischer 8-Bit-Semantik gesondert behandelt oder auf null beziehungsweise ungültig gesetzt werden.

## Rotationen

Ein Byte besitzt maximal acht zyklische Orientierungen:

```text
Rotation 0
Rotation 1
Rotation 2
...
Rotation 7
```

Damit entstehen:

\[
8
\]

eindeutige Rotationspositionen.

Links- und Rechtsrotation können als unterschiedliche Aufgaben bezeichnet werden. Mathematisch gilt aber beispielsweise:

```text
ROTL 1 = ROTR 7
ROTL 2 = ROTR 6
```

Daher liefern beide Richtungen zusammen nicht automatisch 16 unterschiedliche Byteorientierungen.

## Masken

Eine 8-Bit-Maske kann ebenfalls jeden Bytewert besitzen:

\[
2^8=256
\]

Damit existieren:

\[
256
\]

mögliche Masken.

## Binäre Gates

Nehmen wir die sechs binären Gate-Typen:

```text
AND
OR
XOR
NAND
NOR
XNOR
```

Dann gibt es sechs Gate-Auswahlen.

`NOT` wird getrennt behandelt, weil es nur einen Operanden benötigt.

---

# 6. Konkretes Beispiel für den theoretischen AXIOM-Raum

Wenn eine Technik frei kombinieren dürfte:

- 15 Shift-Zustände,
- 8 Rotationspositionen,
- 6 binäre Gates,
- 256 Masken,
- \(C\) Kontexte,

dann entstehen für **einen aktuellen Bytewert**:

\[
15\cdot8\cdot6\cdot256\cdot C
\]

also:

\[
184.320\cdot C
\]

mögliche Prozessrezepte.

Bei einem einzigen Kontext:

\[
184.320
\]

Bei 16 Kontexten:

\[
184.320\cdot16
=
2.949.120
\]

Bei 256 Kontexten:

\[
184.320\cdot256
=
47.185.920
\]

Das sind mögliche **Berechnungspfade aus einem einzigen aktuell geladenen Byte**.

Mit \(T\) verschiedenen Techniken lautet die allgemeine Formel:

\[
P=
15\cdot8\cdot6\cdot256\cdot C\cdot T
\]

also:

\[
P=184.320\cdot C\cdot T
\]

Beispiel mit:

- 16 Kontexten,
- 32 Techniken,

ergibt:

\[
184.320\cdot16\cdot32
=
94.371.840
\]

theoretische Prozessrezepte.

Diese Zahl ist ein **Kombinationsraum**, keine Empfehlung, alle Rezepte auszuführen.

---

# 7. Was wird davon tatsächlich berechnet?

Nicht:

```text
94.371.840 Prozesse blind ausführen
```

Sondern beispielsweise:

```text
94.371.840 theoretisch mögliche Prozesse
              ↓
Themenfilter
              ↓
5.000 passende Prozesse
              ↓
Shape-Filter
              ↓
320 Kandidaten
              ↓
Kontextfilter
              ↓
48 Kandidaten
              ↓
Erfahrungsfilter
              ↓
16 tatsächlich auszuführende Universen
              ↓
4 gültige AnswerPoints
```

Das ist die eigentliche Funktion der AXIOM-Cascade-Filterung.

Damit müssen wir drei Zahlen getrennt messen:

```text
P_theoretical
= alle beschreibbaren Prozesspunkte

P_candidates
= nach Routing ausgewählte Kandidaten

P_executed
= tatsächlich berechnete Prozesse
```

Zusätzlich:

```text
P_retained
= als AnswerPoint oder Erfahrung behaltene Ergebnisse
```

Beispiel:

```text
P_theoretical = 94.371.840
P_candidates  = 320
P_executed    = 16
P_retained    = 4
```

Das wäre eine Reduktion der tatsächlichen Ausführung gegenüber dem theoretischen Raum um den Faktor:

\[
\frac{94.371.840}{16}
=
5.898.240
\]

---

# 8. Was sparen wir im Speicher?

Nehmen wir einen sechsdimensionalen Achtfachraum:

\[
8^6=262.144
\]

## Vollständig gespeichert

Bei nur einem Byte je Prozesspunkt:

```text
262.144 Byte = 256 KiB
```

Bei einem acht Byte großen Ergebnis:

```text
262.144 × 8 Byte
= 2.097.152 Byte
= 2 MiB
```

## Lazy verarbeitet

AXIOM benötigt gleichzeitig möglicherweise nur:

```text
1 Byte aktive NanoNeuron-Projektion
+ aktueller Operatorzustand
+ wenige Schleifenkoordinaten
+ Akkumulator
+ aktueller AnswerPoint
+ Trace-Referenz
```

Die genaue Größe ist noch offen, weil UniverseSpec und AnswerPoint nicht final festgelegt sind. Sie hängt jedoch nicht direkt von allen 262.144 theoretischen Punkten ab.

Speicherkomplexität:

```text
vollständig materialisiert:
O(8^D)

gestreamt oder lazy:
O(1) bis O(D)
```

Die Laufzeit kann im schlechtesten Fall weiterhin sein:

```text
O(8^D)
```

Aber durch Cascade-Filterung soll sie praktisch näher an der tatsächlich relevanten Kandidatenmenge liegen:

```text
O(P_executed)
```

---

# 9. Sehr wichtiger Unterschied: Prozesspfade und Ergebnisse

Obwohl AXIOM Millionen Prozesse aus einem Byte definieren kann, kann ein reines 8-Bit-Ergebnis weiterhin nur einen von 256 Werten besitzen:

\[
0\ldots255
\]

Das bedeutet:

> Millionen unterschiedliche Prozesspfade können zum selben Byteergebnis führen.

Beispiel:

```text
Pfad A:
Shift → XOR → Maske
Resultat: 00110100

Pfad B:
Rotation → AND → Kontext
Resultat: 00110100
```

Das Byteergebnis ist gleich, aber die Bedeutung kann verschieden sein.

Deshalb benötigt AXIOM zusätzlich:

```text
Universe-ID
Technique-ID
Context-ID
Relation-ID
Trace-ID
```

Der AnswerPoint darf nicht nur das rohe Ergebnisbyte speichern.

```text
AnswerPoint
├─ Ergebnis
├─ Bedeutung
├─ Kontext
├─ angewandte Technik
└─ Nachweis
```

---

# 10. Theoretische absolute Obergrenze

Es gibt noch eine deutlich größere theoretische Betrachtung.

## Alle möglichen Funktionen von 8 Bit auf 1 Bit

Ein Byte besitzt 256 mögliche Eingaben. Für jede dieser Eingaben kann eine Boolesche Funktion entweder 0 oder 1 ausgeben.

Daher existieren:

\[
2^{256}
\]

verschiedene mögliche Boolesche Funktionen von acht Eingangsbits auf ein Ausgangsbit.

Das sind ungefähr:

\[
1{,}16\cdot10^{77}
\]

Funktionen.

## Alle möglichen Funktionen von 8 Bit auf 8 Bit

Jede der 256 Eingaben kann einen von 256 Ausgangswerten erhalten.

Daher existieren:

\[
256^{256}
=
2^{2048}
\]

mögliche deterministische Byte-zu-Byte-Abbildungen.

Das sind ungefähr:

\[
3{,}23\cdot10^{616}
\]

Funktionen.

Diese Werte zeigen:

> Ein Byte besitzt nur 256 Zustände, aber eine astronomische Menge möglicher Regeln, die diese Zustände auf Ergebnisse abbilden können.

AXIOM soll selbstverständlich nicht all diese Funktionen durchsuchen. AXIOM nutzt eine kleine, strukturierte und erfahrungsbasierte Auswahl aus:

- Shifts,
- Rotationen,
- Gates,
- Masken,
- Kontexten,
- Pattern-Funktionen,
- TechniqueDefinitions.

---

# 11. Reale Speicherrollen in AXIOM-AI

## Cache

Verwendungszweck:

- aktuell verarbeitete Zustandsbytes,
- häufige Gate- und Maskendaten,
- unmittelbar benötigte Technikschritte,
- kleine Akkumulatoren.

Nicht dauerhaft berechenbar, weil Cachegröße und Belegung von CPU und Laufzeit abhängen.

## RAM

Für \(N\) aktive NanoNeuronen:

### Kanonisch

\[
8N\ \text{Byte}
\]

### Als 1-Byte-Projektion

\[
N\ \text{Byte}
\]

Beispiele:

| NanoNeuronen | Kanonisch mit 8 Byte | Aktive Projektion mit 1 Byte |
|---:|---:|---:|
| 1.000 | 8 KB | 1 KB |
| 1.000.000 | 8 MB | 1 MB |
| 100.000.000 | 800 MB | 100 MB |
| 1.000.000.000 | 8 GB | 1 GB |

Zusätzlich benötigt werden:

- Indizes,
- TechniqueDefinitions,
- Context-Daten,
- AnswerPoints,
- Hot-Skill-Cache,
- Scratchpads.

Deren Größe kann erst nach Definition der Datenverträge exakt berechnet werden.

## VRAM

Nur erforderlich, wenn:

- Gate- oder Pattern-Batches auf einer GPU ausgeführt werden,
- ein Embedding-Modell läuft,
- das themenorientierte SLM trainiert oder inferiert wird.

Die theoretischen Prozessräume müssen auch im VRAM nicht vollständig materialisiert werden.

## SSD

Verwendungszweck:

- kanonische Wissensdaten,
- ExperienceRecords,
- ExperienceCells,
- Traces,
- TechniqueDefinitions,
- SLM- und Embedding-Modelle,
- Trainingsdaten,
- Checkpoints.

Formel:

\[
SSD =
N\cdot8
+
E\cdot R
+
T\cdot S_T
+
M_{\text{SLM}}
+
M_{\text{Training}}
\]

mit:

- \(N\): Anzahl NanoNeuronen,
- \(E\): Anzahl Erfahrungen,
- \(R\): durchschnittliche Record-Größe,
- \(T\): Anzahl Techniken,
- \(S_T\): durchschnittliche Technikgröße.

Die fehlenden Größen sind noch zu messen und dürfen nicht erfunden werden.

---

# Konsolidiertes Ergebnis

Aus einem Byte beziehungsweise acht Bits erhalten wir:

### Direkt gespeichert

- einen aktuellen Bytewert,
- acht Bitpositionen,
- 256 mögliche Zustände.

### Als einfaches Dimensionsmodell

\[
8^D
\]

theoretische Prozesskoordinaten bei \(D\) Achtfachachsen.

### Mit den derzeit vorgesehenen Operatorachsen

Bei:

- 15 Shift-Zuständen,
- 8 Rotationspositionen,
- 6 binären Gates,
- 256 Masken,

entstehen:

\[
184.320
\]

mögliche Prozessrezepte **pro Kontext und Technik**.

Allgemein:

\[
P=184.320\cdot C\cdot T
\]

### Tatsächlich auszuführen

Nicht alle theoretischen Prozesse, sondern nur die durch:

- Semantic Routing,
- Slot-Prinzip,
- Cascade-Filterung,
- Montessori-Shape-Matching,
- Kontextprüfung,
- Experience Ranking

ausgewählten Kandidaten.

> **AXIOM speichert nicht Millionen Kopien eines Bytes. AXIOM speichert den Bytezustand, die ausführbaren Regeln und die Erfahrungen darüber, welche wenigen Wege durch den theoretisch riesigen Prozessraum tatsächlich sinnvoll sind.**
