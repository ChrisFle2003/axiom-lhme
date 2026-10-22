# Axiom LHME – Umbenennungsdesign

**Ziel:** BRUCE LHME wird vollständig und bewusst brechend in Axiom LHME
umbenannt. Der neue GitHub-Slug lautet `axiom-lhme`.

## Entscheidung und Umfang

Die Umbenennung betrifft die aktuell öffentliche Identität des Projekts:

| Bisher | Neu |
| --- | --- |
| Anzeigename `BRUCE LHME` | `Axiom LHME` |
| CMake-Projekt und Paket `bruce_lhme` | `axiom_lhme` |
| CMake-Target `bruce_lhme::core` | `axiom_lhme::core` |
| C++-Namespace `bruce::lhme` | `axiom::lhme` |
| Headerpfad `include/bruce/lhme/` | `include/axiom/lhme/` |
| Statisches Kernziel `bruce_lhme_core` | `axiom_lhme_core` |
| C-API-Präfix `lhme_` | `axiom_lhme_` |
| CLI `bruce_gguf_inspect` | `axiom_gguf_inspect` |
| GitHub-Repository `ChrisFle2003/bruce-lhme` | `ChrisFle2003/axiom-lhme` |

Es werden keine Kompatibilitätsheader, Namespace-Aliase, CMake-Aliase oder
C-API-Aliase bereitgestellt. Bestehende Integrationen müssen ihre Includes,
Namespaces, Linkziele und C-API-Aufrufe aktualisieren.

## Begründung

Die bisherigen Namen sind Teil der öffentlichen Nutzungsoberfläche:
`CMakeLists.txt` exportiert ein Paket und Target, die Beispiele benutzen
`find_package(bruce_lhme)` und `bruce_lhme::core`, und die öffentlichen Header
liegen unter `include/bruce/lhme/`. Ein Teilwechsel würde zwei Marken und zwei
Integrationsoberflächen in derselben Version hinterlassen. Der bestätigte
vollständig brechende Wechsel vermeidet diese Doppelpflege.

## Technische Umsetzung

### Quellbaum und Build

Alle produktiven Includes, Namespaces, CMake-Ziele und Package-Dateien werden
konsistent auf `axiom` bzw. `axiom_lhme` umgestellt. Die Installationsregel
installiert zukünftig `include/axiom`; die Paketkonfigurationsdateien werden
unter `${CMAKE_INSTALL_LIBDIR}/cmake/axiom_lhme` installiert. Die beiden
Consumer-Beispiele und ihre CMake-basierte Integrationstests werden auf das
neue Paket und Target aktualisiert.

### C-API und CLI

Die Typen und Funktionen in der vorhandenen C-Schnittstelle erhalten das
Präfix `axiom_lhme_`, einschließlich der undurchsichtigen Handles, Status- und
Modus-Typen sowie aller Aufrufe. `bruce_gguf_inspect` wird in
`axiom_gguf_inspect` umbenannt. Seine Argumente, Ausgabeformate und
Exit-Codes bleiben unverändert.

### Dokumentation

README, `docs/`, Beispiele, Tests, GitHub-Vorlagen und aktuelle Konzepttexte
erhalten die neue Bezeichnung und die neuen Befehle. Historische
Benchmark-Aussagen behalten ihren fachlichen Inhalt, werden aber als
Messergebnisse des umbenannten Projekts dargestellt. Der vor der Umbenennung
erstellte Dokumentationsentwurf wird ebenfalls aktualisiert, damit die neue
Dokumentation keine veralteten Paketnamen empfiehlt.

### GitHub und lokales Checkout

Nach erfolgreicher lokaler Migration, Build und Test wird das GitHub-
Repository in `ChrisFle2003/axiom-lhme` umbenannt. Danach wird `origin` auf die
neue URL gesetzt und die geprüfte Migration gepusht. Der lokale
Arbeitsverzeichnispfad wird nicht umbenannt: Die Codex-Umgebung ist daran
gebunden und ein lokaler Pfad ist keine Teil der öffentlichen Schnittstelle.

## Nicht im Umfang

- Kein Release, Tag oder Versionssprung; `0.1.0` bleibt unangetastet, solange
  keine getrennte Release-Entscheidung getroffen wird.
- Keine Kompatibilitätsschicht für `bruce`-Namen.
- Keine Änderung der technischen Funktionalität, GGUF-Parserlogik,
  Speicherpolitik oder Testsemantik.
- Keine Änderung des unversionierten Verzeichnisses `model/`.

## Verifikation

1. Sicherstellen, dass in produktiven Quellen, Test- und Build-Dateien keine
   verbliebenen `bruce`-Identitäten mehr vorkommen.
2. Debug- und Release-Preset konfigurieren, bauen und mit CTest ausführen.
3. Die C-Header-Kompilierung und beide externen CMake-Consumer-Tests als Teil
   der CTest-Suite prüfen.
4. Den GGUF-CLI-Test auf den neuen Programmnamen sowie README- und
   Dokumentationslinks prüfen.
5. Erst danach GitHub umbenennen, Remote aktualisieren und pushen.

## Risiken und Migration

Diese Änderung ist absichtlich inkompatibel zu bestehendem Quellcode und
Buildskripten. Externe Nutzer müssen alle bisherigen Namensbestandteile in der
linken Spalte der Tabelle ersetzen. Das Repository enthält keinen Nachweis
eines veröffentlichten Paketmanagers, einer ABI-Kompatibilitätszusage oder
aktiver externer Consumer; deren reale Migrationsbedarf ist daher nicht
eindeutig aus dem Repository ableitbar.
