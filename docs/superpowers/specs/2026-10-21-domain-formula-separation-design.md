# BRUCE Domain–Formula Separation — Refactor Design

**Status:** Zur Nutzerprüfung

## Ziel

Die Library trennt vier bisher teilweise vermischte Rollen dauerhaft:

```text
FormulaRecipe / Technique = erzeugt einen Wert
FormulaAddress            = Position im Formelraum
KEVIN Domain              = hierarchische 8×8-Koordinate und Filterraum
OverrideStore             = seltene explizite Wertabweichungen
```

Eine KEVIN-Adresse ist keine Formeladresse und kein gespeicherter Wert. Sie
kann über eine ausdrücklich registrierte, versionierte Projektion mit einem
Formelraum verbunden werden. Ohne diese Projektion bleibt sie reine Domain-
und Prozesskoordinate.

## Nicht-Ziele

- Keine automatische Kompression beliebiger unabhängiger Daten.
- Keine GPU- oder semantische Runtime-Erweiterung.
- Keine stille Datenmigration oder stilles Ändern der Berechnungsbedeutung.
- Keine Änderung an der bestehenden C-Formula-API.

## Zielkomponenten

### FormulaRecipe und FormulaAddress

`FormulaSpec`, `TechniqueKernel`, `FormulaAddress`, die `Engine` und der
`ResultCache` bleiben der einzige Wertekern. Eine Formel wird direkt über
ihre eigene Adresse ausgewertet; KEVIN ist dafür nicht erforderlich.

### FormulaOverrideStore

Ein neuer Store hält nur explizite Abweichungen. Sein Schlüssel enthält:

```text
Formula- oder Technique-Identität
+ Version und kanonische Parameter
+ FormulaAddress
```

Er beantwortet eine Anfrage zuerst aus einem passenden Override und delegiert
sonst an die Engine. Die Rückgabe kennzeichnet `Override` oder `Computed`.
Snapshots speichern Formula-Bindung und Overrides, nicht Grid-Regeln als
Voraussetzung für einen Formelwert.

### KEVIN Domain

`GridIdentity`, `GridStep` und `GridAddress` bilden nur den hierarchischen
8×8-Raum. Ein Domain-Spec enthält Identität, Version, maximale Tiefe und
optionale Filter-/Budgetvorgaben. Er enthält keine Technique und keine
Formelparameter.

`GridAddress` behält keine implizite `formula_address()`-Methode. Das schützt
vor der falschen Annahme, jede Dimension müsse denselben Formelindexraum
verwenden.

### DomainProjection

Eine Projektion ist die explizite, versionierte Brücke:

```text
DomainIdentity + GridAddress
    -> ProjectionIdentity + Version
    -> FormulaAddress
```

V1 des Refactors liefert eine kanonische `DepthPathProjection`: Sie verwendet
die bisherige Pfadkodierung nur dann, wenn ein Nutzer diese Bindung bewusst
registriert. Spätere Projektionen dürfen andere deterministische Zuordnungen
definieren. Die Projektion gehört in Trace- und Cache-Identitäten, sobald sie
für eine Auswertung eingesetzt wird.

## Datenfluss

### Direkte Formelberechnung

```text
FormulaRecipe + FormulaAddress
-> FormulaOverrideStore
-> Engine
-> ResultCache
-> EvaluationResult
```

### Domainbasierte Verarbeitung

```text
KEVIN Domain + GridAddress
-> DomainProjection
-> FormulaAddress
-> FormulaOverrideStore
-> Engine
-> EvaluationResult
```

Die Filterpipeline bildet ausschließlich Kandidaten im Gridraum. Sie ruft
eine explizite Projektion und einen Formel-Evaluator auf; sie hängt nicht an
einem Grid-Speicherstore.

## Storage-Policy

Die Policy beschreibt Wertrepräsentationen, nicht KEVIN:

```text
FormulaOnly
FormulaWithOverrides
DenseTable
HashTable
```

`FormulaWithOverrides` ersetzt den bisherigen Namen `SparseOverrides`. Die
Policy entscheidet anhand von Override-Dichte und Zugriffsmuster. Eine
spätere Migrationsschicht führt die Datenübertragung kontrolliert aus; die
Policy selbst mutiert keine Daten.

## API- und Migrationsvertrag

Die bisherige `KevinMemoryStore`-API vermischt Domain, Formula-Bindung und
Overrides. Sie wird nicht still umgedeutet.

1. Neue, getrennte APIs werden ergänzt und getestet.
2. Demos und Benchmarks wechseln auf FormulaRecipe + FormulaOverrideStore.
3. KEVIN-Filter wechseln auf DomainProjection + Evaluator.
4. Der alte Store wird als veraltet markiert und mit einem klaren
   Migrationshinweis versehen.
5. Erst in einer späteren Hauptversion darf die alte API entfernt werden.

Der bestehende C-ABI bleibt unverändert, weil er direkt FormulaSpec und
FormulaAddress nutzt.

## Benchmarks

Es werden drei getrennte Messungen geführt:

1. FormulaRecipe gegen materialisierte Tabelle/Hash-Map.
2. FormulaRecipe + OverrideStore bei 0 %, 0,1 %, 1 %, 10 % und 100 %
   Abweichungen.
3. KEVIN-Domain-Projektion und Filter als zusätzlicher Overhead, getrennt von
   der Wertrepräsentation.

Die Prozess-Peak-Messung bleibt die praktische RAM-Metrik. Snapshot-Größe,
Containergröße und Peak-RAM werden nie als identische Metriken ausgegeben.

## Fehler, Gates und Determinismus

- Fehlende oder ungültige Projektionen liefern `InvalidArgument` für den
  betroffenen Branch.
- Ein Grid-Filter-Miss verwirft nur diesen Kandidaten.
- Engine-Cancellation bleibt der einzige Run-weite Abbruchmechanismus.
- Gleiche Domain-, Projektions-, Formel- und Modusidentitäten ergeben dieselbe
  Auswertung und denselben Cache-/Trace-Kontext.

## Abnahmekriterien

- Direkte Formelauswertung funktioniert ohne KEVIN-Objekte.
- Zwei Domains können dieselbe GridAddress auf unterschiedliche
  FormulaAddresses abbilden.
- Eine Domain ohne Projektion kann nicht versehentlich einen Formelwert
  anfordern.
- Formula-Overrides wirken unabhängig von einer KEVIN-Domain.
- Filter behalten deterministische 64-Bit-Masken und lösen nur explizit
  projizierte Formelbewertungen aus.
- Bestehende C-API- und Formula-Tests bleiben unverändert grün.
- Neue und bestehende Benchmarks berichten Formel-, Override- und
  Domain-Overhead getrennt.

## Risiken und Entscheidungen

| Thema | Entscheidung |
|---|---|
| Alte KEVIN-Snapshots | Nicht still laden; Migrationswerkzeug oder explizite Ablehnung. |
| API-Kompatibilität | Alte C++-Store-API zunächst als veraltet erhalten. |
| Projektionsidentität | Versionierter Bestandteil von Trace und Cache, sofern verwendet. |
| Speicherpolitik | 5 % bleibt ein konfigurierbarer konservativer Default, keine Naturkonstante. |
