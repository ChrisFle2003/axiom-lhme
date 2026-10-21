# KEVIN Grid Filter V1.6 — Design

V1.6 ergänzt KEVIN um eine deterministische 64-Bit-Filterstufe für genau eine
8×8-Kachel. `bit = y * 8 + x`; eine gesetzte Position ist ein überlebender
Kandidat. Filter sind rein mathematisch: Modular-Gleichheit und Bitlängen-
Bereich. Sie kombinieren sich durch AND/OR/XOR/NOT und zählen mit Popcount.

Eine Kachel wird ohne BigInt-Materialisierung geprüft, soweit Modular- oder
Metric-Auswertung ausreicht. Nur gesetzte Bits erzeugen eine konkrete
Folgeadresse und dürfen später Exact anfordern. Ein Filter-Miss ist lokal;
er stoppt keine andere Kachel oder den Engine-Lauf. Jeder Filter gibt Maske,
Anzahl und eine nachvollziehbare Filterbezeichnung zurück.

Nicht Teil von V1.6: freie Callbacks, semantische Heuristiken, GPU-Ausführung
oder vollständige Subtree-Persistenz.
