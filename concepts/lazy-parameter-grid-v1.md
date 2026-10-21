# BRUCE Lazy Parameter Grid V1 – Software-Plan

**Ziel:** BRUCE soll große, regelhaft beschriebene Parameteräume gezielt untersuchen, ohne sie vollständig zu speichern oder auszurechnen.

**Architektur:** Das Grid ist eine interne Research-Strategie des BRUCE Research Core. Es erzeugt Kandidaten bei Bedarf, führt günstige Filter vor der teuren Rechnung aus und akzeptiert Treffer nur nach exaktem Replay auf allen bekannten Paaren.

**Technik:** C++20, bestehender BRUCE Research Core, signed-512-Bit-Arithmetik, SSE-Trace und Cockpit.

## Was das Upgrade kann

```text
kleine Grid-Adresse
+ Regel
+ Parameterbereich
+ Rechenbudget
→ ein mathematischer Kandidat
→ Filter
→ exaktes Replay
→ Evidenz oder begründeter Ausschluss
```

Ein Feld ist nicht gespeicherter Inhalt, sondern eine Rechenanfrage.

```text
Adresse:   Pfad [(2,5), (6,1), (0,7)]
Regel:     Potenz
Parameter: Basis = 6, Exponent = 1
Modus:     Exact | Modular | Metric
```

Die Adresse bleibt eindeutig, auch wenn mehrere Parameter denselben Wert ergeben:

```text
2^6 = 64
4^3 = 64
```

Beide Kandidaten müssen unterschiedliche Adressen behalten.

## Datenmodell

### GridAddress

- Ein Schritt ist ein 8×8-Feld: `yyyxxx`, also 0 bis 63.
- Ein Pfad aus solchen Schritten bildet die Tiefe.
- Die Grid-Adresse beschreibt ausschließlich den Ort eines Kandidaten.

### GridSpec

```text
ruleFamily: Power | Affine | ModularPower
depthLimit: maximale Pfadtiefe
bitLimit: maximale exakte Ergebnisgröße
operationLimit: maximale Feldberechnungen
cacheLimit: maximale Cache-Einträge
mode: Exact | Modular | Metric
```

### Candidate und EvaluationResult

```text
Candidate:
- gridAddress
- Regel und konkrete Parameter
- Herkunft: vom Grid erzeugt

EvaluationResult:
- Wert oder Größenmetrik
- Filterentscheidungen
- erster Widerspruch
- Quotient und Rest, falls Division beteiligt ist
- Kosten und StopReason
```

## Ablauf einer Forschung

```text
Lernpaare eingeben
→ GridSpec festlegen
→ nur aussichtsreiche Adressen anfordern
→ Kandidat erzeugen
→ schnelle Filter anwenden
→ genaue Rechnung nur bei Überleben
→ alle Lernpaare exakt replayen
→ optional unabhängige Holdout-Prüfung
→ Evidenzspur archivieren
```

Ein Filter darf nur dann ausschließen, wenn sein Ausschluss mathematisch korrekt ist. Ein schneller 64-Bit-Filter spart Rechenzeit, ist aber kein Wahrheitsbeweis.

## V1: bewusst klein anfangen

V1 unterstützt zunächst nur die Regelfamilie:

\[
f(x,y)=x^y
\]

mit drei Auswertungen:

- **Exact:** Ergebnis als BigInt, solange das Bitlimit eingehalten wird.
- **Modular:** \(x^y \bmod M\), zum schnellen Erkennen von Restklassen und Ausschlüssen.
- **Metric:** Bitlänge und Rechenkosten, ohne riesige Potenzen zu materialisieren.

`0^0` wird nicht stillschweigend gerechnet. Die GridSpec legt fest:

```text
ValueOne       → mathematisch als 1 behandeln
ReservedStop   → Ast nicht erweitern
```

Ein tatsächlicher Abbruch geschieht separat über `ExecutionGate` mit Gründen wie `ResourceLimit`, `Cancelled` oder `GridSentinel`.

## Umsetzungsschritte

### 1. Adress- und Budgetmodell

Neue Kernbausteine:

```text
include/bruce/lazy_parameter_grid.hpp
src/lazy_parameter_grid.cpp
tests/test_lazy_parameter_grid.cpp
```

Tests:

- 64 einzelne 8×8-Felder sind eindeutig kodier- und dekodierbar.
- Gleicher Feldwert erzeugt keine Adresskollision.
- Tiefe 12 beschreibt \(64^{12}\) mögliche Felder, ohne diese anzulegen.
- Ungültige Koordinaten und zu große Tiefe liefern einen klaren Fehler.

### 2. Lazy-Kandidatenerzeugung

Der Core erzeugt einen Kandidaten erst nach einer expliziten Feldanfrage.

Tests:

- Nicht angeforderte Koordinaten erhöhen den Berechnungszähler nicht.
- Wiederholte Anfragen verwenden den begrenzten Cache.
- Überschrittenes Operations- oder Bitlimit beendet nur den betroffenen Ast.
- Ein Grid mit riesiger theoretischer Größe belegt nicht proportional Speicher.

### 3. Potenzregel und Filterpipeline

Neue Strategie:

```text
include/bruce/power_grid_strategy.hpp
src/power_grid_strategy.cpp
tests/test_power_grid_strategy.cpp
```

Tests:

- `2^6` und `4^3` ergeben beide exakt 64, bleiben aber verschiedene Kandidaten.
- Modular- und Metric-Modus materialisieren keine unnötig große Ganzzahl.
- 64 boolesche Kandidatenentscheidungen werden mittels AND, OR, XOR und Popcount korrekt gefiltert.
- Zeilenmasken verhindern Bitüberlauf zwischen 8×8-Zeilen.
- `0^0` folgt ausschließlich der gewählten GridSpec-Regel.

### 4. Exaktes Replay und Evidenzspur

Jeder überlebende Kandidat wird durch den vorhandenen Research Core geprüft.

Tests:

- Jeder Lernwert wird mit derselben konkreten Formel erneut berechnet.
- Nicht-ganzzahlige Division zeigt Quotient und Rest; ein Rest ungleich null ist kein Treffer.
- Ein Widerspruch speichert das erste scheiternde Paar.
- Ein Treffer bleibt „Kandidat“, bis eine unabhängige Holdout-Prüfung bestanden ist.

### 5. Cockpit-Anbindung

Neue Forschungsoption im bestehenden Cockpit:

```text
Lazy Parameter Grid
```

Anzeige:

```text
Grid-Adresse
→ Regel und Parameter
→ Filterspur
→ exakte Rechnung oder Größenmetrik
→ erster Widerspruch / Trefferstatus
→ StopReason
```

Keine neue separate Oberfläche: Die Strategie erscheint neben den bestehenden mathematischen Forschungswegen.

## Abnahmekriterien

- BRUCE kann einen Raum von mindestens \(64^{12}\) **beschreiben**, ohne ihn vollständig zu materialisieren.
- Es werden nur angeforderte oder nachweislich aussichtsreiche Kandidaten berechnet.
- Jeder Ausschluss besitzt eine lesbare Begründung.
- Jeder Treffer wird exakt replayt; es gibt kein Runden und keine stillschweigende Interpolation.
- Große Potenzen sind über Bitlänge oder modularen Wert prüfbar, ohne Speicher zu sprengen.
- Abbruch, Budgetgrenzen und Cache sind in der Trace sichtbar.

## Bewusst nicht Teil von V1

- Vollständige Suche durch exponentielle Räume.
- Behauptungen, eine große Parameterzahl sei bereits Erkenntnis.
- Speicherung beliebiger Daten als angebliche Kompression.
- Automatische mathematische Beweise ohne Replay und Gegenprüfung.