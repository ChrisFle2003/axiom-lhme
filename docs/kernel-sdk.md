# BRUCE Kernel SDK

Mit dem Kernel SDK kann ein C++-Projekt eine eigene deterministische Formel
als `TechniqueKernel` registrieren. V1.3 ist eine Compile-Time-Integration:
Der Kernel wird zusammen mit deiner Anwendung gebaut. Es gibt keinen
Runtime-Plug-in-Loader.

## 1. Paket lokal installieren und einbinden

Wenn du BRUCE aus dem Quellbaum verwendest, erstelle zuerst eine lokale
Installation:

```bash
cmake -S /pfad/zu/BRUCE -B /pfad/zu/BRUCE/build
cmake --build /pfad/zu/BRUCE/build
cmake --install /pfad/zu/BRUCE/build --prefix /pfad/zu/BRUCE/build/install
```

Danach konfigurierst und baust du dein eigenes Projekt gegen dieses Paket:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/pfad/zu/BRUCE/build/install
cmake --build build
```

Die eigene `CMakeLists.txt` enthält:

```cmake
find_package(bruce_lhme 0.1 CONFIG REQUIRED)

add_executable(my_kernel main.cpp)
target_link_libraries(my_kernel PRIVATE bruce_lhme::core)
```

Das exportierte Target liefert die öffentlichen BRUCE-Header, C++20 und die
benötigte Threads-Abhängigkeit.

## 2. Kernel schreiben

Implementiere `TechniqueKernel::identity()` und `TechniqueKernel::evaluate()`.
Eine Identity hat eine Länge von `1–128`, verwendet nur `A–Z`, `a–z`, `0–9`,
`.`, `_` oder `-` und besitzt eine Version größer als null. Parameter sind
kanonische Strings; prüfe sie exakt, bevor gerechnet wird.

```cpp
#include <memory>

#include "bruce/lhme/engine.hpp"
#include "bruce/lhme/kernel_sdk.hpp"

class AffineKernel final : public bruce::lhme::TechniqueKernel {
 public:
  bruce::lhme::TechniqueIdentity identity() const override {
    return {.id = "example.affine", .version = 1};
  }

  bruce::lhme::EvaluationResult evaluate(
      const bruce::lhme::TechniqueRequest& request) const override {
    if (bruce::lhme::cancellation_requested(request)) {
      return bruce::lhme::cancelled_technique_result(
          request.mode, "kernel cancellation requested");
    }
    if (request.identity != identity() ||
        request.parameters != "factor=1:2;offset=1:3" ||
        request.address.index < 0 ||
        request.mode != bruce::lhme::EvaluationMode::Exact) {
      return bruce::lhme::invalid_technique_request_result(
          request.mode, "expected exact affine request");
    }
    return bruce::lhme::confirmed_exact_result(
        bruce::lhme::BigInt{request.address.index} * 2 + 3,
        bruce::lhme::make_trace({}));
  }
};
```

Das Beispiel verwendet die Formel `2 × index + 3`. Bei `index = 21` ist das
bestätigte BigInt-Ergebnis `45`.

## 3. Regeln für robuste Kernel

1. Prüfe Identity, Parameter, Adresse und Modus vor der Berechnung.
2. Prüfe `cancellation_requested(request)` vor jedem potenziell langen
   Rechenschritt.
3. Beachte die Ressourcenlimits aus `request.budget`; `Engine` prüft das
   veröffentlichte Ergebnis zusätzlich.
4. Nutze `confirmed_exact_result` für exakte Ganzzahlen. Der Helper verlangt
   einen Trace; bei einem Null-Trace wird ein leerer Basis-Trace erzeugt.
5. Nutze `invalid_technique_request_result` für ungültige Eingaben und
   `cancelled_technique_result` für Abbruch. Beide liefern einen passenden
   Branch- beziehungsweise Run-Trace.
6. Registriere den Kernel vor der Auswertung:

```cpp
auto engine = bruce::lhme::Engine::create({});
const auto kernel = std::make_shared<AffineKernel>();
if (!engine.register_technique(kernel)) {
  // Identity ist ungültig oder bereits registriert.
}
```

## 4. SDK prüfen

Im BRUCE-Quellbaum prüfen diese Befehle den öffentlichen Vertrag und den
komplett getrennten Consumer:

```bash
ctest --test-dir build -R kernel_sdk_contract_tests --output-on-failure
ctest --test-dir build -R lhme_kernel_sdk_consumer --output-on-failure
```

Das Consumer-Beispiel liegt unter `examples/kernel_sdk_consumer/`. Es nutzt
nur `find_package` und `bruce_lhme::core`; damit ist es die Vorlage für ein
unabhängiges Projekt.
