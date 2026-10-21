# KEVIN Snapshot Store V1.5 — Design

**Status:** Entwurf zur Nutzerfreigabe  
**Scope:** Lokale CPU/RAM-Serialisierung; kein GPU-, Netzwerk- oder Datenbank-Backend

## Ziel

V1.5 speichert und lädt den logischen KEVIN-Zustand: GridSpecs und sparse
exakte Overrides. Reguläre Formelergebnisse und der Engine-Cache bleiben
absichtlich außerhalb des Snapshots. Ein erfolgreicher Load stellt dieselben
versionierten Regeln und Ausnahmen wieder her, ohne einen Gridraum zu
materialisieren.

## Format

Die Datei ist ein eigenes, versioniertes Binärformat:

```text
Header
├─ magic: "BRUCE-KEVIN"
├─ format_version: 1
├─ grid_record_count: u64
├─ override_record_count: u64
├─ payload_length: u64
└─ payload_checksum: u64
```

Der Payload enthält erst GridSpecs, dann Overrides. Strings sind als
`u32 length + UTF-8 bytes` kodiert. Alle Ganzzahlen verwenden festgelegte
Little-Endian-Kodierung. BigInt-Overrides bestehen aus Vorzeichenbit und
kanonischen Dezimalziffern; Null besitzt keine negative Darstellung.

## Sicherheit und Atomizität

`save_snapshot(path)` schreibt in eine eindeutige temporäre Nachbardatei,
schließt sie und benennt sie erst danach auf das Ziel um. Fehler lassen eine
vorherige Zieldatei unverändert.

`load_snapshot(path)` prüft Magic, Version, Längen, Checksumme, maximale
Record-Anzahlen, kanonische BigInts, eindeutige Grid-Identitäten und
Override-Adressen vollständig vor jeder Store-Mutation. Ungültige Dateien
liefern einen Fehler und lassen den vorhandenen Store unverändert.

## Öffentliche API

```cpp
enum class SnapshotStatus { Confirmed, IoError, InvalidSnapshot };
SnapshotStatus save_snapshot(const KevinMemoryStore&, const std::filesystem::path&);
SnapshotStatus load_snapshot(KevinMemoryStore&, const std::filesystem::path&);
```

Der Store exportiert dafür eine unveränderliche Snapshot-Ansicht und importiert
eine bereits validierte Ansicht als vollständigen Ersatz. Import schlägt fehl,
wenn eine Snapshot-GridSpec auf einen in der Engine nicht registrierten Kernel
verweist.

## Tests

- GridSpec- und Override-Roundtrip einschließlich negativem BigInt;
- Leer-Snapshot und mehrere Grids;
- falsches Magic, Version, Checksumme, abgeschnittene Datei und falsche Länge;
- doppelte Identität oder Override-Adresse;
- fehlender Kernel beim Import;
- bestehender Store bleibt nach fehlgeschlagenem Load unverändert;
- Speicheroperation überschreibt eine funktionierende Zieldatei nur nach
vollständigem Erfolg.

## Nicht-Ziele

Keine Speicherung des Engine-Caches, keine inkrementellen Snapshots, keine
Verschlüsselung, keine Netzwerkübertragung, keine GPU- oder Datenbankanbindung.
