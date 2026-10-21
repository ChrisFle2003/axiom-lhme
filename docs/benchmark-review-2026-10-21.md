# BRUCE LHME Benchmark Review — 2026-10-21

## Kurzfazit

`FormulaRecipe` ist für große, **regelmäßig berechenbare** Wertebereiche sehr
speichereffizient. Die gemessene Million-Werte-Tabelle benötigt als reine
Formelbasis 3.38 MB Prozess-Peak statt 11.19 MB für einen dichten
`UInt64`-Vektor oder 43.56 MB für eine `HashTable`.

Der Vorteil gilt nicht pauschal für jede Datenlage: echte `BigInt`-Overrides
werden absichtlich einzeln gespeichert. Bei 10 % Overrides liegt deren
Prozess-Peak bereits bei 35.53 MB; bei 100 % bei 323.08 MB. Sobald die meisten
Werte Ausnahmen von der Formel sind, ist eine materialisierte Tabelle die
bessere Repräsentation.

KEVIN zeigt in diesem Stand eine andere Eigenschaft: Es ist eine 8×8-Domain-
und Filterebene, kein Wertspeicher. Die Pipeline filterte 4,096 virtuelle
Zellen mit null Exact-Materialisierungen und blieb bei 1, 2, 4 und 8 Workern
deterministisch.

## Fair comparison: Track A

Track A beantwortet ausschließlich die Speicherfrage: Wie verhalten sich fünf
Repräsentationen **derselben modularen Skalare**? Die Rezeptur lautet
`h(i) = 100 * (2^i - 1) mod M` mit
`M = 2,305,843,009,213,693,951`. Gemessen wurden `FormulaRecipe`,
`FormulaRecipeWithOverrides`, `DenseVector`, `SortedPairs` und `HashTable`.
Alle verwenden dieselben deterministischen Adressen, denselben Seed
`284849161029` und eine ausgeschaltete Recipe-Cache-Konfiguration.

Die vollständige Release-Matrix lief mit Exit-Status 0 über die Domänen 1.024,
65.536 und 1.000.000, die angeforderten Override-Dichten 0 %, 0,1 %, 1 %,
10 % und 100 % sowie sequential, pseudo-random und repeat-heavy. Jeder Fall
hat einen nicht gewerteten Warm-up-Lauf und danach **eleven samples**; der
Bericht enthält Minimum, Median und Maximum. Kleine Domänen können die
angeforderte Dichte nicht exakt ganzzahlig darstellen: die Ausgabe nennt dann
den tatsächlich realisierten Anteil, beispielsweise 1 von 1.024 als 0,0977 %.

Ausgewählte rohe Release-Werte des 1.024-Adressen-Sequential-Falls:

| Repräsentation | Aufbau | Lookup min / median / max | isolierter Peak RAM |
|---|---:|---:|---:|
| FormulaRecipe | 16 µs | 186 / 186 / 236 µs | 1.482.752 B |
| FormulaRecipeWithOverrides, 0 % | 3 µs | 481 / 495 / 726 µs | 1.470.464 B |
| DenseVector, 0 % | 635 µs | 4 / 4 / 4 µs | 1.290.240 B |
| SortedPairs, 0 % | 441 µs | 4 / 4 / 7 µs | 1.298.432 B |
| HashTable, 0 % | 597 µs | 2 / 2 / 2 µs | 1.515.520 B |

Das ist **not a global ranking**: FormulaRecipe vermeidet die Materialisierung
und ist daher eine Speicherrepräsentation für regelhafte Daten. Dichte und
Hash-Repräsentationen bezahlen Aufbau- und RAM-Kosten, erhalten dafür aber
direkte `UInt64`-Zugriffe. Bei vielen Ausnahmen muss die Entscheidung daher
pro Datenform und Zugriffsmuster fallen.

## Fair comparison: Track B

Track B ist ein archiviertes Messergebnis eines begrenzten, öffentlich erzeugten BSGS-Lehrbeispiels mit
`p = 1019`, Generator `2` und bekanntem Exponenten `777`. Es misst einen
linearen Referenzscan und Baby-Step Giant-Step jeweils mit elf Läufen:

| Solver | Minimum | Median | Maximum |
|---|---:|---:|---:|
| Linear scan | 2 µs | 2 µs | 2 µs |
| Baby-Step Giant-Step | 3 µs | 3 µs | 9 µs |

Das Ergebnis ist nicht vergleichbar mit Track A (`not comparable to Track A`):
hier geht es um eine andere algebraische Aufgabe und eine winzige feste Gruppe,
nicht um die Speicherung einer Million Formelwerte. Es werden keine Passwörter,
Secrets oder produktiven Kryptoparameter verarbeitet.
Die zugehörige ausführbare Toy-Implementierung wird nicht ausgeliefert; dieser
Abschnitt bewahrt ausschließlich das Ergebnis und seine Sicherheitsgrenzen.

## Fair comparison: Track C

Track C ist `PrecomputedChainToy`: ein **synthetic non-security toy** über
einen öffentlichen 16-Bit-Zustandsraum. Es ist ausdrücklich keine Rainbow-
Table und nicht für Sicherheits- oder Credential-Anwendungen bestimmt.

| Parameter / Ergebnis | Wert |
|---|---:|
| deterministische Ketten | 128 |
| Kettenlänge | 64 |
| gespeicherte Endpunkte | 256 B |
| beobachtete Abdeckung | 128 / 128 erzeugte Token |
| Prüfarbeit | 1.067.790 Schritte |
| Endpunkt-Kandidaten | 142 |
| replay-verifizierte Auflösungen | 128 |

Ein gleicher Endpunkt ist nur ein Kandidat. Erst die Wiederholung der zugehörigen
Ursprungskette darf eine Auflösung bestätigen. Auch Track C ist `not comparable
to Track A`: Er bewertet keine RAM-Repräsentation derselben Werte, sondern die
Verifikation eines synthetischen Kettenmodells. Die ausführbare Toy-
Implementierung wird nicht ausgeliefert; erhalten bleibt nur dieses Ergebnis.

## Limitations

- Die Vollmatrix ist ein einzelner Release-Lauf auf diesem Host. Sie ersetzt
  keine Verteilungsanalyse über CPU-Takt, Temperatur, Allocator oder mehrere
  Maschinen.
- Die dokumentierten 1.024er Rohwerte sind eine ausgewählte, lesbare Ansicht;
  sie sind keine Zusammenfassung zu einer Gesamtwertung.
- Isolierter Peak-RAM ist ein Linux-Kindprozesswert und enthält Runtime- sowie
  Allocator-Overhead. Auf nicht unterstützten Plattformen meldet der Runner
  `unavailable`.
- Track B und Track C sind ausschließlich begrenzte, öffentliche Lernexperimente.
  Aus ihren Zeiten oder Trefferzahlen folgt keine Aussage über Kryptografie,
  Passwortsuche oder reale Angriffe.

## Messaufbau

| Eigenschaft | Wert |
|---|---|
| Zeitpunkt | 2026-10-21 |
| Quellstand | `630bdf26bbbf4af66928a13327451f9be9ae63c5` |
| Build | CMake Release-Preset |
| Compiler | GNU C++ 14.2.0 |
| Betriebssystem | Linux 6.12.113+deb13-amd64, x86_64 |
| CPU | AMD Ryzen 7 3800X, 16 logische CPUs sichtbar |
| Hauptspeicher | 65,775,380 KiB sichtbar |
| Prozess-Peak | isolierter Linux-Kindprozess; enthält Allocator- und Runtime-Overhead |

Die Zeiten sind einzelne lokale Messungen mit `std::chrono::steady_clock`.
Sie sind nützlich für Vergleiche auf dieser Maschine, aber keine portable
Leistungszusage. Es gab keine CPU-Pinning-, Frequenz-, Temperatur- oder
Wiederholungsanalyse. Der KEVIN-Test mittelt vier Reduktionen je
Worker-Konfiguration; die anderen Zeitwerte sind einzelne Durchläufe.

## Verifikation

Die vier als CTest registrierten Benchmarks wurden frisch im Release-Profil
ausgeführt und bestanden: `lhme_streaming_benchmark`,
`lhme_table_memory_benchmark`, `lhme_kevin_cpu_benchmark` und
`lhme_hybrid_storage_benchmark` (4/4, 2.44 s). Zusätzlich wurden der
Hybrid-Test mit `--full-overrides` sowie `lhme_controlled_stress` direkt
ausgeführt; beide bestanden.

## 1. Eine Million virtuelle Werte: Speicherform

Alle Varianten repräsentieren einen virtuellen Bereich von 1,000,000
Adressen. Die Formelbasis ist `h(x) = 100 * (2^x - 1)`; sie speichert die
Regel, nicht eine Million materialisierte Riesenwerte.

| Repräsentation | Prozess-Peak RAM | Aufbauzeit | Analytische Containergröße | Einordnung |
|---|---:|---:|---:|---|
| Dichte `UInt64`-Tabelle | 11,190,272 B | 2 ms | 8,000,000 B | kompakt für vollständig materialisierte 64-Bit-Werte |
| Sortierte Paare | 19,243,008 B | 3 ms | 16,000,000 B | Werte plus Adressmetadaten |
| `HashTable` | 43,556,864 B | 46 ms | 24,450,584 B | schneller Direktzugriff, aber hoher Container-Overhead |
| `FormulaOnly` | 3,379,200 B | 0 ms | 144 B | Rezeptur statt Tabelleninhalt |
| Formel + 0 % Overrides | 3,432,448 B | 0 ms | 496 B | leerer Ausnahme-Store |
| Formel + 1 % Overrides | 6,574,080 B | 18 ms | 496 B + 80,000 B Adressmetadaten | weiterhin unter dichter Tabelle |
| Formel + 10 % Overrides | 35,528,704 B | 167 ms | 496 B + 800,000 B Adressmetadaten | `BigInt`-Ausnahmen dominieren den tatsächlichen Peak |
| Formel + 100 % Overrides | 323,076,096 B | 1,714 ms | 496 B + 8,000,000 B Adressmetadaten | kein sparsamer Sonderfall mehr |

### Was die Zahlen bedeuten

- `FormulaOnly` liegt in diesem Prozessmesslauf rund **70 % unter** dem dichten
  Vektor und rund **92 % unter** der `HashTable`.
- Bei **1 % echten Abweichungen** liegt die Formel-plus-Override-Variante noch
  rund **41 % unter** dem dichten Vektor.
- Bei **10 % BigInt-Abweichungen** ist sie bereits rund **3.2×** so groß wie
  der dichte `UInt64`-Vektor. Das ist kein Fehler der Formelbasis, sondern die
  erwartete Kostenfolge einer Million-adressierbaren Hash-Struktur mit
  BigInt-Werten.
- Bei **100 % Overrides** enthält die Struktur faktisch wieder eine Tabelle –
  nur mit schwereren Werten und zusätzlicher Hash-/Allocator-Struktur. Dafür
  soll die `StoragePolicy` auf Dense- oder Hash-Speicher wechseln.

Die analytische Containergröße ist nur eine Untergrenze. Für reale
RAM-Entscheidungen ist hier der isoliert gemessene Prozess-Peak aussagekräftiger.

## 2. Persistenz der Ausnahme-Schicht

Der Hybrid-Benchmark speichert die Formelbasis einmal und serialisiert nur
echte Ausnahmen im derzeitigen ASCII-Snapshotformat.

| Override-Anteil | Overrides | Snapshot-Größe | Einfügezeit |
|---|---:|---:|---:|
| 0 % | 0 | 56 B | 0 ms |
| 0.1 % | 1,000 | 27,848 B | 1 ms |
| 1 % | 10,000 | 278,013 B | 15 ms |
| 10 % | 100,000 | 2,779,671 B | 154 ms |
| 100 % | 1,000,000 | 27,796,163 B | 1,535 ms |

Die Snapshot-Größe wächst hier annähernd linear mit der Zahl der Ausnahmen.
Sie beschreibt das aktuelle Textformat auf Platte, **nicht** den RAM-Verbrauch
und keine generelle Obergrenze für eine spätere Binärserialisierung.

## 3. KEVIN: Domainprojektion und Filter

Der Benchmark bearbeitet 64 Eltern-Tiles × 64 Unterzellen = 4,096 virtuelle
Zellen. Jeder Lauf erzeugte exakt 512 Überlebende und **0** Exact-Evaluierungen.

| Worker | Mittlere Zeit | Speed-up gegen 1 Worker | Parallele Effizienz |
|---:|---:|---:|---:|
| 1 | 12,968 µs | 1.00× | 100 % |
| 2 | 8,252 µs | 1.57× | 78.6 % |
| 4 | 4,664 µs | 2.78× | 69.5 % |
| 8 | 3,818 µs | 3.40× | 42.5 % |

Die abnehmende Effizienz ist bei nur 4,096 kleinen Aufgaben plausibel:
Erzeugung und Synchronisation asynchroner Aufgaben werden relativ teuer. Der
Test belegt daher korrekte parallele Reduktion und eine sinnvolle Beschleunigung
auf diesem Host, aber keine lineare Skalierungsgarantie. Für größere Tile-Pakete
oder einen persistenten Thread-Pool sollten weitere Messreihen folgen.

## 4. Streaming und hierarchische Reduktion

Der kleine Vergleich über 31 Adressen zeigte:

| Variante | Exact-Evaluierungen | Gefiltert | Zurückbehalten | Zeit |
|---|---:|---:|---:|---:|
| Referenz eager | 31 | 0 | 31 | 17 µs |
| Referenz lazy | 31 | 0 | 0 | 8 µs |
| Referenz mit modularer Vorfilterung | 4 | 27 | 4 | 12 µs |
| V2 Summary, 1 Worker | 31 | 0 | 0 | 219 µs |
| V2 Summary, 4 Worker | 31 | 0 | 0 | 252 µs |
| V2 Filter + adaptive Retention | 4 | 27 | 3 | 135 µs |

Das Ergebnis zeigt vor allem die **Arbeitsreduktion**: Der modulare Filter
verhindert 27 von 31 Exact-Auswertungen. Die Zeiten sind bei dieser winzigen
Aufgabe nicht als Throughput-Ranking zu lesen; der parallele Runtime-Overhead
ist größer als der Nutzen. Für eine echte Leistungsentscheidung braucht es
größere Bereiche und wiederholte Messungen.

## 5. Isolierter Millionen-Scan-Stresstest

| Szenario | Zeit | Prozess-Peak RAM | Geprüft | Exact | Gefiltert | Zurückbehalten |
|---|---:|---:|---:|---:|---:|---:|
| Millionen-Scan, 1 Worker | 663 ms | 3,497,984 B | 1,000,001 | 0 | 1,000,001 | 0 |
| Millionen-Scan, 8 Worker | 127 ms | 3,371,008 B | 1,000,001 | 0 | 1,000,001 | 0 |
| Adaptive Treffer | 76 ms | 3,715,072 B | 10,001 | 1,250 | 8,751 | 3 |

Der Acht-Worker-Scan war auf diesem Host rund **5.2×** schneller als der
Ein-Worker-Scan, bei vergleichbarem Prozess-Peak. Entscheidend ist der
Speichervertrag: Der Millionen-Scan filterte alle Kandidaten weg, führte keine
Exact-Evaluation aus und behielt keine Ergebnismenge im RAM.

## Review: Wann welche Repräsentation?

| Datenform und Zugriff | Empfohlene Strategie |
|---|---|
| Werte folgen stabil einer gemeinsamen Formel, kaum Ausnahmen | `FormulaOnly` |
| Formelbasis, wenige explizite Sonderwerte | `FormulaWithOverrides` |
| Hoher Ausnahmeanteil, überwiegend sequenzieller Zugriff | dichte Tabelle |
| Hoher Ausnahmeanteil, überwiegend zufälliger Zugriff | `HashTable` |
| Hierarchische Suche, Dimensionen oder Kandidatenfilter | KEVIN-Domain plus explizite Projektion; Werte weiterhin über `FormulaRecipe` |

## Nächste Benchmark-Schritte

1. Jede Hauptmessung mehrfach wiederholen und Median, Streuung sowie Hardware-
   Taktbedingungen protokollieren.
2. `BigInt`-Override-Store gegen eine kompaktere, sortierte Sparse-Repräsentation
   messen, damit die Umschaltschwelle datenbasiert statt nur policy-basiert wird.
3. Größere KEVIN-Tile-Pakete und einen persistenten Executor messen; der aktuelle
   Test misst bewusst auch Kosten kurzlebiger asynchroner Aufgaben.
4. Die Speicherentscheidung mit realen Nutzdaten testen, nicht nur mit
   synthetischen `UInt64`-Tabellen und kleinen Override-BigInts.
