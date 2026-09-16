#pragma once
#include "detail_scheduler.hpp"
#include "genome.hpp"
#include "physiology.hpp"
#include <cstddef>
#include <cstdint>
namespace rengine::lsg {
struct MemoryAccounting {
  std::uint64_t shared_geometry_bytes{};
  std::uint64_t character_specific_bytes{};
  std::uint64_t shared_generator_bytes{};
  std::uint64_t generated_runtime_bytes{};
  std::uint64_t gpu_buffers_bytes{};
  std::uint64_t gpu_textures_bytes{};
};
struct RuntimeTelemetry {
  float cpu_frame_ms{};
  float gpu_frame_ms{};
  std::uint32_t visible_triangles{};
  DetailBand detail_band{DetailBand::macro_only};
  std::size_t genome_bytes{};
  MemoryAccounting memory{};
};
class CharacterRuntime {
public:
  explicit CharacterRuntime(CharacterGenomeV0 genome);
  void set_physiology(PhysiologyPreset preset) noexcept;
  [[nodiscard]] const CharacterGenomeV0& genome() const noexcept { return genome_; }
  [[nodiscard]] const PhysiologyState& physiology() const noexcept { return physiology_; }
private:
  CharacterGenomeV0 genome_{};
  PhysiologyState physiology_{};
};
}
