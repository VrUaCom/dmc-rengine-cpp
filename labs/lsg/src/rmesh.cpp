#include "rengine/lsg/rmesh.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <type_traits>

namespace rengine::lsg {
namespace {
constexpr std::array<char, 4> kMagic{'R', 'M', 'S', '0'};

std::uint32_t crc32(std::span<const std::byte> data) noexcept {
  std::uint32_t crc = 0xFFFFFFFFu;
  for (const auto byte : data) {
    crc ^= std::to_integer<std::uint8_t>(byte);
    for (int i = 0; i < 8; ++i) {
      crc = (crc >> 1u) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
  }
  return ~crc;
}

template <class T>
void put_le(std::vector<std::byte>& out, T value) {
  using U = std::make_unsigned_t<T>;
  const U bits = static_cast<U>(value);
  for (std::size_t i = 0; i < sizeof(T); ++i) {
    out.push_back(static_cast<std::byte>((bits >> (i * 8u)) & 0xFFu));
  }
}

void put_float(std::vector<std::byte>& out, float value) {
  put_le(out, std::bit_cast<std::uint32_t>(value));
}

template <class T>
bool get_le(std::span<const std::byte> in, std::size_t& offset, T& value) {
  if (offset + sizeof(T) > in.size()) {
    return false;
  }
  using U = std::make_unsigned_t<T>;
  std::uint64_t bits = 0;
  for (std::size_t i = 0; i < sizeof(T); ++i) {
    bits |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(in[offset + i])) << (i * 8u);
  }
  offset += sizeof(T);
  value = static_cast<T>(static_cast<U>(bits));
  return true;
}

bool get_float(std::span<const std::byte> in, std::size_t& offset, float& value) {
  std::uint32_t bits{};
  if (!get_le(in, offset, bits)) {
    return false;
  }
  value = std::bit_cast<float>(bits);
  return true;
}

template <std::size_t N>
bool finite(const std::array<float, N>& values) noexcept {
  return std::all_of(values.begin(), values.end(), [](float value) { return std::isfinite(value); });
}

bool valid_region(std::uint8_t region) noexcept {
  return region <= 10u || region == 255u;
}
}  // namespace

bool validate_rmesh(const RMeshV0& mesh, std::string& error) noexcept {
  if (mesh.vertices.empty()) {
    error = "rmesh has no vertices";
    return false;
  }
  if (mesh.indices.empty() || (mesh.indices.size() % 3u) != 0u) {
    error = "rmesh index buffer is not a non-empty triangle list";
    return false;
  }
  if (mesh.vertices.size() > std::numeric_limits<std::uint32_t>::max() ||
      mesh.indices.size() > std::numeric_limits<std::uint32_t>::max()) {
    error = "rmesh exceeds v0 32-bit element counts";
    return false;
  }
  for (const auto index : mesh.indices) {
    if (index >= mesh.vertices.size()) {
      error = "rmesh index references a missing vertex";
      return false;
    }
  }
  for (const auto& vertex : mesh.vertices) {
    if (!finite(vertex.position) || !finite(vertex.normal) || !finite(vertex.tangent) || !finite(vertex.uv)) {
      error = "rmesh contains NaN/Inf vertex data";
      return false;
    }
    if (!valid_region(vertex.region_id)) {
      error = "rmesh contains an invalid body-region id";
      return false;
    }
    std::uint32_t weight_sum = 0;
    for (std::size_t i = 0; i < vertex.weights_unorm.size(); ++i) {
      weight_sum += vertex.weights_unorm[i];
      if (vertex.weights_unorm[i] != 0u && vertex.joints[i] >= mesh.joint_count && mesh.joint_count != 0u) {
        error = "rmesh skin weight references an invalid joint";
        return false;
      }
    }
    if (weight_sum != 0u && weight_sum != 65535u) {
      error = "rmesh skin weights must sum to 65535 or zero";
      return false;
    }
  }
  error.clear();
  return true;
}

std::vector<std::byte> encode_rmesh(const RMeshV0& mesh) {
  std::string error;
  if (!validate_rmesh(mesh, error)) {
    return {};
  }

  std::vector<std::byte> payload;
  payload.reserve(mesh.vertices.size() * kRMeshVertexStrideV0 + mesh.indices.size() * sizeof(std::uint32_t));
  std::array<float, 3> minimum = mesh.vertices.front().position;
  std::array<float, 3> maximum = mesh.vertices.front().position;

  for (const auto& vertex : mesh.vertices) {
    for (std::size_t i = 0; i < 3; ++i) {
      minimum[i] = std::min(minimum[i], vertex.position[i]);
      maximum[i] = std::max(maximum[i], vertex.position[i]);
      put_float(payload, vertex.position[i]);
    }
    for (const auto value : vertex.normal) put_float(payload, value);
    for (const auto value : vertex.tangent) put_float(payload, value);
    for (const auto value : vertex.uv) put_float(payload, value);
    for (const auto value : vertex.joints) put_le(payload, value);
    for (const auto value : vertex.weights_unorm) put_le(payload, value);
    payload.push_back(static_cast<std::byte>(vertex.region_id));
    for (const auto value : vertex.reserved) payload.push_back(static_cast<std::byte>(value));
  }
  for (const auto index : mesh.indices) put_le(payload, index);

  if (payload.size() > std::numeric_limits<std::uint32_t>::max()) {
    return {};
  }

  std::vector<std::byte> out;
  out.reserve(kRMeshHeaderSize + payload.size());
  for (const auto c : kMagic) out.push_back(static_cast<std::byte>(c));
  put_le(out, kRMeshVersion);
  put_le(out, kRMeshHeaderSize);
  put_le(out, kRMeshVertexStrideV0);
  put_le(out, static_cast<std::uint16_t>(sizeof(std::uint32_t)));
  put_le(out, mesh.flags);
  put_le(out, static_cast<std::uint32_t>(mesh.vertices.size()));
  put_le(out, static_cast<std::uint32_t>(mesh.indices.size()));
  put_le(out, mesh.joint_count);
  put_le(out, static_cast<std::uint16_t>(0));
  put_le(out, static_cast<std::uint32_t>(payload.size()));
  put_le(out, crc32(payload));
  for (const auto value : minimum) put_float(out, value);
  for (const auto value : maximum) put_float(out, value);
  put_le(out, static_cast<std::uint32_t>(0));
  if (out.size() != kRMeshHeaderSize) {
    return {};
  }
  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

bool decode_rmesh(std::span<const std::byte> bytes, RMeshV0& out, std::string& error) {
  if (bytes.size() < kRMeshHeaderSize) {
    error = "rmesh shorter than v0 header";
    return false;
  }
  for (std::size_t i = 0; i < kMagic.size(); ++i) {
    if (std::to_integer<char>(bytes[i]) != kMagic[i]) {
      error = "bad RMS0 magic";
      return false;
    }
  }

  std::size_t offset = 4;
  std::uint16_t version{}, header_size{}, vertex_stride{}, index_stride{}, joint_count{}, reserved16{};
  std::uint32_t flags{}, vertex_count{}, index_count{}, payload_size{}, checksum{}, reserved32{};
  if (!get_le(bytes, offset, version) || !get_le(bytes, offset, header_size) ||
      !get_le(bytes, offset, vertex_stride) || !get_le(bytes, offset, index_stride) ||
      !get_le(bytes, offset, flags) || !get_le(bytes, offset, vertex_count) ||
      !get_le(bytes, offset, index_count) || !get_le(bytes, offset, joint_count) ||
      !get_le(bytes, offset, reserved16) || !get_le(bytes, offset, payload_size) ||
      !get_le(bytes, offset, checksum)) {
    error = "truncated rmesh header";
    return false;
  }
  std::array<float, 3> ignored_min{}, ignored_max{};
  for (auto& value : ignored_min) if (!get_float(bytes, offset, value)) return false;
  for (auto& value : ignored_max) if (!get_float(bytes, offset, value)) return false;
  if (!get_le(bytes, offset, reserved32)) {
    error = "truncated rmesh bounds";
    return false;
  }
  if (version != kRMeshVersion || header_size != kRMeshHeaderSize ||
      vertex_stride != kRMeshVertexStrideV0 || index_stride != sizeof(std::uint32_t)) {
    error = "unsupported rmesh v0 layout";
    return false;
  }
  const std::uint64_t expected_payload = static_cast<std::uint64_t>(vertex_count) * vertex_stride +
                                         static_cast<std::uint64_t>(index_count) * index_stride;
  if (expected_payload != payload_size ||
      static_cast<std::uint64_t>(header_size) + expected_payload != bytes.size()) {
    error = "rmesh declared payload size mismatch";
    return false;
  }
  const auto payload = bytes.subspan(header_size);
  if (crc32(payload) != checksum) {
    error = "rmesh payload checksum mismatch";
    return false;
  }

  RMeshV0 decoded{};
  decoded.flags = flags;
  decoded.joint_count = joint_count;
  decoded.vertices.resize(vertex_count);
  offset = header_size;
  for (auto& vertex : decoded.vertices) {
    for (auto& value : vertex.position) if (!get_float(bytes, offset, value)) return false;
    for (auto& value : vertex.normal) if (!get_float(bytes, offset, value)) return false;
    for (auto& value : vertex.tangent) if (!get_float(bytes, offset, value)) return false;
    for (auto& value : vertex.uv) if (!get_float(bytes, offset, value)) return false;
    for (auto& value : vertex.joints) if (!get_le(bytes, offset, value)) return false;
    for (auto& value : vertex.weights_unorm) if (!get_le(bytes, offset, value)) return false;
    if (offset + 4 > bytes.size()) return false;
    vertex.region_id = std::to_integer<std::uint8_t>(bytes[offset++]);
    for (auto& value : vertex.reserved) value = std::to_integer<std::uint8_t>(bytes[offset++]);
  }
  decoded.indices.resize(index_count);
  for (auto& index : decoded.indices) if (!get_le(bytes, offset, index)) return false;
  if (offset != bytes.size()) {
    error = "unexpected trailing rmesh data";
    return false;
  }
  if (!validate_rmesh(decoded, error)) {
    return false;
  }
  out = std::move(decoded);
  error.clear();
  return true;
}

}  // namespace rengine::lsg
