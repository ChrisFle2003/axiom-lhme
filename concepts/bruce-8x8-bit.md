# 1. Executive Summary

`/home/chrisi/Desktop/Backup/Bruce_raw` existiert, ist aber vollständig leer: keine Quellen, keine Git-Metadaten, keine Build-Dateien, keine Tests, keine Dokumentation und keine Konfiguration.

Es gibt daher **keine nachweisbar implementierte BRUCE_raw-Funktion**. Dieser Bericht ist ein vollständiger **BLUEPRINT-VORSCHLAG** für einen C++-basierten Research-Core, nicht die Beschreibung eines vorhandenen Produkts.

Der Kernentwurf trennt strikt:

```text
Koordinate ≠ berechneter Wert ≠ Bedeutung ≠ Evidenz ≠ Freigabe
```

AXIOM darf Ergebnisse nur als verifiziert markieren, wenn exakte Ausführung, unabhängige Gegenprüfung, vollständige Versionsbezüge und Replay vorhanden sind.

# 2. Untersuchter Repository-Stand

Bereinigter Repository-Baum:

```text
Bruce_raw/
└── <leer>
```

| Pfad | Typ | Zweck | Status | Kritikalität | Beleg |
|---|---|---|---|---|---|
| `Bruce_raw/` | Verzeichnis | vorgesehene Projektwurzel | NICHT AUS DEM REPOSITORY ABLEITBAR | hoch | Verzeichnis enthält keine Einträge |
| `.git/` | Git-Metadaten | Versionskontrolle | nicht vorhanden | hoch | `git rev-parse` nicht erfolgreich |
| `CMakeLists.txt` | Build | C++-Build | nicht vorhanden | hoch | Datei fehlt |
| `src/`, `include/`, `tests/` | Quellcode | Implementierung/Test | nicht vorhanden | hoch | Verzeichnisse fehlen |
| README/Lizenz/CI | Dokumentation/Betrieb | Projektvertrag | nicht vorhanden | mittel | Dateien fehlen |

Ausgeführte Prüfung:

```text
Repository-Pfad geprüft: vorhanden
Datei-Inventarisierung: keine Dateien gefunden
Git-Worktree-Prüfung: nein
Build/Test: nicht möglich, da kein Build-System existiert
```

**Bewertung:** NICHT AUS DEM REPOSITORY ABLEITBAR.

# 3. AXIOM-Identität und Produktziel

AXIOM ist eine integrierte mathematische Forschungs- und Lösungsmaschine. Sichtbar wird sie als Research-Cockpit; intern verwendet sie mehrere austauschbare Strategien.

```text
Aufgabe
  ↓
Normalisierung und Aufgabenmodell
  ↓
Strategieauswahl
  ↓
exakte Ausführung
  ↓
Gegenprüfung / Hold-out / Replay
  ↓
Evidenzgebundenes Resultat
  ↓
Erklärung und Archiv
```

Wichtige Produktgrenze:

- Ein Sprachmodell darf Hypothesen strukturieren oder erklären.
- Ein Sprachmodell darf niemals `VERIFIED`, `PUBLISHABLE`, LearnGate, PublishGate oder Budgetentscheidungen setzen.

**Bewertung:** DOKUMENTIERT / KONZEPT aus der Aufgabenbeschreibung.

# 4. Implementiert vs. dokumentiert vs. geplant vs. offen

| Bereich | Status | Begründung |
|---|---|---|
| Quellcode | NICHT AUS DEM REPOSITORY ABLEITBAR | Repository leer |
| Tests | NICHT AUS DEM REPOSITORY ABLEITBAR | keine Tests vorhanden |
| Build-System | NICHT AUS DEM REPOSITORY ABLEITBAR | kein Manifest vorhanden |
| Single-Triangle-Kernel | DOKUMENTIERT / KONZEPT | Aufgabenbeschreibung |
| Z-Kompatibilitätsraum | DOKUMENTIERT / KONZEPT | Aufgabenbeschreibung |
| Token-/Relationsmodell | DOKUMENTIERT / KONZEPT | Aufgabenbeschreibung |
| Experience Store | DOKUMENTIERT / KONZEPT | Aufgabenbeschreibung |
| C++20/CMake/Ninja | BLUEPRINT-VORSCHLAG | sinnvoller, kleiner Start für BRUCE_raw |
| Persistenzformat | OFFENE ENTSCHEIDUNG | JSON/CBOR/SQLite/Event Store nicht festgelegt |
| Cockpit-Technologie | OFFENE ENTSCHEIDUNG | CLI zuerst, UI später empfohlen |
| Netzwerk/API | OFFENE ENTSCHEIDUNG | für MVP nicht erforderlich |

# 5. Architekturprinzipien und Invarianten

1. **Single Triangle:** Ein festes Koordinatenkernel, kein Baum aus geometrischen Kind-Dreiecken.

2. **Adresse ist nicht Wert:** Eine Adresse ist bijektiv und stabil; ein Rechenwert kann kollidieren.

3. **Fünf Identitäten bleiben getrennt:**

```text
Koordinate: domain, layer, x, y, z
Regel: formula_version, rule_version, policy_version
Token: token_id, token_table_version
Wert: value, type, precision, status
Evidenz: input_fingerprint, trace_id, evidence_id, experience_id
```

4. **Lazy statt Vollmaterialisierung:** Große Kandidatenräume werden nur auf Anfrage berechnet.

5. **Budgets sind Pflicht:** Zeit, Kandidaten, Tiefe, Cache, Trace, Speicher und Gegenprüfungen werden vor Ausführung begrenzt.

6. **Reproduzierbarkeit:** Gleiches Inputpaket und gleiche Versionen müssen identischen Ablauf und identisches Ergebnis erzeugen.

7. **Kandidat ist keine Lösung:** Ohne unabhängige Evidenz und Replay bleibt ein Ergebnis Kandidat.

8. **Kryptogrenze:** Filter-, Grid- und Replay-Technik sind keine Behauptung realer Private-Key-, Mining- oder BSGS-Beschleunigung.

**Bewertung:** DOKUMENTIERT / KONZEPT.

# 6. Gesamtarchitekturdiagramm

```text
Benutzer / Datei / CLI
        │
        ▼
InputNormalizerV1
        │
        ▼
TaskSpecV1 + DomainSpecV1
        │
        ▼
SingleTriangleKernelV1
(domain, layer, x, y) ↔ z
        │
        ├──────────► TokenTableV1 / RelationStoreV1
        │
        ▼
ZCompatibilityEngineV1
(Filtergruppen, Score, Ausschlüsse, Tie-Break)
        │
        ▼
StrategyExecutorV1
 ├─ ExactMathStrategy
 ├─ NanoCellStrategy
 ├─ GridPathStrategy
 └─ HypothesisStrategy
        │
        ▼
Countercheck + Hold-out + Replay
        │
        ▼
EvidenceStoreV1 / ExperienceStoreV1
        │
        ▼
ExplanationV1 / ResearchCockpit
```

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 7. Modulbaum

```text
Bruce_raw/
├── CMakeLists.txt
├── README.md
├── include/axiom/
│   ├── core/
│   ├── coordinate/
│   ├── compatibility/
│   ├── token/
│   ├── evidence/
│   ├── math/
│   ├── application/
│   └── cli/
├── src/
│   ├── core/
│   ├── coordinate/
│   ├── compatibility/
│   ├── token/
│   ├── evidence/
│   ├── math/
│   ├── application/
│   └── cli/
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── replay/
│   └── property/
├── apps/
│   └── bruce_cli.cpp
├── docs/
│   ├── adr/
│   ├── formats/
│   └── architecture/
└── examples/
```

Abhängigkeitsrichtung:

```text
cli → application → math / compatibility / token
math → core / coordinate / evidence
compatibility → core / coordinate / evidence
token → core / compatibility
evidence → core
core → keine AXIOM-Module
```

Verbotene Richtung:

```text
core → UI
evidence → Strategieentscheidung
Strategie → LearnGate
LLM → Verified / Publishable
Cache → dauerhafte Erfahrung
```

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 8. Laufzeitdiagramm von Eingabe bis Ausgabe

```text
1. Benutzer übergibt Aufgabe.
2. InputNormalizerV1 erzeugt NormalizedInputV1.
3. TaskSpecV1 ordnet Domain, Regeln und Budgets zu.
4. SingleTriangleKernelV1 berechnet x, y und z.
5. ZCompatibilityEngineV1 erzeugt Kandidaten und Scores.
6. StrategyExecutorV1 führt exakte Strategie aus.
7. CountercheckRunnerV1 prüft unabhängig.
8. ReplayRunnerV1 wiederholt deterministisch.
9. EvidenceGateV1 klassifiziert Status.
10. ExplanationBuilderV1 erzeugt Erklärung mit Trace.
```

Fehlerpfad:

```text
Ungültige Eingabe / Version / Budget
  ↓
StopReason
  ↓
INCOMPLETE oder REJECTED
  ↓
Trace bleibt erhalten
  ↓
kein LearnGate / kein PublishGate
```

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 9. Datenflussdiagramm

```text
RawInput
  ↓
NormalizedInput + InputFingerprint
  ↓
TaskSpec + VersionBundle
  ↓
CoordinateContext(domain, layer, x, y, z)
  ↓
CompatibilitySnapshot
  ↓
CandidateSelection
  ↓
StrategyResult
  ↓
CountercheckResult + ReplayResult
  ↓
EvidenceRecord
  ↓
ResultStatus + Explanation
```

Ein historischer Lauf muss mindestens referenzieren:

```text
domain_id
layer
x, y, z
formula_version
rule_version
policy_version
token_table_version
experience_version
input_fingerprint
trace_id
```

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 10. Zustandsmaschine für Kandidaten, Evidenz und Ergebnisse

```text
DRAFT
  ↓ validiert
CANDIDATE
  ├─ Eingabe-/Regelfehler ─────► REJECTED
  ├─ Budgetende ───────────────► INCOMPLETE
  ├─ Gegenbeispiel ────────────► REFUTED
  ↓
PENDING_REPLAY
  ├─ Replay-Abweichung ────────► INVALID_EVIDENCE
  ↓
VERIFIED
  ↓ unabhängige Review-Policy
ELIGIBLE_FOR_REVIEW
  ↓ PublishGate
PUBLISHABLE
```

| Status | Bedeutung | Lernen erlaubt? | Veröffentlichung erlaubt? |
|---|---|---:|---:|
| `DRAFT` | noch nicht validiert | nein | nein |
| `CANDIDATE` | berechnet, unbestätigt | nein | nein |
| `REJECTED` | Regel-/Inputfehler | nein | nein |
| `REFUTED` | Gegenbeispiel gefunden | negativ nutzbar | nein |
| `INCOMPLETE` | Budget/Abbruch | nein | nein |
| `PENDING_REPLAY` | Evidenz wartet auf Replay | nein | nein |
| `VERIFIED` | exakte Prüfung bestanden | nicht automatisch | nein |
| `ELIGIBLE_FOR_REVIEW` | Review möglich | Gate-abhängig | nein |
| `PUBLISHABLE` | extern freigegeben | n/a | ja |

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 11. Single-Triangle-/Koordinaten-Kernel

Für nichtnegative Koordinaten:

\[
z=\frac{(x+y)(x+y+1)}{2}+y
\]

Für lokales 8×8-Grid:

\[
z_{local}=8y+x
\]

\[
x=z_{local}\bmod8,\qquad y=\lfloor z_{local}/8\rfloor
\]

Vorgeschlagene Datei:

```text
include/axiom/coordinate/single_triangle_kernel.hpp
src/coordinate/single_triangle_kernel.cpp
tests/property/single_triangle_kernel_tests.cpp
```

| Aspekt | Festlegung |
|---|---|
| Zweck | reversible Koordinatenidentität |
| Eingabe | `domain_id`, `layer`, `x`, `y`, Formelversion |
| Ausgabe | `CoordinateContextV1` mit `z` |
| Datenbesitz | keiner; reine Berechnung |
| erlaubte Abhängigkeit | `core/version.hpp`, `core/ids.hpp` |
| verbotene Abhängigkeit | Token, UI, Cache, Experience Store |
| Fehlerfälle | negative Werte im falschen Modell, Überlauf, ungültige Version |
| Trace-Beitrag | Formel, Eingaben und Z-Ausgabe |
| Tests | Bijektivität, 8×8-Adressierung, Überlauf, Kollisionstrennung |

`x^y` darf nur `ComputedValueV1` sein, nie `CoordinateIdV1`.

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 12. Grid-, Layer- und Adressmodell

```text
Domain
└── Layer
    └── CoordinateContext
        ├── x
        ├── y
        ├── z
        ├── optional local_z
        ├── formula_version
        └── parameter_fingerprint
```

| Modell | Verantwortung | Speicher-/Budgetgrenze |
|---|---|---|
| `DomainIdV1` | fachliche Trennung | 64-Bit-ID oder String-Referenz |
| `LayerIdV1` | Verarbeitungsebene | begrenzter Wertebereich |
| `CoordinateContextV1` | vollständige Adressidentität | keine implizite Mutation |
| `LocalGridAddressV1` | 8×8-Hot-Path | 6 Bit lokal bzw. 64 Bit Container |
| `ComputedValueV1` | Ergebnis unabhängig von Adresse | expliziter Zahlentyp |

Layer dürfen nicht automatisch Vollgitter erzeugen. Jede Abfrage muss über ein `RunBudgetV1` laufen.

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 13. Z-Kompatibilitätsraum

`ZCompatibilityEngineV1` ist das deterministische Bewertungsmodul.

```text
z001 → [true, false, false] → score 1
z002 → [true, true,  true ] → score 3
z003 → [false,false, false] → score 0
```

\[
score(z)=\sum_i weight_i\cdot fulfilled_i
\]

Tie-Break-Reihenfolge:

```text
1. höherer Score
2. weniger harte Widersprüche
3. mehr unabhängige Evidenz
4. kleinere stabile Kandidaten-ID
5. sonst AmbiguitySetV1
```

| Komponente | Zweck | Eigentümer | Eingabe/Ausgabe | Status |
|---|---|---|---|---|
| `CompatibilityRuleV1` | einzelne prüfbare Regel | Rule Registry | Kontext → Treffer/Ausschluss | BLUEPRINT-VORSCHLAG |
| `CompatibilityFilterGroupV1` | bis zu 64 boolesche Kandidaten/Merkmale | Z-Engine | Regeln → Bitmaske | BLUEPRINT-VORSCHLAG |
| `CompatibilitySnapshotV1` | unveränderlicher Regelzustand | Experience Store | Version → Regeln | BLUEPRINT-VORSCHLAG |
| `CandidateScoreV1` | Score und Widerspruchszahl | Z-Engine | Filter → Score | BLUEPRINT-VORSCHLAG |
| `CandidateSelectionV1` | Gewinner oder Mehrdeutigkeit | Z-Engine | Scores → Auswahl | BLUEPRINT-VORSCHLAG |
| `SelectionTraceV1` | Erklärung der Auswahl | Evidence Store | Lauf → Trace | BLUEPRINT-VORSCHLAG |
| `NegativeEvidenceV1` | Gegenbeispiel/Einschränkung | Evidence Store | Evidenz → Ausschluss | BLUEPRINT-VORSCHLAG |
| `ExperienceCorrectionV1` | Verweis auf entwertete Erfahrung | Experience Store | alte/neue Referenz | BLUEPRINT-VORSCHLAG |

Wichtige Grenzen:

- Filtergruppen für positive Treffer, harte Ausschlüsse und negative Evidenz müssen getrennt bleiben.
- Eine neue Erfahrung erzeugt neue Regel-/Experience-Versionen.
- Historische \(z\)-Adressen werden nie geändert.
- Ein Auswahlresultat ohne Trace bleibt `CANDIDATE`.

# 14. Token-, Sprach- und Relationsmodell

Ziel ist begrenztes, regelbasiertes und erklärbares Sprachverständnis, nicht die Behauptung menschlichen Bewusstseins.

```text
Text
→ Normalisierung
→ Tokenisierung
→ Token-IDs
→ Merkmale und Relationen
→ Z-Kompatibilität
→ Rollen-Kandidaten
→ Auswahl oder Mehrdeutigkeit
→ Erklärung
```

| Komponente | Verantwortung | Eingabe | Ausgabe | Status |
|---|---|---|---|---|
| `InputNormalizerV1` | Unicode-/Formatnormalisierung | Rohtext | NormalizedInput | BLUEPRINT-VORSCHLAG |
| `TokenizerV1` | Segmentierung | NormalizedInput | Token-Sequenz | BLUEPRINT-VORSCHLAG |
| `TokenTableV1` | versionierte Tokenzuordnung | Textform | `TokenIdV1` | BLUEPRINT-VORSCHLAG |
| `TokenV1` | Tokenidentität | ID + Tabelle | Token-Datensatz | BLUEPRINT-VORSCHLAG |
| `LexicalFeatureV1` | Merkmale | Token | Bit-/Merkmalsgruppen | BLUEPRINT-VORSCHLAG |
| `RelationV1` | prüfbare Beziehung | Tokenpaare | Relation + Evidenz | BLUEPRINT-VORSCHLAG |
| `RoleAssignmentV1` | Subjekt/Objekt/Verb etc. | Kontext + Kandidaten | Rollenbelegung | BLUEPRINT-VORSCHLAG |
| `ContextWindowV1` | begrenzter Kontext | Token-Sequenz | Kontextreferenz | BLUEPRINT-VORSCHLAG |
| `AmbiguitySetV1` | sichtbare Mehrdeutigkeit | gleichrangige Kandidaten | Alternativen | BLUEPRINT-VORSCHLAG |
| `LanguageExplanationV1` | nachvollziehbare Ausgabe | Trace | Erklärung | BLUEPRINT-VORSCHLAG |
| `ExternalFeedbackV1` | unabhängige Rückmeldung | bestätigter Befund | Evidenzreferenz | BLUEPRINT-VORSCHLAG |

Ein 64-Bit-Filter repräsentiert exakt 64 Boolesche Eigenschaften oder Kandidaten. Allgemeines Unicode benötigt jedoch Token-Tabellen, IDs und mehrere Filtergruppen.

Messbares Ziel für eine begrenzte Domäne:

```text
AXIOM kann Token normalisieren,
Rollen mit festen Regeln zuordnen,
mehrere Interpretationen sichtbar halten,
begründete Relationen speichern,
Gegenbeispiele berücksichtigen
und jede Zuordnung replaybar erklären.
```

# 15. Lernmodell und Append-only Experience Store

```text
Ausführung
  ↓
EvidenceRecord
  ↓
unabhängige Bestätigung oder Gegenbeispiel
  ↓
neues Experience-Ereignis
  ↓
Review
  ↓
neue Rule-/Compatibility-Version
```

Vorgeschlagene Dateien:

```text
include/axiom/evidence/experience_store.hpp
include/axiom/evidence/experience_record.hpp
src/evidence/experience_store.cpp
tests/integration/experience_store_tests.cpp
```

| Feld | Zweck |
|---|---|
| `experience_id` | unveränderliche Ereignisidentität |
| `sequence` | stabile Ereignisreihenfolge |
| `parent_experience_id` | Korrektur-/Abstammungskette |
| `coordinate_context` | Domain, Layer, \(x,y,z\) |
| `version_bundle` | Formel, Regeln, Tokens, Policy |
| `evidence_refs` | unabhängige Nachweise |
| `result_status` | Kandidat, Erfolg, Fehlschlag, Gegenbeispiel |
| `supersedes` | referenziert alte Erfahrung, löscht sie nicht |
| `trace_id` | Replay-Einstieg |

Schutz gegen Selbstbestätigung:

- Gleiches Ergebnis bestätigt nicht seine eigene Regel.
- Wiederholung identischer Eingabe zählt nicht als unabhängige Evidenz.
- Positive Erfahrung benötigt Gegenprüfung oder externes Feedback.
- Gegenbeispiele bleiben persistent sichtbar.
- LearnGate bleibt getrennt von `VERIFIED`.

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 16. AXIOM-Math und exakte mathematische Forschung

```text
TaskSpec
→ HypothesisCandidate
→ ExactStrategy
→ unabhängige Vorwärtsprüfung
→ Gegenbeispielsuche
→ Hold-out
→ Replay
→ EvidenceGate
```

| Komponente | Zweck | Verbotene Verantwortung |
|---|---|---|
| `HypothesisGeneratorV1` | Kandidaten erzeugen | Wahrheit setzen |
| `ExactMathStrategyV1` | exakte Berechnung | PublishGate öffnen |
| `CounterexampleFinderV1` | Gegeninstanzen suchen | Gegenbeispiele unterdrücken |
| `HoldOutRunnerV1` | unabhängige Instanzen testen | Trainingsdaten wiederverwenden |
| `EvidenceGateV1` | Status klassifizieren | mathematische Berechnung ersetzen |

Regeln:

- Interpolation auf bekannten Punkten ist kein Beweis für neue Punkte.
- Ein numerischer Roundtrip ist keine allgemeine Formel.
- Exakte Integerbehauptungen benötigen exakte Ganzzahlarithmetik.
- Ein positiver Lauf ohne Gegenprüfung bleibt Kandidat.

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 17. Hypothesen-, Gegenprüfungs-, Hold-out- und Replay-Pipeline

```text
HypothesisCandidate
  ↓
PrimaryStrategyRun
  ↓
PrimaryEvidence
  ↓
IndependentCountercheck
  ├─ Gegenbeispiel → REFUTED
  └─ bestanden
       ↓
HoldOutRun
       ├─ fehlgeschlagen → REFUTED
       └─ bestanden
            ↓
ReplayRun
            ├─ abweichend → INVALID_EVIDENCE
            └─ identisch → VERIFIED
```

Replay muss prüfen:

```text
InputFingerprint
FormulaVersion
RuleVersion
TokenTableVersion
ExperienceVersion
RunBudget
TraceDigest
ResultDigest
```

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 18. Research-Cockpit und Benutzerworkflow

MVP als CLI:

```text
1. Benutzer gibt Aufgabe ein.
2. CLI zeigt Normalisierung und aktive Versionen.
3. CLI zeigt x, y, z und Kandidatenmasken.
4. CLI zeigt Strategieergebnis und Ausschlussgründe.
5. CLI zeigt Evidenzstatus.
6. CLI kann denselben Trace replayen.
```

Späteres Cockpit:

```text
Aufgabenansicht
├── Eingabe
├── Kandidaten
├── Z-Kompatibilität
├── mathematische Strategie
├── Gegenprüfung
├── Evidenz
├── Replay
└── Experience-Historie
```

Ein sichtbares Ergebnis muss mindestens enthalten:

```text
Status
Ergebnis
verwendete Regeln/Versionen
Ausschlussgründe
Trace-ID
Replay-Status
```

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 19. C++-Modul- und Verzeichnisarchitektur

Empfohlener Start-Stack:

```text
C++20
CMake
Ninja
CTest
keine externen Runtime-Abhängigkeiten im Phase-1-MVP
```

| Modul | Datenbesitz | Öffentliche Schnittstelle | Tests |
|---|---|---|---|
| `core` | IDs, Versionen, Budgets, Status | Werteobjekte | Unit |
| `coordinate` | keine Persistenz | Paarung/Umkehrung | Property |
| `compatibility` | Snapshots/Scoring | Auswahl-API | Unit/Replay |
| `token` | TokenTabellen | Tokenisierung/Relationen | Unit/Integration |
| `evidence` | Traces/Experiences | Append/Replay | Integration |
| `math` | Strategieausführung | `StrategyV1` | Unit/Hold-out |
| `application` | Run-Orchestrierung | `ResearchRunV1` | E2E |
| `cli` | keine Domänendaten | Argumente/Ausgabe | CLI-Test |

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 20. Öffentliche Schnittstellen und Datenformate

Vorgeschlagene Kern-API:

```cpp
struct RunBudgetV1 {
    std::size_t max_candidates;
    std::size_t max_trace_bytes;
    std::size_t max_cache_entries;
    std::size_t max_counterchecks;
    std::size_t max_depth;
};

struct CoordinateContextV1 {
    DomainIdV1 domain_id;
    LayerIdV1 layer;
    std::uint64_t x;
    std::uint64_t y;
    BigUnsignedV1 z;
    VersionBundleV1 versions;
};

class SingleTriangleKernelV1 {
public:
    static CoordinateContextV1 make(...);
    static std::pair<std::uint64_t, std::uint64_t> unpair(...);
};
```

| Schnittstelle | Bestehend? | Empfehlung |
|---|---:|---|
| C++-Bibliotheks-API | nein | zuerst umsetzen |
| CLI | nein | MVP-Schnittstelle |
| REST/GraphQL | nein | später entscheiden |
| Persistentes Datenformat | nein | kanonisch und versioniert definieren |
| Datenbank | nein | erst nach Append-only-MVP entscheiden |
| Plugin-API | nein | nicht Phase 1 |

Für den ersten Replay-Export wird ein kanonisches Textformat empfohlen, zum Beispiel JSON Lines mit festgelegter Feldreihenfolge. Ein Binärformat ist erst nach Größenmessung sinnvoll.

# 21. Konfiguration, Versionierung und Migrationen

```text
sichere Defaults
  ↓
versionierte Projektkonfiguration
  ↓
expliziter CLI-Override
```

| Bereich | Blueprint-Regel |
|---|---|
| `RunBudgetV1` | immer explizit, niemals unendlich |
| `VersionBundleV1` | Pflicht für jeden Run |
| Token-Tabelle | append-only und versioniert |
| Regelset | immutable Snapshot |
| Experience Store | append-only |
| Migration | nur für persistente Formate |
| Cache | flüchtig, nicht migrationsrelevant |

Offene Entscheidung: SQLite, append-only Dateien oder ein dedizierter Event Store. Empfehlung: Erst append-only JSONL/CBOR-Prototyp mit Replay bauen; Datenbank erst nach tatsächlicher Lastmessung.

# 22. Caching, Speicherbudget, CPU-Cache und Performance

| Bereich | Regel | StopReason bei Überschreitung |
|---|---|---|
| 64-Bit-Filter | ein Wort je Filtergruppe | `CandidateBudgetExceeded` |
| Kandidaten | begrenzte Anzahl | `CandidateBudgetExceeded` |
| Cache | LRU, begrenzte Einträge | Cache-Eviction, kein Fehler |
| Trace | harte Bytegrenze | `TraceBudgetExceeded` |
| Suche | Tiefen-/Arbeitslimit | `DepthBudgetExceeded` |
| Gegenprüfung | begrenzte Runs | `CountercheckBudgetExceeded` |
| Experience | Quoten/Segmentierung | `StorageBudgetExceeded` |

CPU-Cache-Vorteil entsteht vor allem bei kompakten 64-Bit-Masken, Bitoperationen und kleinen Werteobjekten. Er ersetzt nicht die notwendige Speicherung von Evidenz und Erfahrungen.

**Bewertung:** BLUEPRINT-VORSCHLAG.

# 23. Fehlerbehandlung, StopReasons und Gates

Vorgeschlagene `StopReasonV1`:

```text
InvalidInput
InvalidUnicode
UnknownTokenTableVersion
UnknownRuleVersion
CoordinateOverflow
AddressCollision
UnsupportedStrategy
CandidateBudgetExceeded
CacheBudgetExceeded
TraceBudgetExceeded
DepthBudgetExceeded
CountercheckBudgetExceeded
ReplayMismatch
EvidenceMissing
ExternalConfirmationMissing
CounterexampleFound
AmbiguousSelection
SecurityPolicyDenied
```

| Gate | Darf prüfen | Darf nicht tun |
|---|---|---|
| `RunGateV1` | Input, Versionen, Budgets | Wahrheit bestätigen |
| `EvidenceGateV1` | Trace, Replay, Gegenprüfung | lernen |
| `LearnGateV1` | unabhängige positive Erfahrung | Publishen |
| `PublishGateV1` | Evidenz und Erklärung | Regeln mutieren |

# 24. Sicherheits- und Missbrauchsgrenzen

| Risiko | Auswirkung | Schutz |
|---|---|---|
| unbestätigter Kandidat als Lösung | falsche mathematische Aussage | harte Status-Gates |
| selbstbestätigendes Lernen | Regelkorruption | externe Evidenz/Gegenprüfung |
| manipulierte Trace-Datei | falsches Replay | Fingerprint und Digest |
| Speicher-/Zeitverbrauch | Denial of Service | RunBudgetV1 |
| Versionsverwechslung | falsche Reproduktion | vollständiges VersionBundle |
| Unicode-/Tokenizer-Angriff | falsche Tokenisierung | Normalisierung und Größenlimits |
| Krypto-/Mining-Fehlinterpretation | Missbrauchsbehauptungen | klare Scope-Grenze |
| Secrets | Datenverlust | aktuell keine Dateien/Secrets gefunden |

Es gibt im leeren Repository keine sichtbaren Schlüssel oder Tokens. Das ist kein Sicherheitsnachweis für spätere Implementierung.

# 25. Teststrategie und konkrete Testmatrix

| Test | Erwartung |
|---|---|
| Bijektivität \(x,y\leftrightarrow z\) | Umkehrung liefert exakt \(x,y\) |
| 8×8-Adresse | `z_local=8y+x` bleibt eindeutig |
| Wert-/Adresskollision | \(2^6\) und \(4^3\) haben getrennte Adressen |
| `0^0`-Policy | `ValueOne` oder `ReservedStop`, nie implizit |
| Filterauswahl | gleiche Masken ergeben gleiche Auswahl |
| Tie-Break | gleiche Scores ergeben stabile dokumentierte Auswahl |
| Mehrdeutigkeit | vollständiger Gleichstand bleibt sichtbar |
| Token-Version | alter Trace nutzt alte Token-Tabelle |
| Regelversion-Replay | alte Regelversion bleibt reproduzierbar |
| Experience-Kette | Korrektur löscht alte Erfahrung nicht |
| manipulierte Evidenz | Digest/Referenzfehler ergibt `INVALID_EVIDENCE` |
| Selbstbestätigung | identischer Input öffnet kein LearnGate |
| Cache-Eviction | Cacheverlust verändert kein Ergebnis |
| Budgetüberschreitung | sauberer StopReason plus Trace |
| Gegenbeispiel | alte Relation wird referenziert entkräftet |
| Hold-out | bekannte Stützstellen reichen nicht |
| End-to-End | Input → Token → \(x,y,z\) → Filter → Strategie → Evidenz → Erklärung → Replay |

# 26. Observability: Trace, Audit, Replay und Explainability

Ein `TraceV1` soll mindestens enthalten:

```text
run_id
timestamp
input_fingerprint
domain/layer/x/y/z
version_bundle
rule_evaluations
filter_masks
rejection_masks
scores
tie_break_steps
strategy_results
counterchecks
replay_digest
stop_reason
```

Erklärungspfad:

```text
Ergebnis
← gewählter Kandidat
← Score und Tie-Break
← erfüllte Regeln
← ausgeschlossene Kandidaten
← verwendete Versionen
← Evidenzreferenzen
← Replay-Status
```

Ein Trace darf nicht bloß Logausgabe sein; er ist Teil des Evidenzvertrags.

# 27. Umsetzungsphasen mit Abnahmekriterien

| Phase | Neue Bereiche | Sichtbares Ergebnis | Abnahme |
|---|---|---|---|
| 1 | `core`, `coordinate`, `compatibility` | \(x,y,z\), Filter, Score, Replay | Bijektivität, Tie-Break, Budgets |
| 2 | `token`, Erklärung | Tokenisierung, Rollen, Mehrdeutigkeit | Token-Version-Replay, Explain-Trace |
| 3 | `evidence`, Store | append-only Experience-Historie | Korrekturen, externe Evidenz, LearnGate |
| 4 | `math`, Gegenprüfung | Hypothesen und Hold-out | Kandidat wird nicht unberechtigt verifiziert |
| 5 | `cli`, später Cockpit | vollständiger Nutzerworkflow | E2E-Eingabe bis Replay |

Phase 1 enthält ausdrücklich nicht:

```text
LLM-Entscheidungen
autonomes Lernen
Datenbankzwang
GPU-Optimierung
Netzwerkzugriff
Krypto-/Mining-Suche
```

# 28. Risiken, Annahmen und offene Entscheidungen

| Frage | Optionen | Empfehlung |
|---|---|---|
| Persistenz | JSONL, CBOR, SQLite, Event Store | JSONL/CBOR-Prototyp zuerst |
| große Z-Werte | BigInt-Bibliothek oder begrenzter Bereich | Grenzvertrag vor Implementierung |
| Scoregewichte | Boolesch, Integer, Rational | MVP mit Boolesch/Integer |
| externe Evidenz | manuell, signiert, Prüfdienst | manuell + unabhängiger Replay zuerst |
| Tokenisierung | einfache Regeln, Unicode-Segmentierung | deterministische Normalisierung |
| UI | CLI, Desktop, Web | CLI vor Cockpit |
| Parallelität | sequenziell, Threadpool | erst nach deterministischem MVP |
| LLM | keiner, Vorschlagsassistent | nur unverbindliche Hypothesen/Erklärung |

# 29. Priorisierte nächste drei Implementierungsschritte

1. **Projektgrundlage schaffen**  
   `CMakeLists.txt`, C++20-Compilerwarnungen, Test-Harness, `core/ids`, `core/version`, `core/budget`.

2. **SingleTriangleKernelV1 und ZCompatibilityEngineV1 als vertikalen Slice bauen**  
   Reversible Adresse, 64-Bit-Filtergruppen, deterministischer Score, Tie-Break, Trace und Replay.

3. **Append-only Experience-MVP hinzufügen**  
   Kanonische Ereignisdatei, VersionBundle, Korrekturreferenz, manipulierte Evidenz ablehnen und LearnGate geschlossen halten.

# 30. Glossar

| Begriff | Bedeutung |
|---|---|
| AXIOM | integrierte mathematische Forschungsmaschine |
| Single Triangle | feste Koordinatenformel, kein Dreiecksbaum |
| \(z\) | reversible Adresse aus \(x,y\) |
| Filtergruppe | 64 boolesche Kandidaten oder Merkmale |
| Kompatibilitätsregel | prüfbare Regel für Treffer/Ausschluss |
| Kandidat | noch unbestätigtes Ergebnis |
| Gegenbeispiel | Evidenz gegen Hypothese oder Relation |
| Replay | gleiche Ausführung mit identischen Versionen |
| Hold-out | unabhängige Prüfinstanz |
| Experience | append-only Lauf- und Evidenzereignis |
| LearnGate | Freigabe für regelveränderndes Lernen |
| PublishGate | Freigabe zur Ergebnisveröffentlichung |
| StopReason | maschinenlesbarer Abbruchgrund |

Keine Dateien wurden verändert. Die Planungsleitlinie hat hier die Trennung zwischen dem leeren Ist-Repository und allen vorgeschlagenen Komponenten bestimmt.