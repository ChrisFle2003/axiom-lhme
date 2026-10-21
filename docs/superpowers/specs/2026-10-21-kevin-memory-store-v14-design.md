# KEVIN Memory Store V1.4 — Design

**Status:** Entwurf zur Nutzerfreigabe  
**Scope:** CPU/RAM; keine Persistenz, keine GPU, keine semantische Suche

## 1. Ziel und Erfolgskriterien

Der KEVIN Memory Store ergänzt den bestehenden LHME-Kern um ein
hierarchisches 8×8-Adressmodell und einen formelbasierten RAM-Store.
Er speichert nicht jede regulär berechenbare große Zahl. Stattdessen hält er
eine versionierte Grid-Regel, die angefragte Grid-Adresse und nur seltene
explizite Abweichungen. Den Wert berechnet er bei Bedarf über die bestehende
`Engine`.

V1.4 ist erfolgreich, wenn:

- eine Grid-Adresse eindeutig und ohne vollständiges Grid adressierbar ist;
- ein 8×8-Pfad bis Tiefe 10 verlustfrei in einen Formelindex überführt wird;
- ein Override den regulären Formelertrag nur für seine eine Adresse ersetzt;
- Symbolic, Metric, Modular und Exact die bestehenden Engine-Verträge
  unverändert verwenden;
- unangeforderte Werte und Untergitter nicht materialisiert werden;
- der Store selbst keine unbeschränkte Ergebnis- oder Cache-Tabelle besitzt;
- jeder Rückgabewert erkennen lässt, ob er aus einem Override oder einer
  Formelberechnung stammt.

## 2. Nicht-Ziele

V1.4 enthält ausdrücklich nicht:

- Dateisnapshots, Serialisierung oder Versionsmigration;
- GPU- oder CUDA-Ausführung;
- vollständige 64-Bit-Filterpipelines oder Subtree-Indexe;
- künstliche Kompression beliebiger, unabhängiger Daten;
- semantische BRUCE-Komponenten wie PeacemakerLLM, AnswerPoints oder
  Experience Records;
- eine zweite allgemeine Wertcache-Implementierung neben `ResultCache`.

Die spätere Snapshot-Stufe V1.5 serialisiert die in diesem Dokument
definierten stabilen Records; sie ändert deren Bedeutung nicht.

## 3. Architektur

```text
GridAddress + GridId + EvaluationMode + Budget
                 |
                 v
          KevinMemoryStore
          |              |
          |              +-- sparse OverrideMap
          |
          +-- GridSpec --+-- TechniqueIdentity + canonical parameters
                          |
                          v
                    bestehende Engine
                          |
                          v
          Symbolic / Metric / Modular / Exact + Trace
```

`KevinMemoryStore` besitzt Grid-Metadaten und Overrides. Die bestehende
`Engine` besitzt Formel-/Technique-Registrierung, Lazy-Auswertung,
Budgetprüfung, ResultCache und Ausführungscancel. Diese Verantwortungen
werden nicht vermischt.

## 4. Öffentliche Datentypen

### 4.1 GridAddress

`GridAddress` ist ein Pfad aus 8×8-Schritten. Ein Schritt besteht aus
`x` und `y`, jeweils im Bereich `0..7`, und entspricht sechs Bits
`yyyxxx`.

```text
GridAddress
├─ steps: [(x0, y0), (x1, y1), ...]
└─ depth: 0..10
```

Die kanonische Abbildung erzeugt zuerst den Pfadwert:

```text
step_bits = (y << 3) | x
path_value = (((0 << 6) | step_bits_0) << 6 | step_bits_1) ...
depth_offset(d) = sum(64^i, i = 0..d-1)
index = depth_offset(depth) + path_value
```

Die linke Pfadkomponente ist damit die höherwertige 6-Bit-Gruppe.
`depth_offset` kodiert die Pfadlänge ausdrücklich: Der leere Pfad und ein
Pfad aus Nullschritten bleiben daher verschieden. Die Summe aller möglichen
Pfade bis Tiefe 10 ist kleiner als `2^61`; jeder Index passt folglich in
einen nichtnegativen `FormulaAddress::index` (`int64_t`). Tiefe über 10 wird
in V1.4 mit `InvalidArgument` abgelehnt; es gibt kein stilles Überlaufen und
keine mehrdeutige Kürzung.

Ein Grid-Adresspfad ist nicht der berechnete Feldwert.

### 4.2 GridSpec

```text
GridIdentity
├─ id: 1..128 ASCII-Zeichen [A-Za-z0-9._-]
└─ version: > 0
```

```text
GridSpec
├─ identity: GridIdentity
├─ technique: registrierte TechniqueIdentity
├─ canonical_parameters: Kernelparameter
├─ max_depth: 0..10
└─ default_budget: Budget
```

V1.4 verwendet ausschließlich `TechniqueKernel`, nicht den älteren direkten
`FormulaSpec`-Weg. Dadurch arbeiten PowerOffset, Fibonacci und künftig über
das Kernel SDK registrierte Formeln über dieselbe Schnittstelle.

Grid-ID und Version sind gemeinsam die Store-Identität. Das Registrieren
derselben Identität ersetzt die vorhandene `GridSpec`; bereits gespeicherte
Overrides für diese Identität bleiben erhalten und werden bei späteren
Anfragen gegen die neue `max_depth` geprüft. V1.4 verlangt, dass
Registrierungen und Override-Mutationen nicht gleichzeitig mit anderen
Store-Aufrufen erfolgen; parallele Lese-/Rechenarbeit bleibt Aufgabe der
bestehenden Engine.

### 4.3 Overrides und Rückgabe

Ein Override ist ein expliziter, exakter `BigInt` für genau eine vollständige
Adresse und wird durch `(GridIdentity, GridAddress)` adressiert. Er ist keine
Formel, keine Teilauswertung und keine Cache-Zeile.

```text
KevinValue
├─ result: EvaluationResult
└─ source: Override | Computed
```

Ein Override wird bei einer `Exact`-Anfrage als bestätigter exakter Wert
geliefert. Für Symbolic liefert er eine explizite Override-Darstellung, für
Metric seine Bitlänge und für Modular den exakten Rest. Das effektive Budget
gilt auch für Overrides; ein zu großer Exact-Override wird vor der Rückgabe
mit `BudgetExceeded` abgewiesen. V1.4 materialisiert keinen zweiten
ResultCache dafür. Eine unzulässige Modulusangabe bleibt ein
`InvalidArgument`.

## 5. Öffentliche Operationen

```text
KevinMemoryStore(Engine& engine)
register_grid(GridSpec) -> bool
set_override(GridIdentity, GridAddress, BigInt) -> Status
remove_override(GridIdentity, GridAddress) -> bool
get(GridIdentity, GridAddress, EvaluationMode, Budget, optional modulus)
  -> KevinValue
inspect() -> KevinStoreStatistics
```

Die C++-Signatur verwendet die explizite `GridIdentity`, damit ein Store
mehrere Grids verwalten kann. `inspect` enthält mindestens die Anzahl
registrierter Grids, Overrides, berechneter Exact-Anfragen und die
weitergereichten Engine-Cachewerte.

Für berechnete Werte kombiniert `get` das angefragte Budget mit dem
`GridSpec::default_budget` durch die strengere Grenze je Budgetfeld. Es
übergibt dann `TechniqueRequest` an die bestehende `Engine`. `set_override`
lehnt Werte ab, deren Bitlänge das `GridSpec::default_budget` bereits
überschreitet.

## 6. Auswertungs- und Fehlervertrag

1. Grid-Identität und `GridAddress` validieren.
2. `GridSpec` nachschlagen.
3. Prüfen, ob die Adresse innerhalb `max_depth` liegt.
4. Override nachschlagen.
5. Bei Override: angefragten Modus aus dem BigInt ableiten; sonst Index
   berechnen und die Engine lazy aufrufen.
6. Quelle, Status und Trace im `KevinValue` zurückgeben.

`InvalidArgument` gilt für unbekannte Grids, ungültige IDs, Koordinaten
außerhalb `0..7`, zu tiefe Pfade, nicht registrierte Techniques sowie einen
fehlenden oder nicht positiven Modular-Modulus. Ein Engine-`BudgetExceeded`
oder `Cancelled` wird unverändert an den Aufrufer weitergegeben. Ein lokaler
Fehler beendet weder andere Grids noch die Engine.

Der globale Engine-Abbruch ist der V1.4-`RUN_GATE`. Die vorhandenen
`GateScope`-Ereignisse unterscheiden Branch, Tile, Run, Cache und Publish;
KEVIN ergänzt keinen zweiten konkurrierenden Cancel-Mechanismus.

## 7. Speicher- und Determinismusvertrag

Der dauerhafte RAM-Verbrauch des Stores wächst mit:

```text
Anzahl GridSpecs + Anzahl Overrides + deren BigInt-Größe
```

Er wächst nicht mit der theoretischen Anzahl von Feldern oder regulären
Formelwerten. Berechnete Werte liegen ausschließlich im bereits budgetierten
Engine-`ResultCache` und können verdrängt werden.

Identische GridSpec-Version, Kernel-Identität, Parameter und vollständige
Adresse führen zu derselben `TechniqueRequest`-Adresse und damit zur
deterministischen Kernelauswertung. V1.4 garantiert Rekonstruierbarkeit nur
für die registrierte, versionierte Kernelbedeutung; sie behauptet keine
Kompression beliebiger unabhängiger Daten.

## 8. Tests und Benchmark

Unit- und Integrationstests decken ab:

- Kodierung/Dekodierung der 8×8-Schritte; jede Tiefe 0..10 sowie
  Kollisionsfreiheit exemplarischer unterschiedlicher Pfade;
- Abweisung ungültiger Koordinaten, IDs und Tiefe 11;
- registriertes PowerOffset- und Fibonacci-Grid in Symbolic, Metric,
  Modular und Exact;
- Override vor Formel für Exact, Metric, Modular und Symbolic;
- Budgetkombination und Weitergabe von Engine-Fehlern/Cancellation;
- Ersatz einer GridSpec ohne unkontrollierten Verlust von Overrides;
- `inspect` und die Obergrenze: ein tiefes Grid erzeugt ohne `get` weder
  Felder noch BigInts.

Ein Demo-/Benchmark-Szenario vergleicht die logische Repräsentation von
einem tiefen, nur formelbasierten Grid mit einer materialisierten
Referenztabelle. Es berichtet Records, Override-Anzahl, Cache-Statistik,
exakte Auswertungsanzahl und Laufzeit getrennt. Es behauptet keinen
generischen Speichergewinn für zufällige Daten.

## 9. Lieferumfang

- öffentliche Header für Store, GridAddress und GridSpec;
- Implementierung in einem eigenen `src/kevin/`-Bereich;
- eigene Tests in `tests/kevin_memory_store_tests.cpp`;
- ein öffentliches Demo-/Benchmark-Programm unter `demos/lhme/benchmarks/`;
- CMake-Registrierung und ein kurzes Nutzungsdokument;
- Aktualisierung des Architekturindex, damit Konzepte und implementierter
  Stand nicht auf eine fehlende Blueprint-Datei verweisen.
