#pragma once
#include <cstdint>

namespace rengine::lsg {
[[nodiscard]] constexpr std::uint32_t pcg_hash(std::uint32_t input) noexcept {
  const std::uint32_t state = input * 747796405u + 2891336453u;
  const std::uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
  return (word >> 22u) ^ word;
}

[[nodiscard]] constexpr std::uint32_t fold_seed64(std::uint64_t seed) noexcept {
  return static_cast<std::uint32_t>(seed) ^ static_cast<std::uint32_t>(seed >> 32u);
}

[[nodiscard]] constexpr std::uint32_t hash_cell3_32(std::uint32_t seed_key,
                                                    std::int32_t x,
                                                    std::int32_t y,
                                                    std::int32_t z) noexcept {
  std::uint32_t h = pcg_hash(seed_key);
  h = pcg_hash(h ^ static_cast<std::uint32_t>(x));
  h = pcg_hash(h ^ static_cast<std::uint32_t>(y));
  h = pcg_hash(h ^ static_cast<std::uint32_t>(z));
  return h;
}

[[nodiscard]] constexpr std::uint32_t hash_cell3(std::uint64_t seed,
                                                 std::int32_t x,
                                                 std::int32_t y,
                                                 std::int32_t z) noexcept {
  return hash_cell3_32(fold_seed64(seed), x, y, z);
}

[[nodiscard]] constexpr std::uint32_t hash5_32(std::uint32_t seed_key, std::uint32_t region,
                                               std::int32_t x, std::int32_t y, std::int32_t z) noexcept {
  std::uint32_t h = pcg_hash(seed_key);
  h = pcg_hash(h ^ region * 0x9e3779b9u);
  h = pcg_hash(h ^ static_cast<std::uint32_t>(x));
  h = pcg_hash(h ^ static_cast<std::uint32_t>(y));
  h = pcg_hash(h ^ static_cast<std::uint32_t>(z));
  return h;
}

[[nodiscard]] constexpr std::uint32_t hash5(std::uint64_t seed, std::uint32_t region,
                                            std::int32_t x, std::int32_t y, std::int32_t z) noexcept {
  return hash5_32(fold_seed64(seed), region, x, y, z);
}

[[nodiscard]] constexpr float hash01(std::uint32_t h) noexcept {
  return static_cast<float>(h >> 8u) * (1.0f / 16777216.0f);
}
}
