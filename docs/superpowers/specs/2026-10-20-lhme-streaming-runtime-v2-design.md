# LHME Streaming Runtime V2 – Design

## Zweck

LHME V2 optimiert große Adressbereiche für CPU und RAM. Die Engine soll
Kacheln ohne Thread-Erzeugung pro Kachel ausführen, Kandidaten vor einer
teuren Exact-Auswertung deterministisch filtern und Ergebnisse sofort
hierarchisch reduzieren. Nur die unter einer klaren Retention-Policy
erforderlichen Kandidaten bleiben im RAM.

Erfolg bedeutet:

- dieselbe Anfrage mit derselben Konfiguration liefert immer dieselbe
  Zusammenfassung und dieselben retained candidates;
- ein großer Bereich benötigt keinen Vektor aller Kacheln oder aller
  bestätigten Exact-Werte;
- Exact wird nur nach einem erfolgreichen, günstigen Filterpfad ausgeführt;
- Budgets sind harte Obergrenzen, kein Wunschdenken.

## Abgrenzung

V2 bleibt eine CPU/RAM-Library. GPU, CUDA, persistente Speicherung, Netzwerk,
KI-Lernen und ein textbasiertes Scenario-Format sind nicht Teil dieses
Schritts. Die vorhandene `evaluate_range`-API bleibt für Kompatibilität
erhalten; V2 ergänzt einen separaten Streaming-Pfad.

## Architektur

```text
StreamingRangeRequest
        │
        ▼
Deterministischer Tile-Generator
        │
        ▼
Fester CPU-Worker-Pool
        │
        ├─ Metric / Modular Vorfilter
        ├─ Exact nur für akzeptierte Adressen
        └─ lokale TileSummary + lokale Kandidaten
        │
        ▼
Kanonischer Reducer (Tile-ID-Reihenfolge)
        │
        ├─ GlobalSummary
        └─ AdaptiveRetention
                 │
                 ├─ SummaryOnly
                 ├─ FilteredCandidates
                 ├─ TopK
                 └─ Adaptive
```

Der Producer erzeugt höchstens eine begrenzte Anzahl geplanter Tiles pro
Worker-Batch. Der Reducer verarbeitet fertige Batches stets in Tile-ID-
Reihenfolge; die physische Fertigstellungsreihenfolge beeinflusst das Ergebnis
nicht.

## Öffentliche V2-Verträge

```cpp
enum class RetentionMode {
  SummaryOnly,
  FilteredCandidates,
  TopK,
  Adaptive,
};

enum class FilterMode {
  AcceptAll,
  ModularEquals,
};

struct CandidateFilter {
  FilterMode mode = FilterMode::AcceptAll;
  BigInt modulus = 1;
  BigInt expected_residue = 0;
};

struct StreamingOptions {
  std::uint32_t worker_count = 1;
  std::uint64_t tile_size = 1;
  RetentionMode retention_mode = RetentionMode::Adaptive;
  std::uint64_t max_retained_candidates = 0;
  std::uint64_t retention_memory_bytes = 0;
  CandidateFilter filter;
};

struct StreamingRangeResult {
  Status status = Status::InternalError;
  GlobalSummary summary;
  std::vector<EvaluationResult> retained_candidates;
  RetentionMode effective_retention_mode = RetentionMode::SummaryOnly;
  std::uint64_t filtered_out_count = 0;
  std::uint64_t exact_evaluation_count = 0;
};

[[nodiscard]] StreamingRangeResult evaluate_streaming_range(
    const EvaluationRequest& exact_request,
    AddressRange range,
    StreamingOptions options);
```

`exact_request.mode` muss `Exact` sein. Die Engine nutzt dessen FormulaSpec,
Budgets und Cancellation-Signal. Falsche Filterparameter, ein nicht-Exact
Request, Null-Worker oder Null-Tile ergeben `InvalidArgument` ohne Exact-
Auswertung.

## Fester Worker-Pool

V2 erhält einen kleinen internen `CpuWorkerPool`. Er wird mit genau
`worker_count` Workern für eine Streaming-Anfrage erzeugt und am Ende der
Anfrage geordnet beendet. Der Pool verarbeitet nur bereits begrenzte Batches;
er hält nie eine unbeschränkte Queue vor.

V1s `evaluate_range` bleibt unverändert. V2 verwendet den Pool nur im neuen
Streaming-Pfad, damit sich Verhalten und Risiko klar isolieren lassen.

## Filterpfad

`AcceptAll` bestätigt jede valide Adresse für die Exact-Stufe.

`ModularEquals` berechnet zunächst:

\[
h(x) \bmod M
\]

und akzeptiert die Adresse nur, wenn der Rest dem positiven normalisierten
`expected_residue` entspricht. `M` muss positiv sein. Ein Filter-Miss ist kein
Branch-Fehler und kein Run-Abbruch; er erhöht ausschließlich
`filtered_out_count`.

Der Filterwert wird nicht als retained candidate gespeichert. Erst ein
akzeptierter Kandidat erreicht Exact und kann anschließend die Retention
durchlaufen.

## Hierarchische Streaming-Reduktion

Jede Kachel erzeugt lokal:

```text
checked_count
confirmed_count
invalid_argument_count
filtered_out_count
exact_evaluation_count
lokale retained candidates
```

Nach jeder deterministisch geordneten Batch-Reduktion kann deren lokaler
Kandidatenvektor freigegeben werden. Die globale Zusammenfassung besteht aus
Zählern und beansprucht konstante Größe.

Bei Cancellation, Budgetüberschreitung oder internem Fehler beendet der
Producer die Ausgabe neuer Tiles. Bereits eingeplante Arbeit wird geordnet
abgewartet; das Ergebnis erhält den Stop-Status und keine neu bestätigten
retained candidates nach dem Stop.

## Adaptive Retention

Alle Modi erfüllen dieselben harten Grenzen:

```text
retained candidates ≤ max_retained_candidates
geschätzte retained bytes ≤ retention_memory_bytes
```

`0` für eine Grenze bedeutet: keine Kandidaten halten. Die Zusammenfassung
bleibt dennoch verfügbar. In diesem Fall meldet `Adaptive` als effektiven
Modus `SummaryOnly`.

| Modus | Regel |
|---|---|
| `SummaryOnly` | nie einen Kandidaten speichern |
| `FilteredCandidates` | akzeptierte Exact-Kandidaten in Adressreihenfolge bis zu beiden Limits speichern |
| `TopK` | die K niedrigsten Adressen bis zu beiden Limits speichern |
| `Adaptive` | beginnt als `FilteredCandidates`; sobald Kandidaten- oder Speicherlimit erreicht ist, wechselt es dauerhaft zu `TopK` über die bereits gesehenen und künftig bestätigten Adressen |

`TopK` ist bewusst nach der kleinsten Adresse geordnet – nicht nach einem
laufzeitabhängigen Wert. Deshalb ist die Auswahl unabhängig von Worker- und
Completion-Reihenfolge.

Die geschätzte Retention-Größe wird konservativ bestimmt:

```text
sizeof(EvaluationResult) + Exact-Dezimalziffern + Trace-Textlänge
```

Eine einzelne Kandidatenstruktur, die das Memorylimit überschreitet, wird
nicht gehalten. Das erzeugt keinen Fehler und verändert nicht die globale
Zusammenfassung.

## Budgets und Fehlerbehandlung

- Das bestehende `max_candidate_count` begrenzt weiterhin die gesamte
  Adressarbeit fail-closed vor der Planung.
- `max_retained_candidates` und `retention_memory_bytes` begrenzen nur die
  Speicherhaltung; bestätigte Werte können weiter in die Zusammenfassung
  eingehen.
- Exact-, Operations-, Temporary-Memory- und Trace-Budgets des bestehenden
  Requests bleiben pro akzeptierter Adresse aktiv.
- Ein Filter-Miss ist normal; ungültige Adressen bleiben lokal gezählt.
- `Cancelled`, `BudgetExceeded` und `InternalError` stoppen die vollständige
  Streaming-Ausführung kontrolliert.

## Teststrategie

| Fall | Nachweis |
|---|---|
| deterministische Parallelität | 1 Worker und 4 Worker liefern gleiche Summary und retained candidates |
| Vorfilter | ein ModularEquals-Filter senkt Exact-Auswertungen und zählt Misses |
| SummaryOnly | keine retained candidates trotz bestätigter Werte |
| TopK | bei anderer Workerzahl bleiben dieselben niedrigsten Adressen erhalten |
| Adaptive | nach Limit wird dauerhaft TopK verwendet und Speicherlimit bleibt eingehalten |
| Budget/Cancel | Stop erzeugt keine Kandidaten nach dem Stop und keine neue Tile-Planung |
| ungültige Optionen | keine Exact-Auswertung und `InvalidArgument` |

## Akzeptanzkriterien

1. Die bestehende V1-Range-API und ihre Tests bleiben unverändert grün.
2. Die neue Streaming-API wird über öffentliche Header bereitgestellt.
3. Workerzahl eins und vier ergeben für gleiche Anfrage, Filter und Retention
   bitgleich dieselbe Summary und Kandidatenliste.
4. Ein `ModularEquals`-Filter führt Exact nur für passende Reste aus.
5. Kein Streaming-Lauf hält mehr Kandidaten oder geschätzte Bytes als erlaubt.
6. `Adaptive` meldet den tatsächlich aktiven finalen Retention-Modus.
7. CTest deckt jeden Modus, Filter, Stop und eine Speichergrenze ab.
