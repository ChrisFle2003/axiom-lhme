# LHME Isolated Peak-Memory Stress Design

## Zweck

Der vorhandene kontrollierte Stresstest meldet `ru_maxrss` als höchsten Wert
des gesamten Testprozesses seit dessen Start. Das beweist begrenzte Retention,
liefert aber keine voneinander isolierten Peak-Werte je Szenario.

Dieser Ausbau führt für Linux einen Controller ein, der jedes Messszenario in
einem frisch gestarteten Kindprozess ausführt. Damit erhält jedes Szenario
einen eigenen, vergleichbaren Prozess-Peak-RAM-Wert.

## Scope

Im Scope ist ausschließlich das manuelle Ziel `lhme_controlled_stress` unter
`demos/lhme/benchmarks/stress/`.

Nicht im Scope sind Library-API-Änderungen, GPU, Persistenz, Netzwerk,
automatische CTest-Ausführung des Millionenlaufs oder plattformübergreifende
Prozessabstraktionen.

## Architektur

```text
Stress-Controller (Elternprozess)
        │ fork + exec eines frischen Selbstprozesses
        ▼
Isolierter Szenario-Prozess
        │ führt genau ein Szenario aus
        │ liest eigenes ru_maxrss
        │ schreibt festen Ergebnisdatensatz in Pipe
        ▼
Stress-Controller
        │ prüft Status, Zähler und Kindprozess-Ende
        ▼
lesbarer Vergleichsbericht
```

Der Controller startet das aktuelle ausführbare Programm über
`/proc/self/exe` mit `--isolated-child <scenario>`. Das `exec` ist
entscheidend: Der Kindprozess beginnt mit einem frischen Adressraum statt mit
Copy-on-Write-Seiten des Controllers.

## Szenarien

| Name | Arbeit | Erwartung |
|---|---|---|
| `million-scan-1` | `0..1_000_000`, niemals treffender Modulfilter, 1 Worker | 1.000.001 geprüft, 0 Exact, 0 retained |
| `million-scan-8` | dieselbe Arbeit, 8 Worker | gleiche Fachzähler, eigenständiger Peak |
| `adaptive-hits` | `0..10_000`, Modulrest 11, Adaptive-Limit 3 | 1.250 Exact, 8.751 Filter-Misses, TopK mit drei niedrigsten Werten |

Der vorhandene Pre-cancel-SafeStop bleibt ein lokaler, sofortiger
Funktionstest; er ist kein sinnvoller RAM-Messfall, da er absichtlich keine
Arbeit ausführt.

## Ergebnisprotokoll

Der Kindprozess schreibt genau einen festen binären Datensatz in eine Pipe.
Er enthält Version, Status, `GlobalSummary`, Filter- und Exact-Zähler,
Retention-Modus, Anzahl retained Kandidaten, Laufzeit in Millisekunden und
Peak-RAM in Bytes.

Controller und Kind sind dieselbe Programmversion. Das Protokoll ist daher
kein öffentlicher oder persistenter Dateiformatvertrag. Ein unvollständiger
Datensatz, ein unbekannter Versionswert, ein nicht erfolgreicher `exec` oder
ein fehlerhafter Prozess-Endstatus gilt fail-closed als Testfehler.

## Plattformverhalten

Linux verwendet `fork`, `exec`, `pipe`, `waitpid`, `/proc/self/exe` und
`getrusage(RUSAGE_SELF)`. `ru_maxrss` wird auf Linux von KiB nach Bytes
umgerechnet.

Auf anderen Plattformen meldet das Ziel klar, dass isolierte Peak-Messungen
nicht verfügbar sind, und führt keine Ersatzmessung mit falscher Semantik
durch.

## Validierung

Der Controller prüft für jeden Kindbericht alle festen Fachzähler sowie einen
positiven, verfügbaren Peak-Wert. Die Zeitwerte bleiben reine Beobachtungs-
daten; keine feste Laufzeit ist ein Pass/Fail-Kriterium.

Das Ziel bleibt bewusst außerhalb von CTest, weil der Millionenlauf ein
manueller, kontrollierter Stresstest ist. Der vollständige Build muss es
jedoch erzeugen.
