#pragma once

#include <filesystem>

#include "axiom/lhme/kevin_memory_store.hpp"

namespace axiom::lhme {
[[nodiscard]] SnapshotStatus save_snapshot(const KevinMemoryStore& store,
                                           const std::filesystem::path& path);
[[nodiscard]] SnapshotStatus load_snapshot(KevinMemoryStore& store,
                                           const std::filesystem::path& path);
}  // namespace axiom::lhme
