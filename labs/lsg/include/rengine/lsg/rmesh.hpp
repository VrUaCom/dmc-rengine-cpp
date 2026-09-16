#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace rengine::lsg {

inline constexpr std::uint16_t kRMeshVersion = 0;
inline constexpr std::uint16_t kRMeshHeaderSize = 64;
inline constexpr std::uint16_t kRMeshVertexStrideV0 = 68;

enum RMeshFlags : std::uint32_t {
  rmesh_has_uv = 1u << 0u,
  rmesh_has_tangents = 1u << 1u,
  rmesh_has_skin = 1u << 2u,
  rmesh_has_regions = 1u << 3u,
};

struct RMeshVertexV0 {
  std::array<float, 3> position{};
  std::array<float, 3> normal{};
  std::array<float, 4> tangent{1.0f, 0.0f, 0.0f, 1.0f};
  std::array<float, 2> uv{};
  std::array<std::uint16_t, 4> joints{};
  std::array<std::uint16_t, 4> weights_unorm{};
  std::uint8_t region_id{255};
  std::array<std::uint8_t, 3> reserved{};
};

struct RMeshV0 {
  std::uint32_t flags{};
  std::uint16_t joint_count{};
  std::vector<RMeshVertexV0> vertices;
  std::vector<std::uint32_t> indices;
};

[[nodiscard]] bool validate_rmesh(const RMeshV0& mesh, std::string& error) noexcept;
[[nodiscard]] std::vector<std::byte> encode_rmesh(const RMeshV0& mesh);
[[nodiscard]] bool decode_rmesh(std::span<const std::byte> bytes,
                                RMeshV0& out,
                                std::string& error);

}  // namespace rengine::lsg
