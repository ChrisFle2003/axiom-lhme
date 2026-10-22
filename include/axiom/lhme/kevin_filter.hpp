#pragma once
#include <bit>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>
#include "axiom/lhme/domain_projection.hpp"
#include "axiom/lhme/formula_evaluator.hpp"
#include "axiom/lhme/kevin_memory_store.hpp"
namespace axiom::lhme {
class CandidateMask64 { public: constexpr CandidateMask64(std::uint64_t bits=0):bits_(bits){} static constexpr CandidateMask64 all(){return CandidateMask64{~0ULL};} constexpr std::uint64_t bits()const{return bits_;} constexpr std::uint32_t popcount()const{return std::popcount(bits_);} friend constexpr CandidateMask64 operator&(CandidateMask64 a,CandidateMask64 b){return {a.bits_&b.bits_};} friend constexpr CandidateMask64 operator|(CandidateMask64 a,CandidateMask64 b){return {a.bits_|b.bits_};} friend constexpr CandidateMask64 operator^(CandidateMask64 a,CandidateMask64 b){return {a.bits_^b.bits_};} friend constexpr CandidateMask64 operator~(CandidateMask64 a){return {~a.bits_};} private: std::uint64_t bits_; };
constexpr std::uint32_t grid_tile_bit(std::uint8_t x,std::uint8_t y){return y*8U+x;}
constexpr std::uint64_t grid_tile_row_mask(std::uint8_t y){return 0xffULL<<(y*8U);}
struct DomainEvaluationContext {
  const GridDomainSpec& domain;
  const DomainProjection* projection = nullptr;
  FormulaEvaluator& evaluator;
};
[[nodiscard]] EvaluationResult evaluate_domain_address(
    const DomainEvaluationContext&, const GridAddress&, EvaluationMode, Budget,
    std::optional<BigInt> modulus = std::nullopt);
[[nodiscard]] CandidateMask64 modular_equals_tile(KevinMemoryStore&, const GridIdentity&, const GridAddress&, const BigInt&, const BigInt&);
[[nodiscard]] CandidateMask64 metric_bit_length_between_tile(KevinMemoryStore&, const GridIdentity&, const GridAddress&, std::uint64_t, std::uint64_t);
struct TileSummary { Status status=Status::Confirmed; std::uint32_t checked=64, modular_survivors=0, metric_survivors=0, final_survivors=0, exact_evaluations=0; std::vector<GridAddress> candidates; };
struct ExactTileResult { TileSummary summary; std::vector<KevinValue> values; };
struct DomainExactTileResult { TileSummary summary; std::vector<EvaluationResult> values; };
[[nodiscard]] CandidateMask64 modular_equals_tile(const DomainEvaluationContext&, const GridAddress&, const BigInt&, const BigInt&);
[[nodiscard]] CandidateMask64 metric_bit_length_between_tile(const DomainEvaluationContext&, const GridAddress&, std::uint64_t, std::uint64_t);
[[nodiscard]] TileSummary filter_tile(const DomainEvaluationContext&, const GridAddress&, const BigInt&, const BigInt&, std::uint64_t, std::uint64_t);
[[nodiscard]] DomainExactTileResult evaluate_survivors_exact(const DomainEvaluationContext&, TileSummary, std::uint64_t, Budget);
[[nodiscard]] TileSummary filter_tile(KevinMemoryStore&, const GridIdentity&, const GridAddress&, const BigInt&, const BigInt&, std::uint64_t, std::uint64_t);
[[nodiscard]] ExactTileResult evaluate_survivors_exact(KevinMemoryStore&, const GridIdentity&, TileSummary, std::uint64_t, Budget);

struct TileReductionSummary {
  Status status = Status::Confirmed;
  std::uint64_t tile_count = 0;
  std::uint64_t checked = 0;
  std::uint64_t modular_survivors = 0;
  std::uint64_t metric_survivors = 0;
  std::uint64_t final_survivors = 0;
  std::uint64_t exact_evaluations = 0;
  std::vector<GridAddress> candidates;
};
struct ExactTileReductionResult { TileReductionSummary summary; std::vector<KevinValue> values; };
[[nodiscard]] TileReductionSummary filter_tiles(KevinMemoryStore&, const GridIdentity&, std::span<const GridAddress>, const BigInt&, const BigInt&, std::uint64_t, std::uint64_t);
[[nodiscard]] TileReductionSummary filter_tiles_cpu(KevinMemoryStore&, const GridIdentity&, std::span<const GridAddress>, const BigInt&, const BigInt&, std::uint64_t, std::uint64_t, std::uint32_t worker_count);
[[nodiscard]] ExactTileReductionResult evaluate_reduced_exact(KevinMemoryStore&, const GridIdentity&, TileReductionSummary, std::uint64_t, Budget);

class KevinTileCpuExecutor {
 public:
  explicit KevinTileCpuExecutor(std::uint32_t worker_count);
  ~KevinTileCpuExecutor();
  KevinTileCpuExecutor(const KevinTileCpuExecutor&) = delete;
  auto operator=(const KevinTileCpuExecutor&) -> KevinTileCpuExecutor& = delete;

  [[nodiscard]] TileReductionSummary filter(
      KevinMemoryStore&, const GridIdentity&, std::span<const GridAddress>,
      const BigInt&, const BigInt&, std::uint64_t, std::uint64_t);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace axiom::lhme
