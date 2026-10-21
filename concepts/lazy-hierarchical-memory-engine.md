# BRUCE Lazy Hierarchical Memory Engine

**Status:** KONZEPT / ENTWURF  
**Version:** 0.1  
**Zweck:** Deterministische, speichersparende Auswertung großer, formelbasiert adressierter Zahlen- und Prozessräume.

> Dieses Dokument beschreibt eine Softwarearchitektur. Sie ist kein Nachweis
> einer bereits implementierten oder leistungsgeprüften Runtime.

## 1. Kurzfassung

Die BRUCE Lazy Hierarchical Memory Engine speichert große Werte nicht
standardmäßig als vollständig expandierte Zahlen. Stattdessen speichert sie
eine kleine, versionierte Berechnungsregel, eine stabile Adresse und die
Auswertungsvorgaben.

Ein Wert wird nur dann erzeugt, wenn er tatsächlich benötigt wird. Viele
Entscheidungen erfolgen vorher mit günstigen Größenmetriken, modularen Resten
oder symbolischen Ausdrücken. CPU-Worker bearbeiten unabhängige
Adressbereiche als Tiles. Ihre lokalen Ergebnisse werden stufenweise und
deterministisch reduziert.

```text
FormulaSpec + Address + EvaluationMode
        ↓
lazy Evaluation innerhalb eines begrenzten Tiles
        ↓
lokale Filter und Branch Gates
        ↓
Tile Summary
        ↓
hierarchische Reduktion
        ↓
verifizierter AnswerPoint, Trace und optionaler Cache
```

Der Kernvorteil ist die Trennung:

```text
Adresse ≠ Formel ≠ expandierter Wert ≠ Cache-Eintrag ≠ Bedeutung
```

## 2. Ziel und Abgrenzung

### 2.1 Ziel

Die Engine soll große theoretische Zahlen- und Prozessräume adressierbar,
reproduzierbar und begrenzt auswertbar machen, ohne sie vollständig im RAM
zu materialisieren.

Sie soll insbesondere:

- identische Spezifikationen deterministisch identisch auswerten;
- nur angeforderte oder durch Filter ausgewählte Adressen berechnen;
- exakte BigInt-Werte nur bei ausdrücklicher Freigabe erzeugen;
- CPU-Threads mit kleinen, lokalen Arbeitsmengen versorgen;
- Ergebnisse und Belege hierarchisch zusammenführen;
- lokale Ablehnungen von globalen Abbrüchen trennen;
- bestätigte Ergebnisse mit FormulaSpec, Adresse und Trace verknüpfen.

### 2.2 Nicht-Ziele

Nicht Teil dieser Version sind:

- die Behauptung, jede große Zahl lasse sich verlustfrei komprimieren;
- eine Garantie, dass Neuberechnen stets schneller als Speichern ist;
- eine vollständige Materialisierung exponentieller Räume;
- eine GPU- oder CUDA-Implementierung;
- physische Speicherhardware oder ein neues Prozessorprodukt;
- eine Aussage über Quantenbeschleunigung.

Eine zufällig wirkende große Zahl besitzt häufig keine kürzere, bekannte
Erzeugungsregel. Der Speichergewinn entsteht nur dann, wenn die Regel plus
Parameter deutlich kleiner sind als der expandierte Wert.

## 3. Referenzformel: exponentielle Höhenleiter

Als einfache Referenz dient:

\[
h(x)=100(2^x-1),\qquad x\in\mathbb{N}_0.
\]

Die Formel ist eine diskrete, rein softwareseitige Koordinatenleiter. Der
Name `height` beschreibt eine virtuelle Prozesshöhe; er ist keine Aussage
über eine reale Luftblase oder ein physikalisches Modell.

| Adresse `x` | Wert `h(x)` |
|---:|---:|
| 0 | 0 |
| 1 | 100 |
| 2 | 300 |
| 3 | 700 |
| 4 | 1.500 |
| 5 | 3.100 |
| 8 | 25.500 |
| 10 | 102.300 |
| 20 | 104.857.500 |

Die Engine darf die Formel alternativ über die exakte Rekursion auswerten:

\[
h(0)=0,
\qquad
h(x+1)=2h(x)+100.
\]

Der Stufenabstand ist:

\[
h(x+1)-h(x)=100\cdot2^x.
\]

Die Rückabbildung ist nur für gültige Leiterwerte definiert:

\[
x=\log_2\left(\frac{h}{100}+1\right).
\]

## 4. Kanonisches Datenmodell

### 4.1 FormulaSpec

Eine `FormulaSpec` ist unveränderlich und wird gemeinsam von allen Workern
gelesen.

```text
FormulaSpec
├─ formula_id
├─ version
├─ operation                 # PowerOffset, Fibonacci, ModularPower, …
├─ base                      # hier: 2
├─ scale                     # hier: 100
├─ domain                    # hier: nonnegative_integer
├─ exact_algorithm           # z. B. binary exponentiation
├─ metric_algorithm
├─ supported_modes
├─ resource_policy
└─ specification_hash
```

Die Formel wird nicht pro Thread kopiert. Worker erhalten lediglich einen
Verweis auf dieselbe freigegebene `FormulaSpec` und ihre jeweiligen
Adressbereiche.

### 4.2 Address und WorkTile

```text
FormulaAddress
├─ formula_id
├─ formula_version
└─ index                     # x

WorkTile
├─ tile_id
├─ address_start
├─ address_end
├─ evaluation_mode
├─ budget
├─ filter_profile
└─ parent_tile_id
```

Eine Adresse beschreibt den Ort im Raum; sie ist nicht der berechnete Wert.
Ein Tile fasst einen begrenzten, unabhängigen Adressbereich für einen Worker
zusammen.

### 4.3 EvaluationMode

```text
Symbolic  Formel und Parameter bleiben unverändert.
Metric    Nur Bitlänge, Größenklasse oder geschätzte Kosten.
Modular   Exaktes Ergebnis modulo M.
Exact     Vollständiger BigInt innerhalb des Budgets.
```

Die Modi sind keine Näherungsstufen desselben Ergebnisses. Jeder Modus
besitzt einen eigenen Vertrag und muss in Cache-Key und Trace enthalten sein.

## 5. Lazy-Auswertung

### 5.1 Ablauf einer einzelnen Adresse

```text
Adresse anfordern
→ FormulaSpec und Versionsbindung prüfen
→ Domain und Budget prüfen
→ günstige Metrik oder modularen Filter auswerten
→ bei Bedarf exakten Wert berechnen
→ Ergebnis verifizieren
→ Trace erstellen
→ optional cachebar machen
```

### 5.2 Budget vor Expansion

Bevor `Exact` einen BigInt erzeugen darf, prüft die Engine mindestens:

- maximale Bitlänge;
- maximale Operationen;
- maximales temporäres Speicherbudget;
- maximale Trace-Größe;
- erlaubten Auswertungsmodus;
- aktiven globalen ExecutionGate-Status.

Ein fehlendes Budget oder eine zu große erwartete Bitlänge führt nicht zur
stillen Teilberechnung. Das Ergebnis bleibt symbolisch oder erhält einen
klaren Ablehnungsgrund.

## 6. CPU-Threads und hierarchische Verarbeitung

### 6.1 Worker-Prinzip

Ein CPU-Worker erhält nicht den gesamten Wertebereich, sondern ein Tile:

```text
Worker 1: x =      0 …    999
Worker 2: x =  1.000 …  1.999
Worker 3: x =  2.000 …  2.999
```

Er verarbeitet seine Adressen lokal, hält temporäre Werte lokal und erzeugt
anschließend eine kompakte `TileSummary`. Das senkt den gemeinsamen
Schreibverkehr und vermeidet, dass alle Worker im Hot Path auf denselben
globalen Cache zugreifen.

### 6.2 Hierarchische Reduktion

```text
Root Space
├─ Tile Group A
│  ├─ Worker Tile A1 → LocalSummary
│  └─ Worker Tile A2 → LocalSummary
│                    ↓
│                 GroupSummary A
└─ Tile Group B
   ├─ Worker Tile B1 → LocalSummary
   └─ Worker Tile B2 → LocalSummary
                     ↓
                  GroupSummary B
                     ↓
                GlobalSummary
```

Die Reduktion verdichtet nur ausdrücklich definierte Informationen:

```text
TileSummary
├─ checked_address_count
├─ accepted_address_count
├─ rejection_counts_by_reason
├─ minimum / maximum / best metric
├─ bounded candidate set
├─ consumed_budget
├─ trace_root_hash
└─ representative evidence references
```

### 6.3 Was nicht weg-reduziert werden darf

Ein gleiches Ergebnis darf als Wert zusammengeführt werden. Seine
unabhängigen Entstehungspfade dürfen dabei nicht unkontrolliert verloren
gehen.

```text
Value identity  → darf dedupliziert werden
Path identity   → muss als Provenienz erhalten bleiben
Semantic status → darf nicht aus einem Wert allein abgeleitet werden
```

Für jeden reduzierten Kandidaten bleiben deshalb mindestens Adresse,
FormulaSpec-Version und Trace-Referenz erhalten.

## 7. Filter und Gates

Filter verringern die Arbeit früh und möglichst günstig:

```text
Domain Check
→ Cost / Bitlength Check
→ Modular Filter
→ Context / Whitelist Check
→ Exact Evaluation
→ Independent Verification
```

Die Gate-Hierarchie unterscheidet strikt:

```text
BRANCH_GATE = 0  betroffenen Ast verwerfen
TILE_GATE   = 0  verbleibende Arbeit dieses Tiles beenden
RUN_GATE    = 0  gesamten Lauf kontrolliert beenden
PUBLISH_GATE= 0  Ergebnis nicht als bestätigt ausgeben oder lernen
```

Ein lokaler Filter-Miss beendet somit nicht automatisch die gesamte
Ausführung.

## 8. Cache-Vertrag

Der Cache ist ein Beschleuniger, keine Wissensquelle und kein Ersatz für die
FormulaSpec.

```text
CacheKey
= specification_hash
+ formula_address
+ evaluation_mode
+ optional modulus
+ algorithm_version
```

Ein Cache-Eintrag enthält:

```text
CacheEntry
├─ result or metric
├─ verification_status
├─ result_hash
├─ trace_reference
├─ size_in_bytes
└─ cache_policy_version
```

Kleine oder häufig angeforderte exakte Werte können zwischengespeichert
werden. Sehr große, selten verwendete Werte bleiben nach ihrer Verwendung
symbolisch; der temporäre BigInt wird verworfen.

## 9. Determinismus trotz Parallelität

Parallelität darf die Bedeutung eines Laufs nicht verändern. Daher gelten:

- Integer- und rationale Berechnungen statt unkontrollierter Gleitkommawerte;
- feste FormulaSpec- und Algorithmusversionen;
- stabile Tile-IDs und kanonische Adressreihenfolge;
- festgelegte Reduktionsregeln;
- kanonisch sortierte Kandidaten bei Gleichstand;
- vollständige Gate- und Budgetereignisse im Trace.

Die Fertigstellungsreihenfolge der Worker darf höchstens die Laufzeit
beeinflussen, nicht Ergebnis, Trace-Bedeutung oder Veröffentlichung.

## 10. Ressourcenmodell

Der Speichergewinn ist ein Tausch, nicht Magie:

```text
weniger persistenter RAM
↔
mehr Rechenzeit bei einer späteren exakten Rekonstruktion
```

Die Engine bevorzugt deshalb diese Reihenfolge:

```text
Symbolic
→ Metric
→ Modular
→ Exact
```

`Exact` ist eine explizite, budgetierte Eskalation. Ein Cache amortisiert
wiederholte exakte Abfragen, darf aber keine unkontrollierte RAM-Expansion
verursachen.

## 11. GPU- und Tensor-Abgrenzung

Die erste Version ist CPU-orientiert. Sie verwendet einen sparse,
adressierbaren Prozessraum und ist kein dicht materialisierter Tensor.

Eine spätere GPU-Ausführung kann sinnvoll sein, wenn viele unabhängige
Adressen mit derselben FormulaSpec und demselben Filterprofil bewertet
werden. Sie muss jedoch dieselben FormulaSpec-, Gate-, Budget- und
Trace-Verträge erfüllen. GPU-Einsatz ist eine Backend-Optimierung, keine
Änderung der mathematischen Semantik.

## 12. Abnahmekriterien für einen Referenzprototyp

- [ ] `h(x)=100(2^x-1)` liefert für bekannte Testwerte die erwarteten
  exakten BigInts.
- [ ] Rekursions- und Direktmodus liefern identische Werte.
- [ ] `Metric`, `Modular` und `Exact` sind im Ergebnisformat klar getrennt.
- [ ] Eine unerlaubte Exact-Expansion wird vor BigInt-Erzeugung abgelehnt.
- [ ] Dieselbe FormulaSpec und Adresse ergeben bei serieller und paralleler
  Ausführung denselben bestätigten Wert und denselben kanonischen Trace.
- [ ] Ein Branch-Miss stoppt keinen anderen zulässigen Branch.
- [ ] Ein globales Gate beendet alle verbleibenden Tiles kontrolliert.
- [ ] Die hierarchische Reduktion bewahrt Kandidaten-Provenienz.
- [ ] Cache-Treffer und Cache-Miss erzeugen dieselbe fachliche Antwort.
- [ ] Ein Größenlimit verhindert unbeschränktes Cache-Wachstum.

## 13. Offene Entscheidungen

| ID | Entscheidung | Aktueller Vorschlag |
|---|---|---|
| LHME-001 | Tile-Größe | adaptiv, aber innerhalb einer Ausführung geloggt und reproduzierbar |
| LHME-002 | Cache-Verdrängung | verifizierte Relevanz + Kosten + Größenlimit statt nur LRU |
| LHME-003 | Kandidatenlimit pro Summary | fester Budgetwert mit expliziter Overflow-Klasse |
| LHME-004 | Reduktionsoperatoren | pro Datentyp versioniert und deterministisch |
| LHME-005 | GPU-Backend | erst nach CPU-Referenztests und Replay-Nachweis |
| LHME-006 | Physikalische Modelle | getrennte, versionierte Domänenmodule; nicht Teil des Kernspeicheralgorithmus |

## 14. Schlussformel

```text
Kleine Regel + stabile Adresse + begrenzte Auswertung
→ großer theoretischer Raum ohne große permanente Speicherlast.
```

Die Engine speichert nicht zwangsläufig die Zahl. Sie speichert, wie eine
zulässige und überprüfbare Berechnung der Zahl bei Bedarf durchgeführt wird.
