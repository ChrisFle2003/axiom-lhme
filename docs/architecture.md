# Implementierte Architektur

Der aktuelle C++-Kern umfasst `FormulaSpec`/Technique-Kernels, BigInt-/Metric-/Modular-/Symbolic-Auswertung, begrenzten `ResultCache`, CPU-Streaming und zwei klar getrennte Schichten:

```text
FormulaRecipe + FormulaAddress
  -> FormulaOverrideStore (nur echte Abweichungen)
  -> Engine
  -> ResultCache

KEVIN GridDomainSpec + GridAddress
  -> explizite DomainProjection
  -> FormulaAddress
  -> FormulaEvaluator
```

Die Formel ist der Wertgenerator. KEVIN ist ausschließlich ein hierarchischer
8×8-Domain-, Filter- und Prozessraum: dieselbe Grid-Adresse kann über zwei
Projektionen auf unterschiedliche Formeladressen zeigen. Ohne Projektion lässt
sich aus einer Grid-Adresse kein Wert ableiten. 64-Bit-Tile-Filter reduzieren
lokal per Modular- und Größenmetrik; ein Filter-Miss verwirft nur den betroffenen
Kandidaten. Der Cache bleibt ein begrenzter Beschleuniger, nie die Quelle der
Wahrheit.

Jede erfolgreiche domainbasierte Auswertung ergänzt den Trace mit
`Domain-ID@Version`, `Projection-ID@Version` und der erreichten
`FormulaAddress`. Fehlende, ungültige oder nicht abbildbare Projektionen werden
als branch-lokale `InvalidArgument`-Spur festgehalten. Damit bleibt sichtbar,
welche Dimension und welche Abbildungsregel einen Formelwert bestimmt haben.

`KevinMemoryStore` und seine Snapshots bleiben als dokumentierte
Kompatibilitätsgrenze für ältere Aufrufer erhalten. Neue Programme verwenden
stattdessen `GridDomainSpec`/`DomainProjection` plus `FormulaOverrideStore`.
Der neue Override-Snapshot besitzt ein eigenes Format und akzeptiert alte
KEVIN-Snapshots nicht stillschweigend. Er prüft Format, kanonische Zahlen,
Checksummen und unerwartete Restdaten fail-closed.

## Stable-Prüfung

Neben dem normalen CTest-Suite-Lauf kann die Library mit einem Runtime-
Sanitizer gebaut werden. Die Konfiguration bleibt absichtlich explizit, damit
normale Release-Binaries keine Debug-Laufzeitkosten tragen:

```text
cmake -S . -B build-address -DBRUCE_SANITIZER=address
cmake --build build-address -j2
ctest --test-dir build-address --output-on-failure
```

Erlaubte Werte sind `address`, `thread` und `undefined`; `none` ist der
Standard. Der Override-Store besitzt zusätzlich einen parallelen Read/Write-
Stresstest, der konsistente Antworten während laufender Override-Updates prüft.

Für reproduzierbare Debug-, Release- und Sanitizer-Builds stehen gleichnamige
CMake-Presets in `CMakePresets.json` bereit. Der vollständige v0.1-Vertrag und
die noch plattformabhängigen Freigabeschritte stehen in `docs/release-v0.1.md`.

Wenn eine überwachte Entwicklungsumgebung LeakSanitizer ausdrücklich als mit
`ptrace` inkompatibel meldet, kann der AddressSanitizer-Testlauf lokal mit
`ASAN_OPTIONS=detect_leaks=0` wiederholt werden. Das deaktiviert nur die in
dieser Umgebung nicht ausführbare Leak-Prüfung; reguläre CI-Läufe sollen sie
aktiv lassen.

NanoCell, semantisches Routing, AnswerPoints, Experience Records und GPU-Backends bleiben Konzepte beziehungsweise spätere Ausbaustufen.
