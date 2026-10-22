#include "test.hpp"
#include "axiom/lhme/kevin_snapshot.hpp"
#include "axiom/lhme/power_offset_kernel.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

TEST(kevin_snapshot_roundtrips_sparse_override) {
  using namespace axiom::lhme;
  const auto path = std::filesystem::temp_directory_path() / "axiom-kevin-snapshot-test.bin";
  std::filesystem::remove(path);
  auto engine = Engine::create({});
  CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  GridSpec grid{{.id="axiom.snapshot",.version=1}, {.id="axiom.power_offset",.version=1}, encode_power_offset_parameters(2,100), 1};
  const std::vector<GridStep> steps{{.x=1,.y=1}};
  const auto address = *GridAddress::from_steps(steps);
  KevinMemoryStore source{engine};
  CHECK(source.register_grid(grid));
  CHECK(source.set_override(grid.identity,address,BigInt{-42}) == Status::Confirmed);
  CHECK(save_snapshot(source,path) == SnapshotStatus::Confirmed);
  KevinMemoryStore restored{engine};
  CHECK(load_snapshot(restored,path) == SnapshotStatus::Confirmed);
  CHECK(restored.get(grid.identity,address,EvaluationMode::Exact,Budget::unlimited()).result.exact_integer() == -42);
  std::filesystem::remove(path);
}

TEST(kevin_snapshot_rejects_bad_magic_without_mutating_store) {
  using namespace axiom::lhme;
  const auto path = std::filesystem::temp_directory_path() / "axiom-kevin-bad-snapshot.bin";
  { std::ofstream out(path, std::ios::binary); out << "not-a-axiom-snapshot"; }
  auto engine = Engine::create({});
  CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  GridSpec grid{{.id="axiom.snapshot.bad",.version=1}, {.id="axiom.power_offset",.version=1}, encode_power_offset_parameters(2,100), 1};
  const auto address = *GridAddress::from_steps(std::vector<GridStep>{{.x=1,.y=1}});
  KevinMemoryStore store{engine}; CHECK(store.register_grid(grid));
  CHECK(store.set_override(grid.identity,address,BigInt{7}) == Status::Confirmed);
  CHECK(load_snapshot(store,path) == SnapshotStatus::InvalidSnapshot);
  CHECK(store.get(grid.identity,address,EvaluationMode::Exact,Budget::unlimited()).result.exact_integer() == 7);
  std::filesystem::remove(path);
}

TEST(kevin_snapshot_rejects_changed_payload_by_checksum) {
  using namespace axiom::lhme;
  const auto path = std::filesystem::temp_directory_path() / "axiom-kevin-checksum-test.bin";
  auto engine = Engine::create({}); CHECK(engine.register_technique(std::make_shared<PowerOffsetKernel>()));
  GridSpec grid{{.id="axiom.checksum",.version=1}, {.id="axiom.power_offset",.version=1}, encode_power_offset_parameters(2,100), 1};
  KevinMemoryStore store{engine}; CHECK(store.register_grid(grid)); CHECK(save_snapshot(store,path)==SnapshotStatus::Confirmed);
  { std::fstream file(path, std::ios::in|std::ios::out|std::ios::binary); file.seekp(0); file.put('X'); }
  CHECK(load_snapshot(store,path)==SnapshotStatus::InvalidSnapshot);
  std::filesystem::remove(path);
}
