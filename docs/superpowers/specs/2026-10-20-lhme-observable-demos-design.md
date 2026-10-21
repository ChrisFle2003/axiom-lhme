# LHME Observable Demos – Design

## Zweck

Die bestehende LHME-Bibliothek soll mit drei eigenständigen End-to-End-Demos
sichtbar und automatisch überprüfbar werden. Die Demos sind kein Teil der
öffentlichen Library-API und verändern keine Produktlogik. Sie zeigen deren
Verhalten für Entwicklung, Debugging und spätere Add-ons nachvollziehbar.

Erfolg bedeutet:

- eine Person kann die drei Kernideen direkt im Terminal sehen;
- CTest prüft dieselben Demonstrationen ohne manuelle Kontrolle;
- eine spätere Erweiterung kann eine weitere Demo ergänzen, ohne bestehende
  Library- oder Unit-Test-Dateien umzubauen.

## Abgrenzung

Im Scope sind CPU/RAM, die vorhandene `PowerOffset`-Formel sowie die bereits
implementierte C++-Engine. GPU, persistenter Speicher, Benutzeroberfläche,
  Netzwerk und neue mathematische Formeln sind nicht Teil dieses Schritts.

`tests/` bleibt ausschließlich für kurze Unit-, API- und Regressionschecks.
Die neuen Programme liegen getrennt unter `demos/lhme/`.

## Struktur

```text
demos/lhme/
├─ support/
│  ├─ demo_report.hpp          # lesbare Tabellen und PASS/FAIL-Sammelstatus
│  └─ reference_values.hpp     # feste, kleine Referenzwerte der Demo
├─ formula_demo.cpp            # Adresse → vier Auswertungsmodi
├─ range_demo.cpp              # CPU-Kacheln → kanonische Reduktion
└─ memory_demo.cpp             # Formel+Adresse → Speichervergleich
```

Die CMake-Konfiguration erzeugt drei ausführbare Ziele:

```text
lhme_formula_demo
lhme_range_demo
lhme_memory_demo
```

Alle drei werden als CTest-Tests registriert. Ein Programm gibt bei jeder
verletzten Erwartung einen von null verschiedenen Exit-Code zurück.

## Gemeinsames Demo-Protokoll

Jede Demo folgt derselben Form:

```text
Titel und Formel
→ Eingabe / Konfiguration
→ kleine Ergebnistabelle
→ erwartete Invariante
→ PASS oder FAIL
```

Die Tabellen sind stabil: kein Zeitstempel, keine Speicheradresse und keine
thread- oder fertigstellungsabhängige Reihenfolge. Deshalb bleiben die
Ausgaben für Debugging, Screenshots und spätere Vergleiche deterministisch.

Bei einem Fehler zeigt die Ausgabe:

```text
FAIL exact value: expected=102300 actual=102301
```

und das Programm beendet sich mit Exit-Code `1`.

## Demo 1 – Formula Modes

`formula_demo.cpp` registriert:

\[
h(x)=100(2^x-1)
\]

und fragt dieselbe Adresse `x = 10` in allen vier Modi ab.

| Modus | Erwartete Aussage |
|---|---|
| Symbolic | Formeltext, keine materialisierte Ganzzahl |
| Metric | Größenmetrik, keine materialisierte Ganzzahl |
| Modular (`mod 17`) | Restwert `11` |
| Exact | Ganzzahl `102300` |

Zusätzlich wird ein Exact-Request mit zu kleinem Bitbudget ausgeführt. Er
muss `BudgetExceeded` liefern und darf keine Ganzzahl enthalten. Die Demo
beweist damit die Auswertungsleiter und den fail-closed Budgetpfad.

## Demo 2 – CPU Range and Deterministic Reduction

`range_demo.cpp` berechnet `x = 0..20` einmal seriell und einmal in
CPU-Kacheln mit mehreren Workern. Es vergleicht:

- globale Prüfsumme (`checked_count`, `confirmed_count`, ungültige Adressen),
- Zahl und Reihenfolge aller Kandidaten,
- letzten exakten Wert `104857500`.

Danach wird ein Bereich `-1..5` ausgeführt. Der negative Zweig muss lokal als
ungültig gezählt werden; die sechs gültigen Adressen müssen trotzdem bestätigt
werden. Dies macht sichtbar, dass Branch-Rejection nicht den gesamten Run
abbricht.

## Demo 3 – Formula/Address versus Materialized Value

`memory_demo.cpp` betrachtet die große Adresse `x = 1_000_000` derselben
Formel. Die Demo führt nur Symbolic und Metric aus, nie Exact.

Sie zeigt drei getrennte Größen:

| Größe | Bedeutung |
|---|---|
| `FormulaSpec` + `FormulaAddress` | aktive beschreibende Eingabe im Prozess |
| Metric-Bitlänge | Größe eines hypothetisch exakten Werts |
| hypothetische Mindestbytes | `ceil(Bitlänge / 8)` ohne BigInt-/Objekt-Overhead |

Die Demo meldet PASS, wenn keine exakte Ganzzahl materialisiert wurde und die
Metric-Bitlänge positiv ist. Sie behauptet ausdrücklich nicht, dass
`sizeof(FormulaSpec)` der vollständige reale RAM-Verbrauch einer Anwendung
ist; Cache, Trace, Container und Runtime-Overhead bleiben getrennte Größen.

## Add-on-Vertrag

Eine spätere Demo wird durch eine neue einzelne Quelldatei und einen CMake-
Eintrag ergänzt. Sie muss:

1. die gemeinsame `demo_report.hpp` verwenden,
2. nur feste, deterministische Eingaben verwenden,
3. mindestens eine fachliche Invariante automatisch prüfen,
4. Library-Zustand nicht auf Platte schreiben und
5. keinen bestehenden Demo-Output stillschweigend verändern.

So können Erweiterungen logisch unabhängig entstehen: Demo hinzufügen,
Erwartung formulieren, CTest registrieren.

## Fehlerbehandlung

- Ein erwarteter Budget- oder Branch-Stop ist ein PASS, wenn Status und
  leere Ergebnisform der Spezifikation entsprechen.
- Unerwartete Statuswerte, Reihenfolgen oder materialisierte Werte sind FAIL.
- Fehlende Ressourcen oder interne Fehler werden mit Status und kurzer
  Erklärung ausgegeben; keine Demo verschluckt diese Fehler.

## Teststrategie

| Ebene | Nachweis |
|---|---|
| Unit/API | bestehende Tests unter `tests/` |
| Sichtbarkeit | Menschen lesen die stabilen Tabellen der drei Programme |
| Automatisierung | CTest führt jedes Demo-Programm aus und verlangt Exit-Code 0 |
| Regression | geänderte Logik bricht die passende fachliche PASS-Invariante |

## Akzeptanzkriterien

1. `cmake --build build` erzeugt alle drei Demo-Programme.
2. Jedes Programm gibt einen kurzen deterministischen Bericht und `PASS` aus.
3. `ctest --test-dir build --output-on-failure` führt die drei Demos zusätzlich
   zu den bestehenden Tests aus.
4. `memory_demo` berechnet für `x = 1_000_000` niemals Exact.
5. Ein absichtlich zu kleines Exact-Bitbudget in `formula_demo` wird sichtbar
   als erwarteter Safe-Stop bestätigt.
