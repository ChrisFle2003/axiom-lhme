#pragma once

#include <filesystem>

#include "bruce/lhme/formula_override_store.hpp"

namespace bruce::lhme {

[[nodiscard]] FormulaOverrideSnapshotStatus save_formula_override_snapshot(
    const FormulaOverrideStore& store, const std::filesystem::path& path);
[[nodiscard]] FormulaOverrideSnapshotStatus load_formula_override_snapshot(
    FormulaOverrideStore& store, const std::filesystem::path& path);

}  // namespace bruce::lhme
