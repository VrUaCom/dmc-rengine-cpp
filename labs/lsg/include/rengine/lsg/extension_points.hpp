#pragma once

#include "physiology.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace rengine::lsg {

struct SurfaceState {
  PhysiologyState physiology{};
  float wetness{};
  float temperature_offset{};
  float strain{};
};

struct DamageState {
  float abrasion{};
  float bruising{};
  float cut_depth{};
  std::uint32_t revision{};
};

struct HairGenome {
  std::uint64_t seed{};
  std::uint8_t density{};
  std::uint8_t thickness{};
  std::uint8_t colour_r{}, colour_g{}, colour_b{};
  std::uint8_t roughness{};
  std::uint8_t curl{};
};

struct ClothGenome {
  std::uint64_t seed{};
  std::uint8_t material_class{};
  std::uint8_t roughness{};
  std::uint8_t thickness{};
  std::uint8_t reserved{};
};

class AnatomicalField {
public:
  virtual ~AnatomicalField() = default;
  [[nodiscard]] virtual float sample(std::array<float, 3> rest_position_m,
                                     std::uint8_t body_region) const noexcept = 0;
};

class StrainField {
public:
  virtual ~StrainField() = default;
  [[nodiscard]] virtual std::array<float, 3> sample(std::array<float, 3> rest_position_m,
                                                    std::uint8_t body_region) const noexcept = 0;
};

class GeneratedSurfaceCache {
public:
  virtual ~GeneratedSurfaceCache() = default;
  virtual void clear() noexcept = 0;
  [[nodiscard]] virtual std::size_t resident_bytes() const noexcept = 0;
};

class SharedSurfaceDecoder {
public:
  virtual ~SharedSurfaceDecoder() = default;
  [[nodiscard]] virtual bool decode(std::span<const std::byte> compact_code,
                                    std::span<float> output_channels) noexcept = 0;
};

} // namespace rengine::lsg
