```markdown
# BRUCE NanoCell - Agenda und Roadmap

> BRUCE NanoCell ist eine deterministische CPU-RAM-NPU-Architektur aus kompakten NanoNeuronen und ExecutionCells. Aus einem kleinen kanonischen Zustand werden bei Bedarf virtuelle Aufgabenuniversen aus Shift, Rotation, Gate, Maske, Kontext und erlernten Techniken berechnet. Die Ergebnisse werden zu überprüfbaren AnswerPoints verdichtet, als Erfahrungen gespeichert und später als nachvollziehbare Fähigkeiten wiederverwendet. Diese Datei dient als zentrale Agenda, Roadmap, Aufgabenliste, Fortschrittsübersicht, Entscheidungsgrundlage und Dokumentationsnavigation.

## Dokumentstatus

| Feld | Wert |
|---|---|
| Projekt | BRUCE NanoCell |
| Dokumenttyp | Agenda und Roadmap |
| Version | 0.1 |
| Gesamtstatus | In Arbeit |
| Letzte Aktualisierung | 2026-07-27 |
| Hauptziel | Deterministische mehrdimensionale Erfahrungs- und Bedeutungsverarbeitung mit kompakten NanoNeuronen |
| Aktuelle Phase | Phase 0 - Architekturverträge und Begriffe festlegen |
| Referenzarchitektur | allgemeine CPU-RAM-NPU mit BRUCE als Referenzimplementierung |
| MVP-ID-Breite | 16 Bit |
| Kanonische NanoNeuron-Größe | 8 Byte |
| Vorgesehene aktive Projektion | möglichst 1 Byte je verarbeitetem NanoNeuron |
| Primäre Ausführungsplattform | klassische 64-Bit-CPU und RAM |
| Spätere Plattformen | SIMD, GPU, FPGA oder spezialisierte NPU |
| Dokumentstatus | lebendes Arbeitsdokument |

---

## Kurzbeschreibung

BRUCE NanoCell soll keine gewöhnliche Black-Box-KI werden, deren Wissen ausschließlich implizit in sehr vielen statistischen Modellparametern liegt. Das System soll Wissen, Regeln, Berechnungen, Erfahrungen und Entscheidungen möglichst explizit, reproduzierbar und prüfbar verarbeiten.

Ein NanoNeuron bleibt als kanonischer Datensatz acht Byte groß:

```text
1 NanoNeuron     =   8 Byte
10 NanoNeuronen  =  80 Byte
100 NanoNeuronen = 800 Byte
```

Für den schnellen Verarbeitungspfad soll nach Möglichkeit nur eine kompakte aktive Projektion geladen werden:

```text
1 NanoNeuron     =   1 Byte aktive Projektion
10 NanoNeuronen  =  10 Byte aktive Projektionen
100 NanoNeuronen = 100 Byte aktive Projektionen
```

Die acht Byte des kanonischen Datensatzes und das eine Byte der aktiven Verarbeitung sind daher keine konkurrierenden Größen, sondern zwei unterschiedliche Speicherverträge:

```text
8 Byte kanonischer NanoNeuron-Datensatz
                    ↓
1 Byte aktive Projektion im Hot Path
```

Aus dem aktiven Zustand erzeugt BRUCE mehrere deterministisch definierte Berechnungsperspektiven. Diese werden im Projekt metaphorisch als Paralleluniversen bezeichnet. Sie sind keine physikalischen Parallelwelten und keine quantenmechanische Superposition. Technisch handelt es sich um kurzlebige Aufgaben- und Berechnungszweige.

```text
NanoNeuron-Zustand
        ↓
Shift-, Rotations-, Gate-, Masken- und Kontextuniversen
        ↓
Muster- und Technikprüfung
        ↓
AnswerPoint
        ↓
semantische Konstellation
        ↓
strukturierte Antwort, Text, Formel oder Aktion
```

Die Werkzeuge Shift, Rotation, Gate, Maske und Kontext werden ähnlich wie spezielle Fähigkeiten in einem Aufgabenraum verwendet. Die Lemmings-Analogie beschreibt nur dieses Prinzip:

```text
Situation erkennen
→ passende Fähigkeit auswählen
→ Wirkung beobachten
→ Erfolg oder Fehlschlag bewerten
→ Erfahrung speichern
```

BRUCE erhält dabei keine Lemmings-orientierte Wissensbasis. BRUCE soll BRUCE-spezifische Techniken lernen, beispielsweise:

```text
Wenn Formelüberlauf,
dann Wertebereich prüfen
und eine exakte alternative Umformung versuchen.
```

```text
Wenn Wortbedeutung mehrdeutig,
dann Bedeutungskandidaten erzeugen
und gegen den Kontext prüfen.
```

```text
Wenn ein gespeicherter Wissenspfad wiederholt erfolgreich war,
dann seinen verifizierten Technikrang erhöhen.
```

Die vollständigen Erfahrungen werden vorzugsweise persistent auf SSD gespeichert. Häufige, erfolgreiche, verifizierte und thematisch passende Fähigkeiten können verdichtet in RAM oder Cache geladen werden.

> **BRUCE-Prinzip:** Nicht nur die Antwort speichern, sondern Situation, Technik, Ergebnis, Bewertung, Gültigkeitsbereich und Nachweis miteinander verbinden.

---

## Statuslegende

- [ ] Noch nicht begonnen
- [~] In Bearbeitung
- [x] Abgeschlossen
- [!] Blockiert
- [?] Unklar oder zu prüfen
- [-] Bewusst verworfen

Zusätzliche Kennzeichnungen:

- `[FESTGELEGT]` bestätigte Projektentscheidung
- `[ABGELEITET]` logisch aus mehreren Aussagen erschlossen
- `[ANNAHME]` erforderlich, aber noch nicht bestätigt
- `[VORSCHLAG]` empfohlene technische Ausgestaltung
- `[PRÜFEN]` technische oder organisatorische Prüfung erforderlich
- `[BLOCKIERT]` Aufgabe kann ohne vorherige Entscheidung nicht sinnvoll abgeschlossen werden
- `[SPÄTER]` nicht Teil des aktiven MVP-Arbeitsumfangs
- `[NICHT-ZIEL]` ausdrücklich nicht Bestandteil der aktuellen Zielarchitektur
- `[KONFLIKT]` widersprüchliche oder noch nicht kompatible Anforderungen
- `[NACHWEIS]` benötigt reproduzierbare Tests, Messungen oder dokumentierte Evidenz

---

## Agenda

1. [Projektgrundlage](#1-projektgrundlage)
   1. [Ausgangslage](#11-ausgangslage)
   2. [Hauptziel](#12-hauptziel)
   3. [Teilziele](#13-teilziele)
   4. [Nicht-Ziele](#14-nicht-ziele)
   5. [Rahmenbedingungen](#15-rahmenbedingungen)
   6. [Bestehende Projektbausteine](#16-bestehende-projektbausteine)
   7. [Quellen und fachliche Grundlagen](#17-quellen-und-fachliche-grundlagen)

2. [Anforderungen](#2-anforderungen)
   1. [Funktionale Anforderungen](#21-funktionale-anforderungen)
   2. [Nichtfunktionale Anforderungen](#22-nichtfunktionale-anforderungen)
   3. [Speicher- und Ressourcenanforderungen](#23-speicher-und-ressourcenanforderungen)
   4. [Sicherheitsanforderungen](#24-sicherheitsanforderungen)
   5. [Qualitätsanforderungen](#25-qualitätsanforderungen)
   6. [Dokumentationsanforderungen](#26-dokumentationsanforderungen)

3. [Architektur und Komponenten](#3-architektur-und-komponenten)
   1. [Leitprinzipien](#31-leitprinzipien)
   2. [Begriffe und Definitionen](#32-begriffe-und-definitionen)
   3. [Gesamtarchitektur](#33-gesamtarchitektur)
   4. [NanoNeuron und ExecutionCell](#34-nanoneuron-und-executioncell)
   5. [Aktive Projektion](#35-aktive-projektion)
   6. [Paralleluniversum und UniverseSpec](#36-paralleluniversum-und-universespec)
   7. [Bit- und Logikkomponenten](#37-bit--und-logikkomponenten)
   8. [Muster- und Cascade-Verarbeitung](#38-muster--und-cascade-verarbeitung)
   9. [Techniksystem](#39-techniksystem)
   10. [AnswerPoints und semantische Konstellation](#310-answerpoints-und-semantische-konstellation)
   11. [Erfahrungs- und Skill-System](#311-erfahrungs--und-skill-system)
   12. [Sprachrekonstruktion](#312-sprachrekonstruktion)
   13. [Trace- und Nachweissystem](#313-trace--und-nachweissystem)
   14. [Schnittstellen](#314-schnittstellen)
   15. [Gesamtdatenfluss](#315-gesamtdatenfluss)

4. [Datenmodell und Speicherverträge](#4-datenmodell-und-speicherverträge)
   1. [NanoNeuron-Datenmodell](#41-nanoneuron-datenmodell)
   2. [ExecutionCell-Datenmodell](#42-executioncell-datenmodell)
   3. [UniverseSpec-Datenmodell](#43-universespec-datenmodell)
   4. [TechniqueDefinition-Datenmodell](#44-techniquedefinition-datenmodell)
   5. [AnswerPoint-Datenmodell](#45-answerpoint-datenmodell)
   6. [ExperienceRecord-Datenmodell](#46-experiencerecord-datenmodell)
   7. [ExperienceCell-Datenmodell](#47-experiencecell-datenmodell)
   8. [Hot-Skill-Cache](#48-hot-skill-cache)
   9. [Persistente und flüchtige Daten](#49-persistente-und-flüchtige-daten)
   10. [Versionierung und Migration](#410-versionierung-und-migration)

5. [Entscheidungs- und Lernlogik](#5-entscheidungs--und-lernlogik)
   1. [Situationserkennung](#51-situationserkennung)
   2. [Technikauswahl](#52-technikauswahl)
   3. [Universumsauswertung](#53-universumsauswertung)
   4. [Ergebnisbewertung](#54-ergebnisbewertung)
   5. [Erfahrungsspeicherung](#55-erfahrungsspeicherung)
   6. [Erfahrungskonsolidierung](#56-erfahrungskonsolidierung)
   7. [Skill-Ranking](#57-skill-ranking)
   8. [Fehlversuche und Gegenregeln](#58-fehlversuche-und-gegenregeln)
   9. [Zustandsmodell](#59-zustandsmodell)

6. [Roadmap](#6-roadmap)
   1. [Phase 0 - Architekturverträge](#61-phase-0---architekturverträge)
   2. [Phase 1 - Bit-Universe-Prototyp](#62-phase-1---bit-universe-prototyp)
   3. [Phase 2 - ExecutionCell-Runtime](#63-phase-2---executioncell-runtime)
   4. [Phase 3 - AnswerPoint-System](#64-phase-3---answerpoint-system)
   5. [Phase 4 - Experience Ledger](#65-phase-4---experience-ledger)
   6. [Phase 5 - Erfahrungsabruf und Skill-Cache](#66-phase-5---erfahrungsabruf-und-skill-cache)
   7. [Phase 6 - Integration mit BRUCE-Kernsystemen](#67-phase-6---integration-mit-bruce-kernsystemen)
   8. [Phase 7 - Sprachrekonstruktion](#68-phase-7---sprachrekonstruktion)
   9. [Phase 8 - Optimierung und Härtung](#69-phase-8---optimierung-und-härtung)
   10. [Phase 9 - Referenzfreigabe](#610-phase-9---referenzfreigabe)
   11. [Meilensteine](#611-meilensteine)

7. [Abhängigkeiten](#7-abhängigkeiten)
   1. [Interne Architekturabhängigkeiten](#71-interne-architekturabhängigkeiten)
   2. [Entscheidungsabhängigkeiten](#72-entscheidungsabhängigkeiten)
   3. [Technische Abhängigkeiten](#73-technische-abhängigkeiten)
   4. [Integrationsabhängigkeiten](#74-integrationsabhängigkeiten)
   5. [Abhängigkeitsmatrix](#75-abhängigkeitsmatrix)

8. [Risiken und offene Fragen](#8-risiken-und-offene-fragen)
   1. [Bestätigte Konflikte](#81-bestätigte-konflikte)
   2. [Offene Entscheidungen](#82-offene-entscheidungen)
   3. [Risiken](#83-risiken)
   4. [Annahmen](#84-annahmen)
   5. [Bewusste Nicht-Ziele](#85-bewusste-nicht-ziele)

9. [Prüfung und Abnahme](#9-prüfung-und-abnahme)
   1. [Teststrategie](#91-teststrategie)
   2. [Unit-Tests](#92-unit-tests)
   3. [Integrationstests](#93-integrationstests)
   4. [End-to-End-Tests](#94-end-to-end-tests)
   5. [Negativ- und Grenzwerttests](#95-negativ--und-grenzwerttests)
   6. [Fehler-Injektion](#96-fehler-injektion)
   7. [Ressourcenmessungen](#97-ressourcenmessungen)
   8. [Gesamtabnahmekriterien](#98-gesamtabnahmekriterien)

10. [Sicherheit, Betrieb und Datenlebenszyklus](#10-sicherheit-betrieb-und-datenlebenszyklus)
    1. [Vertrauensmodell](#101-vertrauensmodell)
    2. [Berechtigungen und Capabilities](#102-berechtigungen-und-capabilities)
    3. [Installation und Initialisierung](#103-installation-und-initialisierung)
    4. [Regulärer Betrieb](#104-regulärer-betrieb)
    5. [Monitoring und Wartung](#105-monitoring-und-wartung)
    6. [Updates und Wiederherstellung](#106-updates-und-wiederherstellung)
    7. [Datenschutz und Aufbewahrung](#107-datenschutz-und-aufbewahrung)

11. [Dokumentation und Repository-Struktur](#11-dokumentation-und-repository-struktur)
    1. [Dokumentationsindex](#111-dokumentationsindex)
    2. [Vorgeschlagene Repository-Struktur](#112-vorgeschlagene-repository-struktur)
    3. [Architekturentscheidungen](#113-architekturentscheidungen)
    4. [Änderungs- und Statuspflege](#114-änderungs--und-statuspflege)

12. [Nächste konkrete Schritte](#12-nächste-konkrete-schritte)

13. [Erinnerungen und Arbeitsregeln](#13-erinnerungen-und-arbeitsregeln)

14. [Ideenparkplatz](#14-ideenparkplatz)

15. [Fortschrittsübersicht](#15-fortschrittsübersicht)

16. [BRUCE-Schlagwörter](#16-bruce-schlagwörter)

---

# 1. Projektgrundlage

Dieser Abschnitt beschreibt den fachlichen Ursprung, den aktuellen Projektstand und die Grenzen der NanoCell-Roadmap.

## 1.1 Ausgangslage

BRUCE ist als deterministische, nachvollziehbare Denk-, Routing-, Rechen- und Prüfarchitektur angelegt. Das Gesamtprojekt enthält bereits mehrere miteinander verwandte Teilsysteme:

- PeacemakerLLM für deterministische Wissens- und Relationsverarbeitung,
- BRUCE-Math für exakte Mathematik, Formelprüfung und Safe Shutdown,
- die CPU-RAM-NPU-Idee aus ExecutionCells und NanoNeuronen,
- Trace- und Nachweisprinzipien,
- Capability-orientierte Service-Zellen,
- thematische Wissens- und Technikbereiche,
- Roadmap-, Manifest- und Validierungsarbeit.

Der NanoCell-Teil soll diese Grundlagen in eine kompakte Verarbeitungseinheit überführen, die nicht alle möglichen Bedeutungen und Reaktionswege dauerhaft speichert. Stattdessen werden relevante Verarbeitungsperspektiven bei Bedarf berechnet.

### Bekannter konzeptioneller Stand

- [x] `[FESTGELEGT]` Die übergeordnete Einheit heißt ExecutionCell.
- [x] `[FESTGELEGT]` Eine ExecutionCell ist ein zusammenhängender Block von NanoNeuronen.
- [x] `[FESTGELEGT]` Der MVP verwendet 16-Bit-IDs beziehungsweise kompakte lokale Indizes.
- [x] `[FESTGELEGT]` Der Konzept-Scope ist eine allgemeine CPU-RAM-NPU mit BRUCE als Referenzimplementierung.
- [x] `[FESTGELEGT]` Ein NanoNeuron soll kanonisch acht Byte groß bleiben.
- [x] `[ABGELEITET]` Eine aktive Projektion kann ein Byte je NanoNeuron umfassen.
- [x] `[FESTGELEGT]` Shift, Rotation, Gate, Maske und Kontext bilden spezielle Aufgaben- und Berechnungsperspektiven.
- [x] `[FESTGELEGT]` Große Dimensionsräume sollen berechnet und nicht vollständig gespeichert werden.
- [x] `[FESTGELEGT]` Erfolgreiche und gescheiterte Technikanwendungen sollen als Erfahrungen gespeichert werden.
- [x] `[FESTGELEGT]` Ein themenorientierter Decoder soll AnswerPoints in Wort und Schrift übersetzen können.
- [ ] `[OFFEN]` Exaktes NanoNeuron-Byte-Layout festlegen.
- [ ] `[OFFEN]` Exaktes AnswerPoint-Format festlegen.
- [ ] `[OFFEN]` Deterministische Situationserkennung spezifizieren.
- [ ] `[OFFEN]` ExperienceRecord- und ExperienceCell-Verträge finalisieren.
- [ ] `[OFFEN]` Ausführbaren Referenzprototyp implementieren.

### Kleine Anleitung

1. Bestehende Entscheidungen nicht erneut als freie Ideen behandeln.
2. Offene Implementierungsdetails von festgelegten Architekturprinzipien trennen.
3. Neue Komponenten nur hinzufügen, wenn ihre Verantwortung klar von bestehenden Komponenten abgegrenzt werden kann.
4. Jede neue Annahme sichtbar markieren.
5. Jede Optimierung zuerst gegen einen einfachen CPU-Referenzpfad prüfen.

> **Erinnerung:** Das Projekt benötigt zuerst einen kleinen nachweisbaren Kern und keine sofortige Umsetzung sämtlicher Zukunftsideen.

### Abnahmekriterien

- [ ] Die Ausgangslage unterscheidet Konzept, vorhandene Komponenten und noch fehlende Implementierung.
- [ ] Alle verbindlichen Speicher- und Architekturentscheidungen sind sichtbar.
- [ ] Keine experimentelle Metapher wird als physikalische Tatsache dargestellt.
- [ ] Der aktive NanoCell-Arbeitsumfang ist vom gesamten BRUCE-Projekt abgegrenzt.

[Zurück zur Agenda](#agenda)

---

## 1.2 Hauptziel

Das Hauptziel ist die Entwicklung einer deterministischen NanoCell-Runtime, in der kompakte NanoNeuronen durch virtuelle Aufgabenuniversen unterschiedliche Techniken anwenden, Ergebnisse als AnswerPoints erzeugen und daraus nachvollziehbare Erfahrungen aufbauen.

```text
kompakter Zustand
→ mehrere deterministische Perspektiven
→ Muster- und Kontextprüfung
→ AnswerPoints
→ Ergebnis
→ ExperienceRecord
→ späterer Technikabruf
```

### Aufgaben

- [x] `[FESTGELEGT]` Hauptidee als kompakte deterministische Erfahrungsverarbeitung definieren.
- [ ] Hauptziel in eine testbare MVP-Spezifikation übersetzen.
- [ ] Minimalen End-to-End-Ablauf festlegen.
- [ ] Hauptziel mit PeacemakerLLM und BRUCE-Math abgleichen.
- [ ] Zielgrenze zwischen NanoCell-Kern und späterem SLM festlegen.
- [ ] Zielgrenze zwischen Erfahrungslernen und klassischem Deep Learning dokumentieren.

### Abnahmekriterien

- [ ] Das Hauptziel kann durch einen lokalen CPU-Prototyp geprüft werden.
- [ ] Der Prototyp benötigt weder GPU noch externes Sprachmodell.
- [ ] Jede ausgeführte Technik ist reproduzierbar.
- [ ] Eine Erfahrung kann gespeichert, geladen und erneut verwendet werden.
- [ ] Das System kann einen Fehlschlag von einer erfolgreichen Technik unterscheiden.

[Zurück zur Agenda](#agenda)

---

## 1.3 Teilziele

- [ ] 8-Byte-NanoNeuron-Vertrag festlegen.
- [ ] 1-Byte-Hot-Projection definieren.
- [ ] UniverseSpec für Shift, Rotation, Gate, Maske und Kontext definieren.
- [ ] lazy Universumsauswertung implementieren.
- [ ] Cascade-Filterung für frühe Kandidatenreduktion spezifizieren.
- [ ] Montessori-Shape-Matching als deterministische Maskenprüfung definieren.
- [ ] TechniqueDefinition als begrenzten Ablaufgraphen festlegen.
- [ ] AnswerPoint als typisierten Ergebnisvertrag spezifizieren.
- [ ] semantische Konstellationen aus 16 bis N AnswerPoints definieren.
- [ ] ExperienceRecord append-only speichern.
- [ ] Erfolgs-, Fehler-, Kosten- und Verifikationswerte trennen.
- [ ] ExperienceCells aus mehreren Erfahrungen reproduzierbar ableiten.
- [ ] themenorientierten Hot-Skill-Cache aufbauen.
- [ ] strukturierte Ausgabe vor generativer Sprachausgabe umsetzen.
- [ ] späteren SLM-Renderer durch semantischen Roundtrip begrenzen.
- [ ] CPU-Referenzimplementierung profilieren.
- [ ] spätere SIMD-, GPU- oder Hardwarepfade nur aus Messdaten ableiten.

[Zurück zur Agenda](#agenda)

---

## 1.4 Nicht-Ziele

- [x] `[NICHT-ZIEL]` keine physikalischen Paralleluniversen,
- [x] `[NICHT-ZIEL]` keine echte quantenmechanische Superposition,
- [x] `[NICHT-ZIEL]` keine echte Quantenteleportation,
- [x] `[NICHT-ZIEL]` keine Behauptung, ein Shift sei eine neue physikalische Dimension,
- [x] `[NICHT-ZIEL]` keine vollständige Materialisierung aller Dimensionskombinationen,
- [x] `[NICHT-ZIEL]` keine direkte Aktorsteuerung durch ein frei formulierendes SLM,
- [x] `[NICHT-ZIEL]` keine ungeprüfte Übernahme generierter Antworten als Wissen,
- [x] `[NICHT-ZIEL]` keine verpflichtende GPU-Abhängigkeit im MVP,
- [x] `[NICHT-ZIEL]` keine Lemmings-orientierte Wissensbasis,
- [x] `[NICHT-ZIEL]` keine ungeprüfte Selbstmodifikation von Technikgraphen,
- [x] `[NICHT-ZIEL]` keine absolute Sicherheits- oder Fehlerfreiheitsgarantie,
- [x] `[NICHT-ZIEL]` kein Ersatz sämtlicher klassischen Datenbanken, Compiler oder Betriebssysteme,
- [x] `[NICHT-ZIEL]` keine sofortige Umsetzung als Spezialchip.

> **Erinnerung:** Metaphern dürfen das Denken unterstützen, aber nicht die technische Abnahme ersetzen.

[Zurück zur Agenda](#agenda)

---

## 1.5 Rahmenbedingungen

### Festgelegte Rahmenbedingungen

| Bereich | Rahmen |
|---|---|
| Ausführung | deterministisch |
| Referenzplattform | 64-Bit-CPU und RAM |
| NanoNeuron | kanonisch 8 Byte |
| aktive Projektion | möglichst 1 Byte je NanoNeuron |
| ExecutionCell-ID | im MVP 16 Bit |
| Persistenz | vollständige Erfahrungen vorzugsweise auf SSD |
| RAM | thematisch relevante, verdichtete Skills |
| Lernmodell | Erfahrung speichern, bewerten, verdichten und wiederverwenden |
| Ausgabe | zunächst strukturiert oder templatebasiert |
| SLM | späterer Renderer, nicht primäre Wahrheitsquelle |
| Trace | verpflichtend für bestätigte Ergebnisse |
| Fehlerverhalten | kontrollierter Abbruch oder Safe Shutdown |

### Aufgaben

- [ ] maximale ExecutionCell-Größe für den MVP festlegen.
- [ ] maximale Universumsanzahl je Technik festlegen.
- [ ] maximales Rechenbudget je Technik definieren.
- [ ] Trace-Tiefen für Entwicklung, Test und Produktion definieren.
- [ ] SSD-Schreibstrategie definieren.
- [ ] RAM-Budget für Hot Skills festlegen.
- [ ] erlaubte Operatoren des MVP abschließend auswählen.

### Abnahmekriterien

- [ ] Alle Ressourcenlimits sind explizit konfigurierbar.
- [ ] Kein Technikgraph kann unbegrenzt wachsen oder laufen.
- [ ] Ein Ressourcenlimit erzeugt einen kontrollierten Status.
- [ ] Die Runtime kann ohne GPU betrieben werden.

[Zurück zur Agenda](#agenda)

---

## 1.6 Bestehende Projektbausteine

### PeacemakerLLM

Bekannte Funktion:

- deterministische Verarbeitung definierter Fragen,
- SQLite-basierte Wissensablage,
- Rule Engine,
- Direct Relation Engine,
- Trace Engine,
- Statusmodelle wie `FOUND`, `UNKNOWN`, `AMBIGUOUS` und `REQUIRES_REVIEW`.

Status:

- [x] `[BESTEHEND]` deterministische Relationsverarbeitung ist als Referenz vorhanden.
- [ ] NanoCell-AnswerPoints auf Peacemaker-Relationen abbilden.
- [ ] Peacemaker-Statusmodell mit NanoCell-Zuständen harmonisieren.
- [ ] vorhandene Trace-Verträge auf Universe- und Experience-Traces erweitern.

### BRUCE-Math

Bekannte Funktion:

- exakte BigInt- und Rational-Berechnung,
- modulare Arithmetik,
- Formelvalidierung,
- Domain Enforcement,
- deterministische Safe Shutdowns,
- Formula- und Benchmark-Traces.

Status:

- [x] `[BESTEHEND]` mathematische Referenzkomponenten sind vorhanden.
- [ ] Math-Operationen als NanoCell-Techniken referenzierbar machen.
- [ ] mathematische Fehlerklassen in ExperienceRecords abbilden.
- [ ] Formula-Trace mit Universe-Trace verbinden.
- [ ] Kosten- und Grenzwertformeln als Technikvoraussetzungen nutzbar machen.

### Service-Zellen und Capability-Architektur

Bekannte Funktion:

- eingefrorene Programme,
- minimale Rechte,
- explizite Capabilities,
- kontrollierte Gateway- und Broker-Grenzen.

Status:

- [x] `[BESTEHEND]` Capability-Prinzip ist als Architekturgrundsatz vorhanden.
- [ ] NanoCell-Runtime mit minimalen Capabilities beschreiben.
- [ ] Experience Store, Renderer und Aktoren in getrennte Vertrauenszonen einordnen.
- [ ] spätere Ausführung in Service-Zellen als Erweiterung prüfen.

### Repository- und Validierungsarbeit

- [x] `[BESTEHEND]` Repository-Manifeste, Architekturtexte und Validierungslogik sind Teil des Gesamtprojekts.
- [ ] NanoCell-Dokumente in Manifest und Dokumentationsindex aufnehmen.
- [ ] NanoCell-spezifische Validierungsregeln definieren.
- [ ] ADR-Duplikate und widersprüchliche Primärquellen verhindern.

> **Architektur:** NanoCell soll vorhandene BRUCE-Komponenten integrieren und nicht parallel dieselben Verantwortungen neu implementieren.

[Zurück zur Agenda](#agenda)

---

## 1.7 Quellen und fachliche Grundlagen

Die folgenden Quellen wurden als Ausgangspunkte für die Begriffs- und Architekturarbeit genannt:

1. [64-bit computing](https://en.wikipedia.org/wiki/64-bit_computing)
2. [Logische Verschiebung](https://de.wikipedia.org/wiki/Logische_Verschiebung)
3. [Bitweiser Operator](https://de.wikipedia.org/wiki/Bitweiser_Operator)
4. [Dimension](https://de.wikipedia.org/wiki/Dimension)
5. [Mustererkennung](https://de.wikipedia.org/wiki/Mustererkennung)
6. [Logikgatter](https://de.wikipedia.org/wiki/Logikgatter)
7. [Logische Verknüpfung](https://de.wikipedia.org/wiki/Logische_Verkn%C3%BCpfung)
8. [Logikfamilie](https://de.wikipedia.org/wiki/Logikfamilie)
9. [Logikbaustein](https://de.wikipedia.org/wiki/Logikbaustein)
10. [Mikrocontroller](https://de.wikipedia.org/wiki/Mikrocontroller)
11. [Quantenteleportation](https://de.wikipedia.org/wiki/Quantenteleportation)
12. [Lemmings](https://de.wikipedia.org/wiki/Lemmings)

### Aufgaben

- [ ] jede Quelle nach verwendbaren Definitionen prüfen.
- [ ] technische Definitionen von Projektmetaphern trennen.
- [ ] Quellenbegriffe in ein BRUCE-Glossar übertragen.
- [ ] Aussagen mit Implementierungsrelevanz kennzeichnen.
- [ ] Begriffe ohne direkte technische Relevanz nur als Inspiration dokumentieren.
- [ ] Quellenstand und Abrufdatum in der späteren Spezifikation erfassen.

> **Dokumentation:** Wikipedia-Links sind Ausgangspunkte zur Begriffsorientierung, aber kein Ersatz für Architekturtests, Primärquellen oder Implementierungsnachweise.

### Abnahmekriterien

- [ ] Jeder übernommene Begriff besitzt eine BRUCE-spezifische Definition.
- [ ] Keine Metapher wird unbemerkt als technischer Mechanismus behandelt.
- [ ] Implementierungsentscheidungen werden nicht allein mit einer Analogie begründet.

[Zurück zur Agenda](#agenda)

---

# 2. Anforderungen

## 2.1 Funktionale Anforderungen

### NanoNeuron und ExecutionCell

- [ ] Die Runtime kann einen kanonischen 8-Byte-NanoNeuron-Datensatz laden.
- [ ] Eine ExecutionCell kann mehrere NanoNeuronen zusammenhängend adressieren.
- [ ] Lokale NanoNeuron-IDs können im MVP mit 16 Bit dargestellt werden.
- [ ] Die Runtime kann aus einem NanoNeuron eine definierte aktive Projektion erzeugen.
- [ ] Match- und Miss-Ziele können deterministisch aufgelöst werden.
- [ ] ungültige Sprungziele werden kontrolliert abgelehnt.

### Aufgabenuniversen

- [ ] logischer Shift nach links implementieren.
- [ ] logischer Shift nach rechts implementieren.
- [ ] Rotation nach links implementieren.
- [ ] Rotation nach rechts implementieren.
- [ ] AND implementieren.
- [ ] OR implementieren.
- [ ] XOR implementieren.
- [ ] NOT implementieren.
- [ ] NAND, NOR und XNOR als spätere MVP-Erweiterung prüfen.
- [ ] Maskenprüfung implementieren.
- [ ] Kontextprüfung implementieren.
- [ ] Kombinationen nur über explizite UniverseSpecs ausführen.
- [ ] nicht definierte Kombinationen kontrolliert ablehnen.

### Musterverarbeitung

- [ ] gesetzte Bits zählen.
- [ ] Hamming-Distanz berechnen.
- [ ] Bitübergänge erkennen.
- [ ] zyklische Muster erkennen.
- [ ] Shape-Masken prüfen.
- [ ] Positiv- und Negativmuster unterscheiden.
- [ ] Cascade-Filter für frühe Ablehnung implementieren.
- [ ] Musterfunktionen über stabile Funktions-IDs referenzieren.

### Technikverarbeitung

- [ ] Techniken als geordnete Operationen oder begrenzten Graphen definieren.
- [ ] Technikvoraussetzungen prüfen.
- [ ] Pflichtuniversen und optionale Universen unterscheiden.
- [ ] Abbruchbedingungen definieren.
- [ ] Ausführungskosten erfassen.
- [ ] alternative Techniken vergleichen.
- [ ] bekannte Gegenbeispiele berücksichtigen.

### AnswerPoints und Ausgabe

- [ ] jedes gültige Universumsergebnis in einen AnswerPoint projizieren.
- [ ] ungültige oder unvollständige Punkte ablehnen.
- [ ] 16 bis N AnswerPoints zu einer Konstellation kombinieren.
- [ ] Widersprüche zwischen Punkten erkennen.
- [ ] strukturierte Ausgabe erzeugen.
- [ ] templatebasierte Textausgabe erzeugen.
- [ ] späteren SLM-Renderer nur mit geschlossener Eingabestruktur betreiben.

### Erfahrungslernen

- [ ] Situation, Technik, Ergebnis und Bewertung gemeinsam speichern.
- [ ] Erfolge und Fehlschläge getrennt klassifizieren.
- [ ] vollständigen Trace referenzieren.
- [ ] frühere Erfahrungen anhand einer SituationSignature suchen.
- [ ] ähnliche Erfahrungen reproduzierbar gruppieren.
- [ ] ExperienceCells aus bestätigten Erfahrungen ableiten.
- [ ] Hot Skills themenbezogen laden.
- [ ] Skill-Rankings aktualisieren.
- [ ] Popularitäts-Bias erkennen und begrenzen.
- [ ] neue Erfahrungen nicht ungeprüft als Wissen aktivieren.

[Zurück zur Agenda](#agenda)

---

## 2.2 Nichtfunktionale Anforderungen

- [ ] identische Eingaben und Versionen erzeugen identische Ergebnisse.
- [ ] jede bestätigte Ausgabe besitzt einen Trace.
- [ ] die Runtime arbeitet ohne Netzwerkzugriff.
- [ ] der CPU-Referenzpfad ist vollständig funktionsfähig.
- [ ] temporäre Universen werden nach der Verarbeitung freigegeben.
- [ ] Speicherbudgets sind konfigurierbar.
- [ ] Laufzeitbudgets sind konfigurierbar.
- [ ] Fehler bleiben lokal begrenzt.
- [ ] Datenformate sind versioniert.
- [ ] die Runtime kann nach einem kontrollierten Fehler weiterarbeiten.
- [ ] Hot-Skill-Caches sind aus persistenten Daten rekonstruierbar.
- [ ] produktive Ausführung benötigt keine nicht deterministischen Zufallsentscheidungen.
- [ ] optionaler Parallelismus verändert das logische Ergebnis nicht.
- [ ] Dokumentation und Implementierung verwenden dieselben Begriffe.

### Kleine Anleitung

1. Für jede nichtfunktionale Anforderung einen prüfbaren Test definieren.
2. Formulierungen wie „schnell“, „klein“ oder „sicher“ vermeiden.
3. Stattdessen konkrete Verträge, Limits oder Vergleichsmessungen festlegen.
4. Noch unbekannte Zielwerte als `[OFFEN]` markieren.

[Zurück zur Agenda](#agenda)

---

## 2.3 Speicher- und Ressourcenanforderungen

### Kanonischer Speicher

```text
M_canonical = 8 × Anzahl_NanoNeuronen
```

### Aktive Projektion

```text
M_active = Projektionsgröße × Anzahl_aktiver_NanoNeuronen
```

Bei einer 1-Byte-Projektion:

```text
M_active = 1 × Anzahl_aktiver_NanoNeuronen
```

### Virtuelle Prozessräume

Die Anzahl theoretischer Kombinationen kann als Produkt der Achsen betrachtet werden:

```text
Universen =
Shift-Varianten
× Rotationsvarianten
× Gate-Varianten
× Masken
× Kontexte
× Technikvarianten
```

Die vollständige Kombination darf nicht automatisch materialisiert werden.

### Projektinterne Dimensionsdarstellung

```text
Layer 0:
1 Byte = 8 Bit

Layer 1:
8 Bitpositionen oder bis zu 8 Shift- beziehungsweise Rotationsperspektiven

Layer 2:
8 × 8 = 64 kombinierte Perspektiven

Layer 3:
8 × 8 × 8 = 512 kombinierte Perspektiven

Cube-to-Cube:
512 × 512 = 262.144 Beziehungen
```

> **Präzisierung:** Diese Layer sind BRUCE-Prozessräume. Sie sind nicht automatisch identisch mit mathematisch unabhängigen Raumdimensionen.

### Aufgaben

- [ ] `[BLOCKIERT]` maximale aktive NanoNeuron-Anzahl festlegen.
  - Grund: repräsentative Workloads fehlen.
  - Benötigte Entscheidung: Zielgröße für den CPU-MVP.

- [ ] maximales Scratchpad je Technik festlegen.
- [ ] maximale AnswerPoint-Anzahl festlegen.
- [ ] maximale Trace-Größe je Ausführung festlegen.
- [ ] maximale ExperienceRecord-Größe festlegen.
- [ ] SSD-Batch- und Flush-Strategie festlegen.
- [ ] Cache-Budget je Thema definieren.
- [ ] Messpunkte für CPU-Zyklen, RAM, SSD-Schreibvolumen und Cache-Misses festlegen.

### Abnahmekriterien

- [ ] Kein Prozessraum wird ohne ausdrückliche Anforderung vollständig materialisiert.
- [ ] Ressourcenüberschreitung erzeugt einen kontrollierten Status.
- [ ] Speicherverbrauch kann je ExecutionCell gemessen werden.
- [ ] SSD-Schreibvolumen kann je Erfahrung gemessen werden.
- [ ] der aktive Cache kann vollständig verworfen und rekonstruiert werden.

[Zurück zur Agenda](#agenda)

---

## 2.4 Sicherheitsanforderungen

- [ ] externe Eingaben als nicht vertrauenswürdig behandeln.
- [ ] TechniqueDefinitions vor Ausführung validieren.
- [ ] zirkuläre oder unbegrenzte Technikgraphen ablehnen.
- [ ] ExperienceRecords append-only speichern.
- [ ] nachträgliche Bewertungen als separate Datensätze speichern.
- [ ] Cache-Promotion nur aus verifizierten Daten erlauben.
- [ ] Renderer von direkter Aktorsteuerung trennen.
- [ ] SLM-Ausgaben semantisch zurückprüfen.
- [ ] kritische Aktionen zusätzlich autorisieren.
- [ ] manipulierte oder beschädigte Erfahrungen quarantänisieren.
- [ ] Versionen und Herkunft jeder Technik protokollieren.
- [ ] Ressourcenlimits fail-closed behandeln.
- [ ] Geheimnisse nicht in AnswerPoints oder Traces schreiben.
- [ ] Service- und Speicherkomponenten nach Least Privilege betreiben.
- [ ] Wiederherstellungs- und Rollback-Pfade testen.

> **Sicherheit:** Ein hoher Skill-Rang ist keine Berechtigung und kein Sicherheitsnachweis.

[Zurück zur Agenda](#agenda)

---

## 2.5 Qualitätsanforderungen

- [ ] jede Komponente besitzt genau beschriebene Verantwortung.
- [ ] jede Komponente dokumentiert Eingaben und Ausgaben.
- [ ] jede Komponente definiert Fehlerzustände.
- [ ] jede Komponente besitzt Unit-Tests.
- [ ] jede kritische Schnittstelle besitzt Integrations- und Negativtests.
- [ ] jede Architekturentscheidung ist als ADR oder gleichwertiger Entscheidungsnachweis dokumentiert.
- [ ] keine Aufgabe gilt ohne Abnahmekriterium als abgeschlossen.
- [ ] keine Erfahrung gilt allein wegen häufiger Nutzung als verifiziert.
- [ ] keine Konsolidierung darf Originalfälle unauffindbar machen.
- [ ] alle Benchmarks nennen Hardware, Version und Testdaten.
- [ ] Primärquellen und importierte Dokumente werden nicht still verändert.

[Zurück zur Agenda](#agenda)

---

## 2.6 Dokumentationsanforderungen

- [ ] Begriffe in einem zentralen Glossar pflegen.
- [ ] Architekturdiagramm und Datenfluss aktuell halten.
- [ ] Speicherverträge separat dokumentieren.
- [ ] UniverseSpec und TechniqueDefinition mit Beispielen dokumentieren.
- [ ] ExperienceRecord-Lebenszyklus dokumentieren.
- [ ] Fehlerklassen und Safe-Shutdown-Verhalten dokumentieren.
- [ ] Testmatrix pflegen.
- [ ] offene Entscheidungen mit stabilen IDs versehen.
- [ ] Risiken mit Gegenmaßnahmen pflegen.
- [ ] Roadmap nach jeder abgeschlossenen Phase aktualisieren.
- [ ] Projektstatus und Manifest synchron halten.
- [ ] experimentelle Metaphern klar kennzeichnen.

[Zurück zur Agenda](#agenda)

---

# 3. Architektur und Komponenten

## 3.1 Leitprinzipien

### Determinismus

Gleiche Eingaben, Versionen und Ressourcenverträge müssen zu gleichen logischen Ergebnissen führen.

### Berechnen statt materialisieren

Virtuelle Prozessräume werden nur an benötigten Koordinaten ausgewertet.

### Kanonische Daten und flüchtige Perspektiven

Das gespeicherte NanoNeuron bleibt unverändert. Universen erzeugen temporäre Ansichten.

### Erfahrung statt bloßer Antwort

BRUCE speichert den Zusammenhang zwischen Situation, Technik, Ergebnis und Bewertung.

### Keine Behauptung ohne Trace

Jede bestätigte Aussage benötigt eine nachvollziehbare Herkunft.

### Trennung von Bedeutung und Formulierung

BRUCE berechnet die Aussage. Der Renderer formuliert sie.

### Fehler als Wissen

Fehlversuche definieren Grenzen und Gegenbeispiele.

### Themenorientierte Aktivierung

RAM wird mit aktuell relevanten und nicht nur global häufigen Skills belegt.

### Capability-Prinzip

Jede Komponente erhält nur ausdrücklich benötigte Fähigkeiten.

### Fail-Closed

Bei fehlenden Pflichtinformationen oder Sicherheitsnachweisen wird keine kritische Aktion freigegeben.

### Aufgaben

- [ ] Leitprinzipien als verbindliche Architekturregeln bestätigen.
- [ ] jedes Prinzip einem oder mehreren Tests zuordnen.
- [ ] Konflikte zwischen Leistung und Nachvollziehbarkeit dokumentieren.
- [ ] Abweichungen nur über ADR erlauben.

[Zurück zur Agenda](#agenda)

---

## 3.2 Begriffe und Definitionen

| Begriff | Bedeutung | Abgrenzung |
|---|---|---|
| NanoNeuron | kleinste kompakte Verarbeitungseinheit | kein biologisches oder gewöhnliches gewichtsbasiertes Neuron |
| ExecutionCell | zusammenhängender Block von NanoNeuronen | keine einzelne Zelle |
| aktive Projektion | für eine Aufgabe geladener kompakter Ausschnitt | nicht der vollständige Datensatz |
| Prozessdimension | Achse alternativer Berechnungsparameter | nicht automatisch mathematische Dimension |
| Paralleluniversum | isolierter deterministischer Berechnungszweig | keine physikalische Parallelwelt |
| UniverseSpec | ausführbare Beschreibung eines Universums | nicht das berechnete Ergebnis |
| Gate | boolescher Operator | keine vollständige Technik |
| Maske | Auswahl relevanter Bits oder Formen | keine semantische Aussage allein |
| Kontext | explizite fachliche Umgebung einer Berechnung | keine unsichtbare globale Variable |
| Technik | geordnete Kombination von Werkzeugen und Regeln | mehr als ein einzelner Operator |
| SituationSignature | normalisierte Problemsignatur | nicht die vollständige Rohdatenkopie |
| AnswerPoint | typisierter Ergebnisbeitrag | nicht zwingend Floating-Point-Embedding |
| semantische Konstellation | geordnete Menge mehrerer AnswerPoints | noch keine freie Textantwort |
| ExperienceRecord | konkrete dokumentierte Technikanwendung | keine verallgemeinerte Regel |
| ExperienceCell | verdichtete Erfahrungsklasse | Originalfälle bleiben erhalten |
| Hot Skill | aktiv geladene, thematisch passende Fähigkeit | nicht automatisch globales Wissen |
| Renderer | erzeugt Text oder Ausgabeform | darf Wahrheit nicht neu bestimmen |
| Semantic State Reconstruction | Ausgabe aus Bedeutungspunkten rekonstruieren | keine echte Quantenteleportation |

### Aufgaben

- [ ] Glossar als eigenes Dokument anlegen.
- [ ] jeden Begriff mit gültigen und ungültigen Beispielen ergänzen.
- [ ] Benennung von `Paralleluniversum` und internem `UniverseSpec` verbindlich trennen.
- [ ] den Begriff `Logikfamilie` für Hardwarekontexte reservieren.
- [ ] für BRUCE stattdessen `Operator Family`, `Gate Profile` oder `Technique Family` verwenden.

[Zurück zur Agenda](#agenda)

---

## 3.3 Gesamtarchitektur

```text
Benutzereingabe, Formel, Sensorwert oder interner Auftrag
                         ↓
               Eingabevalidierung
                         ↓
              Situation Analyzer
                         ↓
               SituationSignature
                         ↓
       Experience Index und Technique Registry
                         ↓
                Technique Selector
                         ↓
          ExecutionCell und NanoNeuron Store
                         ↓
             Active Projection Loader
                         ↓
                Universe Generator
        ┌────────────────┼────────────────┐
        ↓                ↓                ↓
      Shift           Rotation          Gate
        ↓                ↓                ↓
      Maske          Pattern Engine    Kontext
        └────────────────┼────────────────┘
                         ↓
                  Cascade Filter
                         ↓
                AnswerPoint Projector
                         ↓
         Semantic Constellation Builder
                         ↓
        ┌────────────────┴────────────────┐
        ↓                                 ↓
Structured Output Engine              Renderer
        ↓                                 ↓
Aktor oder Datenausgabe        Semantic Roundtrip Validator
        └────────────────┬────────────────┘
                         ↓
                    Endergebnis
                         ↓
                 Experience Ledger
                         ↓
             Evaluation und Consolidation
                         ↓
                  Hot-Skill-Cache
```

### Aufgaben

- [ ] Gesamtarchitektur als verbindliches Diagramm dokumentieren.
- [ ] Verantwortungsgrenzen jeder Komponente bestätigen.
- [ ] direkte Nebenwege ohne Trace identifizieren und verbieten.
- [ ] Datenfluss zwischen Runtime und Experience Store definieren.
- [ ] Renderer als separate Vertrauenszone dokumentieren.
- [ ] Aktorpfad von Textausgabe trennen.

### Abnahmekriterien

- [ ] Jede Pfeilverbindung besitzt einen dokumentierten Datenvertrag.
- [ ] Keine Komponente besitzt überlappende Hauptverantwortung.
- [ ] Persistente und flüchtige Komponenten sind getrennt.
- [ ] Kontroll- und Datenpfad sind unterscheidbar.
- [ ] Fehler können einer konkreten Komponente zugeordnet werden.

[Zurück zur Agenda](#agenda)

---

## 3.4 NanoNeuron und ExecutionCell

### NanoNeuron

Der kanonische NanoNeuron-Datensatz soll acht Byte umfassen. Logisch werden folgende Angaben benötigt:

- Shape-Maske,
- Shape-Wert,
- Funktions-ID,
- Tabellen-ID,
- Ausgangs-Slot,
- Match-Ziel,
- Miss-Ziel,
- Flags,
- lokale Identifikation.

### Konflikt

Eine naive Speicherung aller Felder mit je einem oder zwei Byte kann die Acht-Byte-Grenze überschreiten.

### Mögliche Auflösungen

1. lokale ID aus Position ableiten,
2. relative Match- und Miss-Ziele verwenden,
3. seltene Felder in Seitentabellen auslagern,
4. mehrere feste NanoNeuron-Profile definieren,
5. bestimmte Felder bitweise packen.

### ExecutionCell

Eine ExecutionCell stellt einen zusammenhängenden Block bereit:

```text
ExecutionCell
├─ Header
├─ NanoNeuron 0
├─ NanoNeuron 1
├─ NanoNeuron 2
├─ ...
├─ Seitentabellen
└─ Integritäts- und Versionsdaten
```

### Aufgaben

- [ ] `[BLOCKIERT]` endgültiges 8-Byte-Layout festlegen.
  - Grund: repräsentative Operationen und Zugriffshäufigkeiten müssen verglichen werden.

- [ ] lokale ID aus der Position prüfen.
- [ ] relative Sprungziele prüfen.
- [ ] Seitentabellenmodell definieren.
- [ ] Header-Größe festlegen.
- [ ] Cell-Version und Layout-Version trennen.
- [ ] Integritätsprüfung definieren.
- [ ] maximale Cell-Größe im 16-Bit-MVP bestimmen.
- [ ] Alignment und Endianness festlegen.
- [ ] lesbare Debug-Darstellung definieren.

> **Erinnerung:** Das 8-Byte-Ziel ist ein Speichervertrag, aber kein Grund, kritische Informationen unprüfbar zu komprimieren.

[Zurück zur Agenda](#agenda)

---

## 3.5 Aktive Projektion

Die aktive Projektion enthält nur die für eine konkrete Technik benötigten Felder.

Mögliche Projektionen:

```text
STATE_BYTE
SHAPE_BYTE
GATE_BYTE
FLAGS_BYTE
PACKED_ACTIVATIONS
CUSTOM_TECHNIQUE_VIEW
```

### Zielbild

```text
8-Byte-NanoNeuron
        ↓
Projection Type auswählen
        ↓
1 Byte oder kompakter Arbeitswert
        ↓
CPU-Register, Cache oder RAM
```

### Aufgaben

- [ ] erste Projektion `STATE_BYTE` definieren.
- [ ] Projektionstyp im Technique-Vertrag speichern.
- [ ] Projektionserzeugung deterministisch testen.
- [ ] Projektionen als read-only behandeln.
- [ ] Kopien und Scratchpads getrennt markieren.
- [ ] Projektionslebensdauer definieren.
- [ ] Projektionen nach Technikende verwerfen.
- [ ] Messung des Projektionsspeichers implementieren.

### Abnahmekriterien

- [ ] Quell-NanoNeuron bleibt unverändert.
- [ ] Projektion kann aus der Quelle erneut identisch erzeugt werden.
- [ ] ungültiger Projektionstyp wird abgelehnt.
- [ ] Projektion referenziert Quell- und Layout-Version.

[Zurück zur Agenda](#agenda)

---

## 3.6 Paralleluniversum und UniverseSpec

Ein Paralleluniversum ist technisch eine konkrete Kombination aus Eingabeprojektion, Transformation, Gate, Maske, Kontext und Ausgaberegel.

```text
UniverseSpec
├─ Universe-ID
├─ Version
├─ Projection Type
├─ Transform Type
├─ Transform Parameter
├─ Gate-ID
├─ Mask-ID
├─ Context-ID
├─ Input Bindings
├─ Output Projector
├─ Cost Limit
└─ Abort Policy
```

### Beispiel

```text
Universe-ID: 0042
Projection: STATE_BYTE
Transform: ROTATE_LEFT
Amount: 3
Gate: XOR
Mask: SHAPE_17
Context: WORD_SENSE_VEHICLE
Output: ANSWER_POINT_V1
```

### Aufgaben

- [ ] UniverseSpec-Schema definieren.
- [ ] Pflichtfelder und `NONE`-Werte festlegen.
- [ ] Universe-ID und Version trennen.
- [ ] maximale Transformationskette begrenzen.
- [ ] Abbruchbedingungen definieren.
- [ ] Kostenbudget integrieren.
- [ ] UniverseSpec vor Ausführung validieren.
- [ ] einfache textuelle Debug-Darstellung erstellen.
- [ ] deterministische Serialisierung festlegen.

### Abnahmekriterien

- [ ] identische UniverseSpecs erzeugen identische Ausführungspläne.
- [ ] jede Operation ist im Trace sichtbar.
- [ ] nicht unterstützte Kombinationen werden abgelehnt.
- [ ] ein Universum kann ohne vollständigen Dimensionsspeicher ausgewertet werden.

[Zurück zur Agenda](#agenda)

---

## 3.7 Bit- und Logikkomponenten

### Shift Engine

- logischer Shift links,
- logischer Shift rechts,
- definierter Wertebereich,
- verlustbehaftete Perspektive.

### Rotation Engine

- Rotation links,
- Rotation rechts,
- zyklische Neuordnung,
- keine verlorenen Bits innerhalb der Wortbreite.

### Gate Engine

MVP-Gates:

- AND,
- OR,
- XOR,
- NOT.

Spätere Gates:

- NAND,
- NOR,
- XNOR.

### Mask Engine

Grundprüfung:

```text
(Input AND Mask) == ShapeValue
```

### Context Engine

Prüft:

- Thema,
- Domäne,
- Relation,
- Zeit,
- Ziel,
- Sicherheitsklasse,
- bekannte Einschränkungen.

### Aufgaben

- [ ] Operatorverträge definieren.
- [ ] Eingabebreite explizit festlegen.
- [ ] Verhalten bei Shift um 0 definieren.
- [ ] Verhalten bei Shift größer oder gleich Wortbreite definieren.
- [ ] Rotationsnormalisierung definieren.
- [ ] Wahrheitstabellen für Gates dokumentieren.
- [ ] Maskenformat festlegen.
- [ ] Kontextformat festlegen.
- [ ] alle Operatoren mit bekannten Testvektoren prüfen.
- [ ] Operatorversionen im Trace speichern.

[Zurück zur Agenda](#agenda)

---

## 3.8 Muster- und Cascade-Verarbeitung

### Pattern Engine

Mögliche Merkmale:

- Popcount,
- Hamming-Distanz,
- 0-zu-1-Übergänge,
- 1-zu-0-Übergänge,
- zusammenhängende Bitfolgen,
- Symmetrie,
- Wiederholung,
- zyklische Ähnlichkeit,
- positive Shape-Übereinstimmung,
- negative Shape-Übereinstimmung.

### Montessori-Formenbrett-Prinzip

```text
Shape-Maske = Öffnung
Shape-Wert  = erwartete Form
Eingabe     = einzusetzender Baustein
Match       = Form passt
Miss        = Form passt nicht
```

Die Maske darf nur eine strukturelle Passung bestätigen. Semantische Bedeutung benötigt zusätzlichen Kontext.

### Cascade-Filterung

```text
Themenfilter
    ↓
grober Strukturfilter
    ↓
Shape-Maske
    ↓
Gate-Prüfung
    ↓
Kontextprüfung
    ↓
Erfahrungsabgleich
    ↓
vollständige Technikbewertung
```

### Aufgaben

- [ ] Merkmalsfunktionen priorisieren.
- [ ] jede Funktion mit stabiler ID versehen.
- [ ] Kostenklasse je Funktion festlegen.
- [ ] Filterreihenfolge definieren.
- [ ] frühe Abbruchregeln definieren.
- [ ] Falsch-Positiv- und Falsch-Negativ-Fälle testen.
- [ ] Shape-Match und Semantik-Match getrennt protokollieren.
- [ ] Filterwirkung messen.

> **BRUCE-Prinzip:** Erst den Suchraum reduzieren, danach teure Detailprüfungen ausführen.

[Zurück zur Agenda](#agenda)

---

## 3.9 Techniksystem

Eine Technik ist ein kleines deterministisches Programm:

```text
TechniqueDefinition
├─ Technique-ID
├─ Version
├─ Voraussetzungen
├─ Universe-Graph
├─ Pflichtprüfungen
├─ optionale Prüfungen
├─ Ablehnungsregeln
├─ Erfolgskriterien
├─ Kostenbudget
├─ Sicherheitsklasse
└─ Herkunft
```

### Beispielhafte BRUCE-Techniken

```text
WORD_SENSE_DISAMBIGUATION
```

```text
FORMULA_DOMAIN_RECOVERY
```

```text
DIRECT_RELATION_LOOKUP
```

```text
EXPERIENCE_GUIDED_ROUTE_SELECTION
```

```text
SEMANTIC_OUTPUT_VALIDATION
```

### Aufgaben

- [ ] Technik als DAG oder begrenzte Zustandsmaschine auswählen.
- [ ] Schleifen im MVP verbieten oder strikt begrenzen.
- [ ] Technikvoraussetzungen formalisieren.
- [ ] Pflicht- und optionale Universen definieren.
- [ ] Erfolg, Fehlschlag und Abbruch unterscheiden.
- [ ] Technikherkunft dokumentieren.
- [ ] neue Technikversionen unveränderlich speichern.
- [ ] veraltete Techniken als `SUPERSEDED` markieren.
- [ ] mindestens drei MVP-Techniken definieren.

### Abnahmekriterien

- [ ] jede Technik besitzt klaren Eingabe- und Ausgabevertrag.
- [ ] jede Technik kann einzeln getestet werden.
- [ ] jede Technik kann kontrolliert abgebrochen werden.
- [ ] keine Technik verändert kanonisches Wissen direkt.

[Zurück zur Agenda](#agenda)

---

## 3.10 AnswerPoints und semantische Konstellation

### AnswerPoint

Ein AnswerPoint ist ein kompakter, typisierter Ergebnisbeitrag.

```text
AnswerPoint
├─ Type
├─ Topic-ID
├─ Concept-ID
├─ Relation-ID
├─ Value
├─ Polarity
├─ Evidence Class
├─ Universe-ID
└─ Trace Reference
```

### Semantische Konstellation

Mehrere Punkte ergeben gemeinsam eine Aussage:

```text
Punkt 01: Thema
Punkt 02: Subjekt
Punkt 03: Relation
Punkt 04: Objekt
Punkt 05: Polarität
Punkt 06: Zeit
Punkt 07: Gültigkeitsbereich
Punkt 08: Sicherheit
...
```

Für komplexere Ausgaben werden 16 bis N Punkte verwendet. Die Zahl 16 ist zunächst ein Architekturvorschlag und kein nachgewiesenes universelles Minimum.

### Aufgaben

- [ ] AnswerPoint-Typen definieren.
- [ ] Pflichtfelder bestimmen.
- [ ] kompakte und ausführliche Darstellung trennen.
- [ ] Polarität und Negation explizit speichern.
- [ ] Evidenzklassen definieren.
- [ ] Punktreihenfolge und Dublettenregeln festlegen.
- [ ] Konfliktprüfung implementieren.
- [ ] Mindest- und Maximalgröße einer Konstellation prüfen.
- [ ] strukturierte Ausgabe aus Konstellationen erzeugen.

### Abnahmekriterien

- [ ] kein AnswerPoint kann ohne Trace als bestätigt gelten.
- [ ] widersprüchliche Pflichtpunkte verhindern die Freigabe.
- [ ] Negationen bleiben bei jeder Transformation erhalten.
- [ ] strukturierte Ausgabe ist ohne SLM möglich.

[Zurück zur Agenda](#agenda)

---

## 3.11 Erfahrungs- und Skill-System

### ExperienceRecord

```text
Situation
+ ausgewählte Technik
+ betrachtete Alternativen
+ Universumstraces
+ AnswerPoints
+ Ergebnis
+ Bewertung
+ Versionen
= ExperienceRecord
```

### ExperienceCell

Eine ExperienceCell verdichtet mehrere vergleichbare Erfahrungen:

```text
ExperienceCell
├─ Situation Cluster
├─ bevorzugte Techniken
├─ Gültigkeitsbereich
├─ positive Fälle
├─ negative Fälle
├─ ungeklärte Fälle
├─ Kostenprofil
├─ Verifikationsklasse
└─ Quell-Experience-IDs
```

### Hot Skill

Ein Hot Skill ist eine aktiv geladene Technik oder ExperienceCell, die zum aktuellen Thema passt.

### Aufgaben

- [ ] ExperienceRecord append-only definieren.
- [ ] EvaluationRecord separat definieren.
- [ ] positive, negative und ungeklärte Fälle unterscheiden.
- [ ] ExperienceCell-Ableitung spezifizieren.
- [ ] Originalfälle dauerhaft referenzierbar halten.
- [ ] Hot-Skill-Cache themenorientiert definieren.
- [ ] Cache-Promotion und -Verdrängung spezifizieren.
- [ ] Wiederanlauf nach Cache-Verlust testen.
- [ ] SSD- und RAM-Rollen dokumentieren.

### Speicherprinzip

```text
SSD
├─ vollständige ExperienceRecords
├─ Fehlversuche
├─ Verifikationen
├─ Technikversionen
└─ langfristige Traces

RAM
├─ aktive Situation
├─ thematisch passende ExperienceCells
├─ Top-Techniken
├─ kompakte Indizes
└─ aktuelle Statistiken
```

[Zurück zur Agenda](#agenda)

---

## 3.12 Sprachrekonstruktion

### Architekturgrenze

```text
BRUCE berechnet Bedeutung.
Renderer formuliert Bedeutung.
Validator prüft Formulierung.
```

### Stufe 1

Deterministische Templates.

### Stufe 2

Regelbasierter Satzbau.

### Stufe 3

Themenorientiertes SLM mit geschlossener Eingabe.

### Roundtrip

```text
AnswerPoints
→ Text
→ erneute semantische Analyse
→ extrahierte Aussagen
→ Vergleich mit AnswerPoints
```

### Aufgaben

- [ ] Template-Renderer zuerst umsetzen.
- [ ] zulässige Aussageformen definieren.
- [ ] Pflichtpunkte in der Ausgabe prüfen.
- [ ] zusätzliche Behauptungen erkennen.
- [ ] Negationen zurückprüfen.
- [ ] Unsicherheiten korrekt formulieren.
- [ ] SLM-Sandbox definieren.
- [ ] strukturierten Fallback bereitstellen.
- [ ] Renderer-Version im Trace speichern.

> **Qualität:** Das SLM darf die Verpackung verändern, aber nicht den nachgewiesenen Inhalt.

[Zurück zur Agenda](#agenda)

---

## 3.13 Trace- und Nachweissystem

### Trace-Kette

```text
Eingabe
→ Normalisierung
→ SituationSignature
→ Technique-ID
→ Universe-IDs
→ Operatoren
→ AnswerPoints
→ Bewertung
→ Ausgabe
→ ExperienceRecord
```

### Aufgaben

- [ ] Trace-Ereignistypen definieren.
- [ ] Trace-Level für Entwicklung, Test und Produktion definieren.
- [ ] Hash- oder Integritätsverkettung prüfen.
- [ ] Trace-Größe begrenzen.
- [ ] vertrauliche Daten redigieren.
- [ ] Trace-Replay ermöglichen.
- [ ] Trace-Referenzen in AnswerPoints speichern.
- [ ] Erfahrung und Trace bidirektional verknüpfen.
- [ ] fehlende Trace-Schritte als Validierungsfehler behandeln.

### Abnahmekriterien

- [ ] ein Ergebnis kann aus seinem Trace nachvollzogen werden.
- [ ] ein Trace enthält alle verwendeten Versionen.
- [ ] sensitive Payloads sind nicht unkontrolliert enthalten.
- [ ] ein beschädigter Trace wird erkannt.

[Zurück zur Agenda](#agenda)

---

## 3.14 Schnittstellen

| Schnittstelle | Partner | Zweck | Vertrauensgrenze |
|---|---|---|---|
| Input Interface | Benutzer, API, Sensor | Eingabeübergabe | extern nach intern |
| ExecutionCell Interface | Scheduler, Runtime | Cell laden und ausführen | intern, validierungspflichtig |
| Universe Interface | Technique Executor, Operator Engines | Universum ausführen | intern |
| Experience Store Interface | Runtime, Ledger | Erfahrung speichern und lesen | persistente Grenze |
| Consolidation Interface | Ledger, Consolidator | ExperienceCells erzeugen | privilegierte Ableitung |
| Renderer Interface | Constellation Builder, Renderer | Ausgabe formulieren | Renderer nicht voll vertrauenswürdig |
| Validation Interface | Renderer, Roundtrip Validator | Ausgabe prüfen | Freigabegrenze |
| Actuator Interface | BRUCE, externe Systeme | kontrollierte Aktion | kritische Vertrauensgrenze |
| Peacemaker Interface | NanoCell, Wissenssystem | Relationen und Status | Integrationsgrenze |
| BRUCE-Math Interface | NanoCell, Formula Engine | exakte Rechenoperationen | Integrationsgrenze |

### Aufgaben

- [ ] Ein- und Ausgabeformat jeder Schnittstelle definieren.
- [ ] Fehlercodes definieren.
- [ ] Timeout-Verhalten definieren.
- [ ] Versionierung definieren.
- [ ] Authentifizierung und Autorisierung für externe Grenzen definieren.
- [ ] Retry-Verhalten dokumentieren.
- [ ] Idempotenzanforderungen prüfen.
- [ ] Audit-Ereignisse festlegen.

[Zurück zur Agenda](#agenda)

---

## 3.15 Gesamtdatenfluss

### Normalablauf

1. Eingabe empfangen.
2. Eingabe formal validieren.
3. Eingabe normalisieren.
4. Thema, Konzepte, Formen und Ziel bestimmen.
5. SituationSignature erzeugen.
6. ähnliche Erfahrungen suchen.
7. passende Techniken bestimmen.
8. ExecutionCell laden.
9. aktive Projektion erzeugen.
10. UniverseSpecs validieren.
11. Universen lazy ausführen.
12. Cascade-Filter anwenden.
13. AnswerPoints erzeugen.
14. Konstellation validieren.
15. strukturierte Ausgabe erzeugen.
16. optional Text rendern.
17. Text semantisch zurückprüfen.
18. Ergebnis ausgeben.
19. ExperienceRecord speichern.
20. Bewertung ergänzen.
21. Cache und Rankings kontrolliert aktualisieren.
22. temporäre Daten verwerfen.

### Abbruchablauf

```text
Fehler erkennen
→ lokale Komponente stoppen
→ Status erzeugen
→ Trace abschließen
→ keine ungeprüfte Erfahrung aktivieren
→ alternative Technik oder Safe Shutdown
```

### Aufgaben

- [ ] Normalablauf als Sequenzdiagramm dokumentieren.
- [ ] Abbruchpfade je Komponente dokumentieren.
- [ ] Persistenzfehler vom Berechnungsfehler unterscheiden.
- [ ] Wiederholungs- und Retry-Regeln definieren.
- [ ] manuelle Prüfpfade festlegen.

[Zurück zur Agenda](#agenda)

---

# 4. Datenmodell und Speicherverträge

## 4.1 NanoNeuron-Datenmodell

### Vorgesehene logische Felder

| Feld | Zweck | Status |
|---|---|---|
| lokale Position oder ID | Identifikation | festgelegt, Darstellung offen |
| Shape-Maske | relevante Bits auswählen | vorgesehen |
| Shape-Wert | erwartete Form | vorgesehen |
| Funktions-ID | Operator oder Funktion referenzieren | vorgesehen |
| Tabellen-ID | externe Daten oder Profil referenzieren | vorgesehen |
| Ausgangs-Slot | Ergebnisroute | vorgesehen |
| Match-Ziel | Route bei Treffer | vorgesehen |
| Miss-Ziel | Route bei Fehlschlag | vorgesehen |
| Flags | Status und Verhalten | vorgesehen |

### Aufgaben

- [ ] Feldbreiten anhand realer Techniken simulieren.
- [ ] mindestens drei Layoutvarianten vergleichen.
- [ ] Byte-Packing dokumentieren.
- [ ] Endianness festlegen.
- [ ] reservierte Bits definieren.
- [ ] ungültige Bitkombinationen definieren.
- [ ] Layoutmigration planen.
- [ ] Layout-Validator implementieren.

[Zurück zur Agenda](#agenda)

---

## 4.2 ExecutionCell-Datenmodell

```text
ExecutionCellHeader
├─ Cell-ID
├─ Cell-Version
├─ Layout-Version
├─ NanoNeuron-Anzahl
├─ Seitentabellenreferenz
├─ Capability-Profil
├─ Integritätswert
└─ Flags
```

### Aufgaben

- [ ] Header-Felder festlegen.
- [ ] maximale Zellgröße festlegen.
- [ ] Seitentabellenstruktur definieren.
- [ ] lokale und externe Referenzen unterscheiden.
- [ ] Cell-Ladevalidierung definieren.
- [ ] beschädigte Cells isolieren.
- [ ] read-only und mutable Runtime-Bereiche trennen.
- [ ] Cell-Dump für Debugging definieren.

[Zurück zur Agenda](#agenda)

---

## 4.3 UniverseSpec-Datenmodell

### Anforderungen

- vollständig deterministisch,
- kompakt serialisierbar,
- versioniert,
- validierbar,
- ohne implizite versteckte Parameter.

### Aufgaben

- [ ] Schemaentwurf erstellen.
- [ ] Beispielinstanzen für Shift, Rotation, Gate und Maske erstellen.
- [ ] Kontextreferenz definieren.
- [ ] Kostenbudget integrieren.
- [ ] Output Projector definieren.
- [ ] Validierungsregeln implementieren.
- [ ] Schema-Fuzzing vorbereiten.

[Zurück zur Agenda](#agenda)

---

## 4.4 TechniqueDefinition-Datenmodell

### Anforderungen

- geordnete Universen,
- explizite Abhängigkeiten,
- Pflicht- und optionale Pfade,
- begrenzte Ausführung,
- definierte Erfolgskriterien,
- definierte Sicherheitsklasse.

### Aufgaben

- [ ] DAG-Modell mit Zustandsmaschinenmodell vergleichen.
- [ ] MVP-Modell auswählen.
- [ ] Technikversionierung definieren.
- [ ] Technikherkunft und Autor erfassen.
- [ ] bekannte Grenzen speichern.
- [ ] bekannte Gegenbeispiele referenzieren.
- [ ] Techniksignatur oder Integritätsprüfung vorbereiten.

[Zurück zur Agenda](#agenda)

---

## 4.5 AnswerPoint-Datenmodell

### Offene Varianten

#### Variante A - kompakter 64-Bit-Punkt

Vorteil:

- sehr kleiner Speicher,
- schnelle Verarbeitung.

Nachteil:

- starke Abhängigkeit von Seitentabellen,
- begrenzte Feldbreiten.

#### Variante B - strukturierter Punkt

Vorteil:

- gut lesbar,
- leichter erweiterbar.

Nachteil:

- größerer Speicherbedarf.

#### Variante C - Core und Sidecar

Vorteil:

- kompakter Hot Path,
- zusätzliche Metadaten bei Bedarf.

### Aufgaben

- [ ] benötigte Punktfelder aus drei realen Use Cases ableiten.
- [ ] kompakte und ausführliche Variante vergleichen.
- [ ] Core-plus-Sidecar als bevorzugte Variante prüfen.
- [ ] Evidenz- und Polaritätsfelder verpflichtend machen.
- [ ] Trace-Referenzformat definieren.
- [ ] Point-Versionierung definieren.

[Zurück zur Agenda](#agenda)

---

## 4.6 ExperienceRecord-Datenmodell

```text
ExperienceRecord
├─ Experience-ID
├─ Sequenz oder Zeitreferenz
├─ SituationSignature
├─ ausgewählte Technik
├─ betrachtete Alternativen
├─ Universe-Traces
├─ AnswerPoints
├─ beobachtetes Ergebnis
├─ vorläufige Bewertung
├─ Umgebungs- und Versionsdaten
└─ Eltern- oder Quellreferenzen
```

### Aufgaben

- [ ] Pflicht- und optionale Felder definieren.
- [ ] atomaren Schreibvertrag festlegen.
- [ ] Schreibabbruch und Wiederanlauf definieren.
- [ ] Payload-Kompression prüfen.
- [ ] sensible Rohdaten auslagern.
- [ ] Integritätsprüfung definieren.
- [ ] Record-Replay implementieren.
- [ ] Aufbewahrungs- und Löschkonzept vorbereiten.

[Zurück zur Agenda](#agenda)

---

## 4.7 ExperienceCell-Datenmodell

```text
ExperienceCell
├─ Cell-ID
├─ Version
├─ Situation Cluster
├─ bevorzugte Techniken
├─ Gültigkeitsbedingungen
├─ positive Fälle
├─ negative Fälle
├─ ungeklärte Fälle
├─ Kostenprofil
├─ Verifikationsklasse
└─ Quell-Experience-IDs
```

### Aufgaben

- [ ] deterministische Gruppierungsmerkmale festlegen.
- [ ] Mindestanzahl oder Nachweisbedingung für Konsolidierung definieren.
- [ ] Gegenbeispiele verpflichtend berücksichtigen.
- [ ] Verifikationsklasse definieren.
- [ ] ExperienceCell-Rebuild aus Originaldaten ermöglichen.
- [ ] alte ExperienceCell-Versionen erhalten.
- [ ] Konsolidierungsbericht erzeugen.

[Zurück zur Agenda](#agenda)

---

## 4.8 Hot-Skill-Cache

### Cache-Ziele

- schnelle Technikvorauswahl,
- themenbezogene Aktivierung,
- kleiner RAM-Footprint,
- reproduzierbarer Wiederaufbau.

### Cache-Key

Mögliche Bestandteile:

- Themen-ID,
- Situationstyp,
- Technikfamilie,
- Sicherheitsklasse,
- Hardwareprofil,
- Versionsstand.

### Aufgaben

- [ ] Cache-Eintrag definieren.
- [ ] Cache-Promotion definieren.
- [ ] Cache-Verdrängung definieren.
- [ ] seltene kritische Skills schützen.
- [ ] Popularitäts-Bias begrenzen.
- [ ] Cache-Rebuild testen.
- [ ] Cache-Korruption erkennen.
- [ ] RAM-Budget durchsetzen.

[Zurück zur Agenda](#agenda)

---

## 4.9 Persistente und flüchtige Daten

### Persistente Daten

- kanonische NanoNeuronen,
- ExecutionCell-Versionen,
- TechniqueDefinitions,
- ExperienceRecords,
- EvaluationRecords,
- ExperienceCells,
- Traces,
- Versions- und Migrationsdaten.

### Flüchtige Daten

- aktive Projektionen,
- Universe-Zwischenergebnisse,
- Scratchpads,
- Kandidatenlisten,
- vorläufige AnswerPoints,
- Renderer-Arbeitsspeicher,
- temporäre Textausgaben.

### Regel

> Berechnete Antworten dürfen kurzzeitig in Registern, Cache oder Ausgabepuffern existieren. Sie dürfen jedoch nicht ungeprüft als dauerhafter aktiver Modellzustand übernommen werden.

### Aufgaben

- [ ] Lebensdauer jeder Datenklasse dokumentieren.
- [ ] Lösch- und Freigabepunkte definieren.
- [ ] sensitive Scratchpads nach Verwendung bereinigen.
- [ ] temporäre und persistente IDs unterscheiden.
- [ ] Speicherlecks durch Langzeittests prüfen.

[Zurück zur Agenda](#agenda)

---

## 4.10 Versionierung und Migration

Zu versionieren sind:

- NanoNeuron-Layout,
- ExecutionCell-Format,
- UniverseSpec,
- TechniqueDefinition,
- AnswerPoint,
- ExperienceRecord,
- ExperienceCell,
- Ranking-Funktion,
- Renderer,
- Trace-Schema.

### Aufgaben

- [ ] Versionskompatibilitätsmatrix erstellen.
- [ ] Migrationsstrategie je Datenklasse definieren.
- [ ] read-only Unterstützung alter Versionen prüfen.
- [ ] inkompatible Versionen kontrolliert ablehnen.
- [ ] Migrationen mit Fixtures testen.
- [ ] Rollback-Verfahren definieren.
- [ ] Primärquellen vor Migration sichern.

[Zurück zur Agenda](#agenda)

---

# 5. Entscheidungs- und Lernlogik

## 5.1 Situationserkennung

```text
Rohdaten
→ Normalisierung
→ Thema
→ Konzepte
→ Formen
→ Relationen
→ Ziel
→ Einschränkungen
→ SituationSignature
```

### Aufgaben

- [ ] SituationSignature-Schema definieren.
- [ ] diskrete Merkmale für den MVP auswählen.
- [ ] Kontext verpflichtend machen.
- [ ] Unsicherheiten explizit speichern.
- [ ] Mehrdeutigkeit als Zustand behandeln.
- [ ] ähnliche und identische Situationen unterscheiden.
- [ ] mindestens drei repräsentative Situationstypen definieren.

[Zurück zur Agenda](#agenda)

---

## 5.2 Technikauswahl

### Prioritätsregeln

1. Gültigkeit vor Rang.
2. Sicherheitsanforderungen vor Geschwindigkeit.
3. Kontextpassung vor globaler Häufigkeit.
4. verifizierte Erfahrung vor bloßer Nutzung.
5. Gegenbeispiele vor Erfolgsquote.
6. Ressourcenbudget vor vollständiger Suche.

### Aufgaben

- [ ] Technique Selector implementieren.
- [ ] Preconditions Match definieren.
- [ ] Negative Match definieren.
- [ ] Version Compatibility prüfen.
- [ ] Kostenbudget prüfen.
- [ ] mehrere gleichwertige Techniken behandeln.
- [ ] manuelle Prüfung als Status unterstützen.
- [ ] Auswahlbegründung im Trace speichern.

[Zurück zur Agenda](#agenda)

---

## 5.3 Universumsauswertung

### Ablauf

```text
UniverseSpec validieren
→ Eingabeprojektion binden
→ Transformation ausführen
→ Gate anwenden
→ Maske prüfen
→ Kontext prüfen
→ Musterfunktion ausführen
→ AnswerPoint oder Ablehnung erzeugen
```

### Aufgaben

- [ ] lazy Iterator-Modell definieren.
- [ ] Ausführungsreihenfolge festlegen.
- [ ] unabhängige Universen markieren.
- [ ] kontrollierte Parallelisierung vorbereiten.
- [ ] Abbruch bei Pflichtfehler implementieren.
- [ ] Zwischenergebnisse begrenzen.
- [ ] Universumskosten erfassen.
- [ ] deterministisches Replay ermöglichen.

[Zurück zur Agenda](#agenda)

---

## 5.4 Ergebnisbewertung

### Bewertungsdimensionen

- Erfolg,
- Verifikation,
- Kosten,
- Risiko,
- Vollständigkeit,
- Kontextpassung,
- Neuigkeitswert,
- Fehlerklasse.

### Aufgaben

- [ ] EvaluationRecord definieren.
- [ ] erfolgreiche und technisch nur ausführbare Ergebnisse unterscheiden.
- [ ] unbekannte Bewertung zulassen.
- [ ] menschliche Bewertung als eigene Quelle kennzeichnen.
- [ ] automatische Bewertung von externer Bestätigung trennen.
- [ ] Bewertungsregeln versionieren.
- [ ] nachträgliche Neubewertung ohne Record-Überschreibung ermöglichen.

[Zurück zur Agenda](#agenda)

---

## 5.5 Erfahrungsspeicherung

### Speicherregel

Eine nackte Antwort ist keine vollständige Erfahrung.

```text
Experience =
Situation
+ Technik
+ Ergebnis
+ Bewertung
+ Nachweis
```

### Aufgaben

- [ ] ExperienceRecord atomar schreiben.
- [ ] Persistenzfehler als eigenen Status behandeln.
- [ ] unvollständige Records quarantänisieren.
- [ ] erfolgreiche Ausgabe auch bei Speicherfehler korrekt kennzeichnen.
- [ ] Replay-Index aufbauen.
- [ ] SSD-Schreibvolumen messen.
- [ ] Batch-Schreiben prüfen.
- [ ] Datenintegrität beim Neustart prüfen.

[Zurück zur Agenda](#agenda)

---

## 5.6 Erfahrungskonsolidierung

### Ablauf

```text
vergleichbare ExperienceRecords
→ Merkmale gruppieren
→ Erfolge und Fehlschläge trennen
→ Gültigkeitsbereich bestimmen
→ Gegenbeispiele erhalten
→ ExperienceCell erzeugen
→ Verifikation
→ optional Hot-Skill-Promotion
```

### Aufgaben

- [ ] Konsolidierungsintervall definieren.
- [ ] Gruppierungsalgorithmus deterministisch gestalten.
- [ ] Quell-Experience-IDs erhalten.
- [ ] Konsolidierungsbericht erzeugen.
- [ ] Gegenbeispiele sichtbar dokumentieren.
- [ ] Gültigkeitsbereich nicht über bestehende Evidenz hinaus erweitern.
- [ ] alte ExperienceCell-Versionen nicht überschreiben.
- [ ] Rebuild-Test implementieren.

> **Peacemaker-Prinzip:** Widersprüchliche Erfahrungen nicht still zusammenführen.

[Zurück zur Agenda](#agenda)

---

## 5.7 Skill-Ranking

### Teilwerte

```text
UsageScore
SuccessScore
VerificationScore
TopicRelevanceScore
CostScore
FailureScore
RiskScore
NoveltyScore
```

### Mögliche Gesamtform

```text
Rank =
  positive Gewichtung von Erfolg, Verifikation, Relevanz und Nutzung
- negative Gewichtung von Fehlern, Kosten und Risiko
```

Konkrete Gewichte sind noch nicht festgelegt.

### Aufgaben

- [ ] Ranking-Teilwerte definieren.
- [ ] Gewichte als versionierte Konfiguration behandeln.
- [ ] häufige, aber schlechte Technik testen.
- [ ] seltene, aber wichtige Technik testen.
- [ ] Themenrelevanz gegen globale Nutzung testen.
- [ ] Rang und Gültigkeit strikt trennen.
- [ ] Ranking-Replay ermöglichen.
- [ ] Cache-Promotion aus Ranking ableiten.

[Zurück zur Agenda](#agenda)

---

## 5.8 Fehlversuche und Gegenregeln

### Beispiele

```text
Technik scheiterte,
weil der Wertebereich überschritten wurde.
```

```text
Rotation erzeugte ein Strukturmuster,
das durch den Kontext widerlegt wurde.
```

```text
häufige Technik war verfügbar,
aber ein bekanntes Gegenbeispiel traf zu.
```

### Aufgaben

- [ ] Fehlerklassen definieren.
- [ ] Fachfehler von Implementierungsfehler unterscheiden.
- [ ] Ressourcenfehler nicht als fachliches Gegenbeispiel speichern.
- [ ] Gegenregeln aus bestätigten Fehlschlägen ableiten.
- [ ] unbekannte Fehler nicht automatisch generalisieren.
- [ ] Widersprüche zur manuellen Prüfung markieren.
- [ ] negative Erfahrungen in der Suche berücksichtigen.

[Zurück zur Agenda](#agenda)

---

## 5.9 Zustandsmodell

### Eingabe

```text
RECEIVED
VALIDATED
NORMALIZED
REJECTED
```

### Situation

```text
UNKNOWN
RECOGNIZED
AMBIGUOUS
REQUIRES_REVIEW
```

### Technik

```text
CANDIDATE
ELIGIBLE
REJECTED
EXECUTING
SUCCEEDED
FAILED
ABORTED
```

### AnswerPoint

```text
PROVISIONAL
SUPPORTED
CONFLICTING
REJECTED
PUBLISHED
```

### Erfahrung

```text
RECORDED
EVALUATED
UNVERIFIED
VERIFIED
CONTRADICTED
SUPERSEDED
CONSOLIDATED
```

### Aufgaben

- [ ] Eintrittsbedingungen definieren.
- [ ] erlaubte Übergänge definieren.
- [ ] verbotene Übergänge definieren.
- [ ] persistente und flüchtige Zustände markieren.
- [ ] Reset- und Recovery-Verhalten definieren.
- [ ] Statusübergänge testen.
- [ ] Übergänge im Trace erfassen.

[Zurück zur Agenda](#agenda)

---

# 6. Roadmap

## 6.1 Phase 0 - Architekturverträge

**Status:** In Bearbeitung

**Phasenziel:**  
Alle verbindlichen Kernbegriffe, Speicherverträge, Datenmodelle und Architekturgrenzen festlegen.

**Voraussetzungen:**

- vorhandenes NanoCell-Konzept,
- bekannte ExecutionCell-Entscheidungen,
- BRUCE-, Peacemaker- und BRUCE-Math-Dokumentation,
- die in diesem Dokument erfassten offenen Fragen.

**Aufgaben:**

- [x] ExecutionCell als zusammenhängenden NanoNeuron-Block festlegen.
- [x] MVP-ID-Breite von 16 Bit festhalten.
- [x] allgemeine CPU-RAM-NPU mit BRUCE als Referenz festlegen.
- [x] kanonische NanoNeuron-Größe von acht Byte festhalten.
- [x] aktive Projektion als separates Konzept festhalten.
- [x] Paralleluniversum als deterministischen Berechnungszweig abgrenzen.
- [x] Lemmings und Quantenteleportation als Metaphern abgrenzen.
- [ ] NanoNeuron-Layoutvarianten erstellen.
- [ ] aktiven Projektionstyp festlegen.
- [ ] UniverseSpec-Schema festlegen.
- [ ] TechniqueDefinition-Schema festlegen.
- [ ] AnswerPoint-Schema festlegen.
- [ ] ExperienceRecord-Schema festlegen.
- [ ] ExperienceCell-Schema festlegen.
- [ ] Fehler- und Zustandsmodell bestätigen.
- [ ] Architekturentscheidungen als ADRs dokumentieren.

**Kleine Anleitung:**

1. Drei repräsentative Techniken auswählen.
2. Für diese Techniken alle benötigten Felder erfassen.
3. Daraus den minimalen Speichervertrag ableiten.
4. Erst danach das 8-Byte-Layout bitweise festlegen.
5. Jede Auslagerung in Seitentabellen begründen.

**Ergebnis dieser Phase:**

- verbindliches Architekturglossar,
- NanoNeuron Memory Contract,
- Universe Execution Contract,
- Experience Learning Contract,
- Semantic Rendering Boundary,
- offene Entscheidungen mit stabilen IDs.

**Abnahmekriterien:**

- [ ] Kernkomponenten besitzen eindeutige Verantwortung.
- [ ] Datenmodelle sind versionierbar.
- [ ] Widersprüche sind dokumentiert.
- [ ] der Phase-1-Prototyp kann ohne weitere Grundsatzentscheidung beginnen.
- [ ] alle Primärverträge besitzen Testbeispiele.

**Abhängig von:**

- keine technische Implementierung erforderlich,
- Zugriff auf bestehende Architekturentscheidungen.

**Mögliche Blocker:**

- 8-Byte-Layout nimmt nicht alle benötigten Hot-Path-Felder auf,
- AnswerPoint-Anforderungen sind noch zu breit,
- SituationSignature ist nicht ausreichend definiert.

> **Erinnerung:** Erst Verträge stabilisieren, danach optimieren.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.2 Phase 1 - Bit-Universe-Prototyp

**Status:** Noch nicht begonnen

**Phasenziel:**  
Beweisen, dass ein einzelner Bytezustand durch mehrere deterministische UniverseSpecs ausgewertet werden kann, ohne vollständige Dimensionsräume zu speichern.

**Voraussetzungen:**

- UniverseSpec v1,
- Operatorverträge,
- Trace-Mindestvertrag.

**Aufgaben:**

- [ ] Byte-Eingabe implementieren.
- [ ] Shift links implementieren.
- [ ] Shift rechts implementieren.
- [ ] Rotation links implementieren.
- [ ] Rotation rechts implementieren.
- [ ] AND implementieren.
- [ ] OR implementieren.
- [ ] XOR implementieren.
- [ ] NOT implementieren.
- [ ] feste Shape-Masken implementieren.
- [ ] einfachen Kontextwert integrieren.
- [ ] UniverseSpec-Validator implementieren.
- [ ] lazy Universumsausführung implementieren.
- [ ] Universumsergebnis protokollieren.
- [ ] mindestens 256 Eingangswerte automatisiert testen.
- [ ] Ressourcenmessung vorbereiten.

**Unteraufgaben:**

- [ ] Testvektoren für Shift-Grenzen erstellen.
- [ ] Testvektoren für Rotation um 0 und 8 erstellen.
- [ ] Wahrheitstabellen automatisiert prüfen.
- [ ] Masken-Match und Miss prüfen.
- [ ] ungültige Gate-ID testen.
- [ ] Universumslimit testen.

**Ergebnis dieser Phase:**

```text
1 Byte
→ mehrere definierte Universen
→ deterministische Ergebnisse
→ vollständiger Trace
```

**Abnahmekriterien:**

- [ ] Wiederholungen erzeugen identische Ergebnisse.
- [ ] kein Universum benötigt einen materialisierten Hyperraum.
- [ ] ungültige UniverseSpecs werden kontrolliert abgelehnt.
- [ ] temporäre Zwischenergebnisse werden freigegeben.
- [ ] alle Operator-Unit-Tests bestehen.

**Abhängig von:**

- Phase 0,
- definierte Wortbreite,
- definierte Shift- und Rotationssemantik.

**Mögliche Blocker:**

- uneindeutige Operatorsemantik,
- unvollständiges Trace-Schema,
- fehlende Ressourcenlimits.

> **Erinnerung:** Der erste Prototyp benötigt weder Datenbank, SLM noch GPU.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.3 Phase 2 - ExecutionCell-Runtime

**Status:** Noch nicht begonnen

**Phasenziel:**  
Die Bit-Universe-Engine auf kanonische 8-Byte-NanoNeuronen und zusammenhängende ExecutionCells übertragen.

**Voraussetzungen:**

- Phase 1 abgeschlossen,
- NanoNeuron-Layout v1,
- ExecutionCell-Header definiert.

**Aufgaben:**

- [ ] NanoNeuron-Parser implementieren.
- [ ] Layout-Validator implementieren.
- [ ] ExecutionCell-Loader implementieren.
- [ ] lokale 16-Bit-Indizes implementieren.
- [ ] aktive 1-Byte-Projektion implementieren.
- [ ] relative oder tabellenbasierte Sprünge implementieren.
- [ ] Match-/Miss-Routing implementieren.
- [ ] Cell-Integritätsprüfung implementieren.
- [ ] beschädigte Cells kontrolliert ablehnen.
- [ ] Scratchpad-Limits durchsetzen.
- [ ] Cell-Dump für Debugging implementieren.
- [ ] mehrere NanoNeuronen als 64-Bit-Blöcke profilieren.

**Ergebnis dieser Phase:**

```text
ExecutionCell
→ NanoNeuronen laden
→ aktive Projektionen
→ Universen
→ Match- und Miss-Routing
→ Trace
```

**Abnahmekriterien:**

- [ ] ein NanoNeuron belegt im kanonischen Format exakt acht Byte.
- [ ] die aktive Projektion ist reproduzierbar.
- [ ] Quell-NanoNeuronen bleiben read-only.
- [ ] ungültige Sprungziele werden erkannt.
- [ ] Cell-Neustart erzeugt identisches Ergebnis.
- [ ] Ressourcenbudgets werden eingehalten.

**Abhängig von:**

- NanoNeuron ADR,
- ExecutionCell ADR,
- UniverseSpec v1.

**Mögliche Blocker:**

- Acht-Byte-Layout benötigt zu viele Seitentabellenzugriffe,
- lokale 16-Bit-Ziele reichen für gewünschte Cell-Größe nicht,
- Alignment verursacht unerwarteten Overhead.

> **Architektur:** Erst den korrekten Referenzpfad stabilisieren, danach SIMD-Packing optimieren.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.4 Phase 3 - AnswerPoint-System

**Status:** Noch nicht begonnen

**Phasenziel:**  
Universumsergebnisse in typisierte Bedeutungspunkte projizieren und mehrere Punkte zu einer validierten Konstellation zusammensetzen.

**Voraussetzungen:**

- Phase 2 abgeschlossen,
- AnswerPoint-Schema v1,
- grundlegendes Kontextmodell.

**Aufgaben:**

- [ ] AnswerPoint Projector implementieren.
- [ ] Punkt-Typen definieren.
- [ ] Topic-, Concept- und Relation-IDs abbilden.
- [ ] Polarität verpflichtend speichern.
- [ ] Evidenzklassen implementieren.
- [ ] Trace-Referenzen integrieren.
- [ ] Semantic Constellation Builder implementieren.
- [ ] Dubletten erkennen.
- [ ] Konflikte erkennen.
- [ ] Pflichtpunkte prüfen.
- [ ] strukturierte Ausgabe implementieren.
- [ ] Template-Ausgabe implementieren.
- [ ] mindestens drei vollständige Antwortbeispiele erstellen.

**Ergebnis dieser Phase:**

```text
Universe-Ergebnisse
→ AnswerPoints
→ semantische Konstellation
→ strukturierte oder templatebasierte Ausgabe
```

**Abnahmekriterien:**

- [ ] jeder veröffentlichte Punkt besitzt eine Trace-Referenz.
- [ ] Widersprüche verhindern die Freigabe.
- [ ] Negationen bleiben erhalten.
- [ ] Ausgabe funktioniert ohne generatives Modell.
- [ ] identische Punktmengen erzeugen identische strukturierte Ausgaben.

**Abhängig von:**

- Phase 2,
- Konzept- und Relations-IDs,
- Evidenzmodell.

**Mögliche Blocker:**

- zu große AnswerPoints,
- unklare Relation zwischen Punktreihenfolge und Semantik,
- fehlende Kontext-IDs.

> **Qualität:** Eine schöne Formulierung ist erst relevant, nachdem die Bedeutung korrekt strukturiert wurde.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.5 Phase 4 - Experience Ledger

**Status:** Noch nicht begonnen

**Phasenziel:**  
Konkrete Technikanwendungen append-only speichern, bewerten und reproduzierbar erneut laden.

**Voraussetzungen:**

- Phase 3 abgeschlossen,
- ExperienceRecord-Schema v1,
- Persistenzentscheidung.

**Aufgaben:**

- [ ] Experience Store auswählen.
- [ ] append-only Schreibpfad implementieren.
- [ ] ExperienceRecord atomar speichern.
- [ ] EvaluationRecord separat speichern.
- [ ] Record-Integrität prüfen.
- [ ] Record-Replay implementieren.
- [ ] Persistenzfehler simulieren.
- [ ] unvollständige Records quarantänisieren.
- [ ] Index nach Situation und Technik aufbauen.
- [ ] Erfolg und Fehlschlag getrennt erfassen.
- [ ] SSD-Schreibvolumen messen.
- [ ] Backup- und Restore-Test implementieren.

**Ergebnis dieser Phase:**

```text
Situation
+ Technik
+ Ergebnis
+ Bewertung
+ Trace
→ persistente Erfahrung
```

**Abnahmekriterien:**

- [ ] ExperienceRecords werden nicht überschrieben.
- [ ] nachträgliche Bewertungen bleiben nachvollziehbar.
- [ ] Record-Replay erzeugt dieselben AnswerPoints.
- [ ] beschädigte Records werden erkannt.
- [ ] Neustart verliert keine bestätigte Erfahrung.
- [ ] Speicherfehler erzeugt keinen falschen Lernstatus.

**Abhängig von:**

- Phase 3,
- Speicher- und Integritätsvertrag,
- Datenschutzentscheidung.

**Mögliche Blocker:**

- ExperienceRecords werden zu groß,
- Trace und Payload enthalten sensible Daten,
- atomare Schreibstrategie ist nicht festgelegt.

> **Erinnerung:** Die Ausgabe einer Antwort und das erfolgreiche Speichern einer Erfahrung sind getrennte Ergebnisse.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.6 Phase 5 - Erfahrungsabruf und Skill-Cache

**Status:** Noch nicht begonnen

**Phasenziel:**  
Ähnliche frühere Situationen finden, passende Techniken priorisieren und verifizierte Skills themenbezogen im RAM aktivieren.

**Voraussetzungen:**

- Phase 4 abgeschlossen,
- SituationSignature v1,
- Ranking-Modell v1.

**Aufgaben:**

- [ ] Situation-Index implementieren.
- [ ] deterministische Ähnlichkeit definieren.
- [ ] positive und negative Erfahrungen gemeinsam berücksichtigen.
- [ ] Technique Selector mit Erfahrungsdaten verbinden.
- [ ] Ranking-Teilwerte implementieren.
- [ ] Ranking-Version speichern.
- [ ] ExperienceCell-Consolidator implementieren.
- [ ] Hot-Skill-Cache implementieren.
- [ ] Cache-Rebuild implementieren.
- [ ] Cache-Poisoning-Tests durchführen.
- [ ] Popularitäts-Bias testen.
- [ ] seltene kritische Skills schützen.
- [ ] Themenwechsel testen.

**Ergebnis dieser Phase:**

```text
neue Situation
→ ähnliche Erfahrungen
→ geeignete Technik
→ schnellere und begründete Auswahl
```

**Abnahmekriterien:**

- [ ] ähnliche Situation findet nachvollziehbare Kandidaten.
- [ ] bekannte Gegenbeispiele können eine häufige Technik ablehnen.
- [ ] Cache kann aus SSD-Daten rekonstruiert werden.
- [ ] Rang ersetzt keine Gültigkeitsprüfung.
- [ ] Themenwechsel lädt passende Skills.
- [ ] Ranking-Replay ist reproduzierbar.

**Abhängig von:**

- Phase 4,
- Ähnlichkeitsvertrag,
- ExperienceCell-Schema,
- RAM-Budget.

**Mögliche Blocker:**

- Situationsähnlichkeit ist zu unscharf,
- Konsolidierung generalisiert zu stark,
- Cache bevorzugt nur häufige Skills.

> **BRUCE-Prinzip:** Der passende Slot wird zuerst bestimmt; innerhalb des Slots wird weitergesucht.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.7 Phase 6 - Integration mit BRUCE-Kernsystemen

**Status:** Noch nicht begonnen

**Phasenziel:**  
NanoCell mit PeacemakerLLM, BRUCE-Math und dem gemeinsamen Trace- und Statusmodell verbinden.

**Voraussetzungen:**

- Phase 5 abgeschlossen,
- stabile interne Schnittstellen,
- bekannte Versionen der Zielsysteme.

**Aufgaben:**

### PeacemakerLLM

- [ ] Relationsergebnisse in AnswerPoints überführen.
- [ ] `FOUND`, `UNKNOWN`, `AMBIGUOUS` und `REQUIRES_REVIEW` abbilden.
- [ ] Direct-Relation-Technik definieren.
- [ ] Word-Sense- und Kontexttechniken integrieren.
- [ ] Peacemaker-Trace referenzieren.

### BRUCE-Math

- [ ] Formula Evaluation als Technik definieren.
- [ ] Domain Enforcement als Pflichtuniversum abbilden.
- [ ] Safe-Shutdown-Status integrieren.
- [ ] exakte mathematische Ergebnisse in AnswerPoints abbilden.
- [ ] Rechenkosten und Grenzwerte erfassen.
- [ ] Formula-Trace und ExperienceRecord verbinden.

### Capability- und Service-Zellen

- [ ] minimale Runtime-Capabilities dokumentieren.
- [ ] Experience Store als getrennte Capability definieren.
- [ ] Renderer-Capability begrenzen.
- [ ] Aktor-Capability separat autorisieren.
- [ ] optionalen Service-Zellen-Betrieb prüfen.

**Ergebnis dieser Phase:**

- gemeinsames BRUCE-Statusmodell,
- gemeinsame Trace-Referenzen,
- wiederverwendbare Wissens- und Mathematiktechniken,
- klare Vertrauensgrenzen.

**Abnahmekriterien:**

- [ ] Peacemaker-Ergebnis kann ohne Bedeutungsverlust übertragen werden.
- [ ] BRUCE-Math-Safe-Shutdown bleibt erhalten.
- [ ] NanoCell verändert bestehende Primärquellen nicht.
- [ ] Integrationsfehler bleiben lokal.
- [ ] End-to-End-Trace verbindet alle beteiligten Systeme.

**Abhängig von:**

- stabile APIs oder interne Verträge,
- kompatible Trace-Schemas,
- Integrationsfixtures.

**Mögliche Blocker:**

- unterschiedliche Statussemantik,
- inkompatible IDs,
- doppelte Verantwortungen,
- zu hoher Trace-Overhead.

> **Peacemaker-Prinzip:** Bestehende Systeme nicht still umdeuten. Konflikte sichtbar abbilden.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.8 Phase 7 - Sprachrekonstruktion

**Status:** Noch nicht begonnen

**Phasenziel:**  
Validierte AnswerPoints sicher in verständliche Sprache übersetzen.

**Voraussetzungen:**

- Phase 6 abgeschlossen,
- stabile semantische Konstellationen,
- definierte Renderer-Grenze.

**Aufgaben:**

- [ ] Template-Renderer erweitern.
- [ ] regelbasierten Satzbau prüfen.
- [ ] Renderer-Eingabevertrag schließen.
- [ ] verpflichtende Aussagen markieren.
- [ ] verbotene Ergänzungen definieren.
- [ ] Semantic Roundtrip Validator implementieren.
- [ ] zusätzliche Behauptungen erkennen.
- [ ] Negationsfehler erkennen.
- [ ] Unsicherheitsformulierung prüfen.
- [ ] SLM-Sandbox vorbereiten.
- [ ] themenorientiertes SLM evaluieren.
- [ ] strukturierten Fallback erzwingen.
- [ ] Renderer-Ausgabe nicht automatisch als Wissen speichern.

**Ergebnis dieser Phase:**

```text
AnswerPoints
→ Text
→ semantische Rückprüfung
→ freigegebene Ausgabe
```

**Abnahmekriterien:**

- [ ] alle Pflichtaussagen sind im Text enthalten.
- [ ] keine unbekannte Behauptung wird ergänzt.
- [ ] Polarität und Relationen bleiben korrekt.
- [ ] Roundtrip-Fehler führt zum Fallback.
- [ ] Textausgabe ist vom Aktorpfad getrennt.
- [ ] Renderer-Version ist im Trace enthalten.

**Abhängig von:**

- Phase 6,
- semantischer Parser,
- geschlossener Renderer-Vertrag.

**Mögliche Blocker:**

- semantischer Roundtrip ist zu tolerant,
- SLM fügt plausible, aber unbelegte Details hinzu,
- thematisches SLM benötigt zu viele Ressourcen.

> **Erinnerung:** BRUCE denkt, der Renderer spricht.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.9 Phase 8 - Optimierung und Härtung

**Status:** Noch nicht begonnen

**Phasenziel:**  
Leistung, Ressourcenverbrauch, Ausfallsicherheit und Missbrauchsresistenz der Referenzimplementierung verbessern.

**Voraussetzungen:**

- funktionierender End-to-End-Pfad,
- belastbare Benchmarks,
- vollständige Testmatrix.

**Aufgaben:**

### Leistung

- [ ] CPU-Profiling durchführen.
- [ ] 64-Bit-Packing optimieren.
- [ ] SIMD-Potenzial messen.
- [ ] Cache-Layouts vergleichen.
- [ ] SSD-Batch-Schreiben optimieren.
- [ ] Trace-Level vergleichen.
- [ ] GPU-Eignung einzelner Operationen messen.

### Härtung

- [ ] Universums-Explosion testen.
- [ ] manipulierte TechniqueDefinitions testen.
- [ ] Cache Poisoning testen.
- [ ] Experience-Vergiftung testen.
- [ ] beschädigte Seitentabellen testen.
- [ ] Renderer-Ausbruch testen.
- [ ] Strom- oder Prozessabbruch während Persistenz testen.
- [ ] Update- und Rollback-Verfahren testen.
- [ ] Langzeitbetrieb testen.

**Ergebnis dieser Phase:**

- dokumentiertes Leistungsprofil,
- begründete Optimierungen,
- reproduzierbare Wiederherstellung,
- gehärtete Vertrauensgrenzen.

**Abnahmekriterien:**

- [ ] Optimierungen verändern logische Ergebnisse nicht.
- [ ] Benchmarks nennen Hardware und Versionen.
- [ ] Ressourcenlimits werden unter Last eingehalten.
- [ ] Recovery-Tests bestehen.
- [ ] kritische Missbrauchsfälle sind dokumentiert.
- [ ] verbleibende Risiken sind sichtbar.

**Abhängig von:**

- alle vorherigen Phasen,
- repräsentative Workloads,
- Benchmark-Hardware.

**Mögliche Blocker:**

- Trace-Kosten dominieren Laufzeit,
- SSD-Schreibvolumen ist zu hoch,
- GPU-Transfer kostet mehr als die Berechnung,
- Seitentabellen verhindern Cache-Effizienz.

> **Qualität:** Keine Optimierung ohne Vergleich gegen den korrekten Referenzpfad.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.10 Phase 9 - Referenzfreigabe

**Status:** Noch nicht begonnen

**Phasenziel:**  
Eine dokumentierte, reproduzierbare BRUCE-NanoCell-Referenzversion veröffentlichungs- oder integrationsbereit machen.

**Voraussetzungen:**

- Phase 8 abgeschlossen,
- Test- und Sicherheitsnachweise,
- dokumentierte Einschränkungen.

**Aufgaben:**

- [ ] Release-Scope festlegen.
- [ ] Versionen einfrieren.
- [ ] vollständige Test-Suite ausführen.
- [ ] Reproduzierbarkeitsbericht erstellen.
- [ ] bekannte Einschränkungen dokumentieren.
- [ ] Installations- und Betriebsanleitung erstellen.
- [ ] Backup- und Restore-Anleitung erstellen.
- [ ] Migrationsanleitung erstellen.
- [ ] Referenzfixtures veröffentlichen.
- [ ] Roadmap und Projektstatus aktualisieren.
- [ ] nächste Forschungsphase getrennt planen.

**Ergebnis dieser Phase:**

- versionierte Referenzimplementierung,
- vollständige Dokumentation,
- reproduzierbare Testnachweise,
- klarer Upgrade-Pfad.

**Abnahmekriterien:**

- [ ] frische Installation besteht die Referenztests.
- [ ] dokumentierte Fixtures erzeugen erwartete Traces.
- [ ] Backup und Restore funktionieren.
- [ ] bekannte Risiken sind dokumentiert.
- [ ] keine offene kritische Entscheidung bleibt unmarkiert.
- [ ] Release enthält keine ungeprüften experimentellen Funktionen.

**Mögliche Blocker:**

- offene kritische Sicherheitsfrage,
- nicht reproduzierbare Benchmarks,
- inkompatible Datenmigration,
- fehlende End-to-End-Nachweise.

> **Definition of Done:** Freigabe bedeutet nachgewiesene Reproduzierbarkeit und nicht nur „läuft auf einem Entwicklungsrechner“.

[Zurück zur Roadmap](#6-roadmap) · [Zurück zur Agenda](#agenda)

---

## 6.11 Meilensteine

| ID | Meilenstein | Inhalt | Abnahmekriterium | Status |
|---|---|---|---|---|
| M0 | Architekturverträge | NanoNeuron, UniverseSpec, AnswerPoint und ExperienceRecord definiert | alle Verträge versioniert und testbar | In Arbeit |
| M1 | Bit-Universe Proof | Bytezustände durch Shift, Rotation, Gates und Masken auswerten | deterministische Operator-Tests bestehen | Offen |
| M2 | ExecutionCell Runtime | 8-Byte-NanoNeuronen und 1-Byte-Projektionen ausführen | Cell-Replay erzeugt identisches Ergebnis | Offen |
| M3 | Semantic Points | AnswerPoints und Konstellationen erzeugen | strukturierte Ausgabe ohne SLM | Offen |
| M4 | Experience Roundtrip | Erfahrung speichern, laden und erneut ausführen | Replay und Integritätsprüfung bestehen | Offen |
| M5 | Skill Reuse | passende Technik aus früheren Erfahrungen abrufen | Gegenbeispiele und Themenkontext wirken | Offen |
| M6 | BRUCE Integration | Peacemaker und BRUCE-Math anbinden | gemeinsamer End-to-End-Trace | Offen |
| M7 | Language Reconstruction | Text sicher aus AnswerPoints erzeugen | Roundtrip und Fallback bestehen | Offen |
| M8 | Hardened Reference | Leistung, Recovery und Sicherheit geprüft | vollständige Abnahmematrix | Offen |
| M9 | Reference Release | dokumentierte Referenzversion | reproduzierbare Installation | Offen |

[Zurück zur Agenda](#agenda)

---

# 7. Abhängigkeiten

## 7.1 Interne Architekturabhängigkeiten

```text
NanoNeuron-Layout
        ↓
aktive Projektion
        ↓
UniverseSpec
        ↓
TechniqueDefinition
        ↓
AnswerPoint
        ↓
ExperienceRecord
        ↓
ExperienceCell
        ↓
Hot-Skill-Cache
        ↓
Renderer
```

### Aufgaben

- [ ] Abhängigkeiten als maschinenlesbare Matrix pflegen.
- [ ] zyklische Architekturabhängigkeiten vermeiden.
- [ ] jede Schemaänderung auf nachgelagerte Komponenten prüfen.
- [ ] Änderungen an Primärverträgen über ADR steuern.

[Zurück zur Agenda](#agenda)

---

## 7.2 Entscheidungsabhängigkeiten

| Entscheidung | Blockiert |
|---|---|
| NanoNeuron-Layout | ExecutionCell-Runtime |
| aktive Projektion | Operator- und Cache-Optimierung |
| AnswerPoint-Format | semantische Konstellation und Renderer |
| SituationSignature | Erfahrungsabruf |
| Ranking-Modell | Hot-Skill-Cache |
| Persistenzmodell | Experience Ledger |
| Trace-Schema | End-to-End-Reproduzierbarkeit |
| Renderer-Grenze | SLM-Integration |
| Capability-Modell | externe Aktoren und Service-Zellen |

[Zurück zur Agenda](#agenda)

---

## 7.3 Technische Abhängigkeiten

Für den MVP bevorzugt:

- 64-Bit-CPU,
- lokale Dateisystem- oder Datenbankpersistenz,
- deterministische Standardbibliotheken,
- reproduzierbare Tests,
- keine verpflichtenden Cloud-Dienste.

Später möglich:

- SIMD-Instruktionen,
- GPU-Compute,
- FPGA,
- spezialisierte NPU,
- verteilte Experience Stores.

### Aufgaben

- [ ] minimale unterstützte Laufzeitumgebung definieren.
- [ ] Programmiersprache und Buildsystem bestätigen.
- [ ] Datenbank oder Dateiformat für Experience Ledger vergleichen.
- [ ] Benchmark-Hardware dokumentieren.
- [ ] optionale Beschleuniger hinter stabile Schnittstellen setzen.

[Zurück zur Agenda](#agenda)

---

## 7.4 Integrationsabhängigkeiten

### PeacemakerLLM

Benötigt:

- stabile Relation-IDs,
- Statusmapping,
- Trace-Referenzen.

### BRUCE-Math

Benötigt:

- stabile Formula-Tool-Verträge,
- Fehlerklassen,
- Trace- und Kosteninformationen.

### Service-Zellen

Benötigt:

- Capability-Profil,
- persistente Speichergrenze,
- Broker- oder Gateway-Regeln.

### Repository-Validierung

Benötigt:

- Manifest-Einträge,
- ADR-Regeln,
- Tests und Dokumentationsprüfungen.

[Zurück zur Agenda](#agenda)

---

## 7.5 Abhängigkeitsmatrix

| Komponente | Benötigt | Liefert | Kritischer Blocker |
|---|---|---|---|
| Active Projection Loader | NanoNeuron-Layout | kompakte Zustände | Layout offen |
| Universe Generator | UniverseSpec | Ausführungsplan | Schema offen |
| Technique Executor | TechniqueDefinition, Universe Engine | AnswerPoint-Kandidaten | Technikmodell offen |
| Constellation Builder | AnswerPoint-Schema | validierte Bedeutung | Punktformat offen |
| Experience Ledger | ExperienceRecord-Schema | persistente Erfahrungen | Persistenz offen |
| Consolidator | ExperienceRecords, Gruppierungsregeln | ExperienceCells | Ähnlichkeit offen |
| Hot-Skill-Cache | Ranking, ExperienceCells | aktive Skills | Promotion offen |
| Renderer | Konstellation, Ausgabegrenzen | Text | Roundtrip offen |
| BRUCE Integration | stabile interne Verträge | End-to-End-Funktion | Statusmapping offen |

[Zurück zur Agenda](#agenda)

---

# 8. Risiken und offene Fragen

## 8.1 Bestätigte Konflikte

### C-001 - Ein Byte gegenüber acht Byte

**Konflikt:**  
Ein NanoNeuron wird gleichzeitig als ein Byte und acht Byte beschrieben.

**Auswirkung:**  
Ohne Trennung ist kein klarer Speichervertrag möglich.

**Empfohlene Auflösung:**

```text
8 Byte kanonischer Datensatz
1 Byte aktive Projektion
```

**Status:** konzeptionell aufgelöst, technisch noch zu spezifizieren.

---

### C-002 - Antwort nicht im aktiven Speicher

**Konflikt:**  
Eine Antwort soll niemals in aktiven hochverfügbaren Speicher geladen werden, muss zur Berechnung und Ausgabe aber vorübergehend existieren.

**Empfohlene Auflösung:**  
Kurzlebige Register, Scratchpads und Ausgabepuffer sind erlaubt. Die Antwort darf nicht ungeprüft den dauerhaften aktiven Modellzustand verändern.

**Status:** konzeptionell aufgelöst.

---

### C-003 - Große Dimensionen gegenüber begrenztem Speicher

**Konflikt:**  
Die Anzahl möglicher Dimensionen und Kombinationen wächst stark, RAM und VRAM bleiben begrenzt.

**Empfohlene Auflösung:**  
Lazy Auswertung, Cascade-Filterung, explizite TechniqueDefinitions und Ressourcenbudgets.

**Status:** konzeptionell aufgelöst, Leistungsnachweis offen.

---

### C-004 - Lernen gegenüber Unveränderlichkeit

**Konflikt:**  
BRUCE soll lernen, gleichzeitig sollen kanonische Daten nicht unkontrolliert überschrieben werden.

**Empfohlene Auflösung:**  
Append-only ExperienceRecords, getrennte Bewertungen, versionierte ExperienceCells und kontrollierte Promotion.

**Status:** konzeptionell aufgelöst, Implementierung offen.

---

### C-005 - SLM gegenüber deterministischer Aussage

**Konflikt:**  
Ein generatives Modell kann sprachlich passende, aber unbelegte Aussagen ergänzen.

**Empfohlene Auflösung:**  
geschlossene AnswerPoint-Eingabe, Semantic Roundtrip Validator und strukturierter Fallback.

**Status:** Architekturvorschlag, Nachweis offen.

[Zurück zur Agenda](#agenda)

---

## 8.2 Offene Entscheidungen

| ID | Priorität | Frage | Bedeutung | Benötigte Entscheidung | Status |
|---|---|---|---|---|---|
| Q-001 | Kritisch | Wie lautet das endgültige 8-Byte-NanoNeuron-Layout? | blockiert Runtime | Feldbreiten und Seitentabellen festlegen | Offen |
| Q-002 | Kritisch | Welches Byte bildet die erste aktive Projektion? | bestimmt Hot Path | Projection Type v1 festlegen | Offen |
| Q-003 | Kritisch | Wie werden Match- und Miss-Ziele adressiert? | beeinflusst Cell-Größe | relativ, lokal oder Seitentabelle | Offen |
| Q-004 | Kritisch | Wie ist ein AnswerPoint binär aufgebaut? | beeinflusst Semantik und Cache | Core-, Structured- oder Sidecar-Modell | Offen |
| Q-005 | Kritisch | Wann gilt eine Erfahrung als verifiziert? | verhindert falsches Lernen | Verifikationsregeln festlegen | Offen |
| Q-006 | Kritisch | Wie wird die maximale Technikkomplexität begrenzt? | verhindert Endlossuche | DAG-, Tiefen- und Budgetlimits | Offen |
| Q-007 | Hoch | Wie wird Situationsähnlichkeit deterministisch berechnet? | Basis für Erfahrungsabruf | Merkmals- und Distanzmodell | Offen |
| Q-008 | Hoch | Welcher Persistenztyp wird verwendet? | Ledger, Replay und Backup | Datei, SQLite oder anderes lokales Modell | Offen |
| Q-009 | Hoch | Wie werden Rankings gewichtet? | beeinflusst Skill-Auswahl | Ranking v1 festlegen | Offen |
| Q-010 | Hoch | Wie werden Gegenbeispiele priorisiert? | verhindert Fehlgeneralisation | Negative-Match-Regel | Offen |
| Q-011 | Hoch | Welche Trace-Tiefe ist produktiv notwendig? | Speicher und Nachweis | Trace-Level definieren | Offen |
| Q-012 | Hoch | Wie wird ein SLM auf AnswerPoints begrenzt? | Ausgabeintegrität | Renderer Contract | Offen |
| Q-013 | Mittel | Müssen AnswerPoints acht Byte groß sein? | Optimierung | nach Prototyp entscheiden | Offen |
| Q-014 | Mittel | Welche Gates gehören zum MVP? | Scope | AND, OR, XOR, NOT bestätigen | Offen |
| Q-015 | Mittel | Wann erfolgt Konsolidierung? | Betriebskosten | zeit-, mengen- oder ereignisbasiert | Offen |
| Q-016 | Mittel | Wie werden seltene kritische Skills geschützt? | Cache-Qualität | Mindestreservierung oder Pinning | Offen |
| Q-017 | Niedrig | Welche Metaphern erscheinen in der UI? | Verständlichkeit | Benutzerterminologie festlegen | Offen |
| Q-018 | Niedrig | Wie werden Universen visualisiert? | Debugging | Diagramm- oder Würfelansicht | Offen |

[Zurück zur Agenda](#agenda)

---

## 8.3 Risiken

Die Bewertungen sind qualitative Ersteinschätzungen ohne Messdaten.

| ID | Risiko | Wahrscheinlichkeit | Auswirkung | Gegenmaßnahme | Restrisiko | Status |
|---|---|---|---|---|---|---|
| R-001 | exponentiell wachsender Suchraum | Hoch | Hoch | explizite Techniken, Cascade-Filter, Budgets | Mittel | Offen |
| R-002 | 8-Byte-Layout zu klein | Hoch | Hoch | Layoutbenchmarks und Seitentabellen | Mittel | Offen |
| R-003 | zu viele Seitentabellenzugriffe | Mittel | Hoch | Hot-Path-Felder priorisieren | Mittel | Offen |
| R-004 | Häufigkeits-Bias im Ranking | Hoch | Hoch | getrennte Qualitätswerte | Mittel | Offen |
| R-005 | falsche Konsolidierung | Mittel | Hoch | Gegenbeispiele und reproduzierbare Regeln | Mittel | Offen |
| R-006 | SLM ergänzt Behauptungen | Hoch | Hoch | Roundtrip und Fallback | Niedrig bis Mittel | Offen |
| R-007 | Experience Poisoning | Mittel | Hoch | Herkunft, Quarantäne, Verifikation | Mittel | Offen |
| R-008 | SSD-Schreibvolumen zu hoch | Mittel | Mittel | Batch, Kompression, Trace-Level | Mittel | Offen |
| R-009 | Kontextverwechslung | Hoch | Hoch | explizite Context-ID | Mittel | Offen |
| R-010 | Trace-Overhead | Hoch | Mittel | abgestufte Trace-Level | Mittel | Offen |
| R-011 | Verlust seltener Grenzfälle | Mittel | Hoch | Originalrecords erhalten | Niedrig | Offen |
| R-012 | Versionsdrift | Hoch | Hoch | Kompatibilitätsmatrix und Migrationstests | Niedrig bis Mittel | Offen |
| R-013 | Metaphern werden technisch missverstanden | Mittel | Mittel | formale Definitionen | Niedrig | Offen |
| R-014 | Übermäßige Regelstarre | Mittel | Mittel | Review- und Kandidatenstatus | Mittel | Offen |
| R-015 | zu breite Generalisierung | Mittel | Hoch | enge Gültigkeitsbereiche | Mittel | Offen |
| R-016 | zu geringe Generalisierung | Mittel | Mittel | ExperienceCells und Merkmalsmodelle | Mittel | Offen |
| R-017 | beschädigte Experience-Daten | Niedrig bis Mittel | Hoch | Integrität, Backup, Replay | Niedrig | Offen |
| R-018 | GPU-Optimierung ohne Nutzen | Mittel | Mittel | CPU-Profiling zuerst | Niedrig | Offen |
| R-019 | Datenschutzkonflikt mit append-only Ledger | Mittel | Hoch | getrennte Payloads und Löschkonzept | Mittel | Offen |
| R-020 | Aktor wird durch falsche Ausgabe ausgelöst | Niedrig bis Mittel | Sehr hoch | separate Autorisierung und Fail-Closed | Niedrig bis Mittel | Offen |

[Zurück zur Agenda](#agenda)

---

## 8.4 Annahmen

- [ ] `[ANNAHME]` Die für den Hot Path benötigten NanoNeuron-Felder passen in acht Byte oder können sinnvoll referenziert werden.
- [ ] `[ANNAHME]` Ein Byte aktive Projektion reicht für den ersten Operatorprototyp.
- [ ] `[ANNAHME]` AnswerPoints können diskret und typisiert modelliert werden.
- [ ] `[ANNAHME]` vollständige Erfahrungen können auf SSD gespeichert werden.
- [ ] `[ANNAHME]` Hot Skills können aus ExperienceRecords reproduzierbar rekonstruiert werden.
- [ ] `[ANNAHME]` deterministische SituationSignature ist für ausgewählte MVP-Domänen möglich.
- [ ] `[ANNAHME]` ein thematischer Renderer kann durch geschlossene Eingaben begrenzt werden.
- [ ] `[ANNAHME]` CPU und RAM reichen für einen aussagekräftigen MVP.
- [ ] `[ANNAHME]` spätere SIMD- oder GPU-Optimierung kann hinter stabilen Verträgen erfolgen.

### Aufgaben

- [ ] jede Annahme mindestens einem Test oder Prototypnachweis zuordnen.
- [ ] widerlegte Annahmen als Architekturänderung dokumentieren.
- [ ] Annahmen nicht als abgeschlossene Funktion markieren.

[Zurück zur Agenda](#agenda)

---

## 8.5 Bewusste Nicht-Ziele

- keine unkontrollierte generative Wissensbildung,
- keine physikalische Quantenarchitektur,
- keine globale Vollsuche sämtlicher Bitkombinationen,
- keine automatische Freigabe kritischer Aktionen,
- keine GPU-Pflicht,
- kein sofortiger Spezialchip,
- keine ungeprüfte Technikselbstmodifikation,
- keine Löschung von Gegenbeispielen zugunsten schöner Erfolgsquoten.

[Zurück zur Agenda](#agenda)

---

# 9. Prüfung und Abnahme

## 9.1 Teststrategie

Jedes Hauptziel benötigt mindestens einen Nachweis:

| Hauptziel | Nachweis |
|---|---|
| 8-Byte-NanoNeuron | binäre Größen- und Layouttests |
| aktive Projektion | reproduzierbarer Projection-Test |
| deterministische Universen | wiederholte Testvektoren |
| lazy Auswertung | Speicherprofil ohne Vollmaterialisierung |
| AnswerPoints | Schema- und Konflikttests |
| Experience Ledger | Persistenz-, Replay- und Integritätstest |
| Skill-Abruf | bekannte Situation mit Gegenbeispiel |
| Renderer-Grenze | Halluzinations- und Negationstest |
| Recovery | Backup-, Restore- und Crash-Test |
| Integration | End-to-End-Trace |

[Zurück zur Agenda](#agenda)

---

## 9.2 Unit-Tests

- [ ] Shift links für alle Bytewerte testen.
- [ ] Shift rechts für alle Bytewerte testen.
- [ ] Rotation links für alle Bytewerte testen.
- [ ] Rotation rechts für alle Bytewerte testen.
- [ ] Gate-Wahrheitstabellen testen.
- [ ] Masken-Match testen.
- [ ] Masken-Miss testen.
- [ ] Hamming-Distanz testen.
- [ ] Popcount testen.
- [ ] Projection Packing testen.
- [ ] Layout-Validator testen.
- [ ] UniverseSpec-Validator testen.
- [ ] AnswerPoint-Encoding testen.
- [ ] Zustandsübergänge testen.
- [ ] Ranking-Teilwerte testen.

### Mathematische Eigenschaften

- [ ] `ROTL(b, 8) == b`
- [ ] `ROTR(b, 8) == b`
- [ ] `b XOR b == 0`
- [ ] `b AND 255 == b`
- [ ] `b OR 0 == b`
- [ ] `NOT(NOT(b)) == b`

[Zurück zur Agenda](#agenda)

---

## 9.3 Integrationstests

- [ ] NanoNeuron zu aktiver Projektion.
- [ ] Projektion zu UniverseSpec-Ausführung.
- [ ] Universum zu AnswerPoint.
- [ ] mehrere AnswerPoints zu Konstellation.
- [ ] Konstellation zu strukturierter Ausgabe.
- [ ] Technik zu ExperienceRecord.
- [ ] ExperienceRecord zu Replay.
- [ ] mehrere Records zu ExperienceCell.
- [ ] ExperienceCell zu Hot-Skill-Cache.
- [ ] Peacemaker-Ergebnis zu AnswerPoints.
- [ ] BRUCE-Math-Ergebnis zu AnswerPoints.
- [ ] Renderer zu Roundtrip Validator.

[Zurück zur Agenda](#agenda)

---

## 9.4 End-to-End-Tests

### E2E-001 - Muster direkt erkannt

```text
Eingabe
→ Shape-Match
→ AnswerPoint
→ Ausgabe
→ ExperienceRecord
```

### E2E-002 - Muster erst nach Rotation

```text
Eingabe
→ Originaluniversum Miss
→ Rotationsuniversum Match
→ Technik erfolgreich
→ Erfahrung speichern
```

### E2E-003 - Shape passt, Kontext widerspricht

```text
Shape Match
→ Context Miss
→ Technik ablehnen
→ negative Erfahrung
```

### E2E-004 - frühere Erfahrung beschleunigt Auswahl

```text
erste Situation
→ mehrere Techniken prüfen
→ Erfahrung speichern

ähnliche Situation
→ ExperienceCell finden
→ passende Technik zuerst prüfen
```

### E2E-005 - häufige Technik scheitert an Gegenbeispiel

```text
hoher Rank
→ Gegenregel trifft
→ Technik ablehnen
→ seltene gültige Technik wählen
```

### E2E-006 - Renderer ergänzt Aussage

```text
AnswerPoints
→ Text mit zusätzlicher Behauptung
→ Roundtrip Miss
→ Text verwerfen
→ Template-Fallback
```

[Zurück zur Agenda](#agenda)

---

## 9.5 Negativ- und Grenzwerttests

- [ ] leere ExecutionCell.
- [ ] maximaler lokaler Index.
- [ ] ungültige Cell-Version.
- [ ] unbekannter Projektionstyp.
- [ ] ungültige Shift-Anzahl.
- [ ] unbekannte Gate-ID.
- [ ] fehlende Maske.
- [ ] fehlender Kontext.
- [ ] zirkulärer Technikgraph.
- [ ] überschrittenes Universumslimit.
- [ ] überschrittenes Scratchpad-Limit.
- [ ] null AnswerPoints.
- [ ] widersprüchliche Pflichtpunkte.
- [ ] beschädigter ExperienceRecord.
- [ ] voller Hot-Skill-Cache.
- [ ] nicht kompatible Ranking-Version.
- [ ] fehlender Trace.
- [ ] SLM-Ausfall.
- [ ] SSD nicht verfügbar.
- [ ] RAM-Druck.

[Zurück zur Agenda](#agenda)

---

## 9.6 Fehler-Injektion

- [ ] Prozessabbruch während Record-Schreibvorgang.
- [ ] beschädigter Index.
- [ ] falscher Integritätswert.
- [ ] Timeout eines Universums.
- [ ] Operator wirft internen Fehler.
- [ ] Renderer liefert leere Antwort.
- [ ] Renderer vertauscht Negation.
- [ ] Cache enthält inkompatible ExperienceCell.
- [ ] Migration wird unterbrochen.
- [ ] Seitentabelle fehlt.
- [ ] Trace-Speicher erreicht Limit.
- [ ] Backup ist unvollständig.

### Abnahmekriterien

- [ ] Fehler bleibt lokal.
- [ ] keine unvollständige Erfahrung wird verifiziert.
- [ ] Wiederanlauf ist dokumentiert.
- [ ] Fehlerstatus erscheint im Trace.
- [ ] kritische Aktionen bleiben gesperrt.

[Zurück zur Agenda](#agenda)

---

## 9.7 Ressourcenmessungen

Zu messen sind:

- CPU-Zyklen je Operator,
- Zeit je Universum,
- Zeit je Technik,
- RAM je ExecutionCell,
- Scratchpad-Größe,
- AnswerPoint-Speicher,
- Cache-Miss-Rate,
- SSD-Schreibvolumen je Erfahrung,
- Experience-Suchzeit,
- Konsolidierungszeit,
- Trace-Größe,
- Renderer- und Roundtrip-Kosten.

### Aufgaben

- [ ] Benchmark-Harness definieren.
- [ ] Hardwareprofil automatisch erfassen.
- [ ] Warm- und Cold-Cache unterscheiden.
- [ ] Referenzdaten festlegen.
- [ ] Messergebnisse versionieren.
- [ ] keine theoretischen Schätzungen als reale Benchmarks ausgeben.

[Zurück zur Agenda](#agenda)

---

## 9.8 Gesamtabnahmekriterien

- [ ] Die Komponente besitzt eine eindeutig beschriebene Verantwortung.
- [ ] Eingaben und Ausgaben sind dokumentiert.
- [ ] Fehlerzustände sind definiert.
- [ ] Sicherheitsgrenzen sind beschrieben.
- [ ] Abhängigkeiten sind nachvollziehbar.
- [ ] Tests decken Normal-, Grenz- und Fehlerfälle ab.
- [ ] Ergebnisse sind reproduzierbar.
- [ ] Entscheidungen sind begründet dokumentiert.
- [ ] keine kritische Ausgabe erfolgt ohne Trace.
- [ ] kanonische NanoNeuronen werden nicht unkontrolliert verändert.
- [ ] ExperienceRecords sind unveränderlich.
- [ ] Hot Skills können rekonstruiert werden.
- [ ] Renderer kann keine unbelegte Aussage freigeben.
- [ ] Recovery-Verfahren sind getestet.

[Zurück zur Agenda](#agenda)

---

# 10. Sicherheit, Betrieb und Datenlebenszyklus

## 10.1 Vertrauensmodell

```text
Externe Eingabe
    ↓ nicht vertrauenswürdig
Input Validator
    ↓ kontrolliert
NanoCell Runtime
    ↓ deterministisch, aber prüfpflichtig
Experience Ledger
    ↓ persistent und integritätsgeschützt
Consolidator
    ↓ darf ableiten, nicht überschreiben
Renderer
    ↓ inhaltlich nicht voll vertrauenswürdig
Roundtrip Validator
    ↓ Freigabegrenze
Ausgabe oder Aktor
```

### Aufgaben

- [ ] Vertrauenszonen dokumentieren.
- [ ] Datenübergänge zwischen Zonen definieren.
- [ ] Rechte je Komponente festlegen.
- [ ] kritische Freigabegrenzen testen.
- [ ] Threat Model erstellen.

[Zurück zur Agenda](#agenda)

---

## 10.2 Berechtigungen und Capabilities

### Vorgesehene Capabilities

- `READ_EXECUTION_CELL`
- `EXECUTE_UNIVERSE`
- `READ_EXPERIENCE`
- `APPEND_EXPERIENCE`
- `CONSOLIDATE_EXPERIENCE`
- `REBUILD_SKILL_CACHE`
- `RENDER_TEXT`
- `VALIDATE_RENDERED_TEXT`
- `EXECUTE_EXTERNAL_ACTION`

### Aufgaben

- [ ] Capability-Namen und Semantik festlegen.
- [ ] Runtime ohne Schreibrecht auf kanonische Cells betreiben.
- [ ] Renderer ohne Experience-Schreibrecht betreiben.
- [ ] Aktor-Capability separat freigeben.
- [ ] administrative Capabilities protokollieren.
- [ ] Least-Privilege-Tests erstellen.

> **Sicherheit:** Keine Komponente erhält pauschal Zugriff auf alle Daten und Aktionen.

[Zurück zur Agenda](#agenda)

---

## 10.3 Installation und Initialisierung

### Installation

- Runtime installieren,
- Speicherpfade anlegen,
- NanoNeuron-Schema registrieren,
- Technique Registry laden,
- Experience Store initialisieren,
- Testfixtures installieren.

### Initialisierung

1. Versionen prüfen.
2. Layouts validieren.
3. Experience Ledger prüfen.
4. ExperienceCells laden.
5. Hot-Skill-Cache rekonstruieren.
6. Operator-Selbsttests ausführen.
7. Systemstatus veröffentlichen.

### Aufgaben

- [ ] Installationsanleitung schreiben.
- [ ] Initialisierungsreihenfolge automatisieren.
- [ ] beschädigten Ledger erkennen.
- [ ] leeren Erststart unterstützen.
- [ ] Wiederanlauf nach Cache-Verlust unterstützen.

[Zurück zur Agenda](#agenda)

---

## 10.4 Regulärer Betrieb

- Eingaben validieren,
- Techniken auswählen,
- Universen ausführen,
- Ergebnisse prüfen,
- Erfahrungen speichern,
- Rankings an kontrollierten Zeitpunkten aktualisieren,
- Cache überwachen,
- Ressourcenlimits durchsetzen.

### Aufgaben

- [ ] Betriebszustände definieren.
- [ ] Wartungsmodus definieren.
- [ ] Konsolidierung vom Live-Hot-Path trennen.
- [ ] Schreiblast überwachen.
- [ ] Safe-Shutdown-Verhalten dokumentieren.
- [ ] manuelle Review-Warteschlange definieren.

[Zurück zur Agenda](#agenda)

---

## 10.5 Monitoring und Wartung

Zu überwachen:

- Fehlerraten,
- Abbruchraten,
- Universumsanzahl,
- Techniklaufzeiten,
- SSD-Schreibvolumen,
- Cache-Hit-Rate,
- widersprüchliche Erfahrungen,
- Renderer-Roundtrip-Fehler,
- beschädigte Records,
- Versionsinkompatibilitäten.

### Aufgaben

- [ ] Monitoring-Metriken definieren.
- [ ] Warnschwellen erst nach Messdaten festlegen.
- [ ] Wartungsberichte erzeugen.
- [ ] Quarantänebereiche überwachen.
- [ ] Konsolidierungsqualität prüfen.
- [ ] alte Technikversionen kontrolliert auslaufen lassen.

[Zurück zur Agenda](#agenda)

---

## 10.6 Updates und Wiederherstellung

### Updates

- Runtime,
- Layout,
- Technique Registry,
- Ranking,
- Renderer,
- Trace-Schema

getrennt versionieren.

### Wiederherstellung

```text
bekannte Runtime-Version
+ kanonische NanoNeuronen
+ TechniqueDefinitions
+ Experience Ledger
+ EvaluationRecords
→ ExperienceCells neu erzeugen
→ Hot-Skill-Cache neu erzeugen
```

### Aufgaben

- [ ] Backupumfang festlegen.
- [ ] Restore-Prozess automatisieren.
- [ ] Cache als rekonstruierbar behandeln.
- [ ] Migration und Rollback testen.
- [ ] inkompatible Daten isolieren.
- [ ] Recovery Point und Recovery Time erst nach Tests bewerten.

[Zurück zur Agenda](#agenda)

---

## 10.7 Datenschutz und Aufbewahrung

### Mögliche sensible Daten

- Benutzereingaben,
- Kommunikationsinhalte,
- Sensorwerte,
- Bewertungsdaten,
- personenbezogene SituationSignatures,
- Traces mit Rohdaten.

### Aufgaben

- [ ] Datenklassen nach Sensibilität einstufen.
- [ ] Rohdaten von Technikmetadaten trennen.
- [ ] Pseudonymisierung prüfen.
- [ ] Lösch- und Sperrmodell definieren.
- [ ] Aufbewahrungsfristen als konfigurierbar behandeln.
- [ ] Exportverfahren definieren.
- [ ] Zugriff auf ExperienceRecords protokollieren.
- [ ] append-only Ledger mit Löschanforderungen abgleichen.
- [ ] verschlüsselte Payload-Segmente prüfen.

> **Hinweis:** Eine konkrete rechtliche Bewertung muss separat und anhand des tatsächlichen Einsatzgebiets erfolgen.

[Zurück zur Agenda](#agenda)

---

# 11. Dokumentation und Repository-Struktur

## 11.1 Dokumentationsindex

### Benötigte Kerndokumente

- [ ] NanoCell Overview
- [ ] NanoNeuron Memory Contract
- [ ] ExecutionCell Runtime Model
- [ ] Universe Execution Model
- [ ] Technique Definition Model
- [ ] AnswerPoint and Semantic Constellation Model
- [ ] Experience Learning Model
- [ ] Skill Ranking and Cache Model
- [ ] Semantic Rendering Boundary
- [ ] Trace and Replay Model
- [ ] Security and Capability Model
- [ ] Test and Validation Plan
- [ ] Benchmark Plan
- [ ] Open Questions
- [ ] Project Status
- [ ] Repository Manifest

### Kleine Anleitung

1. Primärdokumente eindeutig kennzeichnen.
2. Detaildokumente auf Primärverträge verlinken.
3. Begriffe nicht in mehreren Dateien unterschiedlich definieren.
4. Änderungen an Verträgen gleichzeitig in Roadmap und ADRs nachführen.
5. Importierte Primärquellen nicht still korrigieren.

[Zurück zur Agenda](#agenda)

---

## 11.2 Vorgeschlagene Repository-Struktur

> `[VORSCHLAG]` Die folgenden Pfade sind eine mögliche Zielstruktur und noch nicht als bestehende Dateien zu behandeln.

```text
docs/
├─ architecture/
│  ├─ NANOCELL_OVERVIEW.md
│  ├─ NANONEURON_MEMORY_CONTRACT.md
│  ├─ EXECUTION_CELL_RUNTIME.md
│  ├─ UNIVERSE_EXECUTION_MODEL.md
│  ├─ TECHNIQUE_DEFINITION_MODEL.md
│  ├─ ANSWER_POINT_MODEL.md
│  ├─ EXPERIENCE_LEARNING_MODEL.md
│  ├─ SKILL_CACHE_MODEL.md
│  ├─ SEMANTIC_RENDERING_BOUNDARY.md
│  └─ TRACE_AND_REPLAY_MODEL.md
├─ security/
│  ├─ TRUST_MODEL.md
│  └─ CAPABILITY_MODEL.md
├─ project/
│  ├─ ROADMAP.md
│  ├─ PROJECT_STATUS.md
│  ├─ OPEN_QUESTIONS.md
│  └─ REPOSITORY_MANIFEST.md
├─ testing/
│  ├─ TEST_PLAN.md
│  ├─ FIXTURE_MODEL.md
│  └─ BENCHMARK_PLAN.md
└─ decisions/
   ├─ ADR-XXXX-nanoneuron-eight-byte-layout.md
   ├─ ADR-XXXX-active-projection-model.md
   ├─ ADR-XXXX-universe-specification.md
   ├─ ADR-XXXX-experience-ledger.md
   ├─ ADR-XXXX-skill-ranking.md
   └─ ADR-XXXX-semantic-rendering-boundary.md

src/
└─ nanocell/
   ├─ runtime/
   ├─ neuron/
   ├─ execution_cell/
   ├─ universe/
   ├─ gates/
   ├─ patterns/
   ├─ techniques/
   ├─ answer_points/
   ├─ experience/
   ├─ ranking/
   ├─ rendering/
   └─ trace/

tests/
├─ unit/
├─ integration/
├─ end_to_end/
├─ fixtures/
├─ negative/
└─ benchmarks/

tools/
└─ validation/
```

### Aufgaben

- [ ] bestehende Repository-Struktur prüfen.
- [ ] vorgeschlagene Pfade mit vorhandenen Konventionen abgleichen.
- [ ] keine parallelen Duplikatstrukturen erzeugen.
- [ ] Manifest und Validator aktualisieren.
- [ ] Dateiverantwortung dokumentieren.

[Zurück zur Agenda](#agenda)

---

## 11.3 Architekturentscheidungen

### Empfohlene ADRs

- [ ] ADR - Acht-Byte-NanoNeuron-Vertrag
- [ ] ADR - aktive Projektion
- [ ] ADR - lokale 16-Bit-IDs
- [ ] ADR - UniverseSpec statt materialisiertem Dimensionsraum
- [ ] ADR - TechniqueDefinition als begrenzter Graph
- [ ] ADR - append-only Experience Ledger
- [ ] ADR - ExperienceCell-Konsolidierung
- [ ] ADR - Rang ist keine Gültigkeit
- [ ] ADR - Renderer ist keine Wahrheitsquelle
- [ ] ADR - CPU-Referenzpfad vor GPU-Optimierung
- [ ] ADR - Quantenteleportation und Paralleluniversum nur als Metaphern

### Abnahmekriterien

- [ ] Jede ADR nennt Kontext, Entscheidung, Alternativen und Konsequenzen.
- [ ] widersprüchliche ADRs werden erkannt.
- [ ] veraltete Entscheidungen werden als ersetzt markiert.
- [ ] Primärquellen werden eindeutig referenziert.

[Zurück zur Agenda](#agenda)

---

## 11.4 Änderungs- und Statuspflege

Nach jeder relevanten Änderung aktualisieren:

- Roadmap,
- Projektstatus,
- offene Fragen,
- Architekturdiagramm,
- ADRs,
- Testmatrix,
- Repository-Manifest,
- Fortschrittsübersicht.

### Aufgaben

- [ ] Statuspflege in Pull-Request- oder Review-Checkliste aufnehmen.
- [ ] Validator für fehlende Dokumentlinks prüfen.
- [ ] veraltete Versionsangaben erkennen.
- [ ] offene Fragen nach Entscheidungen schließen oder aktualisieren.
- [ ] keine Aufgabe ohne Nachweis auf `[x]` setzen.

> **Dokumentation:** Implementierung, Tests und Architekturtext müssen denselben Stand beschreiben.

[Zurück zur Agenda](#agenda)

---

# 12. Nächste konkrete Schritte

1. [ ] Drei repräsentative NanoCell-Techniken als konkrete Use Cases auswählen.
2. [ ] Für diese Techniken alle benötigten NanoNeuron-Felder erfassen.
3. [ ] Drei mögliche 8-Byte-NanoNeuron-Layouts gegenüberstellen.
4. [ ] `STATE_BYTE` als erste aktive Projektion präzise definieren.
5. [ ] UniverseSpec v1 für Shift, Rotation, Gate, Maske und Kontext formulieren.
6. [ ] AnswerPoint v1 mit Pflichtfeldern und Trace-Referenz definieren.
7. [ ] ExperienceRecord v1 als append-only Vertrag formulieren.
8. [ ] ADRs für Speichervertrag, UniverseSpec und Experience Ledger erstellen.
9. [ ] den minimalen Phase-1-Testkatalog für alle 256 Bytewerte erstellen.
10. [ ] erst danach das Repository-Grundgerüst für den CPU-Prototyp anlegen.

> **Nächster Fokus:** Der größte Erkenntnisgewinn entsteht durch einen kleinen Bit-Universe-Prototypen mit stabilen Datenverträgen.

[Zurück zur Agenda](#agenda)

---

# 13. Erinnerungen und Arbeitsregeln

> **BRUCE-Prinzip:** Erst den passenden Slot bestimmen, danach innerhalb dieses Slots weiterarbeiten.

> **Cascade-Filterung:** Grobe und kostengünstige Prüfungen vor teuren Detailberechnungen ausführen.

> **Montessori-Formenbrett-Prinzip:** Informationen nur dem Abschnitt, Datentyp oder Verarbeitungspfad zuordnen, zu dem ihre Funktion tatsächlich passt.

> **Peacemaker-Prinzip:** Widersprüche nicht verdecken. Beide Seiten dokumentieren, Auswirkungen beschreiben und eine Entscheidung anfordern.

> **Capability-Prinzip:** Komponenten besitzen nur die Fähigkeiten, die sie für ihre konkrete Verantwortung benötigen.

> **Determinismus:** Gleiche Eingaben und Versionen müssen gleiche logische Ergebnisse erzeugen.

> **Nachweis:** Kein bestätigtes Ergebnis ohne Trace.

> **Lernen:** Nicht nur Erfolge speichern. Fehlschläge, Grenzen und Gegenbeispiele gehören ebenfalls zur Erfahrung.

> **Ranking:** Häufigkeit ist kein Qualitätsnachweis.

> **Speicher:** Virtuelle Räume berechnen, nicht automatisch materialisieren.

> **Renderer:** Sprache ist Ausgabeform und keine neue Wahrheitsquelle.

> **Optimierung:** Zuerst den korrekten CPU-Referenzpfad messen, danach SIMD, GPU oder Hardware prüfen.

> **Dokumentation:** Keine Aufgabe als abgeschlossen markieren, wenn ihr Abnahmekriterium nicht nachgewiesen ist.

[Zurück zur Agenda](#agenda)

---

# 14. Ideenparkplatz

- [ ] `[SPÄTER]` SIMD-Ausführung mehrerer NanoNeuronen
  - Nutzen: mehrere aktive Zustände gemeinsam verarbeiten.
  - Voraussetzung: stabiler CPU-Referenzpfad.
  - mögliche Phase: Phase 8.
  - offene Frage: geeignetes Packing und Alignment.

- [ ] `[SPÄTER]` GPU-Backend für große gleichartige Universumsmengen
  - Nutzen: parallele Gate- und Maskenoperationen.
  - Voraussetzung: Profiling weist einen realen Vorteil nach.
  - mögliche Phase: nach Phase 8.
  - offene Frage: Transfer- und Trace-Kosten.

- [ ] `[SPÄTER]` FPGA- oder Hardware-NanoCell
  - Nutzen: spezialisierte deterministische Ausführung.
  - Voraussetzung: stabile Operator- und Speicherverträge.
  - mögliche Phase: Forschung nach Referenzfreigabe.
  - offene Frage: welche Teile lohnen sich in Hardware?

- [ ] `[SPÄTER]` selbstständige Technikkomposition
  - Nutzen: bekannte Werkzeuge zu neuen Technikkandidaten kombinieren.
  - Voraussetzung: sichere Kandidaten- und Review-Pipeline.
  - mögliche Phase: experimentell nach Phase 9.
  - offene Frage: Wie wird Suchraumexplosion verhindert?

- [ ] `[SPÄTER]` binäre Semantic State Reconstruction
  - Nutzen: AnswerPoints direkt zwischen BRUCE-Systemen übertragen.
  - Voraussetzung: stabiler AnswerPoint-Vertrag.
  - mögliche Phase: nach Phase 7.
  - offene Frage: Kompatibilität zwischen Wissensversionen.

- [ ] `[SPÄTER]` visuelle Paralleluniversums- und Würfelansicht
  - Nutzen: Debugging und Erklärung.
  - Voraussetzung: stabiler Trace.
  - mögliche Phase: Phase 8 oder später.
  - offene Frage: Welche Darstellung bleibt bei hohen Dimensionen verständlich?

- [ ] `[SPÄTER]` thematische Skill-Pakete
  - Nutzen: kleinere und isolierte Hot-Sets.
  - Voraussetzung: funktionierender Themenindex.
  - mögliche Phase: Phase 5 oder später.
  - offene Frage: Paketgrenzen und Versionskompatibilität.

- [ ] `[SPÄTER]` verteilte Experience Stores
  - Nutzen: mehrere BRUCE-Instanzen teilen bestätigte Erfahrungen.
  - Voraussetzung: Provenienz, Signaturen und Konfliktmodell.
  - mögliche Phase: nach Phase 9.
  - offene Frage: Wie werden widersprüchliche Erfahrungen zusammengeführt?

- [ ] `[SPÄTER]` Service-Zellen-Ausführung
  - Nutzen: isolierte und capability-basierte NanoCell-Komponenten.
  - Voraussetzung: stabile Runtime und Capability-Verträge.
  - mögliche Phase: Integrationsforschung.
  - offene Frage: Persistenz- und Trace-Broker.

- [ ] `[SPÄTER]` tieferes themenorientiertes SLM
  - Nutzen: natürlichere Sprache.
  - Voraussetzung: sicherer Roundtrip.
  - mögliche Phase: nach Phase 7.
  - offene Frage: Wie wird freie Wissensbeimischung verhindert?

- [ ] `[-]` vollständige Materialisierung aller Dimensionsräume
  - Grund: unverhältnismäßiger Speicher- und Rechenbedarf.
  - Alternative: lazy Auswertung und Cascade-Filterung.

- [ ] `[-]` direkte kritische Aktorsteuerung durch generierten Text
  - Grund: unzureichende Nachweis- und Sicherheitsgrenze.
  - Alternative: strukturierter Aktorvertrag mit separater Autorisierung.

[Zurück zur Agenda](#agenda)

---

# 15. Fortschrittsübersicht

| Bereich oder Phase | Status | Erledigt | Offen | Blockiert | Nächster Schritt |
|---|---|---:|---:|---:|---|
| Projektgrundlage | Teilweise abgeschlossen | Nicht bewertet | Nicht bewertet | 0 | Hauptziel in MVP-Verträge übersetzen |
| Architekturbegriffe | In Bearbeitung | Nicht bewertet | Nicht bewertet | 0 | Glossar und ADRs erstellen |
| NanoNeuron-Speichervertrag | In Bearbeitung | Nicht bewertet | Nicht bewertet | 1 | drei Layoutvarianten vergleichen |
| UniverseSpec | In Bearbeitung | Nicht bewertet | Nicht bewertet | 0 | Schema v1 formulieren |
| Bit-Universe-Prototyp | Nicht begonnen | 0 | Nicht bewertet | 1 | Operatorverträge abschließen |
| ExecutionCell-Runtime | Nicht begonnen | 0 | Nicht bewertet | 1 | NanoNeuron-Layout festlegen |
| AnswerPoint-System | Nicht begonnen | 0 | Nicht bewertet | 1 | Punktformat festlegen |
| Experience Ledger | Nicht begonnen | 0 | Nicht bewertet | 1 | Persistenzmodell auswählen |
| Experience Consolidation | Nicht begonnen | 0 | Nicht bewertet | 1 | SituationSignature definieren |
| Hot-Skill-Cache | Nicht begonnen | 0 | Nicht bewertet | 1 | Ranking-Modell definieren |
| Peacemaker-Integration | Nicht begonnen | 0 | Nicht bewertet | 0 | Status- und ID-Mapping planen |
| BRUCE-Math-Integration | Nicht begonnen | 0 | Nicht bewertet | 0 | Formula-Technikvertrag planen |
| Sprachrekonstruktion | Nicht begonnen | 0 | Nicht bewertet | 1 | AnswerPoint-System zuerst abschließen |
| Optimierung und Härtung | Nicht begonnen | 0 | Nicht bewertet | 0 | erst nach End-to-End-Prototyp |
| Referenzfreigabe | Nicht begonnen | 0 | Nicht bewertet | 0 | alle vorherigen Phasen |

## Gesamtbewertung

| Aspekt | Status |
|---|---|
| Grundidee | klar beschrieben |
| zentrale Metaphern | technisch abgegrenzt |
| Speicherziel | konzeptionell geklärt |
| exaktes Byte-Layout | offen |
| Runtime | noch nicht implementiert |
| Erfahrungslernen | konzeptionell beschrieben |
| Persistenzvertrag | offen |
| SLM-Grenze | konzeptionell beschrieben |
| End-to-End-Nachweis | fehlt |
| produktive Reife | nicht erreicht |

[Zurück zur Agenda](#agenda)

---

# 16. BRUCE-Schlagwörter

## Zentrale BRUCE-Begriffe

`BRUCE` · `BRUCE-Architektur` · `BRUCE-Prinzip` · `BRUCE-Hierarchie` · `BRUCE NanoCell` · `NanoCell` · `deterministische Bedeutungsverarbeitung` · `deterministische Erfahrungsverarbeitung`

## Struktur und Routing

`Semantic Routing` · `BRUCE-Semantic-Routing-Architecture` · `Cascade-Filterung` · `Suchraum-Reduktion` · `Slot-Prinzip` · `Montessori-Formenbrett-Prinzip` · `Shape Matching` · `Hierarchisches Routing` · `Deterministisches Routing` · `SituationSignature` · `Context Routing` · `Match-Routing` · `Miss-Routing`

## Verarbeitung und Ausführung

`CPU-RAM-NPU` · `ExecutionCell` · `NanoNeuron` · `8-Byte-NanoNeuron` · `1-Byte-Projektion` · `Active Projection` · `Funktions-ID` · `Tabellen-ID` · `Shape-Maske` · `Shape-Wert` · `Match-Ziel` · `Miss-Ziel` · `Ausgangs-Slot` · `Flags` · `16-Bit-ID` · `lokaler Index` · `Seitentabelle` · `Deterministische Ausführung` · `64-Bit-Packing` · `SIMD`

## Paralleluniversen und Prozessdimensionen

`Paralleluniversum` · `UniverseSpec` · `Universe Generator` · `virtueller Prozessraum` · `BRUCE-Prozessdimension` · `lazy evaluation` · `bedarfsorientierte Berechnung` · `Dimensionsraum` · `Cube-to-Cube` · `AnswerPoint Projection` · `Semantic State Reconstruction`

## Bitoperationen und Logik

`Logischer Shift` · `Shift Left` · `Shift Right` · `Rotation Left` · `Rotation Right` · `Bitweiser Operator` · `AND` · `OR` · `XOR` · `NOT` · `NAND` · `NOR` · `XNOR` · `Gate Engine` · `Gate Profile` · `Operator Family` · `Logikgatter` · `Logikbaustein` · `Wahrheitstabelle`

## Muster und Merkmale

`Mustererkennung` · `Pattern Engine` · `Mask Engine` · `Context Engine` · `Popcount` · `Hamming-Distanz` · `Bitübergang` · `Symmetrie` · `zyklisches Muster` · `Positivmuster` · `Negativmuster` · `Shape Profile` · `Montessori-Matching`

## Techniken und Erfahrungslernen

`TechniqueDefinition` · `Technique Registry` · `Technique Selector` · `Technique Executor` · `Technikgraph` · `DAG` · `Zustandsmaschine` · `ExperienceRecord` · `EvaluationRecord` · `ExperienceCell` · `Experience Ledger` · `Experience Replay` · `Skill Ranking` · `Hot Skill` · `Hot-Skill-Cache` · `Gegenregel` · `Gegenbeispiel` · `prozedurales Gedächtnis` · `episodisches Gedächtnis` · `Erfahrungskonsolidierung`

## AnswerPoints und Sprache

`AnswerPoint` · `semantische Konstellation` · `Topic-ID` · `Concept-ID` · `Relation-ID` · `Polarität` · `Evidenzklasse` · `Structured Output Engine` · `Template Renderer` · `SLM Renderer` · `Small Language Model` · `Semantic Roundtrip Validator` · `Vector-to-Language` · `sprachliche Rekonstruktion`

## Wissen und Entscheidungen

`PeacemakerLLM` · `Rule Engine` · `Direct Relation Engine` · `Trace Engine` · `Response Builder` · `FOUND` · `UNKNOWN` · `AMBIGUOUS` · `REQUIRES_REVIEW` · `SUPPORTED` · `CONFLICTING` · `VERIFIED` · `CONTRADICTED` · `SUPERSEDED` · `CONSOLIDATED`

## Mathematik und Formeln

`BRUCE-Math` · `BigInt` · `Rational Arithmetic` · `Modular Arithmetic` · `Formula Engine` · `Domain Enforcement` · `Safe Shutdown` · `Formula Trace` · `Exact Evaluation` · `Parameter Validation` · `Cost Formula` · `Resource Bound`

## Architektur und Isolation

`Service-Zelle` · `Kompilierte Service-Zelle` · `Capability` · `Capability-Based Architecture` · `Least Privilege` · `Zero Trust` · `Trust Boundary` · `Gateway` · `Broker` · `Allow-List` · `Read-Only` · `Fail-Closed` · `Sandbox` · `Aktor-Capability`

## Speicher und Datenlebenszyklus

`SSD Experience Store` · `RAM Skill Cache` · `append-only` · `Persistenz` · `flüchtiger Zustand` · `Scratchpad` · `Versionierung` · `Migration` · `Integritätsprüfung` · `Backup` · `Restore` · `Replay` · `Datenaufbewahrung` · `Quarantäne`

## Qualität und Nachvollziehbarkeit

`Determinismus` · `Reproduzierbarkeit` · `Traceability` · `Audit Trail` · `Safe Shutdown` · `Validierung` · `Verifikation` · `Abnahmekriterium` · `Regressionstest` · `Grenzfall` · `Fehlerfall` · `Property Test` · `Fehler-Injektion` · `End-to-End-Test` · `Benchmark` · `Resource Budget`

## Planung und Dokumentation

`Roadmap` · `Agenda` · `Milestone` · `Phase` · `Task` · `Subtask` · `Blocker` · `Dependency` · `Definition of Done` · `Acceptance Criteria` · `ADR` · `Projektstatus` · `Repository Manifest` · `Open Questions` · `Test Plan` · `Benchmark Plan`

## Projektspezifische Metaphern

`Lemmings-Prinzip` · `Wenn Klippe dann Brücke` · `Wenn Wand dann Leiter` · `Paralleluniversum` · `Bedeutungs-Teleportation` · `Quantenteleportation als Metapher` · `philosophischer Binärtraum` · `Formenbrett` · `Aufgabenuniversum`

## Projektspezifische Phasen

`Phase 0 Architekturverträge` · `Phase 1 Bit-Universe-Prototyp` · `Phase 2 ExecutionCell-Runtime` · `Phase 3 AnswerPoint-System` · `Phase 4 Experience Ledger` · `Phase 5 Erfahrungsabruf` · `Phase 6 BRUCE-Integration` · `Phase 7 Sprachrekonstruktion` · `Phase 8 Optimierung und Härtung` · `Phase 9 Referenzfreigabe`
```
