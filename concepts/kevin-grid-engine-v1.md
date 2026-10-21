# KEVIN Grid Engine – Mini-Upgrade V1

**Ziel:** Ein hierarchisches 8×8-Gitter, das Felder nur berechnet, wenn sie tatsächlich angefordert oder durch einen Filter ausgewählt wurden.

## Kernidee

Eine Gitteradresse ist nie der Feldinhalt.

Jede Ebene enthält ein 8×8-Raster. Ein Schritt im Raster braucht sechs Bits:

```text
yyyxxx
```

- `x`: 0–7
- `y`: 0–7
- ein Adresspfad besteht aus mehreren solchen Schritten
- Tiefe 1: 64 Felder
- Tiefe 2: 4.096 Felder
- Tiefe 4: 16.777.216 theoretische Felder

Gespeichert werden nur Grid-Regel, Adresse, aktive Berechnungen und ein begrenzter Cache.

## Bausteine

### 1. GridAddress

Repräsentiert die eindeutige, kollisionsfreie Koordinate.

```text
level = 3
path = [(3,5), (6,2), (1,7)]
```

Die Koordinate bleibt stabil, auch wenn die Feldregel später wechselt.

### 2. GridSpec

Beschreibt, wie ein Feld berechnet wird.

```text
GridSpec:
- valueType: Boolean | UInt64 | BigInt | TextSymbol
- rule: Power | ModularPower | Lookup | CustomFormula
- limits: maxDepth, maxBits, maxOperations
- zeroZeroPolicy: ValueOne | ReservedStop
```

Für die erste Version wird `Power` umgesetzt:

```text
value(x, y) = x^y
```

Dabei bleibt `0^0` ausdrücklich konfigurierbar. Als reservierter Grid-Zustand kann es „Ast nicht erweitern“ bedeuten, aber der tatsächliche Abbruch läuft immer über ein separates ExecutionGate.

### 3. Lazy Evaluator

Ablauf einer Feldanfrage:

```text
Adresse anfordern
→ GridSpec und Rechenbudget prüfen
→ fehlende Abhängigkeiten berechnen
→ Feldwert exakt, modular oder als Größenmetrik auswerten
→ Ergebnis mit Begründung zurückgeben
→ optional begrenzt cachen
```

Drei Rechenmodi:

- `Exact`: Ergebnis als Ganzzahl; nur innerhalb des Bitlimits.
- `Modular`: `x^y mod M`; geeignet für Bitfilter und Zyklen.
- `Metric`: nur Bitlänge, Rechenkosten oder Größenklasse; riesige Zahlen werden nicht materialisiert.

### 4. Filter Engine mit 64-Bit-Wörtern

Filter arbeiten nicht auf „beliebigen Daten“, sondern auf klar definierten Kandidaten.

Ein 64-Bit-Wort repräsentiert 64 Kandidaten als Pass/Fail-Bits:

```text
AND      = Kandidat muss beide Prüfungen bestehen
OR       = mindestens eine Prüfung genügt
XOR      = Unterschied markieren
popcount = verbleibende Kandidaten zählen
```

Startfilter:

- Wertebereich
- Teilbarkeit oder Restklasse
- Bitlänge
- exakter Treffer auf Lernpaaren
- Ausschluss wegen Widerspruch
- Stop-Sentinel

Wichtig: Bei räumlichen Verschiebungen schützen Zeilenmasken davor, dass Bits über eine Rasterzeile hinweg wandern.

### 5. ExecutionGate

Das Gate kontrolliert die Ausführung unabhängig von der Mathematik.

```text
StopReason:
- GridSentinel
- ResourceLimit
- Cancelled
- InvalidCoordinate
- InvalidFormula
```

So bleibt ein reserviertes `0^0` lesbar, ohne als einziger Not-Aus missbraucht zu werden.

### 6. Nachvollziehbare Suche

Jeder Kandidat erhält eine Spur:

```text
Adresse
→ angewandte Regel
→ Filterergebnisse
→ erster Widerspruch oder Treffer
→ Rechenkosten
```

Damit ist der Raum nicht nur groß, sondern überprüfbar. Ein Filter darf Kandidaten nur verwerfen, wenn der Ausschluss mathematisch begründet ist.

## Umsetzung in sechs kleinen Schritten

1. **Adressmodell**
   - `GridAddress` mit Ebene und 8×8-Pfad.
   - Tests für eindeutige Kodierung und Rückumwandlung.

2. **Feldtypen und GridSpec**
   - Typ-Tags für Bit, 64-Bit-Zahl, BigInt und Symbol.
   - Konfigurierbare Regel und Berechnungslimits.

3. **Lazy Evaluator**
   - Einzelnes Feld auf Anfrage berechnen.
   - Begrenzter LRU-Cache, kein vollständiges Grid im RAM.

4. **Power-Regel und sichere Sonderfälle**
   - Exaktes `x^y`, modularer Modus und Metrikmodus.
   - Testfälle für `2^6`, `4^3`, `0^5`, `1^n` und die konfigurierte Behandlung von `0^0`.

5. **64-Bit-Filterpipeline**
   - Bitset-Filter für 64 Kandidaten.
   - Tests für AND/OR/XOR, Popcount und Zeilengrenzen.

6. **Kleines Kommandozeilen-Demo**
   - Adresse, Regel, Modus und Filter übergeben.
   - Ausgabe enthält Wert, Entscheidung, Stopgrund und Prüfspur.

## Abnahmekriterien für V1

- Ein Feld in Tiefe 4 ist direkt anfragbar, ohne 16 Millionen Felder anzulegen.
- Die Adresse bleibt eindeutig; gleiche Werte wie `2^6` und `4^3` kollidieren nicht.
- Große Potenzen können als Metrik geprüft werden, ohne riesige Ganzzahlen zu speichern.
- Ein Filterlauf zeigt exakt, warum ein Kandidat behalten oder ausgeschlossen wurde.
- Cache und Rechenbudget verhindern unkontrolliertes Wachstum.
- `0^0` kann einen Grid-Stopp markieren, ohne den allgemeinen Abbruchmechanismus zu ersetzen.

## Bewusst nicht Teil von V1

- Speicherung beliebiger Textmengen als angebliche Kompression.
- Semantische Suche nach „ähnlichen Bedeutungen“.
- Unbegründete Heuristiken, die gültige mathematische Kandidaten still verwerfen.
- Vollständiges Ausrollen des exponentiellen Gitters in RAM oder auf Platte.