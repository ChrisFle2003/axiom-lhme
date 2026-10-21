#include "test.hpp"

#include "bruce/lhme/formula_override_snapshot.hpp"
#include "bruce/lhme/power_offset_kernel.hpp"

#include <filesystem>
#include <fstream>
#include <memory>

TEST(formula_override_snapshot_round_trips_binding_and_value) {
  using namespace bruce::lhme;
  const auto path = std::filesystem::temp_directory_path() /
                    "bruce-formula-override-snapshot-test.bin";
  std::filesystem::remove(path);

  auto source_engine = Engine::create({});
  const auto binding = source_engine.register_formula(make_power_offset_spec(2, 100, 1));
  FormulaOverrideStore source{source_engine};
  CHECK(source.set_override(binding, {.index = 10}, BigInt{-42}) == Status::Confirmed);
  CHECK(save_formula_override_snapshot(source, path) ==
        FormulaOverrideSnapshotStatus::Confirmed);

  auto restored_engine = Engine::create({});
  const auto restored_binding =
      restored_engine.register_formula(make_power_offset_spec(2, 100, 1));
  FormulaOverrideStore restored{restored_engine};
  CHECK(load_formula_override_snapshot(restored, path) ==
        FormulaOverrideSnapshotStatus::Confirmed);
  const auto value = restored.evaluate({.binding = restored_binding,
                                        .address = {.index = 10},
                                        .mode = EvaluationMode::Exact,
                                        .budget = Budget::unlimited()});
  CHECK(value.source == FormulaValueSource::Override);
  CHECK(value.result.exact_integer() == -42);
  std::filesystem::remove(path);
}

TEST(formula_override_snapshot_rejects_bruce_kevin_legacy_magic) {
  using namespace bruce::lhme;
  const auto path = std::filesystem::temp_directory_path() /
                    "bruce-formula-override-legacy-magic.bin";
  {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << "BRUCE-KEVIN-1\n";
  }
  auto engine = Engine::create({});
  const auto binding = engine.register_formula(make_power_offset_spec(2, 100, 1));
  FormulaOverrideStore store{engine};
  CHECK(store.set_override(binding, {.index = 3}, BigInt{7}) == Status::Confirmed);

  CHECK(load_formula_override_snapshot(store, path) ==
        FormulaOverrideSnapshotStatus::InvalidSnapshot);
  CHECK(store.evaluate({.binding = binding,
                        .address = {.index = 3},
                        .mode = EvaluationMode::Exact,
                        .budget = Budget::unlimited()})
            .result.exact_integer() == 7);
  std::filesystem::remove(path);
}

TEST(formula_override_snapshot_round_trips_a_technique_binding) {
  using namespace bruce::lhme;
  const auto path = std::filesystem::temp_directory_path() /
                    "bruce-formula-override-technique-test.bin";
  std::filesystem::remove(path);

  auto source_engine = Engine::create({});
  CHECK(source_engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  const TechniqueBinding binding{{.id = "bruce.power_offset", .version = 1},
                                 encode_power_offset_parameters(2, 100)};
  FormulaOverrideStore source{source_engine};
  CHECK(source.set_override(binding, {.index = 8}, BigInt{13}) == Status::Confirmed);
  CHECK(save_formula_override_snapshot(source, path) ==
        FormulaOverrideSnapshotStatus::Confirmed);

  auto restored_engine = Engine::create({});
  CHECK(restored_engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  FormulaOverrideStore restored{restored_engine};
  CHECK(load_formula_override_snapshot(restored, path) ==
        FormulaOverrideSnapshotStatus::Confirmed);
  const auto value = restored.evaluate({.binding = binding,
                                        .address = {.index = 8},
                                        .mode = EvaluationMode::Exact,
                                        .budget = Budget::unlimited()});
  CHECK(value.source == FormulaValueSource::Override);
  CHECK(value.result.exact_integer() == 13);
  std::filesystem::remove(path);
}

TEST(formula_override_snapshot_rejects_trailing_data_without_mutating_store) {
  using namespace bruce::lhme;
  const auto path = std::filesystem::temp_directory_path() /
                    "bruce-formula-override-trailing-test.bin";
  std::filesystem::remove(path);

  auto source_engine = Engine::create({});
  const auto binding = source_engine.register_formula(make_power_offset_spec(2, 100, 1));
  FormulaOverrideStore source{source_engine};
  CHECK(source.set_override(binding, {.index = 6}, BigInt{17}) == Status::Confirmed);
  CHECK(save_formula_override_snapshot(source, path) ==
        FormulaOverrideSnapshotStatus::Confirmed);
  {
    std::ofstream output(path, std::ios::binary | std::ios::app);
    output << "unexpected trailing data";
  }

  auto target_engine = Engine::create({});
  const auto target_binding =
      target_engine.register_formula(make_power_offset_spec(2, 100, 1));
  FormulaOverrideStore target{target_engine};
  CHECK(target.set_override(target_binding, {.index = 3}, BigInt{7}) ==
        Status::Confirmed);
  CHECK(load_formula_override_snapshot(target, path) ==
        FormulaOverrideSnapshotStatus::InvalidSnapshot);
  CHECK(target.evaluate({.binding = target_binding,
                         .address = {.index = 3},
                         .mode = EvaluationMode::Exact,
                         .budget = Budget::unlimited()})
            .result.exact_integer() == 7);
  std::filesystem::remove(path);
}
