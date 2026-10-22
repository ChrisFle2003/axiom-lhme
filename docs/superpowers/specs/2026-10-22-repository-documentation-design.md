# Repository-Dokumentation – Design

**Ziel:** Eine deutschsprachige, quellengestützte technische Dokumentation für
Entwicklung, Betrieb und technische Einordnung von BRUCE LHME bereitstellen.

## Ausgangslage

BRUCE LHME ist laut `CMakeLists.txt` ein C- und C++20-Projekt in Version
`0.1.0`. Es baut die statische Bibliothek `bruce_lhme_core`, exportiert sie
als CMake-Paket `bruce_lhme::core`, stellt eine C-API bereit und baut das
Kommandozeilenprogramm `bruce_gguf_inspect`. Das Projekt enthält Tests,
Beispiele, Demos und Benchmarks.

Die bestehende Dokumentation enthält bereits fachliche Vertiefungen für den
Kernel SDK, die GGUF-Inspektion, KEVIN-Kompatibilität, einen Release-Vertrag
und Benchmark-Ergebnisse. Diese Dokumente bleiben erhalten. Die vorhandene
`docs/architecture.md` beschreibt denselben Gegenstand wie die neue
Architekturübersicht, deckt jedoch die gewünschte Zielstruktur nicht ab und
wird deshalb durch eine umfassendere, weiterhin quellengestützte Fassung
ersetzt.

## Nicht-Ziele und Grenzen

- Es werden keine Produktivquellen, Build-Regeln oder Tests geändert.
- Es werden keine Betriebs-, Sicherheits- oder API-Aussagen ergänzt, die nicht
  in Quelltext, Konfiguration oder bestehender Dokumentation belegt sind.
- Fehlende Informationen werden als offene Frage oder als „Nicht eindeutig aus
  dem Repository ableitbar“ gekennzeichnet.
- Insbesondere dokumentiert das Repository keinen bereitgestellten Server,
  kein Deployment-System, keine Laufzeitkonfiguration per Umgebungsvariable,
  keine Authentifizierung oder Autorisierung und kein Monitoring-Backend.

## Zielgruppen

- Neue C++-Entwicklerinnen und -Entwickler, die Bibliothek oder CMake-Paket
  lokal bauen und erweitern möchten.
- Integratoren, die die C++-API, C-API oder den Kernel SDK nutzen.
- Betreiber des lokalen GGUF-Inspector-Werkzeugs.
- Technische Stakeholder, die Grenzen, Tests, CI und Release-Stand einordnen.

## Informationsarchitektur

| Dokument | Verantwortung | Primäre Repository-Belege |
| --- | --- | --- |
| `README.md` | Einstieg, Schnellstart, Befehlsübersicht, Dokumentationsindex | `CMakePresets.json`, `CMakeLists.txt` |
| `docs/architecture.md` | Komponenten, Verantwortlichkeiten, Datenflüsse und Mermaid-Diagramm | `include/bruce/lhme/`, `src/`, `CMakeLists.txt` |
| `docs/setup.md` | Voraussetzungen, Konfiguration, lokale Installation und Fehlersuche | `CMakePresets.json`, CMake-Paketexport, CI |
| `docs/development.md` | Struktur, Workflow, Tests, Qualitätsprüfungen und Beiträge | `tests/`, `.github/`, CMake-Ziele |
| `docs/api.md` | Öffentliche C++- und C-Schnittstellen sowie GGUF-CLI | `include/bruce/lhme/`, `tools/gguf_inspect/` |
| `docs/deployment.md` | Erkennbarer Paket-/Release-/CI-Ablauf und nicht vorhandene Deployment-Artefakte | `CMakeLists.txt`, `.github/workflows/ci.yml`, `docs/release-v0.1.md` |
| `docs/security.md` | Eingabevalidierung, lokale Dateiverarbeitung, Snapshots, Grenzen | GGUF-, Snapshot- und C-API-Implementierung |
| `docs/decision-log.md` | Aus Code ableitbare Entscheidungen, offene Punkte und Annahmen | CMake, Header, Implementierungen, CI |

Die vorhandenen Spezialdokumente werden aus den passenden neuen Übersichten
verlinkt. Sie werden nicht umgeschrieben, sofern kein korrekter Link oder eine
offensichtliche Einstiegsreferenz fehlt.

## Quellen- und Aussageprinzip

Jede relevante technische Aussage verweist auf mindestens einen relativen
Dateipfad. Detailaussagen benutzen zusätzlich Symbole, CMake-Ziele oder
Optionen, damit sie ohne Vermutung im Quellbaum überprüfbar sind. Das
Dokumentationspaket trennt implementierte Funktionen von Konzepten unter
`concepts/`; konzeptionelle Inhalte werden nicht als ausgelieferte Funktion
dargestellt.

## Dokumentationsumfang

### README

Die README bleibt die kurze Startseite. Sie benennt das Projekt als C++20-
Bibliothek, zeigt die vorhandenen CMake-Presets und verweist auf die neuen
Rollen-Dokumente sowie auf den bestehenden Kernel- und GGUF-Leitfaden.

### Architektur

Das Diagramm stellt die öffentliche Bibliothek, Engine, Cache,
FormulaOverrideStore, KEVIN-Domain/Projection, C-API und GGUF-Inspector dar.
Es beschreibt ausdrücklich, dass `ResultCache` ein Beschleuniger ist und dass
KEVIN nicht implizit eine Formel auswählt. Der GGUF-Pfad bleibt als lokales,
lesendes Werkzeug vom Auswertungs-/Engine-Pfad abgegrenzt.

### Setup und Entwicklung

Diese Seiten verwenden ausschließlich die im Repository vorhandenen CMake-
Presets und CTest-Befehle. Sie beschreiben auch die optionalen Sanitizer und
das installierbare CMake-Paket. Fehlende Paketinstallationsanweisungen für
bestimmte Betriebssysteme werden nicht geraten.

### API

Die API-Seite gliedert sich in C++-Basisformen, Engine/Formula/Technique,
Override- und Snapshot-Schnittstellen, KEVIN, C-API und GGUF-CLI. Beispiele
werden nur aus den vorhandenen Beispielen, Headern und CLI-Parserregeln
abgeleitet. Fehler- und Statusfälle verwenden die vorhandenen Enums.

### Betrieb und Sicherheit

Die Deployment-Seite hält transparent fest, dass CMake-Installation und
GitHub-Actions-CI belegt sind, aber kein Hosting- oder Release-Automatisierungs-
ablauf vorhanden ist. Die Sicherheitsseite erläutert fail-closed-Prüfungen für
Snapshots/GGUF, Ressourcengrenzen und lokale Dateipfade; sie behauptet keine
Authentifizierung, Verschlüsselung oder kryptografische Integritätsgarantie.

## Qualitätsprüfung

Nach dem Schreiben werden alle Markdown-Links und lokalen Pfade geprüft. Die
Dokumentation wird gegen die tatsächlichen CMake-Presets, CTest-Ziele,
Header-Namen und CLI-Optionen gegengeprüft. Produktcode bleibt unverändert.
