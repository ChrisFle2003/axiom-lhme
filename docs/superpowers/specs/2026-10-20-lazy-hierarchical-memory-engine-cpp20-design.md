# BRUCE Lazy Hierarchical Memory Engine — C++20 V1 Design

**Status:** Freigegebene Architektur, Implementierung noch nicht begonnen  
**Datum:** 2026-10-20  
**Umfang:** Wiederverwendbare C++20-Library; CPU und RAM ausschließlich in V1

## 1. Zweck

Die BRUCE Lazy Hierarchical Memory Engine (LHME) wertet formelbasiert
adressierte Zahlen- und Prozessräume aus, ohne jeden Wert dauerhaft im RAM zu
materialisieren. Sie bietet deterministische, budgetierte Auswertung,
CPU-Tiles, hierarchische Ergebnisreduktion, begrenztes Caching und
nachvollziehbare Ergebnisse.

V1 ist eine Library, keine Anwendung. Sie ist über eine typsichere C++-API
und eine schmale C-ABI wiederverwendbar.

Das Grundlagenkonzept steht in
[`concepts/lazy-hierarchical-memory-engine.md`](../../../concepts/lazy-hierarchical-memory-engine.md).

## 2. Festgelegte Entscheidungen

| Bereich | Entscheidung |
|---|---|
| Primärsprache | C++20 |
| Auslieferung | Library mit öffentlicher C++-API |
| Interoperabilität | Kleine, stabile C-ABI mit undurchsichtigen Handles |
| Backend V1 | CPU und RAM |
| GPU | Späteres, austauschbares Backend |
| Kernarithmetik | Deterministische Ganzzahl- und rationale Operationen |
| Referenzformel | `h(x) = 100(2^x - 1)`, `x >= 0` |
| Auswertungsleiter | Symbolic → Metric → Modular → Exact |
| Parallelität | CPU-Work-Tiles mit kanonischem Merge |
| Cache | Begrenzter Beschleuniger, niemals Wahrheitsquelle |

## 3. Umfang

### 3.1 V1 liefert

- unveränderliche, versionierte FormulaSpecs;
- stabile FormulaAddresses;
- symbolische, metrische, modulare und exakte Auswertung;
- Bit-, Operations-, temporäre Speicher- und Trace-Budgets;
- CPU-Work-Tiles und deterministische Reduktion;
- Branch-, Tile-, Run- und Publish-Gates;
- begrenztes Result-Cache;
- strukturierte Traces und Ergebnisstatus;
- öffentliche C++20-API und minimale C-ABI.

### 3.2 V1 liefert nicht

- GPU-, CUDA-, OpenCL- oder SIMD-spezifische Ausführung;
- einen beliebigen Formelparser;
- unkontrollierte BigInt-Expansion;
- unendliche Queues, Caches oder Traces;
- persistente Datenbanken, Lernen oder automatische Spec-Mutation;
- die Behauptung, jede große Zahl sei komprimierbar oder stets günstiger
  rekonstruierbar als speicherbar.

## 4. Modulgrenzen

```text
bruce_lhme_core
├─ öffentliche Datenverträge
├─ FormulaSpec-Validierung
├─ deterministische Auswertungsverträge
└─ Reduktionsverträge

bruce_lhme_cpu
├─ Tile-Scheduler
├─ worker-lokale Auswertung
├─ kanonischer Merge-Koordinator
└─ kontrollierte Abbrüche

bruce_lhme_cache
├─ begrenzter Cache
└─ Admission- und Eviction-Policy

bruce_lhme_trace
├─ Trace-Ereignisse
├─ Provenienzreferenzen
└─ kanonische Serialisierung

bruce_lhme_c_api
├─ opaque Handles
├─ C-kompatible Views
└─ Statusübersetzung
```

`bruce_lhme_core` hängt weder von Threads noch von einer konkreten
Cache-Implementierung oder einem GPU-Framework ab. Backends konsumieren die
Core-Verträge und verändern nicht deren Semantik.

## 5. Kernverträge

### 5.1 FormulaSpec

`FormulaSpec` ist nach der Registrierung unveränderlich.

```text
FormulaSpec
├─ formula_id
├─ specification_version
├─ operation_kind
├─ parameters
├─ domain_constraints
├─ supported_modes
├─ exact_algorithm_id
├─ metric_algorithm_id
├─ resource_policy
└─ specification_hash
```

V1 implementiert `PowerOffset`:

```text
value(x) = scale * (base^x - 1)
base      = 2
scale     = 100
domain    = nonnegative_integer
```

Die direkte und die rekursive Form müssen exakt übereinstimmen:

\[
h(x)=100(2^x-1),\qquad h(0)=0,\qquad h(x+1)=2h(x)+100.
\]

### 5.2 FormulaAddress und WorkTile

```text
FormulaAddress
├─ formula_id
├─ specification_version
└─ index

WorkTile
├─ tile_id
├─ parent_tile_id
├─ address_start
├─ address_end
├─ formula_spec_reference
├─ evaluation_mode
├─ filter_profile
└─ budget_allocation
```

Eine Adresse identifiziert einen Ort im Formelraum — nicht den Wert und nicht
den Cache-Zustand.

### 5.3 EvaluationMode

| Modus | Vertrag |
|---|---|
| `Symbolic` | kanonische Formel- und Adressreferenz |
| `Metric` | Bitlänge oder spezifizierte Kosten-/Größenmetrik |
| `Modular` | exakter Wert modulo positivem Modulus |
| `Exact` | vollständiger BigInt innerhalb des Budgets |

Der Modus ist Teil der semantischen Identität. Ein modularer Wert ersetzt
keinen exakten Wert; eine Metrik ersetzt keinen Wert.

### 5.4 Budget und Resultat

```text
EvaluationBudget
├─ max_bit_length
├─ max_operations
├─ max_temporary_bytes
├─ max_trace_bytes
├─ max_candidate_count
└─ cancellation_token

EvaluationResult
├─ status
├─ mode
├─ address
├─ specification_hash
├─ value_or_metric
├─ consumed_budget
├─ trace_id
├─ cache_status
└─ verification_status
```

Zulässige V1-Statuswerte:

```text
Confirmed | Rejected | BudgetExceeded | Cancelled |
InvalidArgument | UnsupportedMode | InternalError
```

## 6. Gate-Hierarchie

```text
BRANCH_GATE = 0  nur diese Adresse verwerfen
TILE_GATE   = 0  restliche Arbeit dieses Tiles beenden
RUN_GATE    = 0  alle verbleibenden Tiles kontrolliert abbrechen
PUBLISH_GATE= 0  Ergebnis nicht als bestätigt ausgeben oder cachen
```

Ein lokaler Miss darf keine unabhängige, zulässige Adresse stoppen. Ein
globaler Abbruch blockiert neue Exact-Expansionen und gibt Ressourcen
kontrolliert frei.

## 7. CPU-Backend

### 7.1 Scheduling

V1 zerlegt inklusive Bereiche nichtnegativer Indizes in begrenzte Tiles.
Worker werten ein Tile lokal aus und schreiben nicht im Hot Path in einen
globalen Ergebniszustand.

- Ein begrenzter Worker-Pool und Work-Stealing sind zulässig.
- Tile-Identitäten und finaler Merge werden vor der Ausführung festgelegt.
- Scheduling darf Latenz, nicht aber Ergebnis, Trace-Bedeutung oder Cache-Key
  beeinflussen.
- Jeder Split oder Retry ist ein versioniertes Trace-Ereignis.

### 7.2 Auswertungsfolge

```text
FormulaSpec, Adresse und Modus validieren
→ Domain und Budget prüfen
→ zulässigen Cache-Eintrag prüfen
→ Symbolic / Metric / Modular vorfiltern
→ Exact nur bei Bedarf und Freigabe
→ Invarianten unabhängig prüfen
→ LocalSummary und Trace erzeugen
```

## 8. Hierarchische Reduktion

```text
LocalSummary → GroupSummary → GlobalSummary → EvaluationResult
```

Reduktionsfunktionen sind versioniert und deterministisch. Verdichtet werden
dürfen:

- Anzahl geprüfter und akzeptierter Adressen;
- Ablehnungsgründe je Klasse;
- Budgetverbrauch;
- deterministische Minimum-/Maximum-Metriken;
- Bitmasken;
- begrenzte, kanonisch sortierte Top-K-Kandidaten;
- Trace-Root-Hashes und Evidenzreferenzen.

Unabhängige Provenienz darf nicht verschwinden. Gleiche Werte können eine
Value Identity teilen, aber jeder behaltene Kandidat bewahrt Adresse,
FormulaSpec-Version und Trace-Referenz.

## 9. Cache-Vertrag

```text
CacheKey = specification_hash
         + formula_address
         + evaluation_mode
         + modulus, falls Modular
         + algorithm_version
```

Nur bestätigte und verifizierte Resultate sind cachebar. Die Admission-Policy
berücksichtigt Ergebnisgröße, Wiederverwendung, Verifikationsstatus und
Kapazität. Cache-Hit und Cache-Miss müssen dieselbe semantische Antwort
liefern.

Die Eviction-Policy ist begrenzt, konfigurierbar und getraced. Sie berücksichtigt
verifizierte Relevanz, Ergebnisgröße, Rekonstruktionskosten und Aktualität.
Ein Cache-Eintrag kann nie eine FormulaSpec oder einen Trace ungültig machen.

## 10. Determinismus und Trace

```text
TraceRecord
├─ Eingabeadressen und FormulaSpec-Hash
├─ Modus und Algorithmusversion
├─ Budgets und Gate-Übergänge
├─ Tile-IDs und Reduktionsversion
├─ Cache-Entscheidung
├─ Ergebnisstatus
└─ kanonische Parent-/Evidence-Referenzen
```

Der Kern nutzt exakte Ganzzahlen oder rationale Werte. Gleitkommawerte sind
höchstens in einer ausdrücklich versionierten Darstellungsschicht zulässig.

## 11. API-Grenzen

Die C++-API verwendet Werttypen für unveränderliche Eingaben und RAII für
Engine- und Resultatbesitz. Sie exponiert weder Threadobjekte noch
Cache-Interna oder rohe veränderliche Pointer.

```text
Engine::create(configuration)
Engine::register_formula(spec)
Engine::evaluate(request)
Engine::evaluate_range(request, address_range)
Engine::inspect(result)
Engine::cancel(run)
```

Die C-ABI ist absichtlich klein und exception-frei:

```text
lhme_engine_create        lhme_engine_destroy
lhme_formula_register     lhme_evaluate
lhme_result_inspect       lhme_result_destroy
lhme_cancel
```

Sie verwendet opaque Handles, Statuscodes und klar dokumentierten Besitz von
Strings und Bytes. Jede Header- und Runtime-Schnittstelle enthält eine
ABI-Version.

## 12. GPU-Grenze

Ein späteres GPU-Backend implementiert dieselben Request-/Result-Verträge. Es
darf gleichartige FormulaSpecs und Filterprofile bündeln, muss aber Formel-,
Modus-, Budget-, Gate-, Reduktions- und Trace-Semantik des CPU-Referenzpfads
beibehalten. GPU-Verfügbarkeit ist keine Voraussetzung für Korrektheit.

## 13. Abnahmetests

1. Direkt- und Rekursionsauswertung von `h(x)` stimmen für alle getesteten,
   budgetierten Indizes überein.
2. Referenzwerte: `h(0)=0`, `h(1)=100`, `h(5)=3100`, `h(10)=102300`,
   `h(20)=104857500`.
3. Serielle und mehrthreadige Bereichsauswertung ergeben kanonisch gleiche
   Resultate.
4. Ein lokaler Reject beendet kein anderes Tile.
5. Globales Cancel stoppt pending Exact-Expansionen mit `Cancelled`.
6. Bitlimit blockiert Exact vor einer BigInt-Allokation.
7. Cache-Hit und Cache-Miss sind in Wert, Status und Trace-Semantik gleich.
8. Die Cache-Größe bleibt bei vielen verschiedenen Anfragen begrenzt.
9. Jeder reduzierte Kandidat besitzt Adresse, FormulaSpec-Hash und Trace.
10. Die C-ABI kann Engine erstellen, Referenzformel registrieren, `h(10)`
    auswerten, Resultat lesen und alle Handles freigeben.

## 14. Zukünftige Implementierungspakete

1. Core-Typen, `PowerOffset` und Referenztests.
2. Status, Budgets, Gates und Traces.
3. CPU-Tiles und deterministische Reduktion.
4. Begrenzter Cache.
5. C-ABI und Integrationstests.

GPU-Arbeit ist ausdrücklich nicht Teil dieses ersten Implementierungsplans.
