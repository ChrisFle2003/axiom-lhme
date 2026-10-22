# AXIOM NanoCell: Deterministische mehrdimensionale Erfahrungs- und Bedeutungsverarbeitung

## Dokumentstatus und Kennzeichnung

Dieses Konzept konsolidiert die bisher festgelegte NanoNeuron- und ExecutionCell-Architektur mit der neuen Idee virtueller Prozessdimensionen, deterministischer „Paralleluniversen“, erfahrungsbasierten Lernens und einer getrennten sprachlichen Rekonstruktion.

Die folgenden Kennzeichnungen werden verwendet:

- **Festgelegt:** unmittelbar aus den bisherigen Projektentscheidungen übernommen.
- **Abgeleitet:** logisch aus mehreren festgelegten Aussagen erschlossen.
- **Annahme:** für die Beschreibung notwendig, aber noch nicht bestätigt.
- **Vorschlag:** empfohlene technische Ausgestaltung.
- **Offen:** entscheidungsrelevanter Punkt ohne abschließende Festlegung.
- **Konflikt:** zwei Aussagen oder Anforderungen sind noch nicht vollständig miteinander vereinbar.

---

## 1. Kurzfassung

AXIOM NanoCell ist eine deterministische Rechen- und Wissensarchitektur, in der kompakte NanoNeuronen nicht sämtliche möglichen Bedeutungen, Muster und Reaktionen dauerhaft speichern. Stattdessen bildet jedes NanoNeuron einen kleinen kanonischen Ausgangszustand, aus dem bei Bedarf mehrere isolierte Berechnungsvarianten erzeugt werden.

Diese Berechnungsvarianten werden im Projekt metaphorisch als **Paralleluniversen** bezeichnet. Ein Paralleluniversum beschreibt keine physikalische Parallelwelt und keinen quantenmechanischen Zustand. Es ist ein deterministisch definierter Verarbeitungspfad, der denselben Ausgangszustand mit einer bestimmten Kombination aus Verschiebung, Rotation, Logikgatter, Maske, Kontext und Technik betrachtet.

Ein solches Universum wird nicht dauerhaft materialisiert. Es existiert nur als Berechnung:

```text
Ausgangszustand
→ Transformation
→ logische Prüfung
→ Musterbewertung
→ Ergebnisprojektion
→ Verwerfen des Zwischenraums
```

Jedes berechnete Universum liefert einen kleinen, typisierten **AnswerPoint**. Mehrere AnswerPoints bilden gemeinsam eine semantische Konstellation. Diese Konstellation kann anschließend von einem themenorientierten Small Language Model, einem regelbasierten Renderer oder einem anderen Decoder in Text, Binärcode, eine Formel oder eine Steuerungsaktion übersetzt werden.

Das Sprachmodell soll nicht die eigentliche Antwort erfinden. AXIOM berechnet die zugrunde liegenden Aussagen, Relationen, Negationen und Nachweise. Das Sprachmodell übernimmt ausschließlich die sprachliche Darstellung. Seine Ausgabe wird anschließend gegen die ursprünglichen AnswerPoints zurückgeprüft.

Zusätzlich soll AXIOM aus angewandten Techniken lernen. Eine Technik besteht aus einer geordneten Kombination von Operationen und Entscheidungsregeln. Wird eine Technik auf eine erkannte Situation angewandt, entsteht eine Erfahrung:

```text
Situation
+ angewandte Technik
+ beobachtetes Ergebnis
+ Bewertung
+ Nachweis
= Erfahrung
```

Die vollständigen Erfahrungen werden persistent auf SSD gespeichert. Häufige, erfolgreiche und für das aktuelle Thema relevante Techniken können in verdichteter Form in RAM oder Cache geladen werden. AXIOM lernt dadurch nicht primär durch die unkontrollierte Veränderung eines großen Gewichtsmodells, sondern durch nachvollziehbares Speichern, Vergleichen, Bewerten, Verdichten und Wiederverwenden von Erfahrungen.

Die Lemmings-Metapher beschreibt lediglich das Grundprinzip: Ein laufender Prozess trifft auf eine Situation, eine passende Fähigkeit wird ausgewählt, die Wirkung wird beobachtet und die Erfahrung wird gespeichert. Im Spiel greifen Aktionen gezielt in einen sonst automatisch ablaufenden Weg ein. Dieses Prinzip wird auf AXIOM verallgemeinert und erhält keine spielbezogene Wissensorientierung. citeturn430988view9turn906457view2

---

## 2. Ausgangslage und Problemstellung

### 2.1 Technische Ausgangslage

Gewöhnliche neuronale Modelle speichern einen großen Teil ihrer Fähigkeiten implizit in zahlreichen Parametern. Einzelne Entscheidungen lassen sich deshalb häufig nicht unmittelbar auf eine kleine, eindeutig lesbare Regel oder Erfahrung zurückführen.

AXIOM verfolgt einen anderen Ansatz:

- Wissen soll möglichst explizit referenzierbar sein.
- Berechnungen sollen reproduzierbar sein.
- Entscheidungen sollen einen nachvollziehbaren Trace besitzen.
- aktive Speicherbereiche sollen klein bleiben.
- Zwischenräume sollen nur bei Bedarf berechnet werden.
- erfolgreiche Techniken sollen als Erfahrung gespeichert werden.
- Sprachgenerierung soll von der Wahrheits- und Entscheidungslogik getrennt werden.

Bitweise Operatoren eignen sich als elementare Grundlage, weil sie direkt auf einzelne Bits, Bitfolgen und Bitvektoren wirken. Logische Verschiebungen bewegen die Bits eines Operanden um eine festgelegte Positionsanzahl und füllen frei werdende Positionen typischerweise mit Nullen. citeturn430988view0turn430988view1

Logikgatter bilden binäre Eingangswerte durch eine boolesche Funktion auf ein binäres Ergebnis ab. Logische Verknüpfungen sind dadurch eindeutig durch ihre Eingaben beziehungsweise Wahrheitstabellen bestimmt. Diese Eigenschaft passt zur angestrebten deterministischen Verarbeitung. citeturn906457view0turn430988view5

Mustererkennung bezeichnet allgemein das Erkennen von Regelmäßigkeiten, Wiederholungen, Ähnlichkeiten oder Gesetzmäßigkeiten in Daten. AXIOM soll diese Aufgabe nicht ausschließlich durch ein trainiertes statistisches Gesamtmodell lösen, sondern zusätzlich durch explizite Masken, Gatterfolgen, Formen, Kontexte und gespeicherte Erfahrungen. citeturn430988view3

### 2.2 Speicherproblem

Die bisherige Diskussion enthält zwei unterschiedliche Speicherangaben:

```text
1 Byte aktiver Speicher je NanoNeuron
```

und:

```text
1 NanoNeuron = 8 Byte kanonische Größe
```

Diese Aussagen können miteinander vereinbart werden, wenn zwischen dem vollständigen Datensatz und seiner aktiven Projektion unterschieden wird.

**Empfohlene Auflösung:**

```text
8 Byte = kanonischer NanoNeuron-Datensatz
1 Byte  = aktive Zustandsprojektion im Hot Path
```

Damit gelten weiterhin:

```text
1 NanoNeuron    = 8 Byte kanonischer Speicher
10 NanoNeuronen = 80 Byte
100 NanoNeuronen = 800 Byte
```

Während einer konkreten Verarbeitung kann jedoch nur das relevante Zustandsbyte oder ein anderer kompakter Ausschnitt in einem zusammenhängenden Arbeitsbereich liegen.

Ein 64-Bit-Datenwort ist acht Byte breit. Eine 64-Bit-Architektur arbeitet mit entsprechend breiten Registern oder Datenoperationen, auch wenn reale Adressräume nicht zwingend alle 64 möglichen Adressbits verwenden. Für AXIOM ergibt sich daraus die Möglichkeit, einen vollständigen 8-Byte-Datensatz oder acht aktive Zustandsbytes in einer 64-Bit-Einheit zu betrachten. citeturn861276view0

### 2.3 Problem exponentieller Prozessräume

Werden mehrere Achsen aus Bitpositionen, Transformationen, Gattern, Masken und Kontexten kombiniert, wächst die Anzahl möglicher Kombinationen stark.

Beispiel:

\[
8 \times 8 = 64
\]

\[
8 \times 8 \times 8 = 512
\]

\[
512 \times 512 = 262\,144
\]

Das Konzept darf daraus nicht ableiten, dass alle möglichen Kombinationen gleichzeitig als vollständiger Tensor im RAM oder VRAM angelegt werden müssen.

Die zentrale Herausforderung lautet daher:

> Wie kann AXIOM sehr große virtuelle Such- und Beziehungsräume nutzen, ohne deren vollständige Zustände dauerhaft zu speichern?

Die vorgesehene Antwort ist eine lazy beziehungsweise bedarfsgesteuerte Ausführung:

```text
Koordinate oder Kandidat bestimmen
→ benötigte Daten laden
→ Ergebnis berechnen
→ kompakten Punkt erzeugen
→ Zwischenzustand verwerfen
```

### 2.4 Problem des Lernens

Das reine Speichern einer fertigen Antwort reicht nicht aus, um eine wiederverwendbare Fähigkeit zu erzeugen.

AXIOM muss nachvollziehen können:

- Welche Situation wurde erkannt?
- Welche Technik wurde ausgewählt?
- Welche Alternativen wurden ausgeschlossen?
- Welche Operationen wurden ausgeführt?
- Welches Ergebnis entstand?
- War das Ergebnis erfolgreich?
- Welche Kosten und Risiken entstanden?
- Unter welchen Bedingungen gilt die Erfahrung?
- Welche Gegenbeispiele sind bekannt?
- Welcher Trace belegt die Aussage?

Das Lernproblem besteht somit nicht nur im Wiederfinden einer Antwort, sondern im Aufbau eines nachvollziehbaren Zusammenhangs zwischen Situation, Technik, Ergebnis und Gültigkeitsbereich.

---

## 3. Zielsetzung

### 3.1 Hauptziel

**Festgelegt:**

Entwicklung einer kompakten, deterministischen CPU-RAM-NPU-Architektur, in der NanoNeuronen durch virtuelle mehrdimensionale Prozessräume unterschiedliche Verarbeitungstechniken anwenden, daraus typisierte Ergebnisse erzeugen und erfolgreiche beziehungsweise lehrreiche Erfahrungen persistent speichern können.

### 3.2 Teilziele

1. Beibehaltung des kanonischen 8-Byte-NanoNeuron-Vertrags.
2. Nutzung einer kompakten aktiven Projektion von vorzugsweise einem Byte je verarbeitetem NanoNeuron.
3. Erzeugung mehrerer deterministischer Aufgabenuniversen aus demselben Ausgangszustand.
4. Unterstützung von Shift-, Rotations-, Gate-, Masken- und Kontextoperationen.
5. Vermeidung einer vollständigen Materialisierung großer Dimensionsräume.
6. Reduktion jedes Berechnungsuniversums auf einen kleinen AnswerPoint.
7. Zusammensetzung mehrerer AnswerPoints zu einer semantischen Antwortstruktur.
8. Trennung von Bedeutungsberechnung und sprachlicher Darstellung.
9. Speicherung vollständiger Erfahrungen auf persistentem Speicher.
10. Verdichtung bestätigter Erfahrungen zu wiederverwendbaren Techniken.
11. Aufbau eines themenbezogenen Hot-Skill-Caches in RAM.
12. Speicherung und Berücksichtigung erfolgreicher sowie gescheiterter Anwendungen.
13. Reproduzierbare Bewertung und Rangfolge von Techniken.
14. Integration in die vorhandene ExecutionCell-, Trace- und AXIOM-Architektur.
15. Vorbereitung einer späteren CPU-, SIMD-, GPU- oder spezialisierten NPU-Ausführung.

### 3.3 Nicht-Ziele

**Festgelegt beziehungsweise abgeleitet:**

- keine physikalische Quantenteleportation,
- keine echte quantenmechanische Superposition,
- keine Behauptung, klassische Berechnungszweige seien physikalische Paralleluniversen,
- keine vollständige Ablösung klassischer CPU- oder Speicherarchitekturen im ersten Prototyp,
- kein unkontrolliertes selbstmodifizierendes System,
- keine ungeprüfte Übernahme generierter Texte als neues Wissen,
- keine Speicherung aller möglichen Dimensionspunkte,
- kein verpflichtendes Deep Learning im ersten MVP,
- keine zwingende GPU-Abhängigkeit,
- keine spielbezogene Lemmings-Wissensbasis,
- keine absolute Sicherheits- oder Fehlerfreiheitsgarantie,
- keine permanente Speicherung jedes flüchtigen Zwischenwerts.

### 3.4 Erfolgskriterien

Der erste Konzeptnachweis gilt als erfolgreich, wenn:

1. ein 8-Byte-NanoNeuron-Datensatz definiert und reproduzierbar geladen werden kann;
2. daraus eine aktive 1-Byte-Projektion erzeugt werden kann;
3. mindestens Shift-, Rotate-, AND-, OR-, XOR- und NOT-basierte Universen deterministisch ausführbar sind;
4. alle Ausführungen bei identischen Eingaben dieselben Ergebnisse und Traces erzeugen;
5. ein Dimensionsraum iterativ ausgewertet wird, ohne vollständig materialisiert zu werden;
6. jedes Universum einen typisierten AnswerPoint erzeugt oder kontrolliert abgelehnt wird;
7. mehrere AnswerPoints zu einer strukturierten Antwort zusammengeführt werden;
8. eine angewandte Technik als ExperienceRecord gespeichert und wieder geladen werden kann;
9. Erfolg und Fehlschlag getrennt gespeichert werden;
10. eine ähnliche Situation eine passende frühere Technik finden kann;
11. der Hot-Skill-Cache aus persistenten Erfahrungen reproduzierbar aufgebaut wird;
12. ein sprachlicher Renderer keine unbelegten Aussagen hinzufügen darf;
13. beschädigte, widersprüchliche oder unvollständige Erfahrungen nicht als bestätigtes Wissen aktiviert werden.

---

## 4. Leitprinzipien

### 4.1 Kanonische Daten, flüchtige Perspektiven

Ein NanoNeuron besitzt einen kanonischen Datensatz. Shift, Rotation, Masken und Kontext erzeugen daraus keine permanenten Kopien, sondern kurzlebige berechnete Perspektiven.

### 4.2 Berechnen statt materialisieren

Große Prozessräume werden als Funktionen, Iteratoren und Koordinaten beschrieben. Nur tatsächlich benötigte Punkte werden berechnet.

### 4.3 Erfahrung statt bloßer Antwortspeicherung

Gespeichert wird nicht ausschließlich, was geantwortet wurde, sondern warum eine Technik ausgewählt wurde, wie sie ausgeführt wurde und ob sie funktionierte.

### 4.4 Deterministische Verarbeitung

Eine identische Eingabe unter derselben Version von Daten, Regeln und Technikdefinitionen muss dasselbe Ergebnis erzeugen.

### 4.5 Keine Behauptung ohne Trace

Jeder bestätigte AnswerPoint, jede verdichtete Erfahrung und jede ausgegebene Aussage benötigt eine Referenz auf ihren Ursprung.

### 4.6 Trennung von Denken und Formulieren

Die Berechnung der Aussage erfolgt durch AXIOM. Ein SLM oder Renderer darf die Aussage sprachlich darstellen, aber ihren Inhalt nicht eigenmächtig erweitern.

### 4.7 Lernen ohne unkontrollierte Mutation

Neue Erfahrungen verändern nicht unmittelbar bestehendes kanonisches Wissen. Sie werden zunächst als Ereignisse gespeichert, bewertet, verifiziert und gegebenenfalls später konsolidiert.

### 4.8 Fehler sind Lernmaterial

Gescheiterte Techniken und Gegenbeispiele werden nicht gelöscht, sondern als Grenzen des Gültigkeitsbereichs erfasst.

### 4.9 Themenbezogene Aktivierung

Nicht global häufige, sondern für das aktuelle Thema geeignete Techniken sollen bevorzugt in den aktiven Speicher geladen werden.

### 4.10 Lokale Fehlerbegrenzung

Ein fehlerhaftes Universum, eine ungültige Technik oder ein beschädigter ExperienceRecord darf nicht die gesamte ExecutionCell oder Wissensbasis korrumpieren.

### 4.11 Explizite Versionierung

NanoNeuron-Layout, Technikdefinition, Bewertungsfunktion, Decoder und Erfahrungsformat müssen versioniert werden.

### 4.12 Physikalische Begriffe nur als Metapher

„Paralleluniversum“, „Kollaps“ und „Teleportation“ sind anschauliche Projektnamen. Die technische Spezifikation verwendet dafür klassische und überprüfbare Rechenmodelle.

---

## 5. Begriffe und Definitionen

### NanoNeuron

**Bedeutung:**  
Kleinste kompakte, adressierbare Verarbeitungseinheit der CPU-RAM-NPU.

**Aufgabe im Konzept:**  
Speichert kanonischen Zustand, Funktionsreferenzen, Formmerkmale, Routing- oder Steuerinformationen.

**Abgrenzung:**  
Kein biologisches Neuron und kein gewöhnliches künstliches Neuron mit frei trainierten Fließkommagewichten.

---

### ExecutionCell

**Bedeutung:**  
Zusammenhängender Block von NanoNeuronen.

**Aufgabe im Konzept:**  
Lokale Ausführung, kompakte Indizierung, begrenzter Arbeitsraum und deterministische Ablaufsteuerung.

**Abgrenzung:**  
Keine einzelne NanoNeuron-Zelle, sondern die übergeordnete ausführbare Einheit.

---

### Aktive Projektion

**Bedeutung:**  
Kompakter Ausschnitt eines kanonischen NanoNeurons, der für eine aktuelle Berechnung in RAM, Cache oder Register geladen wird.

**Aufgabe im Konzept:**  
Reduzierung des aktiven Speicherbedarfs.

**Abgrenzung:**  
Nicht mit dem vollständigen persistenten NanoNeuron-Datensatz identisch.

---

### Prozessdimension

**Bedeutung:**  
Projektspezifische Achse, über die alternative Berechnungen oder Relationen erzeugt werden.

**Aufgabe im Konzept:**  
Systematische Organisation von Bitpositionen, Transformationen, Gattern, Masken oder Kontexten.

**Abgrenzung:**  
Der Begriff ist nicht automatisch mit mathematischer Raumdimension gleichzusetzen. In der Mathematik kann Dimension etwa die Anzahl unabhängiger Freiheitsgrade bezeichnen; bei Arrays wird häufig die Zahl der Indizes als Dimension bezeichnet. AXIOM muss daher seine eigene Prozessdimension ausdrücklich definieren. citeturn430988view2

---

### Paralleluniversum

**Bedeutung:**  
Metaphorische Bezeichnung für einen isolierten, deterministisch parametrisierten Berechnungszweig.

**Aufgabe im Konzept:**  
Bewertung desselben Ausgangszustands unter einer bestimmten Kombination von Operationen und Kontexten.

**Abgrenzung:**  
Keine physikalische Parallelwelt und kein Quantenzustand.

---

### UniverseSpec

**Bedeutung:**  
Maschinenlesbare Beschreibung eines Paralleluniversums.

**Mögliche Inhalte:**

- Transformationsart,
- Transformationsparameter,
- Gate-ID,
- Masken-ID,
- Kontext-ID,
- Technik-ID,
- Abbruchgrenzen,
- Bewertungsregel,
- Versionsreferenzen.

---

### Technik

**Bedeutung:**  
Geordnete Kombination aus Universen, Operatoren, Prüfungen, Entscheidungsregeln und Abbruchbedingungen.

**Aufgabe im Konzept:**  
Löst einen wiederkehrenden Situationstyp.

**Abgrenzung:**  
Ein einzelnes Gate ist ein Werkzeug. Eine Technik ist ein vollständiger anwendbarer Ablauf.

---

### Situation

**Bedeutung:**  
Normalisierte Beschreibung eines aktuell erkannten Problems oder Zustands.

**Mögliche Bestandteile:**

- Thema,
- beteiligte Konzepte,
- erkannte Form,
- Kontext,
- Einschränkungen,
- Ressourcen,
- Ziel,
- Unsicherheiten.

---

### ExperienceRecord

**Bedeutung:**  
Unveränderlicher Datensatz einer konkreten Technikanwendung.

**Aufgabe im Konzept:**  
Dokumentiert Situation, Technik, Ergebnis, Bewertung und Nachweise.

**Abgrenzung:**  
Keine bereits verallgemeinerte Regel.

---

### ExperienceCell

**Bedeutung:**  
Verdichtete, versionierte Zusammenfassung mehrerer vergleichbarer ExperienceRecords.

**Aufgabe im Konzept:**  
Schnelles Wiederverwenden bestätigter Erfahrungen.

**Abgrenzung:**  
Die vollständigen Originalerfahrungen bleiben erhalten.

---

### AnswerPoint

**Bedeutung:**  
Kompaktes Ergebnis eines Universums oder einer geprüften Teilberechnung.

**Mögliche Inhalte:**

- Konzept-ID,
- Relations-ID,
- Wert,
- Polarität,
- Themen-ID,
- Evidenzklasse,
- Universe-ID,
- Trace-Referenz.

---

### Semantische Konstellation

**Bedeutung:**  
Geordnete Menge aus 16 bis N AnswerPoints, die gemeinsam eine Aussage, Lösung oder Aktion beschreiben.

**Abgrenzung:**  
Kein zwingend kontinuierlicher Floating-Point-Embedding-Vektor.

---

### SLM-Renderer

**Bedeutung:**  
Themenorientiertes kleines Sprachmodell oder alternativer Decoder.

**Aufgabe im Konzept:**  
Übersetzt eine semantische Konstellation in Wort und Schrift.

**Abgrenzung:**  
Darf nicht eigenständig den Wahrheitsgehalt festlegen.

---

### Semantic State Reconstruction

**Bedeutung:**  
Rekonstruktion einer Ausgabe aus kompakten, nachweisbaren Bedeutungspunkten.

**Abgrenzung:**  
Nur metaphorisch mit Quantenteleportation vergleichbar. Reale Quantenteleportation überträgt einen quantenmechanischen Zustand unter Nutzung eines Quanten- und eines klassischen Kanals; diese physikalischen Voraussetzungen existieren im AXIOM-Konzept nicht. citeturn430988view10turn906457view3

---

### Hot Skill

**Bedeutung:**  
Verdichtete, aktuell relevante Technik im schnellen Arbeitsspeicher.

**Aufgabe im Konzept:**  
Reduziert Such- und Ladeaufwand.

**Abgrenzung:**  
Nicht automatisch globales oder dauerhaft bestätigtes Wissen.

---

## 6. Gesamtarchitektur

```text
Benutzereingabe / Sensorwert / Rechenauftrag
                     ↓
        Eingabeprüfung und Normalisierung
                     ↓
      Themen-, Konzept- und Situationserkennung
                     ↓
        SituationSignature erzeugen
                     ↓
   Erfahrungsindex und Technikregister abfragen
                     ↓
          Kandidatentechniken bestimmen
                     ↓
       ExecutionCell und NanoNeuronen laden
                     ↓
         aktive 1-Byte-Projektionen bilden
                     ↓
       Paralleluniversen bedarfsgesteuert öffnen
       ├─ Shift
       ├─ Rotation
       ├─ Gate
       ├─ Maske
       ├─ Kontext
       └─ zusammengesetzte Technik
                     ↓
       Cascade- und Montessori-Filterung
                     ↓
         AnswerPoints erzeugen oder ablehnen
                     ↓
       semantische Konstellation aufbauen
                     ↓
     Ergebnis bewerten und technisch validieren
                     ↓
    ┌────────────────┴─────────────────┐
    ↓                                  ↓
strukturierte Aktion             SLM-/Text-Renderer
    ↓                                  ↓
Ausführungskontrolle          semantische Rückprüfung
    └────────────────┬─────────────────┘
                     ↓
               Ausgabe erzeugen
                     ↓
          ExperienceRecord speichern
                     ↓
      Rankings und ExperienceCells aktualisieren
                     ↓
          Hot-Skill-Cache neu bewerten
```

### 6.1 Steuerungsebene

Die Steuerungsebene entscheidet:

- welche Situation vorliegt,
- welche Techniken grundsätzlich zulässig sind,
- welche Daten geladen werden,
- welche Universen ausgeführt werden,
- wann abgebrochen wird,
- welche Ergebnisse bestätigt oder verworfen werden.

### 6.2 Verarbeitungsebene

Die Verarbeitungsebene führt bitweise Transformationen, Masken, Gatterfolgen, Vergleiche und Musterprüfungen aus.

### 6.3 Erfahrungsebene

Die Erfahrungsebene speichert konkrete Anwendungen und verdichtet sie später zu wiederverwendbaren Fähigkeiten.

### 6.4 Darstellungsebene

Die Darstellungsebene übersetzt bereits geprüfte AnswerPoints in eine für Menschen oder externe Systeme geeignete Ausgabe.

---

## 7. Komponenten

## 7.1 Input- und Normalisierungsmodul

**Zweck:**  
Überführt rohe Eingaben in ein reproduzierbares internes Format.

**Eingaben:**

- Text,
- strukturierte Daten,
- Formeln,
- binäre Zustände,
- Sensordaten,
- interne Systemereignisse.

**Ausgaben:**

- normalisierte Eingabe,
- Themenkandidaten,
- erkannte Entitäten,
- Gültigkeitsstatus,
- Eingabe-Trace.

**Fehlerfälle:**

- ungültiges Format,
- nicht unterstützter Datentyp,
- unvollständige Eingabe,
- mehrdeutige Interpretation.

**Sicherheitsgrenze:**  
Rohe Eingaben dürfen keine direkten Technik- oder Speicheroperationen ungeprüft auslösen.

---

## 7.2 Situation Analyzer

**Zweck:**  
Erzeugt eine normalisierte SituationSignature.

**Verarbeitung:**

```text
Eingabe
→ Thema bestimmen
→ relevante Konzepte bestimmen
→ beobachtete Form und Relationen bestimmen
→ Ziel und Einschränkungen erfassen
→ Unsicherheiten markieren
```

**Ausgabe:**  
SituationSignature mit stabiler Hash- oder ID-Referenz.

**Offen:**  
Wie kontinuierliche oder sprachlich mehrdeutige Situationen in diskrete Signaturen überführt werden.

---

## 7.3 NanoNeuron Store

**Zweck:**  
Persistente Speicherung kanonischer NanoNeuronen.

**Anforderungen:**

- feste oder versionierte Datensatzgröße,
- reproduzierbare Reihenfolge,
- lokale und globale Identifikation,
- Integritätsprüfung,
- keine stille Layoutänderung.

**Konflikt:**  
Die logisch benötigten Felder könnten in Summe größer als acht Byte werden. Ein exaktes Packing muss deshalb experimentell und formal validiert werden.

**Vorschlag:**  
Der 8-Byte-Kern enthält nur hot-path-relevante Felder. Seltene oder variable Metadaten werden über Seitentabellen referenziert.

---

## 7.4 Active Projection Loader

**Zweck:**  
Erzeugt aus vollständigen NanoNeuronen einen kompakten aktiven Arbeitsbereich.

**Ausgabevarianten:**

```text
1 Byte je NanoNeuron
8 NanoNeuronen je 64-Bit-Wort
64 Aktivierungsbits je 64-Bit-Wort
```

**Annahme:**  
Nicht jede Technik benötigt dieselbe Projektion. Projektionstypen müssen daher versioniert und in der Technikdefinition angegeben werden.

---

## 7.5 Universe Generator

**Zweck:**  
Erzeugt deterministische Berechnungspläne aus einer UniverseSpec.

**Beispiel:**

```text
Universe-ID: 0042
Input Projection: STATE_BYTE
Transform: ROTATE_LEFT
Amount: 3
Gate: XOR
Mask: SHAPE_17
Context: VEHICLE_POWER
Output Projector: ANSWER_POINT_V1
```

**Wichtig:**  
Der Generator erzeugt vorzugsweise keinen vollständigen Dimensionsspeicher, sondern einen Iterator oder ausführbaren Plan.

---

## 7.6 Shift Engine

**Zweck:**  
Führt logische Verschiebungen aus.

**Semantik:**

\[
U_{\text{LSL},s}(b)=(b \ll s)\bmod 2^8
\]

\[
U_{\text{LSR},s}(b)=b \gg s
\]

für:

\[
s\in\{0,\dots,7\}
\]

**Eigenschaft:**  
Bits können den Wertebereich verlassen und gehen dabei verloren.

**Einsatz:**

- richtungsabhängige Filter,
- Entfernung niedriger oder hoher Bitbereiche,
- Gewichtung bestimmter Positionen,
- Erzeugung absichtlich verlustbehafteter Perspektiven.

---

## 7.7 Rotation Engine

**Zweck:**  
Verschiebt Bits zyklisch, ohne sie aus dem Byte zu entfernen.

**Semantik:**

\[
\operatorname{ROTL}_8(b,s)
=
((b \ll s)\lor(b \gg(8-s)))\bmod 2^8
\]

**Einsatz:**

- perspektivische Neuordnung,
- rotationsinvariante Musterprüfung,
- Suche nach zyklischen Formen,
- alternative Positionierung desselben Informationsbestands.

**Abgrenzung:**  
Rotation und Shift müssen unterschiedliche Universe-IDs und Traces besitzen.

---

## 7.8 Gate Engine

**Zweck:**  
Führt boolesche Verknüpfungen aus.

**Mindestoperatoren:**

- AND,
- OR,
- XOR,
- NOT,
- NAND,
- NOR,
- XNOR.

Ein Gate verarbeitet binäre Eingänge zu einem binären Ergebnis. Mehrere Gates können als virtuelle Logikbausteine zu einer komplexeren Funktion zusammengesetzt werden. Der Begriff „Logikbaustein“ kann neben realer Hardware auch eine virtuelle Beschreibung digitaler Schaltfunktionen bezeichnen. citeturn906457view0turn430988view7

**Begriffliche Abgrenzung:**  
„Logikfamilie“ bezeichnet in der Digitaltechnik normalerweise kompatible Bausteine mit gemeinsamer Fertigungs- und Schaltungstechnik. Für AXIOM sollte deshalb besser von **Gate Profile**, **Operator Family** oder **Technique Family** gesprochen werden. citeturn430988view6

---

## 7.9 Mask Engine

**Zweck:**  
Begrenzt die Prüfung auf relevante Bitbereiche oder Merkmale.

**Grundprüfung:**

\[
(b \land m)=v
\]

mit:

- \(b\): Eingabewert,
- \(m\): Maske,
- \(v\): erwarteter Shape-Wert.

**Montessori-Analogie:**

- Maske = Öffnung,
- Shape-Wert = passende Form,
- Eingabewert = einzusetzender Körper,
- Match = Form passt,
- Miss = Form passt nicht.

**Grenze:**  
Ein Maskentreffer allein beweist noch keine semantische Bedeutung. Kontext- und Evidenzprüfung bleiben notwendig.

---

## 7.10 Context Engine

**Zweck:**  
Bestimmt, welche Bedeutung eine ansonsten identische Bit- oder Formstruktur im aktuellen Thema besitzt.

**Mögliche Kontextachsen:**

- Domäne,
- Sprache,
- Relationstyp,
- Zeit,
- Benutzerabsicht,
- mathematische Aufgabe,
- Hardwarezustand,
- Sicherheitsklasse.

**Anforderung:**  
Kontext muss als explizite Eingabe behandelt werden und darf nicht unsichtbar in der Ausführung variieren.

---

## 7.11 Pattern Engine

**Zweck:**  
Erkennt strukturierte Merkmale in den Universumsergebnissen.

**Mögliche Merkmale:**

- gesetzte Bits,
- Hamming-Distanz,
- Übergänge zwischen 0 und 1,
- zusammenhängende Folgen,
- Symmetrie,
- zyklische Wiederholung,
- Übereinstimmung mit Shape-Profilen,
- Negativmuster,
- Nachbarschaftsrelationen.

**Vorschlag:**  
Jede Merkmalsfunktion erhält:

- stabile Function-ID,
- Eingabe- und Ausgabevertrag,
- Kostenklasse,
- Versionsnummer,
- deterministische Testvektoren.

---

## 7.12 Cascade Filter

**Zweck:**  
Verwirft ungeeignete Kandidaten möglichst früh.

```text
grober Themenfilter
→ Strukturfilter
→ Shape-Maske
→ Gate-Prüfung
→ Kontextprüfung
→ Erfahrungsabgleich
→ vollständige Technikbewertung
```

**Vorteil:**  
Nicht jeder Kandidat muss alle teuren Universen durchlaufen.

---

## 7.13 Technique Registry

**Zweck:**  
Speichert ausführbare Techniken und ihre Voraussetzungen.

**Technikdefinition:**

```text
Technique
├─ Technique-ID
├─ Version
├─ Situation Preconditions
├─ Universe Sequence oder DAG
├─ Required Evidence
├─ Abort Rules
├─ Cost Budget
├─ Result Projector
└─ Known Limitations
```

**Vorschlag:**  
Techniken sollen als gerichteter azyklischer Graph oder begrenzte Zustandsmaschine beschrieben werden. Unbegrenzte Schleifen sind im MVP nicht zulässig.

---

## 7.14 Technique Selector

**Zweck:**  
Wählt Kandidatentechniken für eine Situation.

**Prioritäten:**

1. Gültigkeitsbereich passt.
2. Sicherheitsvoraussetzungen sind erfüllt.
3. Verifikation ist ausreichend.
4. bekannte Gegenbeispiele widersprechen nicht.
5. erwartete Kosten sind zulässig.
6. Themenrelevanz ist hoch.
7. Erfolgsquote ist ausreichend.
8. Technikversion ist kompatibel.

---

## 7.15 Technique Executor

**Zweck:**  
Führt ausgewählte Techniken kontrolliert aus.

**Eigenschaften:**

- begrenztes Ressourcenbudget,
- deterministische Reihenfolge,
- kontrollierte Parallelisierung,
- lokale Fehlerbehandlung,
- vollständiger Trace,
- keine direkte Wissensmutation.

---

## 7.16 AnswerPoint Projector

**Zweck:**  
Reduziert ein Universumsergebnis auf einen kleinen Bedeutungspunkt.

**Beispiel:**

```text
AnswerPoint
├─ topic_id
├─ concept_id
├─ relation_id
├─ value
├─ polarity
├─ evidence_class
├─ universe_id
└─ trace_reference
```

**Offen:**  
Ob AnswerPoints zwingend acht Byte groß sein sollen oder ein variables Format erhalten.

---

## 7.17 Semantic Constellation Builder

**Zweck:**  
Kombiniert 16 bis N AnswerPoints zu einer vollständigen Aussage.

**Aufgaben:**

- Dubletten entfernen,
- Widersprüche erkennen,
- Pflichtpunkte prüfen,
- Reihenfolge bestimmen,
- Negationen erhalten,
- Evidenzklassen zusammenführen,
- unvollständige Konstellationen ablehnen.

---

## 7.18 Structured Output Engine

**Zweck:**  
Erzeugt eine nichtsprachliche, vollständig kontrollierte Ausgabe.

**Mögliche Formate:**

- Relationsobjekt,
- Formelresultat,
- Steuerkommando,
- Entscheidungskarte,
- JSON-ähnliche interne Struktur,
- Binärausgabe.

**Sicherheitsregel:**  
Externe Aktoren benötigen zusätzliche Autorisierung. Ein AnswerPoint allein darf keine kritische Hardwareaktion auslösen.

---

## 7.19 SLM-Renderer

**Zweck:**  
Übersetzt eine validierte semantische Konstellation in Wort und Schrift.

**Eingabe:**  
Nur bestätigte AnswerPoints und ein zulässiger Formulierungsrahmen.

**Ausgabe:**  
Textentwurf mit Referenz auf die verwendeten Punkte.

**Nicht erlaubt:**

- neue Tatsachen hinzufügen,
- Negationen umkehren,
- Sicherheit übertreiben,
- unbekannte Zahlen erzeugen,
- fehlende Relationen ergänzen,
- Quellen verschleiern.

---

## 7.20 Semantic Roundtrip Validator

**Zweck:**  
Prüft die sprachliche Ausgabe erneut.

```text
AnswerPoints
→ Text erzeugen
→ Text erneut analysieren
→ Aussagen extrahieren
→ mit AnswerPoints vergleichen
```

**Erfolg:**

- alle Pflichtaussagen enthalten,
- keine zusätzliche Behauptung,
- Relationen korrekt,
- Polarität korrekt,
- Unsicherheit korrekt dargestellt.

**Fehlschlag:**

- neue Formulierung versuchen,
- Template-Renderer verwenden,
- strukturierte Ausgabe statt Freitext,
- kontrollierter Safe Shutdown.

---

## 7.21 Experience Ledger

**Zweck:**  
Append-only-Speicherung konkreter Erfahrungen.

**Eigenschaften:**

- Originaldatensatz unveränderlich,
- nachträgliche Bewertung als eigener Datensatz,
- Verweise statt Überschreiben,
- Integritätsprüfung,
- vollständige Versionsreferenzen.

---

## 7.22 Experience Consolidator

**Zweck:**  
Verdichtet mehrere vergleichbare Erfahrungen zu einer ExperienceCell.

**Anforderungen:**

- Originalfälle bleiben referenzierbar,
- Gegenbeispiele dürfen nicht verloren gehen,
- Gültigkeitsbereich muss explizit bleiben,
- Verdichtung muss reproduzierbar sein,
- Änderungen erzeugen neue Versionen.

---

## 7.23 Skill Ranking Engine

**Zweck:**  
Ermittelt, welche Techniken in den Hot-Skill-Cache gelangen.

**Vorschlag für getrennte Teilwerte:**

```text
UsageScore
SuccessScore
VerificationScore
RelevanceScore
CostScore
FailureScore
RiskScore
NoveltyScore
```

Eine mögliche kombinierte Bewertungsform ist:

\[
R =
w_sS+
w_vV+
w_tT+
w_uU
-
w_fF
-
w_cC
-
w_rR_i
\]

Die Gewichte sind noch nicht festgelegt und müssen versioniert werden.

**Wichtige Regel:**  
Häufigkeit allein darf nicht über Qualität entscheiden.

---

## 7.24 Hot-Skill-Cache

**Zweck:**  
Hält aktuell relevante Techniken und ExperienceCells im RAM.

**Cache-Key kann enthalten:**

- Themen-ID,
- Situationstyp,
- Technikfamilie,
- Sicherheitsklasse,
- Hardwareprofil,
- Versionsstand.

**Verdrängungsregel:**  
Nicht nur Least Recently Used, sondern Kombination aus Relevanz, Verifikation, Kosten und erwarteter Wiederverwendung.

---

## 7.25 Trace Engine

**Zweck:**  
Verbindet Eingabe, Technik, Universen, Ergebnisse, Ausgabe und Lernerfahrung.

**Trace-Inhalte:**

- Eingabehash,
- Datenversionen,
- Technique-ID,
- Universe-IDs,
- ausgeführte Operationen,
- Abbrüche,
- AnswerPoints,
- Renderer-Version,
- Bewertungsresultat,
- ExperienceRecord-ID.

---

## 8. Datenmodell

## 8.1 Kanonisches NanoNeuron

**Festgelegt:**

```text
Größe: 8 Byte
```

**Logisch benötigte Felder aus dem bisherigen Projekt:**

- ID beziehungsweise lokaler Index,
- Shape-Maske,
- Shape-Wert,
- Funktions-ID,
- Tabellen-ID,
- Ausgangs-Slot,
- Match-Ziel,
- Miss-Ziel,
- Flags.

### Konflikt

Eine naive Speicherung aller Felder mit jeweils einem oder zwei Byte kann die 8-Byte-Grenze überschreiten.

### Lösungsvarianten

#### Variante A: Lokale implizite ID

Die NanoNeuron-ID ergibt sich aus:

\[
ID = ExecutionCellBase + LocalIndex
\]

Die ID wird nicht im Datensatz gespeichert.

#### Variante B: Seitentabellen

Selten benötigte Angaben werden in separaten Tabellen gespeichert.

#### Variante C: Profilabhängiges Layout

Verschiedene NanoNeuron-Typen verwenden unterschiedliche 8-Byte-Layouts.

#### Variante D: Komprimierte Sprungziele

Match- und Miss-Ziele werden relativ zur aktuellen Position gespeichert.

### Empfehlung

Für den MVP:

- implizite lokale ID,
- ein festes 8-Byte-Core-Layout,
- externe Seitentabelle für seltene Metadaten,
- keine variablen Felder im Core.

Das genaue Byte-Packing bleibt **offen** und muss durch repräsentative Workloads bestimmt werden.

---

## 8.2 ActiveProjection

```text
ActiveProjection
├─ execution_cell_id
├─ projection_type
├─ byte_length
├─ source_version
└─ packed_state[]
```

Der einfachste Projektionstyp enthält genau ein Zustandsbyte je NanoNeuron.

---

## 8.3 UniverseSpec

```text
UniverseSpec
├─ universe_id
├─ version
├─ projection_type
├─ transform_type
├─ transform_parameter
├─ gate_id
├─ mask_id
├─ context_id
├─ input_bindings
├─ output_projector
├─ cost_limit
└─ abort_policy
```

Nicht verwendete Felder müssen ausdrücklich als `NONE` markiert werden. Implizite Standardwerte sollen vermieden werden.

---

## 8.4 SituationSignature

```text
SituationSignature
├─ situation_id
├─ topic_ids
├─ concept_ids
├─ relation_candidates
├─ shape_features
├─ context_constraints
├─ target_state
├─ resource_constraints
├─ uncertainty_flags
└─ source_trace
```

---

## 8.5 TechniqueDefinition

```text
TechniqueDefinition
├─ technique_id
├─ version
├─ preconditions
├─ universe_graph
├─ mandatory_checks
├─ rejection_rules
├─ success_conditions
├─ cost_budget
├─ safety_class
└─ provenance
```

---

## 8.6 ExperienceRecord

```text
ExperienceRecord
├─ experience_id
├─ timestamp_or_sequence
├─ situation_signature
├─ selected_technique
├─ considered_alternatives
├─ universe_traces
├─ answer_points
├─ observed_result
├─ evaluation
├─ verification_status
├─ environment_version
└─ parent_experience
```

**Persistenzregel:**  
Der Record wird nach dem Schreiben nicht verändert. Spätere Korrekturen referenzieren ihn.

---

## 8.7 EvaluationRecord

```text
EvaluationRecord
├─ experience_id
├─ evaluator_id
├─ result_class
├─ success_value
├─ cost_value
├─ risk_value
├─ error_class
├─ verification_reference
└─ evaluation_version
```

---

## 8.8 ExperienceCell

```text
ExperienceCell
├─ cell_id
├─ version
├─ situation_cluster
├─ preferred_techniques
├─ validity_constraints
├─ positive_case_count
├─ negative_case_count
├─ unresolved_case_count
├─ aggregate_cost
├─ verification_class
└─ source_experience_ids
```

---

## 8.9 AnswerPoint

**Vorschlag:**

```text
AnswerPoint
├─ type
├─ topic
├─ concept
├─ relation
├─ value
├─ polarity
├─ evidence
└─ trace
```

Für einen späteren kompakten Binärvertrag können einzelne Felder in Tabellen-IDs und lokale Indizes umgewandelt werden.

---

## 8.10 Datenklassen

### Unveränderlich

- kanonische Version eines NanoNeurons,
- TechniqueDefinition-Version,
- ExperienceRecord,
- ursprünglicher Trace,
- EvaluationRecord.

### Versionierbar

- ExperienceCell,
- Rangfunktion,
- Cache-Profil,
- Decoder,
- Maskenbibliothek,
- Gate-Profile.

### Flüchtig

- aktive Projektionen,
- Universum-Zwischenwerte,
- Scratchpads,
- Kandidatenlisten,
- temporäre AnswerPoints vor Bestätigung,
- Decoder-Arbeitsspeicher.

---

## 9. Prozess- und Datenfluss

## 9.1 Normalablauf

1. Eine Eingabe wird empfangen.
2. Das Input-Modul prüft Format und Grenzen.
3. Die Eingabe wird normalisiert.
4. Der Situation Analyzer bestimmt Thema, Konzepte, Formen und Ziel.
5. Eine SituationSignature wird erzeugt.
6. Der Erfahrungsindex sucht ähnliche bestätigte Situationen.
7. Die Technique Registry liefert grundsätzlich passende Techniken.
8. Der Technique Selector ordnet die Kandidaten.
9. Die benötigte ExecutionCell wird geladen.
10. Der Active Projection Loader erzeugt den minimalen Arbeitsdatensatz.
11. Die erste Technik wird ausgeführt.
12. Ihre Universen werden bedarfsgesteuert berechnet.
13. Frühe Filter verwerfen ungeeignete Pfade.
14. Erfolgreiche Teilberechnungen erzeugen AnswerPoints.
15. Die semantische Konstellation wird aufgebaut.
16. Widersprüche und fehlende Pflichtpunkte werden geprüft.
17. Das strukturierte Ergebnis wird validiert.
18. Optional erzeugt der SLM-Renderer einen Text.
19. Der Roundtrip Validator prüft den Text gegen die AnswerPoints.
20. Das Ergebnis wird ausgegeben.
21. Ein ExperienceRecord wird persistent gespeichert.
22. Eine Bewertung wird ergänzt.
23. Ranking und Cache werden kontrolliert aktualisiert.
24. Flüchtige Universen und Scratchpads werden verworfen.

---

## 9.2 Beispiel: „Wenn Klippe, dann baue Brücke“

Dieses Beispiel dient nur als anschauliche Abstraktion.

```text
Situation:
    Hindernis = Lücke
    Ziel = gegenüberliegende Fläche
    Spannweite = innerhalb bekannter Grenze
    Untergrund = geeignet

Technikkandidat:
    BRIDGE_BUILDING

Universen:
    U1: Shape-Maske für Lücke
    U2: Distanzprüfung
    U3: Stabilitätskontext
    U4: Ressourcenprüfung
    U5: bekannte Gegenbeispiele

Entscheidung:
    alle Pflichtuniversen gültig

Ergebnis:
    Technik anwendbar

Erfahrung:
    Situation + Technik + Resultat + Kosten + Trace
```

Die AXIOM-Übertragung könnte lauten:

```text
Wenn Formelüberlauf,
dann Domain-Grenzen prüfen
und eine exakte alternative Umformung versuchen.
```

oder:

```text
Wenn Wortbedeutung mehrdeutig,
dann Bedeutungskandidaten erzeugen,
gegen Kontext prüfen
und nicht passende Bedeutungen verwerfen.
```

---

## 9.3 Fehlschlagablauf

```text
Situation erkannt
→ Technik ausgewählt
→ Pflichtuniversum schlägt fehl
→ Technik abbrechen
→ Fehlergrund speichern
→ Gegenbeispiel erzeugen
→ nächste zulässige Technik versuchen
```

Ein Fehlschlag kann verschiedene Bedeutungen besitzen:

- Technik grundsätzlich ungeeignet,
- Technik nur in diesem Wertebereich ungeeignet,
- fehlende Daten,
- Ressourcenlimit,
- Implementierungsfehler,
- widersprüchlicher Kontext,
- unsicheres Ergebnis.

Diese Klassen dürfen nicht vermischt werden.

---

## 9.4 Lernablauf

```text
ExperienceRecord speichern
→ Bewertung erfassen
→ vergleichbare Erfahrungen gruppieren
→ Erfolgs- und Fehlermuster bestimmen
→ Gültigkeitsbereich ableiten
→ ExperienceCell erzeugen
→ Verifikation durchführen
→ Hot-Skill-Kandidat bilden
```

Eine ExperienceCell darf erst aktiviert werden, wenn ihre Ableitung reproduzierbar und ihr Gültigkeitsbereich dokumentiert ist.

---

## 9.5 Sprachrekonstruktion

```text
16–N bestätigte AnswerPoints
→ gewünschtes Ausgabeformat bestimmen
→ zulässige Wörter und Relationen bereitstellen
→ Text erzeugen
→ Text erneut analysieren
→ Behauptungen vergleichen
→ Ausgabe freigeben oder verwerfen
```

---

## 10. Entscheidungslogik

## 10.1 Technikpriorisierung

| Kriterium | Bedeutung | Wirkung |
|---|---|---|
| Preconditions Match | Situation liegt im bekannten Einsatzbereich | zwingend |
| Negative Match | bekanntes Gegenbeispiel trifft zu | Technik ablehnen |
| Verification | Qualität der Nachweise | Rang erhöhen oder begrenzen |
| Topic Relevance | Passung zum aktuellen Thema | Rang erhöhen |
| Success History | frühere bestätigte Erfolge | Rang erhöhen |
| Failure History | frühere Fehlschläge | Rang senken |
| Cost | geschätzter Ressourcenbedarf | bei Budgetüberschreitung ablehnen |
| Risk | mögliche unerwünschte Wirkung | zusätzliche Freigabe |
| Version Compatibility | Daten und Technik kompatibel | zwingend |

## 10.2 Mehrdeutigkeit

Bei mehreren gleichwertigen Techniken gelten mögliche Strategien:

1. kostengünstigste vollständig gültige Technik,
2. am besten verifizierte Technik,
3. Technik mit kleinstem bekannten Risikobereich,
4. parallele Simulation mehrerer Kandidaten,
5. Status `REQUIRES_REVIEW`.

Die Strategie muss je Anwendungsdomäne festgelegt werden.

## 10.3 Abbruchbedingungen

- Pflichtdaten fehlen,
- kein gültiger Kontext,
- Kostenbudget überschritten,
- maximale Universumsanzahl erreicht,
- widersprüchliche Pflichtpunkte,
- nicht unterstützte Technikversion,
- beschädigter Trace,
- unerlaubte Aktion,
- Renderer erzeugt zusätzliche Behauptungen.

## 10.4 Ranglistenregel

Die Rangliste ist nur eine Vorauswahlhilfe. Sie darf keine ungültige Technik legitimieren.

```text
Gültigkeit vor Rang
Sicherheit vor Geschwindigkeit
Nachweis vor Häufigkeit
Kontextpassung vor globaler Popularität
```

---

## 11. Zustandsmodell

### Eingabestatus

```text
RECEIVED
VALIDATED
REJECTED
NORMALIZED
```

### Situationsstatus

```text
UNKNOWN
RECOGNIZED
AMBIGUOUS
REQUIRES_REVIEW
```

### Technikstatus

```text
CANDIDATE
ELIGIBLE
REJECTED
EXECUTING
SUCCEEDED
FAILED
ABORTED
```

### Erfahrungsstatus

```text
RECORDED
EVALUATED
UNVERIFIED
VERIFIED
CONTRADICTED
SUPERSEDED
CONSOLIDATED
```

### AnswerPoint-Status

```text
PROVISIONAL
SUPPORTED
CONFLICTING
REJECTED
PUBLISHED
```

### Erlaubter Erfahrungsübergang

```text
RECORDED
→ EVALUATED
→ VERIFIED
→ CONSOLIDATED
```

Alternative:

```text
RECORDED
→ EVALUATED
→ CONTRADICTED
```

### Verbotene Übergänge

- `UNVERIFIED → PUBLISHED` ohne Freigaberegel,
- `FAILED → SUCCEEDED` durch Überschreiben,
- `CONTRADICTED → VERIFIED` ohne neue Bewertungsversion,
- Änderung eines bestehenden ExperienceRecords,
- Aktivierung einer inkompatiblen TechniqueDefinition.

---

## 12. Schnittstellen

## 12.1 Input Interface

**Partner:** Benutzer, API, Sensor oder interne Komponente  
**Zweck:** Eingabeübergabe  
**Format:** noch offen  
**Vertrauensgrenze:** extern zu intern  
**Fehlerverhalten:** kontrollierte Ablehnung  
**Protokollierung:** Eingabehash und Validierungsstatus

## 12.2 ExecutionCell Interface

**Partner:** Scheduler und Runtime  
**Zweck:** Laden und Ausführen kompakter Zellen  
**Eingabe:** Cell-ID, Version, Projektionstyp  
**Ausgabe:** AnswerPoints, Status, Trace  
**Vertrauensgrenze:** nur signierte oder verifizierte Cell-Versionen

## 12.3 Experience Store Interface

**Partner:** Runtime, Consolidator, Analyzer  
**Zweck:** append-only Speicherung und Suche  
**Anforderung:** atomare Schreibvorgänge  
**Fehlerverhalten:** Ergebnis darf ausgegeben, aber nicht als gelernt bestätigt werden, wenn die Persistenz fehlschlägt.

## 12.4 Renderer Interface

**Partner:** Semantic Constellation Builder und SLM  
**Zweck:** sprachliche Ausgabe  
**Eingabe:** geschlossene AnswerPoint-Menge  
**Ausgabe:** Text und verwendete Punktreferenzen  
**Sicherheitsgrenze:** kein freier Zugriff auf ungeprüfte Wissensspeicher

## 12.5 Actuator Interface

**Partner:** externe Systeme oder Hardware  
**Zweck:** kontrollierte Aktionen  
**Autorisierung:** domänenspezifisch erforderlich  
**Fehlerverhalten:** fail-closed bei kritischen Aktionen  
**Offen:** konkrete Protokolle.

---

## 13. Sicherheit und Vertrauensmodell

## 13.1 Schützenswerte Ressourcen

- kanonische NanoNeuron-Daten,
- Technikdefinitionen,
- Erfahrungsverlauf,
- Bewertungsregeln,
- Trace-Ketten,
- Sicherheitskontexte,
- externe Aktoren,
- Renderer-Grenzen,
- Benutzer- und Sensordaten.

## 13.2 Mögliche Bedrohungen

- manipulierte ExperienceRecords,
- vergiftete Techniken,
- Cache-Promotion schlechter Erfahrungen,
- falsch zugeordnete Kontexte,
- absichtliche Universums-Explosion,
- Ressourcenerschöpfung,
- zusätzliche Behauptungen durch das SLM,
- manipulierte Bewertungsgewichte,
- Replay veralteter Technikversionen,
- beschädigte Seitentabellen.

## 13.3 Vertrauenszonen

```text
Externe Eingabe
    ↓ nicht vertrauenswürdig
Input Validator
    ↓ kontrolliert
Deterministische Runtime
    ↓ intern vertrauenswürdig, aber prüfpflichtig
Experience Ledger
    ↓ persistent und integritätsgeschützt
Consolidator
    ↓ darf ableiten, nicht überschreiben
Renderer
    ↓ inhaltlich nicht vertrauenswürdig
Roundtrip Validator
    ↓ Freigabegrenze
Externe Ausgabe oder Aktion
```

## 13.4 Sicherheitsmaßnahmen

- append-only Erfahrungsprotokoll,
- Hash-Verknüpfung oder vergleichbare Integritätsprüfung,
- signierte beziehungsweise freigegebene Technikversionen,
- feste Ressourcenbudgets,
- begrenzte Graph- und Schleifentiefe,
- getrennte Renderer-Sandbox,
- keine direkte Aktorfreigabe durch SLM,
- kontextgebundene Autorisierung,
- reproduzierbare Cache-Erzeugung,
- Rollback auf bekannte Versionen.

## 13.5 Verbleibende Risiken

- Fehler in Bewertungsfunktionen,
- unvollständige Gegenbeispiele,
- falsche SituationSignature,
- semantische Unterschiede trotz erfolgreichem Roundtrip,
- Datenkorruption außerhalb der Anwendung,
- Seiteneffekte bei realen Aktoren,
- statistische Fehler eines optionalen Embedding-Modells.

---

## 14. Fehler- und Ausnahmebehandlung

| Fehlerursache | Erkennung | Lokale Reaktion | Auswirkung | Wiederherstellung |
|---|---|---|---|---|
| Ungültige Eingabe | Schema- und Grenzprüfung | ablehnen | keine Ausführung | korrigierte Eingabe |
| Fehlender Kontext | Context Engine | `AMBIGUOUS` | keine endgültige Technik | weitere Evidenz |
| Unbekannte Situation | Erfahrungsindex leer | Basistechniken prüfen | langsamere Verarbeitung | neue Erfahrung speichern |
| Universumslimit erreicht | Runtime-Counter | abbrechen | unvollständiges Ergebnis | engerer Filter |
| Ressourcenmangel | Scheduler | Technik stoppen | `ABORTED` | kleineres Budget oder alternative Technik |
| Gate-/Maskenfehler | Testvektor oder Exception | Universum verwerfen | Teiltechnik ungültig | Implementierung korrigieren |
| Widersprüchliche AnswerPoints | Constellation Builder | keine Freigabe | `CONFLICTING` | alternative Technik oder Review |
| Experience-Schreibfehler | Storage-Fehler | Ausgabe markieren | nicht gelernt | späterer Wiederholungsversuch |
| Beschädigter ExperienceRecord | Integritätsprüfung | isolieren | nicht für Ranking verwenden | Backup oder Rekonstruktion |
| SLM-Halluzination | Roundtrip-Miss | Text verwerfen | strukturierte Ausgabe | Template-Fallback |
| Inkompatible Version | Versionsprüfung | Ausführung sperren | Technik nicht verfügbar | Migration |
| Fehlgeschlagene Konsolidierung | Reproduzierbarkeitsprüfung | alte ExperienceCell behalten | keine Promotion | Konsolidator korrigieren |

---

## 15. Betriebsmodell

## 15.1 Installation

- Runtime installieren,
- NanoNeuron-Schema registrieren,
- Technique Registry initialisieren,
- Experience Store anlegen,
- Integritätsparameter erzeugen,
- Test-ExecutionCells laden.

## 15.2 Initialisierung

1. Versionskompatibilität prüfen.
2. kanonische Quellen validieren.
3. Experience Ledger prüfen.
4. ExperienceCells laden.
5. Hot-Skill-Cache deterministisch aufbauen.
6. Selbsttests für Gates, Masken und Projektionen ausführen.

## 15.3 Regulärer Betrieb

- Eingaben verarbeiten,
- Ressourcen messen,
- Traces schreiben,
- Erfahrungen speichern,
- Rankings nur an definierten Konsolidierungspunkten aktualisieren.

## 15.4 Wartung

- beschädigte Datensätze isolieren,
- veraltete Techniken markieren,
- neue Technikversionen testen,
- Cache neu erzeugen,
- Konsolidierungen reproduzieren.

## 15.5 Updates

Updates müssen getrennt versionieren:

- Runtime,
- NanoNeuron-Layout,
- Technique Registry,
- Ranking-Funktion,
- Experience-Schema,
- Renderer,
- Roundtrip Validator.

## 15.6 Backup

Zu sichern sind:

- kanonische NanoNeuronen,
- Technikdefinitionen,
- vollständiger Experience Ledger,
- Evaluationsdatensätze,
- Versionstabellen,
- Integritätsmetadaten.

Der Hot-Skill-Cache muss nicht zwingend gesichert werden, wenn er deterministisch rekonstruierbar ist.

## 15.7 Entwicklungs-, Test- und Produktionsbetrieb

### Entwicklung

- neue Operatoren,
- flexible Diagnose,
- synthetische Daten,
- keine produktiven ExperienceCells.

### Test

- feste Fixtures,
- deterministische Seeds, falls überhaupt Zufall verwendet wird,
- Fehler-Injektion,
- Ressourcenmessung.

### Produktion

- nur freigegebene Technikversionen,
- begrenzte Budgets,
- manipulationsgeschützte Traces,
- kontrollierte Konsolidierung.

---

## 16. Skalierung und Ressourcen

## 16.1 Speicher

Der kanonische Speicher wächst linear:

\[
M_{\text{canonical}} = 8N
\]

mit \(N\) NanoNeuronen.

Die aktive 1-Byte-Projektion wächst:

\[
M_{\text{active}} = N
\]

zuzüglich Index-, Scratchpad- und Trace-Overhead.

## 16.2 Virtuelle Universen

Für Mengen:

- \(S\): Shift-Varianten,
- \(R\): Rotationsvarianten,
- \(G\): Gates,
- \(M\): Masken,
- \(C\): Kontexte,

kann ein vollständiger kartesischer Raum theoretisch umfassen:

\[
U=|S|\cdot|R|\cdot|G|\cdot|M|\cdot|C|
\]

Das bedeutet nicht, dass alle Kombinationen sinnvoll oder auszuführen sind.

**Vorschlag:**  
TechniqueDefinitions geben explizit nur zulässige Kombinationen an. Der vollständige kartesische Raum wird nicht automatisch durchsucht.

## 16.3 Parallelisierung

Gut parallelisierbar:

- unabhängige Universen,
- gleiche Gate-Operation auf vielen Zuständen,
- Maskenprüfungen,
- unabhängige Techniksimulationen,
- Batch-Auswertung von ExperienceRecords.

Schwerer parallelisierbar:

- dynamisches Match-/Miss-Routing,
- kontextabhängige Abbrüche,
- sequenzielle Technikgraphen,
- globale Konsistenzprüfung.

## 16.4 CPU und GPU

**Empfohlener MVP:** CPU und RAM.

CPU-Vorteile:

- feingranulares Routing,
- Verzweigungen,
- geringe Startkosten,
- einfache Trace-Erzeugung,
- gute Eignung für kleine ExecutionCells.

GPU-Potenzial:

- viele identische Gate-Operationen,
- große Batch-Maskenprüfungen,
- parallele Universen,
- Vektor- und Popcount-ähnliche Operationen.

Eine GPU-Ausführung soll erst nach einem profilierten CPU-Referenzmodell erfolgen.

## 16.5 Mikrocontroller-Perspektive

Mikrocontroller integrieren Rechenkern, Speicher- und Peripheriefunktionen für konkrete eingebettete Aufgaben. Einige besitzen programmierbare digitale oder hybride Funktionsblöcke. Das unterstützt als Architekturmetapher die Idee kleiner spezialisierter AXIOM-Funktionsblöcke, beweist jedoch noch keine direkte Hardwareeffizienz des NanoNeuron-Formats. citeturn906457view1

---

## 17. Datenschutz und Datenlebenszyklus

## 17.1 Datenerhebung

Nur Daten speichern, die für:

- Situationserkennung,
- Technikbewertung,
- Fehleranalyse,
- Nachweis,
- spätere Wiederverwendung

erforderlich sind.

## 17.2 Zweckbindung

ExperienceRecords dürfen nur für definierte Lern- und Validierungszwecke verwendet werden.

## 17.3 Personenbezogene Daten

Falls Eingaben Benutzer-, Kommunikations- oder Sensordaten enthalten, können ExperienceRecords personenbezogene Inhalte oder ableitbare Profile enthalten.

**Vorschlag:**

- personenbezogene Rohdaten von Technikmetadaten trennen,
- pseudonyme Referenzen verwenden,
- sensible Inhalte nicht in den Hot-Skill-Cache übernehmen,
- Lösch- und Sperrmarkierungen unterstützen.

## 17.4 Aufbewahrung

Noch offen:

- Dauer vollständiger ExperienceRecords,
- Behandlung gescheiterter Versuche,
- Archivierung alter Technikversionen,
- Löschbarkeit bei gleichzeitigem Integritätsnachweis.

## 17.5 Löschung

Ein append-only Ledger steht potenziell im Konflikt mit Löschanforderungen.

Mögliche Lösung:

- verschlüsselte Datensegmente,
- Löschung des zugehörigen Schlüssels,
- separate personenbezogene Payloads,
- Ledger enthält nur nicht sensible Referenzen.

Eine konkrete rechtliche Bewertung ist nicht Bestandteil dieses Konzepts.

---

## 18. Umsetzungsvorschlag

## Phase 0: Begriffs- und Vertragsklärung

- 8-Byte-NanoNeuron-Vertrag bestätigen,
- aktive 1-Byte-Projektion definieren,
- Unterschied zwischen Prozessdimension und mathematischer Dimension dokumentieren,
- UniverseSpec festlegen,
- AnswerPoint-Schema bestimmen,
- ExperienceRecord-Vertrag definieren,
- Lern- und Verifikationsstatus festlegen.

## Phase 1: Minimaler Bit-Universe-Prototyp

Umfang:

- ein Byte Eingang,
- Shift links und rechts,
- Rotation links und rechts,
- AND, OR, XOR und NOT,
- feste Masken,
- deterministischer Trace,
- keine Persistenz außer Testprotokollen.

Ziel:

```text
1 Byte
→ mehrere Universen
→ je ein Ergebnis
→ identischer Trace bei Wiederholung
```

## Phase 2: ExecutionCell-Integration

- 8-Byte-NanoNeuron laden,
- aktive Projektion erzeugen,
- lokale 16-Bit-Indizes verwenden,
- Match-/Miss-Routing,
- begrenzte Technikgraphen,
- Ressourcenbudgets.

## Phase 3: AnswerPoint- und Konstellationsmodell

- typisierte AnswerPoints,
- Pflicht- und Negativpunkte,
- Konflikterkennung,
- strukturierte Ausgabe,
- noch kein generatives SLM erforderlich.

## Phase 4: Experience Ledger

- ExperienceRecords persistent speichern,
- Erfolg und Fehlschlag unterscheiden,
- Integritätsprüfung,
- Wiederladen und Replay,
- Versionierung.

## Phase 5: Erfahrungsabruf

- SituationSignature,
- Ähnlichkeit über diskrete Merkmale,
- Technikvorauswahl,
- Gegenbeispielprüfung,
- reproduzierbares Ranking.

## Phase 6: Experience Consolidation

- ähnliche Fälle gruppieren,
- Gültigkeitsbereiche ableiten,
- ExperienceCells erzeugen,
- Originalfälle referenzierbar halten,
- Hot-Skill-Cache aufbauen.

## Phase 7: Sprachrekonstruktion

- zunächst Template-Renderer,
- anschließend optional themenorientiertes SLM,
- geschlossene Eingabestruktur,
- Roundtrip-Validierung,
- strukturierter Fallback.

## Phase 8: Ressourcenoptimierung

- Profiling,
- SIMD- und 64-Bit-Packing,
- Cache-Layouts,
- Batch-Ausführung,
- optional GPU-Backend.

## Phase 9: Härtung

- manipulierte Erfahrungen,
- Cache Poisoning,
- Universums-Explosion,
- beschädigte Traces,
- Update- und Rollback-Tests,
- lang laufende Belastungstests.

---

## 19. Test- und Validierungsstrategie

## 19.1 Unit-Tests

- jede Shift-Operation,
- jede Rotation,
- jedes Gate,
- Masken-Match und Miss,
- Projection Packing,
- AnswerPoint-Encoding,
- Ranking-Teilwerte,
- Zustandsübergänge.

## 19.2 Property-Tests

Beispiele:

\[
\operatorname{ROTL}(b,8)=b
\]

\[
b\oplus b=0
\]

\[
b\land 255=b
\]

\[
b\lor 0=b
\]

\[
\operatorname{NOT}(\operatorname{NOT}(b))=b
\]

## 19.3 Integrationstests

- NanoNeuron → Projektion → Universum → AnswerPoint,
- Technik → mehrere Universen → Konstellation,
- Konstellation → Renderer → Roundtrip,
- ExperienceRecord → Speicherung → Wiederladen,
- ExperienceCell → Hot-Skill-Cache.

## 19.4 End-to-End-Test

```text
Eingabe
→ Situation
→ Technik
→ Universen
→ AnswerPoints
→ Ausgabe
→ Erfahrung
→ erneute ähnliche Eingabe
→ Technikabruf
```

## 19.5 Negativtests

- ungültige Shift-Anzahl,
- unbekannte Gate-ID,
- fehlende Maske,
- widersprüchlicher Kontext,
- zirkulärer Technikgraph,
- zu viele Universen,
- manipulierte Erfahrung,
- zusätzliche SLM-Behauptung.

## 19.6 Grenzwerttests

- leere ExecutionCell,
- maximale lokale ID,
- maximale Cell-Größe,
- null AnswerPoints,
- exakt 16 AnswerPoints,
- sehr große N-Konstellation,
- voller Hot-Skill-Cache,
- SSD-Ausfall,
- RAM-Druck.

## 19.7 Fehler-Injektion

- Schreibfehler während ExperienceRecord,
- beschädigter Index,
- inkonsistente Versionstabellen,
- Timeout eines Universums,
- Renderer-Ausfall,
- verlorener Cache.

## 19.8 Reproduzierbarkeit

Jeder Testfall speichert:

- Eingabe,
- Runtime-Version,
- Datenversion,
- Technikversion,
- erwartete AnswerPoints,
- erwarteten Trace-Hash.

## 19.9 Leistungsnachweise

Noch keine konkreten Leistungswerte festlegen.

Zu messen sind:

- Nanosekunden beziehungsweise Zyklen je Gate-Operation,
- Kosten je Universum,
- Cache-Miss-Rate,
- RAM je aktiver ExecutionCell,
- SSD-Schreibvolumen je ExperienceRecord,
- Zeit für Experience-Suche,
- Kosten der Roundtrip-Prüfung.

---

## 20. Risiken und Gegenmaßnahmen

Die Bewertungen sind qualitative Ersteinschätzungen ohne Messdaten.

| Risiko | Ursache | Auswirkung | Wahrscheinlichkeit | Gegenmaßnahme | Restrisiko |
|---|---|---|---|---|---|
| Dimensions-Explosion | zu viele kombinierte Achsen | unvertretbare Laufzeit | hoch | Technikgraphen und Budgets | mittel |
| 8-Byte-Layout zu klein | zu viele logische Felder | Seitentabellen oder häufige Zugriffe | hoch | repräsentative Layout-Benchmarks | mittel |
| Häufigkeits-Bias | oft genutzte Technik steigt automatisch | falsche Cache-Dominanz | hoch | getrennte Qualitätswerte | mittel |
| Fehlerhafte Konsolidierung | unähnliche Fälle gruppiert | falsche allgemeine Regel | mittel | reproduzierbare Clusterregeln | mittel |
| SLM ergänzt Aussagen | generative Freiheit | unbelegte Ausgabe | hoch | Roundtrip und Template-Fallback | niedrig bis mittel |
| Erfahrungsvergiftung | manipulierte Eingaben oder Bewertungen | falsche Skills | mittel | Provenienz und Quarantäne | mittel |
| Fehlversuche werden missverstanden | Implementierungsfehler gilt als Fachwissen | falsche Gegenregel | mittel | Fehlerklassen trennen | niedrig bis mittel |
| SSD-Schreibvolumen | jeder Versuch wird vollständig gespeichert | Verschleiß und Kosten | mittel | Batch, Verdichtung, abgestufte Traces | mittel |
| Kontextfehler | identisches Muster in anderer Domäne | falsche Technik | hoch | explizite Context-ID | mittel |
| Trace-Overhead | sehr detaillierte Protokolle | Laufzeit und Speicherbedarf | hoch | Trace-Level und Kompression | mittel |
| Verlust seltener Fälle | aggressive Verdichtung | Grenzfälle vergessen | mittel | Originalrecords erhalten | niedrig |
| Versionsdrift | neue Runtime mit alten Skills | inkonsistente Ergebnisse | hoch | strikte Kompatibilitätsmatrix | niedrig |
| Metaphernverwechslung | Quanten- und Parallelweltbegriffe | falsche technische Erwartungen | mittel | klare formale Begriffe | niedrig |
| Unzureichende Generalisierung | zu konkrete ExperienceRecords | geringe Wiederverwendbarkeit | mittel | explizite Gültigkeitsbereiche | mittel |
| Zu starke Generalisierung | zu breite ExperienceCells | falsche Anwendungen | mittel | Gegenbeispiele und Review | mittel |

---

## 21. Offene Fragen

### Kritisch

1. Wie lautet das endgültige 8-Byte-NanoNeuron-Layout?
2. Welches Feld bildet die aktive 1-Byte-Projektion?
3. Sind Match- und Miss-Ziele lokal, relativ oder über Seitentabellen adressiert?
4. Wie wird ein AnswerPoint binär dargestellt?
5. Welche Bedingungen machen eine Erfahrung zu bestätigtem Wissen?
6. Wie wird verhindert, dass manipulierte Erfahrungen den Hot-Skill-Cache beeinflussen?
7. Welche maximale Technik- und Universumskomplexität ist im MVP zulässig?

### Hoch

1. Wie wird Situationsähnlichkeit deterministisch berechnet?
2. Wie werden Kontextwechsel erkannt?
3. Wann werden ExperienceRecords konsolidiert?
4. Wie werden Gegenbeispiele gewichtet?
5. Welche Ranking-Funktion wird verwendet?
6. Wie wird ein SLM technisch auf die AnswerPoints begrenzt?
7. Welche Trace-Tiefe ist für Produktion erforderlich?
8. Wie werden Seitentabellen im Cache behandelt?

### Mittel

1. Sollen AnswerPoints acht Byte groß sein?
2. Werden Universen sequenziell, parallel oder gemischt ausgeführt?
3. Welche Operationen gehören zur ersten Operatorbibliothek?
4. Welche Daten dürfen in ExperienceCells verdichtet werden?
5. Wie werden seltene, aber wichtige Techniken im Cache geschützt?
6. Soll die Experience-Suche tabellen-, graph- oder indexbasiert erfolgen?

### Niedrig

1. Welche metaphorischen Namen erscheinen in Benutzeroberflächen?
2. Welche Visualisierung zeigt Paralleluniversen?
3. Wie werden Technikfamilien benannt?
4. Soll ein Debug-Modus universelle Würfel und Muster grafisch darstellen?

---

## 22. Erweiterungsmöglichkeiten

## 22.1 Naheliegende Erweiterungen

### SIMD-Ausführung

Mehrere Zustandsbytes gemeinsam verarbeiten.

**Auswirkung:**  
Mehr Leistung, aber zusätzliche Packing- und Alignment-Anforderungen.

### GPU-Backend

Große Mengen gleichartiger Universen parallel ausführen.

**Auswirkung:**  
Höhere Komplexität bei Verzweigungen, Trace und Datenübertragung.

### Erweiterte Pattern-Bibliothek

Zusätzliche Formen, Nachbarschaften und Sequenzen.

**Auswirkung:**  
Größere Technique Registry und höherer Testaufwand.

### Thematische Skill-Pakete

Getrennte Technikbibliotheken für Mathematik, Sprache, Infrastruktur oder Steuerung.

**Auswirkung:**  
Bessere Isolation und kleinere Hot-Sets.

---

## 22.2 Optionale Funktionen

- visuelle Universumsanalyse,
- interaktive Trace-Wiedergabe,
- manuelle Bewertung von Erfahrungen,
- Skill-Import und -Export,
- verteilte Experience Stores,
- signierte Technikpakete.

---

## 22.3 Experimentelle Ideen

### Selbstständige Technikkomposition

AXIOM kombiniert bekannte Werkzeuge zu einer neuen Technik.

**Bedingung:**  
Neue Technik bleibt zunächst Kandidat und benötigt Tests sowie Freigabe.

### Binäre Semantic State Reconstruction

Eine Antwort wird nicht als Text, sondern als kompakter Zustandsvertrag an ein anderes AXIOM-System übertragen.

### Hardware-NanoCell

Umsetzung ausgewählter Gate- und Routingprofile in FPGA oder spezialisierter Hardware.

### Mehrstufige Decoder

```text
AnswerPoints
→ Fachdecoder
→ Sprachdecoder
→ Stilrenderer
```

---

## 22.4 Derzeit nicht empfohlene Erweiterungen

- unbeschränkte selbstmodifizierende Technikgraphen,
- direkte Aktorsteuerung durch ein SLM,
- globale Konsolidierung ohne Gegenbeispiele,
- vollständige Materialisierung hochdimensionaler Räume,
- ungeprüfte automatische Promotion neuer Skills,
- Behauptung einer quantenmechanischen Funktionsweise.

---

## 23. Kritische Bewertung

### 23.1 Technische Plausibilität

Die Grundelemente sind technisch plausibel:

- kompakte Datensätze,
- bitweise Operationen,
- Masken,
- deterministische Zustandsmaschinen,
- lazy berechnete Suchräume,
- persistente Ereignisprotokolle,
- Cache aus verdichteten Erfahrungen.

Nicht bewiesen ist bislang, dass ein 8-Byte-NanoNeuron für alle vorgesehenen Aufgaben die beste oder effizienteste Repräsentation darstellt.

### 23.2 Stärken

- klare Nachvollziehbarkeit,
- kleine aktive Zustände,
- explizite Technikdefinitionen,
- Lernen aus Erfolg und Fehlschlag,
- Trennung von Berechnung und Sprache,
- potenzielle CPU- und SIMD-Effizienz,
- reproduzierbare Erfahrungskonsolidierung,
- modulare Themenbereiche.

### 23.3 Schwächen

- hoher Architektur- und Metadatenaufwand,
- mögliche Explosion von Technikvarianten,
- schwierige Definition von Situationsähnlichkeit,
- komplexe Versionierung,
- Trace- und SSD-Kosten,
- Gefahr übermäßig starrer Regeln,
- schwierige Übertragung kontinuierlicher Daten in diskrete Punkte.

### 23.4 Unnötige Komplexität

Die Paralleluniversumsmetapher ist anschaulich, darf aber nicht dazu führen, jede einfache Operation als eigenes schwergewichtiges Objekt zu behandeln.

Ein Universum sollte im MVP nur ein kompakter Ausführungsplan sein:

```text
Operator-ID
+ Parameter
+ Eingabereferenz
+ Ausgaberegel
```

### 23.5 Zentrale Abhängigkeiten

- belastbares NanoNeuron-Layout,
- effiziente Experience-Indizes,
- korrekte Kontextmodellierung,
- reproduzierbare Konsolidierung,
- sichere Renderer-Grenze,
- realistische Ressourcenbudgets.

### 23.6 Schwer prüfbare Annahmen

- dass 16 bis N AnswerPoints für komplexe Antworten ausreichen,
- dass ein kleines thematisches SLM zuverlässig begrenzt werden kann,
- dass häufig wiederkehrende Techniken genügend Generalisierung ermöglichen,
- dass das 8-Byte-Format keinen übermäßigen Seitentabellenzugriff verursacht.

### 23.7 Einfachere Alternative

Ein erster Prototyp könnte ohne SLM, Embedding-Modell und GPU auskommen:

```text
8-Byte-NanoNeuron
→ aktive Projektion
→ Bituniversen
→ AnswerPoints
→ deterministische Texttemplates
→ Experience Ledger
```

Dieser Aufbau besitzt den größten Erkenntniswert bei geringstem Risiko.

### 23.8 Forschungsbedarf

- kompakte symbolische Erfahrungsrepräsentation,
- deterministische Situationsähnlichkeit,
- verlustarme Experience-Konsolidierung,
- semantischer Roundtrip,
- adaptive Caches ohne Popularitätsverzerrung,
- effizientes Routing in kleinen Datensätzen.

---

## 24. Empfohlene Zielarchitektur

### Übernehmen

- 8-Byte-NanoNeuron als kanonisches Ziel,
- ExecutionCells mit lokalen Indizes,
- 1-Byte-Hot-Projection,
- Shift- und Rotationsuniversen,
- Gate-, Masken- und Kontextverarbeitung,
- bedarfsgesteuerte Dimensionsauswertung,
- AnswerPoints,
- Experience Ledger,
- Speicherung von Erfolgen und Fehlschlägen,
- thematischer Hot-Skill-Cache,
- Trennung von AXIOM-Logik und SLM-Sprache.

### Verändern beziehungsweise präzisieren

- „Dimension“ als **AXIOM-Prozessdimension** definieren,
- „Paralleluniversum“ intern als UniverseSpec implementieren,
- „Quantenteleportation“ nur als Metapher verwenden,
- Häufigkeitsranking um Erfolg, Verifikation, Kosten und Risiko ergänzen,
- vollständige Antworten nicht vollständig aus dem RAM verbannen, sondern nur kurzlebig und nicht lernend behandeln,
- ExperienceCells nicht mit Originalerfahrungen verwechseln.

### Zunächst entfallen

- GPU-Backend,
- echtes Embedding-SLM,
- freie Technikselbstkomposition,
- verteilte Experience Stores,
- automatische Hardwaresteuerung,
- vollständige hochdimensionale Visualisierung.

### Empfohlene Reihenfolge

```text
Bitoperatoren
→ NanoNeuron-Projektion
→ UniverseSpec
→ AnswerPoint
→ TechniqueDefinition
→ ExperienceRecord
→ Experience-Suche
→ ExperienceCell
→ Hot-Skill-Cache
→ Template-Renderer
→ begrenztes SLM
→ GPU oder Spezialhardware
```

### Minimaler Prototyp mit größtem Erkenntnisgewinn

Ein lokales CPU-Programm mit:

- 256 möglichen Eingabebytes,
- acht Rotationen,
- acht logischen Shifts,
- vier Grundgattern,
- mehreren festen Masken,
- drei einfachen Techniken,
- einem append-only Experience Store,
- einer deterministischen Rangliste,
- Template-Ausgabe.

Beispielsituationen:

1. Muster vollständig erkannt.
2. Muster erst nach Rotation erkannt.
3. Maske passt, Kontext widerspricht.
4. häufige Technik scheitert an einem Gegenbeispiel.
5. seltene, aber besser verifizierte Technik gewinnt.
6. frühere Erfahrung beschleunigt eine zweite ähnliche Aufgabe.

---

## 25. Schlussfolgerung

Das AXIOM-NanoCell-Konzept verbindet kompakte Bitrepräsentation, deterministische Mehrfachauswertung, Mustererkennung und erfahrungsbasiertes Lernen zu einer eigenständigen Architektur.

Der entscheidende Gedanke besteht nicht darin, immer mehr neuronale Zustände zu speichern. Stattdessen wird ein kleiner kanonischer Zustand unter mehreren klar definierten Perspektiven geprüft. Jede Perspektive existiert nur lange genug, um einen nachweisbaren Ergebnisbeitrag zu liefern.

Aus einzelnen Universen entstehen AnswerPoints. Aus AnswerPoints entsteht eine semantische Konstellation. Aus angewandten Techniken entstehen Erfahrungen. Aus wiederholt bestätigten Erfahrungen entstehen Fähigkeiten.

Damit wird Lernen nicht als unsichtbare Veränderung eines großen Parameterraums verstanden, sondern als nachvollziehbarer Prozess:

```text
beobachten
→ Technik auswählen
→ ausführen
→ Ergebnis prüfen
→ Erfahrung speichern
→ Grenzen erkennen
→ Wissen verdichten
→ erneut anwenden
```

Das Konzept kann funktionieren, wenn die Projektmetaphern konsequent in kleine, überprüfbare technische Verträge übersetzt werden. Besonders wichtig sind der endgültige 8-Byte-Speichervertrag, das ExperienceRecord-Modell, die kontrollierte Dimensionsauswertung und die klare Trennung zwischen berechneter Bedeutung und sprachlicher Formulierung.

Der nächste entscheidende Schritt ist deshalb kein großes Sprachmodell und keine GPU-Optimierung. Er ist ein kleiner deterministischer Referenzprototyp, der beweist, dass ein NanoNeuron aus demselben Zustand mehrere virtuelle Aufgabenuniversen erzeugen, daraus AnswerPoints bilden und eine erfolgreiche oder gescheiterte Technik reproduzierbar als Erfahrung speichern kann.

---

# Festgelegte Bestandteile

- AXIOM als deterministische Prüf- und Verarbeitungsarchitektur.
- ExecutionCell als zusammenhängender NanoNeuron-Block.
- 16-Bit-IDs beziehungsweise lokale Indizes im MVP-Rahmen.
- ein kanonisches NanoNeuron soll acht Byte umfassen.
- aktive Verarbeitung soll möglichst mit einem Byte je NanoNeuron auskommen.
- Shift, Rotation, Gate, Maske und Kontext bilden alternative Aufgabenuniversen.
- Zwischenräume sollen berechnet und nicht dauerhaft materialisiert werden.
- Ergebnisse werden als kompakte Vektorpunkte beziehungsweise AnswerPoints dargestellt.
- mehrere AnswerPoints bilden die Grundlage einer Antwort.
- Erfahrungen werden persistent gespeichert.
- häufige und geeignete Techniken können in den aktiven Speicher geladen werden.
- AXIOM soll aus erfolgreichen und gescheiterten Anwendungen lernen.
- Lemmings dient nur als Funktionsmetapher.
- Quantenteleportation dient nur als philosophische Metapher.

# Abgeleitete Bestandteile

- Trennung zwischen 8-Byte-Kanon und 1-Byte-Hot-Projection.
- UniverseSpec als ausführbarer Vertrag eines Paralleluniversums.
- Technik als geordnete Kombination mehrerer Universen.
- ExperienceRecord als Situation–Technik–Ergebnis–Bewertung–Trace-Verbund.
- ExperienceCell als verdichtete Fähigkeit.
- Hot-Skill-Cache als themenbezogener RAM-Bereich.
- Roundtrip-Prüfung für SLM-Ausgaben.
- Speicherung von Gegenbeispielen und Gültigkeitsgrenzen.
- lazy Auswertung anstelle vollständiger Tensoren.
- Rangfolge nach mehreren Qualitätsmerkmalen statt reiner Nutzungshäufigkeit.

# Verwendete Annahmen

- nicht jedes Feld eines NanoNeurons muss direkt im 8-Byte-Core liegen.
- Seitentabellen sind für seltene Metadaten zulässig.
- AnswerPoints können diskret und typisiert statt als Floating-Point-Embeddings dargestellt werden.
- ein themenorientierter Decoder kann auf geschlossene AnswerPoint-Mengen begrenzt werden.
- vollständige Erfahrungen können auf SSD und verdichtete Fähigkeiten im RAM liegen.
- die erste Implementierung erfolgt auf klassischer CPU und RAM.

# Erkannte Konflikte

1. **1 Byte gegenüber 8 Byte je NanoNeuron:**  
   Auflösung durch aktive Projektion und kanonischen Datensatz.

2. **Keine Antwort im aktiven Speicher gegenüber notwendiger Ausgabeerzeugung:**  
   Antworten dürfen kurzzeitig in Registern oder Puffern existieren, aber nicht ungeprüft den dauerhaften Modellzustand verändern.

3. **Sehr große Dimensionen gegenüber begrenztem RAM:**  
   Prozessräume werden virtuell iteriert und nicht vollständig gespeichert.

4. **Lernen gegenüber Unveränderlichkeit:**  
   Neue Erfahrungen werden append-only gespeichert; kanonisches Wissen wird nicht still überschrieben.

5. **SLM-Ausgabe gegenüber deterministischer Wahrheit:**  
   Das SLM formuliert, während AXIOM Bedeutung und Nachweis vorgibt.

6. **Logikfamilie als Projektbegriff gegenüber elektrotechnischer Bedeutung:**  
   Intern besser Operator Family oder Gate Profile verwenden.

# Wichtigste offene Entscheidungen

- endgültiges 8-Byte-NanoNeuron-Layout,
- Definition der aktiven Projektion,
- binäres AnswerPoint-Format,
- ExperienceRecord-Schema,
- deterministische Situationsähnlichkeit,
- Ranking- und Promotion-Regeln,
- maximale Universums- und Technikkomplexität,
- Umgang mit Seitentabellen,
- Grenzen und Validierung eines späteren SLM-Renderers.

# Empfohlener nächster Schritt

Implementierung eines kleinen CPU-Referenzprototyps:

```text
8-Byte-NanoNeuron
→ 1-Byte-Projektion
→ Shift-/Rotate-/Gate-/Masken-Universen
→ AnswerPoints
→ drei feste Techniken
→ ExperienceRecord auf SSD
→ deterministischer Wiederabruf
→ Template-Ausgabe
```

Dieser Prototyp sollte zunächst ausschließlich die technische Kernfrage beantworten:

> Kann AXIOM aus kompakten NanoNeuron-Zuständen mehrere virtuelle Aufgabenperspektiven berechnen, deren Ergebnisse zu einer Technik verbinden und deren Erfolg oder Fehlschlag reproduzierbar als neue Erfahrung speichern?
