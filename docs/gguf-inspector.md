# AXIOM GGUF Inspector V1

Der AXIOM GGUF Inspector untersucht lokale GGUF-V3-Modelldateien und erstellt
einen deterministischen CPU/RAM-Ladeplan. Er ersetzt weder GGUF noch ggml oder
llama.cpp und führt keine Inferenz aus.

## Was V1 macht

- Validiert Header, Metadaten, Tensor-Offsets, Alignment und Datenbereiche
  fail-closed.
- Inventarisiert Tensorname, Shape, GGML-Typ, gespeicherte Bytes und
  geschätzten F32-dekodierten RAM.
- Plant Tensor-Chunks als `resident`, `on_demand`, `deferred` oder `rejected`.
- Erzeugt einen stabilen Textbericht und optional einen JSON-Plan.
- Kann in explizit ausgewählten Chunks Kandidaten für konstante Bytes,
  wiederholte Byteblöcke oder bitgenaue F32-Folgen melden.

## Was V1 ausdrücklich nicht macht

- Keine Modellgewichte schreiben, quantisieren, dequantisieren oder umsortieren.
- Keine Tensorachsen und keine Skalarkoordinaten verändern.
- Keine GGUF-Datei umschreiben und keine Inferenz implementieren.
- Keine Formelrepräsentation automatisch verwenden. Ein Pattern-Kandidat ist
  **kein** bewiesenes `FormulaRecipe` und hat immer `exactly_verified=false`.

Die gespeicherte Reihenfolge eines Tensors ist Teil seiner Modellbedeutung.
AXIOM sortiert nur Inventar- und Planbeschreibungen für reproduzierbare
Berichte; es sortiert niemals einzelne Gewichtswerte. Quantisierte,
trainierte Gewichte gelten nicht automatisch als formelkomprimierbar.

## Unterstützter Container und Tensorlayouts

V1 akzeptiert nur lokale **GGUF-Version 3** Dateien. Unbekannte Versionen und
GGML-Typen werden abgelehnt, statt geraten zu werden. Die Größenrechnung
unterstützt:

| Typ | Elemente/Block | Bytes/Block |
|---|---:|---:|
| F32 | 1 | 4 |
| F16 | 1 | 2 |
| Q4_0, Q4_1, Q5_0, Q5_1, Q8_0, Q8_1 | 32 | 18, 20, 22, 24, 34, 40 |
| Q2_K, Q3_K, Q4_K, Q5_K, Q6_K, Q8_K | 256 | 84, 110, 144, 176, 210, 292 |

Der RAM-Wert ist eine konservative Schätzung für eine F32-Dekodierung:
`logical_element_count × 4`. Er ist keine Vorhersage einer konkreten
Inferenz-Runtime.

## Bauen und verwenden

```bash
cmake --preset release
cmake --build --preset release --target axiom_gguf_inspect

./out/build/release/axiom_gguf_inspect model.gguf \
  --memory-budget 8589934592 \
  --chunk-size 67108864 \
  --policy budgeted \
  --json-output plan.json
```

Die drei Ladepolitiken sind:

- `all-resident`: Alle Chunks müssen gemeinsam ins RAM-Budget passen. Sonst
  wird der gesamte Plan abgelehnt.
- `budgeted`: Ein optionales Nutzungsprofil erhält zuerst Resident-RAM; danach
  folgt die stabile Reihenfolge Dateioffset, Tensorname und Chunkindex. Nicht
  passende Chunks bleiben `on_demand`.
- `metadata-only`: Alle Chunks bleiben `deferred`; nur Inventar und Plan werden
  erzeugt.

Ein Nutzungsprofil ist eine lokale Textdatei mit einem Tensorname pro Zeile:

```bash
./out/build/release/axiom_gguf_inspect model.gguf \
  --memory-budget 8589934592 \
  --chunk-size 67108864 \
  --policy budgeted \
  --use-profile first-use.txt \
  --scan
```

`--scan` ist begrenzt und rein lesend. Die Quelldatei wird dabei nicht
verändert. Der Inspector lädt weder Modelle aus dem Netzwerk herunter noch
öffnet er Netzwerkverbindungen.

## Determinismus und Grenzen

Gleiche GGUF-Bytes und gleiche Konfiguration liefern dieselbe
Inventarreihenfolge, den gleichen Plan und denselben Trace. Der
Eingabe-Fingerprint ist ein deterministischer FNV-abgeleiteter Identifikator
für Vergleich und Trace, kein kryptografischer Sicherheitsnachweis.

V1 ist bewusst ein Inspector und Planer. Ein späterer, optionaler lossless
Rewriter oder FormulaRecipe-Sidecar benötigt eine getrennte Kompatibilitäts-
und Bytevalidierung und ist nicht Teil dieser Version.
