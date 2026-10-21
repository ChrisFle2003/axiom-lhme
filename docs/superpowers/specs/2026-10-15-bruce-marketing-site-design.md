# BRUCE Marketing Website – Design Spec

**Status:** vorgeschlagen und vom Nutzer in der Seitenarchitektur bestätigt  
**Zielgruppe:** Allgemeinpublikum; Menschen mit Interesse an nachvollziehbarer KI, Forschung und Projektkooperationen  
**Visuelle Richtung:** „Orbital Odyssey“ – cinematic, kosmisch, hochwertig, präzise  
**Leitsatz:** *Answers you can follow. Intelligence with evidence.*

## 1. Zweck und Erfolgskriterien

Die Website erklärt BRUCE ohne Vorwissen. Sie vermittelt nicht die vollständige technische Spezifikation, sondern die zentrale Idee: Eine Antwort soll Herkunft, Evidenz und Grenzen zeigen können.

Die Seite erfüllt drei gleichwertige Aufgaben:

1. BRUCE verständlich machen.
2. Den Entwicklungsstand anhand von GitHub-Branches nachvollziehbar zeigen.
3. Qualifizierte Anfragen für Forschungs-, Kooperations- oder Pilotprojekte ermöglichen.

Erfolg bedeutet:

- Besuchende können nach dem Hero in eigenen Worten erklären, dass BRUCE keine Black-Box-Antworten verspricht.
- Der Konzept- und Entwicklungsstatus wird weder verschleiert noch als fertiges Produkt ausgegeben.
- Die Branch-Übersicht verweist auf überprüfbare GitHub-Quellen.
- Eine Projektanfrage ist in höchstens zwei Interaktionen erreichbar.
- Die Seite bleibt auf Mobilgeräten lesbar und zugänglich.

## 2. Tonalität und Claims

Die Sprache ist menschlich, klar und ambitioniert. Sie vermeidet unprüfbare Superlative, künstliche Allgemeinplätze und Aussagen über nicht existierende Quanten-, Hardware- oder Sicherheitsfähigkeiten.

Primärer Claim:

> Don’t just get an answer. Follow it.

Sekundärer Claim:

> Intelligence with evidence.

Deutsche Begleitlinie:

> Antworten, denen man folgen kann.

Wörter wie „Universum“, „Orbit“ und „Signal“ sind ausschließlich visuelle Metaphern für klassische Software- und Rechenprozesse. Das wird in der Architektursektion einmal eindeutig erklärt.

## 3. Informationsarchitektur

| Abschnitt | Leserfrage | Inhalt | Primäre Aktion |
| --- | --- | --- | --- |
| Hero – The Signal | Was ist BRUCE? | Claim, kurze Erklärung, Orbital-Key-Visual | Architektur entdecken / Projekt anfragen |
| Manifest – Why BRUCE | Warum ist das relevant? | Herkunft, Evidenz, Grenzen, Bedeutung | Zur Answer Journey scrollen |
| Answer Journey | Wie entsteht eine Antwort? | Signal → Route → Verify → Remember → Answer | Architektur öffnen |
| BRUCE System | Woraus besteht BRUCE? | Interaktive Komponentenkarte | Einzelne Komponente verstehen |
| Development Navigator | Wie weit ist das wirklich? | GitHub-Branch-Tabelle mit ehrlichen Statusindikatoren | Branch auf GitHub öffnen |
| Project Inquiry | Wie kann ich mitmachen? | Kurzer Kooperations-CTA und Anfrageformular | Anfrage absenden |
| Footer – The Trace | Wo finde ich Details? | GitHub, Kontakt, Prinzipien, Transparenzhinweis | Externe Quellen öffnen |

## 4. Visuelles System

Die visuelle Sprache basiert auf einer dunklen kosmischen Forschungsumgebung:

- **Grundfläche:** Near-black und Midnight Navy, damit Inhalt und Lichtspuren klar bleiben.
- **Akzentfarben:** Electric Cyan für Evidenz/Verbindung, Violet für abstrakte Suchräume, Icy White für bestätigte Resultate.
- **Key Visual:** ein abstrakter, leuchtender BRUCE-Kern mit Bahnen und Knoten. Keine Roboter, keine Raumschiffe, keine Planeten und keine pseudo-wissenschaftlichen Versprechen.
- **Bewegung:** reduziert und bedeutungsvoll — langsame Orbitbewegung, ein aufleuchtender Trace beim Scrollen, respektiert `prefers-reduced-motion`.
- **Typografie:** große, kompakte Headlines; ruhige, gut lesbare Erklärungstexte; technische Metadaten kleiner, aber kontrastreich.

Der Stil transportiert Ambition, darf aber die Lesbarkeit niemals dem Effekt opfern.

## 5. Abschnittsdetails

### 5.1 Hero – The Signal

Der Hero nutzt das Orbital-Key-Visual als Hintergrund. Links liegt eine dunkle, ausreichend kontrastierende Textfläche; der Kern sitzt rechts. Der Hero enthält genau zwei gleichwertig sichtbare Aktionen:

- `Discover BRUCE` scrollt zur Answer Journey.
- `Start a project` scrollt zur Projektanfrage.

Ein dezenter Statushinweis `Research architecture · Open development` verhindert, dass Besuchende BRUCE für ein fertiges Massenprodukt halten.

### 5.2 Manifest – Why BRUCE

Vier kompakte Karten erklären die fundamentalen Unterschiede zu einer Black Box:

1. Origin – Eingaben, Regeln und Versionen bleiben verbunden.
2. Evidence – Rechnung, Gegenprüfung und Replay stützen Resultate.
3. Limits – Budgets und Gates zeigen Unsicherheit offen.
4. Meaning – Der Renderer erklärt bestätigte Resultate, ohne Fakten hinzuzuerfinden.

Jede Karte ist eine Übersetzung eines Architekturprinzips und enthält keine Performanceversprechen.

### 5.3 Answer Journey

Die Journey visualisiert das System als kontrollierte Folge:

```text
Signal → Route → Verify → Remember → Answer
```

Die Labels werden für das Allgemeinpublikum benutzt. Eine optionale Detailansicht ordnet sie später den Fachbegriffen zu:

| Öffentliche Bezeichnung | Architekturbezug |
| --- | --- |
| Signal | Input Normalization / TaskSpec |
| Route | Semantic Routing / Strategy Selection |
| Verify | Exact Execution / Countercheck / Replay |
| Remember | Experience Ledger / Hot Skill Cache |
| Answer | AnswerPoint / Explanation Renderer |

### 5.4 BRUCE System

Eine scroll- oder klickbare Sternenkarte zeigt fünf Hauptknoten:

- **Semantic Routing:** erkennt, welcher Denkweg zu einer Frage passt.
- **KEVIN Grid:** beschreibt große virtuelle Suchräume, ohne sie vollständig zu speichern.
- **BRUCE-Math:** führt Mathematik exakt und innerhalb von Budgets aus.
- **Evidence Gate:** trennt Kandidaten von verifizierten Aussagen.
- **Experience Ledger:** speichert nachvollziehbare Erfahrungen statt unsichtbarer Änderungen.

Der Knoten „Language Layer“ erklärt ausdrücklich: Sprache präsentiert Ergebnisse; sie setzt keinen Verified- oder Publish-Status.

### 5.5 Development Navigator

Die Tabelle zeigt nur Daten, die aus dem ausgewählten GitHub-Repository verlässlich geliefert werden können:

| Feld | Quelle | Verhalten bei fehlenden Daten |
| --- | --- | --- |
| Branch-Name | GitHub Branch API oder statische Konfiguration | Zeile nicht anzeigen |
| Kurzbeschreibung | versionierte lokale Zuordnung | `Beschreibung folgt` |
| Status | versionierte lokale Zuordnung, nicht automatisch abgeleitet | `Nicht klassifiziert` |
| Letzter Commit | GitHub Commit API | `Unbekannt` |
| Link | GitHub-Branch-URL | Link ausblenden |

Die Statuswerte sind `CONCEPT`, `IN PROGRESS`, `PREVIEW`, `NEXT` und `ARCHIVED`. Sie sind redaktionelle Einordnungen und werden nie automatisch aus Commit-Zahlen abgeleitet.

Der aktuelle Konzeptordner enthält keine verlässlich auslesbare Git-Historie. Bis ein konkretes öffentliches Repository festgelegt ist, muss die Website daher entweder eine redaktionell gepflegte Beispieltabelle deutlich als Vorschau markieren oder die Sektion mit `GitHub-Verbindung folgt` kennzeichnen. Es dürfen keine erfundenen Branches als reale Versionen gezeigt werden.

### 5.6 Project Inquiry

Die Anfrage beginnt mit der Formulierung `Bring a question worth following.` Das Formular bleibt absichtlich klein:

- Name
- E-Mail
- Organisation (optional)
- Art des Vorhabens: Forschung, Pilotprojekt, Kooperation, Sonstiges
- Kurzbeschreibung
- Einwilligung zur Kontaktaufnahme

Nach dem Absenden zeigt die Seite einen klaren Erfolgs- oder Fehlerzustand. Es werden keine Sensitivdaten abgefragt. Ein Versand-Endpunkt und eine Datenschutzerklärung sind Voraussetzung für einen echten Live-Start.

## 6. Datenfluss und Zustände

```text
GitHub-Repository
  → serverseitiger Fetch mit Cache
  → Branch-Konfiguration (Beschreibung + redaktioneller Status)
  → Development Navigator

Projektformular
  → clientseitige Pflichtfeld- und E-Mail-Prüfung
  → serverseitige Validierung und Rate Limit
  → bestätigter Versand / Fehlerzustand
  → Kontaktkanal
```

GitHub-Fehler dürfen den Rest der Seite nicht beeinträchtigen. Die Tabelle zeigt dann einen lesbaren Hinweis und einen direkten Link zum Repository. Formulardaten werden ausschließlich für die Anfrage verwendet; nach einem Fehler bleibt der eingegebene Text erhalten.

## 7. Zugänglichkeit und Qualität

- Alle Bilder erhalten einen sinnvollen Alternativtext oder werden bei rein dekorativer Funktion als dekorativ markiert.
- Text auf dem Key Visual erhält unabhängig vom Bild ausreichenden Kontrast.
- Interaktive Knoten, Branch-Links und Buttons sind per Tastatur bedienbar und besitzen sichtbare Fokuszustände.
- Animationen dürfen nicht erforderlich sein, um Inhalte zu verstehen.
- Mobile Darstellung priorisiert Lesereihenfolge und große Touch-Ziele; die Desktop-Sternenkarte wird zu einer vertikalen Kartenfolge.
- Branch-Tabelle bleibt auf kleinen Displays horizontal scrollbar oder wechselt in zugängliche Karten.

## 8. Verifikation vor Launch

1. Inhaltliche Prüfung gegen die BRUCE-Konzepte: keine nicht belegten Behauptungen.
2. Branch-Daten gegen das festgelegte GitHub-Repository prüfen.
3. Formular mit Erfolg, fehlerhafter E-Mail, Netzwerkausfall und Rate-Limit testen.
4. Keyboard-, Screenreader- und `prefers-reduced-motion`-Prüfung durchführen.
5. Mobile und Desktop visuell prüfen.
6. Performance testen: Key Visual responsiv ausliefern und verzögert laden, ohne den Hero-Inhalt zu blockieren.

## 9. Nicht im ersten Release

- Live-Demonstration der BRUCE-Runtime.
- Behauptete Echtzeit- oder mathematische Leistungswerte ohne implementierten, geprüften Kern.
- Vollständige technische Dokumentation im Marketing-Flow.
- Automatische semantische Einordnung von GitHub-Branches.
- Newsletter oder Kontoverwaltung.

## 10. Umsetzungsentscheidung

Nach Freigabe dieser Spezifikation wird ein separater Implementierungsplan erstellt. Die genaue Technologie für Website, Hosting, Formularversand und GitHub-Anbindung wird dabei festgelegt, sobald ein Ziel-Repository und ein Kontaktkanal benannt sind.
