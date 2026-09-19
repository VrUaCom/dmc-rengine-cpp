#include "rengine/lsg/body_regions.hpp"
#include "rengine/lsg/rmesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {
using rengine::lsg::RMeshV0;
using rengine::lsg::RMeshVertexV0;

struct Vec3 { float x{}, y{}, z{}; };
Vec3 add(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 sub(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 mul(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 cross(Vec3 a, Vec3 b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
Vec3 normalize(Vec3 v) {
  const float len2 = dot(v, v);
  if (len2 <= 1.0e-20f || !std::isfinite(len2)) return {0.0f, 1.0f, 0.0f};
  return mul(v, 1.0f / std::sqrt(len2));
}
Vec3 position_of(const RMeshVertexV0& v) { return {v.position[0], v.position[1], v.position[2]}; }
Vec3 normal_of(const RMeshVertexV0& v) { return {v.normal[0], v.normal[1], v.normal[2]}; }

std::array<std::uint16_t, 4> quantize_weights(const fastgltf::math::fvec4& input) {
  std::array<float, 4> weights{};
  float sum = 0.0f;
  for (std::size_t i = 0; i < 4; ++i) { weights[i] = std::max(0.0f, input[i]); sum += weights[i]; }
  if (!(sum > 1.0e-8f) || !std::isfinite(sum)) return {};
  std::array<std::uint16_t, 4> out{};
  std::uint32_t used = 0;
  for (std::size_t i = 0; i < 3; ++i) {
    const auto quantized = static_cast<std::uint32_t>(std::lround((weights[i] / sum) * 65535.0f));
    const auto clamped = std::min(quantized, 65535u - used);
    out[i] = static_cast<std::uint16_t>(clamped); used += clamped;
  }
  out[3] = static_cast<std::uint16_t>(65535u - used);
  return out;
}

void regenerate_normals(RMeshV0& mesh) {
  std::vector<Vec3> sums(mesh.vertices.size());
  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const auto i0 = mesh.indices[i], i1 = mesh.indices[i + 1], i2 = mesh.indices[i + 2];
    const Vec3 face = cross(sub(position_of(mesh.vertices[i1]), position_of(mesh.vertices[i0])),
                            sub(position_of(mesh.vertices[i2]), position_of(mesh.vertices[i0])));
    sums[i0] = add(sums[i0], face); sums[i1] = add(sums[i1], face); sums[i2] = add(sums[i2], face);
  }
  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const Vec3 normal = normalize(sums[i]); mesh.vertices[i].normal = {normal.x, normal.y, normal.z};
  }
}

struct SourceNormalSeamReport {
  float max_duplicate_normal_delta{0.0f};
  float normal_length_min{std::numeric_limits<float>::infinity()};
  float normal_length_max{0.0f};
};

float length(Vec3 v) {
  const float len2 = dot(v, v);
  return len2 > 0.0f && std::isfinite(len2) ? std::sqrt(len2) : 0.0f;
}

bool regenerate_normals_by_source_position(
    RMeshV0& mesh,
    const std::vector<std::uint32_t>& source_position_by_vertex,
    std::size_t source_position_count,
    SourceNormalSeamReport& report,
    std::string& error) {
  if (mesh.vertices.empty() || mesh.indices.empty() ||
      source_position_by_vertex.size() != mesh.vertices.size() ||
      source_position_count == 0u) {
    error = "source-position normal smoothing requires OBJ provenance";
    return false;
  }

  std::vector<Vec3> sums(source_position_count);
  std::vector<bool> used(source_position_count, false);
  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const std::uint32_t render[3] = {
        mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]};
    std::uint32_t source[3]{};
    for (int corner = 0; corner < 3; ++corner) {
      if (render[corner] >= mesh.vertices.size()) {
        error = "source-position normal smoothing saw an out-of-range render index";
        return false;
      }
      source[corner] = source_position_by_vertex[render[corner]];
      if (source[corner] >= source_position_count) {
        error = "source-position normal smoothing saw an out-of-range source index";
        return false;
      }
      used[source[corner]] = true;
    }

    const Vec3 face = cross(
        sub(position_of(mesh.vertices[render[1]]), position_of(mesh.vertices[render[0]])),
        sub(position_of(mesh.vertices[render[2]]), position_of(mesh.vertices[render[0]])));
    if (!std::isfinite(face.x) || !std::isfinite(face.y) || !std::isfinite(face.z)) {
      error = "source-position normal smoothing produced a non-finite face normal";
      return false;
    }
    for (const auto source_index : source) {
      sums[source_index] = add(sums[source_index], face);
    }
  }

  std::vector<Vec3> source_normals(source_position_count);
  for (std::size_t source = 0; source < source_position_count; ++source) {
    if (!used[source]) continue;
    const float len2 = dot(sums[source], sums[source]);
    if (!(len2 > 1.0e-20f) || !std::isfinite(len2)) {
      error = "source-position normal smoothing found a degenerate source normal";
      return false;
    }
    source_normals[source] = mul(sums[source], 1.0f / std::sqrt(len2));
  }

  std::vector<Vec3> first_normal(source_position_count);
  std::vector<bool> seen(source_position_count, false);
  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const auto source = source_position_by_vertex[i];
    if (source >= source_position_count || !used[source]) {
      error = "render vertex has no valid source-position normal";
      return false;
    }

    const Vec3 normal = source_normals[source];
    mesh.vertices[i].normal = {normal.x, normal.y, normal.z};

    const float normal_length = length(normal);
    if (!std::isfinite(normal_length)) {
      error = "source-position normal smoothing produced a non-finite normal length";
      return false;
    }
    report.normal_length_min = std::min(report.normal_length_min, normal_length);
    report.normal_length_max = std::max(report.normal_length_max, normal_length);

    if (seen[source]) {
      report.max_duplicate_normal_delta =
          std::max(report.max_duplicate_normal_delta,
                   length(sub(normal, first_normal[source])));
    } else {
      first_normal[source] = normal;
      seen[source] = true;
    }
  }

  if (!std::isfinite(report.normal_length_min) ||
      report.normal_length_min < 0.99f ||
      report.normal_length_max > 1.01f ||
      report.max_duplicate_normal_delta > 1.0e-5f) {
    error = "source-position normal smoothing failed seam/length validation";
    return false;
  }
  return true;
}

void regenerate_tangents(RMeshV0& mesh) {
  std::vector<Vec3> tangent_sum(mesh.vertices.size()), bitangent_sum(mesh.vertices.size());
  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const auto i0 = mesh.indices[i], i1 = mesh.indices[i + 1], i2 = mesh.indices[i + 2];
    const auto& v0 = mesh.vertices[i0]; const auto& v1 = mesh.vertices[i1]; const auto& v2 = mesh.vertices[i2];
    const Vec3 e1 = sub(position_of(v1), position_of(v0)), e2 = sub(position_of(v2), position_of(v0));
    const float du1 = v1.uv[0] - v0.uv[0], dv1 = v1.uv[1] - v0.uv[1];
    const float du2 = v2.uv[0] - v0.uv[0], dv2 = v2.uv[1] - v0.uv[1];
    const float determinant = du1 * dv2 - dv1 * du2;
    if (std::abs(determinant) <= 1.0e-12f) continue;
    const float inverse = 1.0f / determinant;
    const Vec3 tangent = mul(sub(mul(e1, dv2), mul(e2, dv1)), inverse);
    const Vec3 bitangent = mul(sub(mul(e2, du1), mul(e1, du2)), inverse);
    for (const auto index : {i0, i1, i2}) { tangent_sum[index] = add(tangent_sum[index], tangent); bitangent_sum[index] = add(bitangent_sum[index], bitangent); }
  }
  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const Vec3 normal = normal_of(mesh.vertices[i]);
    Vec3 tangent = normalize(sub(tangent_sum[i], mul(normal, dot(normal, tangent_sum[i]))));
    const float handedness = dot(cross(normal, tangent), bitangent_sum[i]) < 0.0f ? -1.0f : 1.0f;
    mesh.vertices[i].tangent = {tangent.x, tangent.y, tangent.z, handedness};
  }
}

bool assign_component_regions(RMeshV0& mesh,
                              const std::vector<std::uint32_t>& source_position_by_vertex,
                              std::size_t source_position_count,
                              std::size_t& component_count,
                              std::string& error) {
  if (mesh.vertices.empty() || mesh.indices.empty() ||
      source_position_by_vertex.size() != mesh.vertices.size() ||
      source_position_count == 0u) {
    error = "component tagging requires OBJ source-position provenance";
    return false;
  }

  std::vector<std::vector<std::uint32_t>> adjacency(source_position_count);
  std::vector<bool> used(source_position_count, false);
  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const std::uint32_t tri[3] = {mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]};
    std::uint32_t source[3]{};
    for (int corner = 0; corner < 3; ++corner) {
      if (tri[corner] >= source_position_by_vertex.size()) {
        error = "component tagging saw an out-of-range render index";
        return false;
      }
      source[corner] = source_position_by_vertex[tri[corner]];
      if (source[corner] >= source_position_count) {
        error = "component tagging saw an out-of-range source position index";
        return false;
      }
      used[source[corner]] = true;
    }
    for (int edge = 0; edge < 3; ++edge) {
      const auto a = source[edge];
      const auto b = source[(edge + 1) % 3];
      if (a == b) continue;
      adjacency[a].push_back(b);
      adjacency[b].push_back(a);
    }
  }

  std::vector<int> component(source_position_count, -1);
  component_count = 0;
  std::vector<std::uint32_t> stack;
  for (std::uint32_t root = 0; root < source_position_count; ++root) {
    if (!used[root] || component[root] >= 0) continue;
    if (component_count > 10u) {
      error = "RMS0 v0 component tagging supports at most 11 diagnostic components";
      return false;
    }

    stack.clear();
    stack.push_back(root);
    component[root] = static_cast<int>(component_count);
    while (!stack.empty()) {
      const auto current = stack.back();
      stack.pop_back();
      for (const auto neighbour : adjacency[current]) {
        if (component[neighbour] >= 0) continue;
        component[neighbour] = static_cast<int>(component_count);
        stack.push_back(neighbour);
      }
    }
    ++component_count;
  }

  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const auto source = source_position_by_vertex[i];
    if (source >= component.size() || component[source] < 0) {
      error = "render vertex has no connected source component";
      return false;
    }
    mesh.vertices[i].region_id = static_cast<std::uint8_t>(component[source]);
  }
  return true;
}

void assign_regions(RMeshV0& mesh) {
  std::array<float, 3> minimum = mesh.vertices.front().position, maximum = mesh.vertices.front().position;
  for (const auto& vertex : mesh.vertices) for (std::size_t i = 0; i < 3; ++i) { minimum[i] = std::min(minimum[i], vertex.position[i]); maximum[i] = std::max(maximum[i], vertex.position[i]); }
  const float height = std::max(1.0e-6f, maximum[1] - minimum[1]);
  const float half_width = std::max(1.0e-6f, 0.5f * (maximum[0] - minimum[0]));
  const float center_x = 0.5f * (maximum[0] + minimum[0]);
  for (auto& vertex : mesh.vertices) {
    const float y = (vertex.position[1] - minimum[1]) / height;
    const float x = std::abs(vertex.position[0] - center_x) / half_width;
    rengine::lsg::BodyRegion region = rengine::lsg::BodyRegion::abdomen;
    if (y > 0.88f) region = rengine::lsg::BodyRegion::head;
    else if (y > 0.81f) region = rengine::lsg::BodyRegion::neck;
    else if (x > 0.55f && y > 0.66f) region = rengine::lsg::BodyRegion::upper_arm;
    else if (x > 0.72f && y > 0.52f) region = rengine::lsg::BodyRegion::forearm;
    else if (x > 0.88f && y > 0.42f) region = rengine::lsg::BodyRegion::hand;
    else if (y > 0.62f) region = vertex.position[2] < 0.0f ? rengine::lsg::BodyRegion::back : rengine::lsg::BodyRegion::chest;
    else if (y > 0.48f) region = rengine::lsg::BodyRegion::abdomen;
    else if (y > 0.26f) region = rengine::lsg::BodyRegion::thigh;
    else if (y > 0.08f) region = rengine::lsg::BodyRegion::lower_leg;
    else region = rengine::lsg::BodyRegion::foot;
    vertex.region_id = static_cast<std::uint8_t>(region);
  }
}

bool append_primitive(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive, RMeshV0& out, std::string& error) {
  if (primitive.type != fastgltf::PrimitiveType::Triangles) { error = "only triangle-list glTF primitives are accepted in RMS0 v0"; return false; }
  const auto position_it = primitive.findAttribute("POSITION"), uv_it = primitive.findAttribute("TEXCOORD_0");
  if (position_it == primitive.attributes.end()) { error = "primitive has no POSITION"; return false; }
  if (uv_it == primitive.attributes.end()) { error = "primitive has no TEXCOORD_0; RMS0 v0 requires UVs"; return false; }
  if (!primitive.indicesAccessor.has_value()) { error = "primitive has no generated/index accessor"; return false; }
  const auto& positions = asset.accessors[position_it->accessorIndex]; const auto& uvs = asset.accessors[uv_it->accessorIndex];
  if (positions.count == 0 || positions.count != uvs.count) { error = "POSITION/TEXCOORD_0 accessor count mismatch"; return false; }
  const std::size_t base = out.vertices.size(); out.vertices.resize(base + positions.count);
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positions, [&](const auto& p, std::size_t i) { out.vertices[base + i].position = {p[0], p[1], p[2]}; });
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, uvs, [&](const auto& uv, std::size_t i) { out.vertices[base + i].uv = {uv[0], uv[1]}; });
  const auto joints_it = primitive.findAttribute("JOINTS_0"), weights_it = primitive.findAttribute("WEIGHTS_0");
  if ((joints_it == primitive.attributes.end()) != (weights_it == primitive.attributes.end())) { error = "JOINTS_0 and WEIGHTS_0 must appear together"; return false; }
  if (joints_it != primitive.attributes.end()) {
    const auto& joints = asset.accessors[joints_it->accessorIndex]; const auto& weights = asset.accessors[weights_it->accessorIndex];
    if (joints.count != positions.count || weights.count != positions.count) { error = "skin accessor count mismatch"; return false; }
    fastgltf::iterateAccessorWithIndex<fastgltf::math::uvec4>(asset, joints, [&](const auto& value, std::size_t i) {
      for (std::size_t c = 0; c < 4; ++c) out.vertices[base + i].joints[c] = static_cast<std::uint16_t>(std::min(value[c], 65535u));
    });
    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, weights, [&](const auto& value, std::size_t i) { out.vertices[base + i].weights_unorm = quantize_weights(value); });
    out.flags |= rengine::lsg::rmesh_has_skin;
  }
  const auto& indices = asset.accessors[*primitive.indicesAccessor]; const std::size_t first = out.indices.size(); out.indices.resize(first + indices.count);
  fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset, indices, [&](std::uint32_t value, std::size_t i) { out.indices[first + i] = static_cast<std::uint32_t>(base + value); });
  return true;
}

bool load_gltf(const std::filesystem::path& input, RMeshV0& mesh, std::string& error) {
  auto source = fastgltf::MappedGltfFile::FromPath(input);
  if (!source) { error = std::string{"failed to open glTF: "} + std::string{fastgltf::getErrorMessage(source.error())}; return false; }
  fastgltf::Parser parser;
  constexpr auto options = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices;
  auto loaded = parser.loadGltf(source.get(), input.parent_path(), options);
  if (!loaded) { error = std::string{"failed to parse glTF: "} + std::string{fastgltf::getErrorMessage(loaded.error())}; return false; }
  if (const auto validation = fastgltf::validate(loaded.get()); validation != fastgltf::Error::None) { error = std::string{"glTF validation failed: "} + std::string{fastgltf::getErrorMessage(validation)}; return false; }
  const auto& asset = loaded.get(); if (asset.meshes.empty()) { error = "glTF contains no meshes"; return false; }
  std::size_t max_joints = 0; for (const auto& skin : asset.skins) max_joints = std::max(max_joints, skin.joints.size());
  mesh.joint_count = static_cast<std::uint16_t>(std::min<std::size_t>(max_joints, 65535u));
  for (const auto& source_mesh : asset.meshes) for (const auto& primitive : source_mesh.primitives) if (!append_primitive(asset, primitive, mesh, error)) return false;
  return true;
}

struct ObjKey {
  std::int32_t position{-1};
  std::int32_t uv{-1};
  bool operator==(const ObjKey&) const noexcept = default;
};
struct ObjKeyHash {
  std::size_t operator()(const ObjKey& key) const noexcept {
    return (static_cast<std::size_t>(static_cast<std::uint32_t>(key.position)) << 32u) ^ static_cast<std::uint32_t>(key.uv);
  }
};

bool parse_integer(std::string_view text, std::int32_t& out) {
  if (text.empty()) return false;
  const auto* begin = text.data(); const auto* end = begin + text.size();
  const auto result = std::from_chars(begin, end, out);
  return result.ec == std::errc{} && result.ptr == end;
}

bool resolve_obj_index(std::int32_t raw, std::size_t count, std::int32_t& out) {
  if (raw == 0 || count > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) return false;
  const auto resolved = raw > 0 ? static_cast<std::int64_t>(raw) - 1 : static_cast<std::int64_t>(count) + raw;
  if (resolved < 0 || resolved >= static_cast<std::int64_t>(count)) return false;
  out = static_cast<std::int32_t>(resolved); return true;
}

bool parse_obj_corner(std::string_view token, std::size_t position_count, std::size_t uv_count, ObjKey& out) {
  const auto first = token.find('/');
  const auto second = first == std::string_view::npos ? std::string_view::npos : token.find('/', first + 1);
  const auto position_text = first == std::string_view::npos ? token : token.substr(0, first);
  const auto uv_text = first == std::string_view::npos ? std::string_view{} : token.substr(first + 1, (second == std::string_view::npos ? token.size() : second) - first - 1);
  std::int32_t raw_position{}, raw_uv{};
  if (!parse_integer(position_text, raw_position) || !parse_integer(uv_text, raw_uv)) return false;
  return resolve_obj_index(raw_position, position_count, out.position) && resolve_obj_index(raw_uv, uv_count, out.uv);
}

bool load_obj(const std::filesystem::path& input, RMeshV0& mesh,
              std::vector<std::uint32_t>* source_position_by_vertex,
              std::size_t* source_position_count,
              std::string& error) {
  std::ifstream stream(input); if (!stream) { error = "failed to open OBJ"; return false; }
  std::vector<std::array<float, 3>> positions; std::vector<std::array<float, 2>> uvs;
  std::unordered_map<ObjKey, std::uint32_t, ObjKeyHash> vertex_map;
  std::string line; std::size_t line_number = 0;
  auto materialize = [&](const ObjKey& key) -> std::uint32_t {
    if (const auto found = vertex_map.find(key); found != vertex_map.end()) return found->second;
    RMeshVertexV0 vertex{}; vertex.position = positions[static_cast<std::size_t>(key.position)]; vertex.uv = uvs[static_cast<std::size_t>(key.uv)];
    const auto index = static_cast<std::uint32_t>(mesh.vertices.size());
    mesh.vertices.push_back(vertex);
    if (source_position_by_vertex != nullptr) {
      source_position_by_vertex->push_back(static_cast<std::uint32_t>(key.position));
    }
    vertex_map.emplace(key, index);
    return index;
  };
  while (std::getline(stream, line)) {
    ++line_number; if (line.empty() || line[0] == '#') continue;
    std::istringstream values(line); std::string tag; values >> tag;
    if (tag == "v") {
      std::array<float, 3> value{}; if (!(values >> value[0] >> value[1] >> value[2]) || !std::isfinite(value[0]) || !std::isfinite(value[1]) || !std::isfinite(value[2])) { error = "invalid OBJ position at line " + std::to_string(line_number); return false; }
      positions.push_back(value);
    } else if (tag == "vt") {
      std::array<float, 2> value{}; if (!(values >> value[0] >> value[1]) || !std::isfinite(value[0]) || !std::isfinite(value[1])) { error = "invalid OBJ UV at line " + std::to_string(line_number); return false; }
      uvs.push_back(value);
    } else if (tag == "f") {
      std::vector<ObjKey> face; std::string token;
      while (values >> token) { ObjKey key{}; if (!parse_obj_corner(token, positions.size(), uvs.size(), key)) { error = "OBJ face requires valid v/vt corners at line " + std::to_string(line_number); return false; } face.push_back(key); }
      if (face.size() < 3) { error = "OBJ face has fewer than three corners at line " + std::to_string(line_number); return false; }
      for (std::size_t i = 1; i + 1 < face.size(); ++i) { mesh.indices.push_back(materialize(face[0])); mesh.indices.push_back(materialize(face[i])); mesh.indices.push_back(materialize(face[i + 1])); }
    }
  }
  if (positions.empty() || uvs.empty() || mesh.vertices.empty() || mesh.indices.empty()) { error = "OBJ produced no renderable UV-mapped geometry"; return false; }
  if (source_position_count != nullptr) *source_position_count = positions.size();
  return true;
}

bool write_file(const std::filesystem::path& path, const std::vector<std::byte>& bytes) {
  std::ofstream stream(path, std::ios::binary); if (!stream) return false;
  stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())); return static_cast<bool>(stream);
}
}  // namespace

int main(int argc, char** argv) {
  if (argc != 3 && argc != 4) {
    std::cerr << "usage: lsg_prepare_mesh <mesh.obj|mesh.glb|mesh.gltf> <mesh.rmesh> [--component-regions]\n";
    return 2;
  }
  const bool component_regions = argc == 4 && std::string_view{argv[3]} == "--component-regions";
  if (argc == 4 && !component_regions) {
    std::cerr << "mesh preparation failed: unknown option " << argv[3] << '\n';
    return 2;
  }

  const std::filesystem::path input = argv[1], output = argv[2];
  RMeshV0 mesh{}; mesh.flags = rengine::lsg::rmesh_has_uv | rengine::lsg::rmesh_has_tangents | rengine::lsg::rmesh_has_regions;
  std::string extension = input.extension().string(); std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  std::string error;
  std::vector<std::uint32_t> source_position_by_vertex;
  std::size_t source_position_count = 0;
  const bool loaded = extension == ".obj"
      ? load_obj(input, mesh,
                 component_regions ? &source_position_by_vertex : nullptr,
                 component_regions ? &source_position_count : nullptr,
                 error)
      : (extension == ".glb" || extension == ".gltf") ? load_gltf(input, mesh, error) : false;
  if (component_regions && extension != ".obj") {
    std::cerr << "mesh preparation failed: --component-regions currently requires OBJ input\n";
    return 3;
  }
  if (!loaded) { if (error.empty()) error = "unsupported mesh extension"; std::cerr << "mesh preparation failed: " << error << '\n'; return 3; }

  SourceNormalSeamReport source_normal_report{};
  if (component_regions) {
    if (!regenerate_normals_by_source_position(
            mesh, source_position_by_vertex, source_position_count,
            source_normal_report, error)) {
      std::cerr << "mesh preparation failed: " << error << '\n';
      return 4;
    }
  } else {
    regenerate_normals(mesh);
  }
  regenerate_tangents(mesh);
  std::size_t component_count = 0;
  if (component_regions) {
    if (!assign_component_regions(mesh, source_position_by_vertex, source_position_count,
                                  component_count, error)) {
      std::cerr << "mesh preparation failed: " << error << '\n';
      return 4;
    }
  } else {
    assign_regions(mesh);
  }

  if (!rengine::lsg::validate_rmesh(mesh, error)) { std::cerr << "RMS0 validation failed: " << error << '\n'; return 5; }
  const auto bytes = rengine::lsg::encode_rmesh(mesh);
  if (bytes.empty() || !write_file(output, bytes)) { std::cerr << "failed to write RMS0\n"; return 6; }
  if (component_regions) {
    std::cout << "SOURCE NORMAL SEAMS PASS: source_positions=" << source_position_count
              << " max_duplicate_normal_delta=" << source_normal_report.max_duplicate_normal_delta
              << " normal_length_min=" << source_normal_report.normal_length_min
              << " normal_length_max=" << source_normal_report.normal_length_max << '\n';
  }
  std::cout << "RMS0 PASS: vertices=" << mesh.vertices.size() << " indices=" << mesh.indices.size()
            << " triangles=" << mesh.indices.size() / 3u << " joints=" << mesh.joint_count
            << " components=" << component_count << " bytes=" << bytes.size() << '\n';
  return 0;
}
