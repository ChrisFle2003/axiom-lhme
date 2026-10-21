# KEVIN Memory Store (Legacy Compatibility)

`KevinMemoryStore` bleibt für bestehende Programme verfügbar, ist aber nicht
die Architektur für neue Datenpfade. Er koppelt historisch eine Grid-Regel,
eine Technique-Bindung und seltene Overrides. Seine Semantik und sein
Snapshotformat bleiben für diese Release-Linie unverändert; ein alter KEVIN-
Snapshot wird niemals still als neuer FormulaOverrideStore-Snapshot gelesen.

Neue Programme trennen die Rollen:

```text
GridDomainSpec + GridAddress + DomainProjection  -> Domain/Filter-Koordinate
FormulaSpec oder TechniqueBinding + FormulaAddress -> Wertgenerator
FormulaOverrideStore                              -> seltene Abweichungen
```

Für die Migration ersetzt man `GridSpec` durch einen `GridDomainSpec`, wählt
eine explizite Projektion (V1: `DepthPathProjection`) und legt die bisherige
Technique-Bindung als `TechniqueBinding` im `FormulaOverrideStore` ab. Direkte
FormulaRecipe-Auswertung benötigt dabei kein KEVIN-Objekt.

KEVIN speichert Grid-Regel, 8×8-Adresse und seltene Overrides, nicht jedes reguläre BigInt. `get()` leitet normale Anfragen lazy an die bestehende Engine weiter. Deterministische Rekonstruktion benötigt Grid- und Kernel-Version, kanonische Parameter und den vollständigen Pfad.

```cpp
KevinMemoryStore store{engine};
store.register_grid(spec);
auto value = store.get(spec.identity, address, EvaluationMode::Metric,
                       Budget::unlimited());
store.set_override(spec.identity, address, BigInt{42});
```

Der Legacy-Store enthält nur GridSpecs und Overrides; berechnete Werte liegen ausschließlich im begrenzten Engine-Cache. Der Cache ist ein Beschleuniger, niemals die Quelle der gespeicherten Wahrheit.

## 64er-Tile-Filter (V1.6)

Für einen Elternpfad betrachtet KEVIN ein einzelnes 8×8-Tile als genau 64 mögliche
Kindadressen. Die Bitposition ist stabil definiert als `bit = y * 8 + x`: Bit 0 steht
für `(0,0)`, Bit 63 für `(7,7)`.

```text
64 Adressen
  -> Modular-Filter (ohne BigInt-Materialisierung)
  -> Größen-/Bitlängenfilter (ohne BigInt-Materialisierung)
  -> AND-Maske der Überlebenden
  -> optional: Exact nur für eine ausdrücklich begrenzte Zahl von Kandidaten
```

Die Maske ist lokal für dieses Tile. Ein nicht passender Kandidat entfernt weder
andere Tiles noch beendet er die Engine. `lhme_kevin_grid_filter_demo` zeigt den
Ablauf und prüft, dass die Filterphase keine exakte Berechnung auslöst.

`filter_tiles(...)` führt mehrere Eltern-Tiles in der übergebenen Reihenfolge
zusammen. Dabei bleiben die 64-Bit-Masken und deren Ablehnungen lokal pro Tile;
erst die überlebenden Adressen werden in einer gemeinsamen, deterministischen
Kandidatenliste zusammengefasst. `evaluate_reduced_exact(...)` wendet danach ein
einziges globales Kandidatenlimit an. So kann beispielsweise ein CPU-Worker zuerst
viele Tiles billig vorfiltern und nur die wichtigsten Überlebenden materialisieren.

Für CPU-Parallelität ruft `filter_tiles_cpu(..., worker_count)` höchstens so viele
Tiles gleichzeitig auf, wie Worker erlaubt sind. Die Ergebnisse werden danach immer
in der ursprünglichen Tile-Reihenfolge zusammengeführt; die Antwort bleibt damit bei
gleicher Eingabe deterministisch. `worker_count = 0` ist ungültig. Store-Tabellen,
Engine-Registry, Cache und Exact-Statistik sind für parallele Leseauswertung
abgesichert. Konfigurationsänderungen wie Grid-Registrierung oder Snapshot-Austausch
bleiben bewusst schreibende, exklusiv geschützte Operationen.

Bei vielen aufeinanderfolgenden Aufträgen verwendet `KevinTileCpuExecutor` denselben
Pool dauerhaft. Die Worker werden beim Erzeugen des Executors gestartet und erst bei
seiner Zerstörung beendet. `executor.filter(...)` erhält weiterhin eine feste
Eingabereihenfolge und gibt die Kandidaten in genau dieser Reihenfolge zurück.

## Vergleichbare Benchmarks

`lhme_table_memory_benchmark` misst die Speicherform einer Million materialisierter
`UInt64`-Werte gegen eine symbolische Formel-Rezeptur. `lhme_kevin_cpu_benchmark`
misst separat die KEVIN-Filterpipeline für 64 Tiles (4.096 virtuelle Zellen) mit
1, 2, 4 und 8 CPU-Workern. Er prüft dabei gleiche Überlebende und null Exact-
Materialisierungen für jede Worker-Zahl und mittelt vier Durchläufe je persistentem
Worker-Pool. Laufzeiten sind maschinenabhängig; die
deterministische Gleichheit von Kandidaten und Zählern ist der feste Vertrag.

`lhme_hybrid_storage_benchmark` erweitert den Vergleich auf einen virtuellen
Adressraum von einer Million Werten. Es speichert die Formelbasis einmal und fügt
nur echte Abweichungen als BigInt-Overrides hinzu. Standardmäßig misst es 0 %, 0,1
%, 1 % und 10 % Overrides gegen die Containergröße einer Millionenwert-Hash-Map;
`--full-overrides` aktiviert zusätzlich die 100-%-Gegenprobe. Die ausgegebene
Snapshot-Größe ist die Größe des aktuellen ASCII-Snapshotformats auf Platte, nicht
die universelle Größe einer späteren Binärserialisierung. Sie belegt daher das
Verhalten dieser Implementierung, keine allgemeine RAM-Obergrenze.

`lhme_table_memory_benchmark` misst zusätzlich den Peak-RAM in je einem
isolierten Linux-Kindprozess. Neben dichter Tabelle, sortierten Paaren und
Hash-Map misst er `FormulaOnly` sowie `FormulaWithOverrides` mit 0 %, 1 %,
10 % und 100 % expliziten Abweichungen. Alle Varianten decken denselben
virtuellen Raum von einer Million Formeladressen ab. KEVIN ist keine
Wertrepräsentation in diesem Vergleich. Die Prozessmessung enthält daher auch
Allocator- und Runtime-Overhead; sie ist für die praktische RAM-Entscheidung
aussagekräftiger als die analytischen Containergrößen. Die Messung legt keine
allgemeingültige Grenze fest, denn Werte hängen von Plattform, Compiler und
Datenwerten ab.

## Automatische Speicherentscheidung

`choose_storage_strategy(...)` ist der öffentliche, deterministische
Entscheidungskern für aufrufenden Code. Mit `StoragePolicy` ist die
Sparse-Grenze in Basispunkten konfigurierbar (Standard: 500 = 5 %). Die
Entscheidung verwendet keine Fließkommazahlen:

```text
0 Overrides                 -> FormulaOnly
bis zur Sparse-Grenze        -> FormulaWithOverrides
darüber, sequenzieller Zugriff -> DenseTable
darüber, zufälliger Zugriff    -> HashTable
```

Ungültige Eingaben — ein leerer Adressraum, mehr Overrides als Adressen oder
eine Grenze über 100 % — werden mit `Status::InvalidArgument` abgelehnt. Die
Policy bestimmt aktuell die Zielstrategie; eine künftige Migrationsschicht
wechselt gespeicherte Daten kontrolliert in diese Zielrepräsentation.
