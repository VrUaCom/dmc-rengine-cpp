#include "rengine/lsg/body_regions.hpp"
#include "rengine/lsg/rmesh.hpp"

#include <fastgltf/core.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
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
  std::array<float, 4> w{};
  float sum = 0.0f;
  for (std::size_t i = 0; i < 4; ++i) {
    w[i] = std::max(0.0f, input[i]);
    sum += w[i];
  }
  if (!(sum > 1.0e-8f) || !std::isfinite(sum)) return {};
  std::array<std::uint16_t, 4> out{};
  std::uint32_t used = 0;
  for (std::size_t i = 0; i < 3; ++i) {
    const auto q = static_cast<std::uint32_t>(std::lround((w[i] / sum) * 65535.0f));
    const auto clamped = std::min(q, 65535u - used);
    out[i] = static_cast<std::uint16_t>(clamped);
    used += clamped;
  }
  out[3] = static_cast<std::uint16_t>(65535u - used);
  return out;
}

void regenerate_normals(RMeshV0& mesh) {
  std::vector<Vec3> sums(mesh.vertices.size());
  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const auto i0 = mesh.indices[i], i1 = mesh.indices[i + 1], i2 = mesh.indices[i + 2];
    const Vec3 p0 = position_of(mesh.vertices[i0]);
    const Vec3 p1 = position_of(mesh.vertices[i1]);
    const Vec3 p2 = position_of(mesh.vertices[i2]);
    const Vec3 face = cross(sub(p1, p0), sub(p2, p0));
    sums[i0] = add(sums[i0], face); sums[i1] = add(sums[i1], face); sums[i2] = add(sums[i2], face);
  }
  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const Vec3 n = normalize(sums[i]);
    mesh.vertices[i].normal = {n.x, n.y, n.z};
  }
}

void regenerate_tangents(RMeshV0& mesh) {
  std::vector<Vec3> tangent_sum(mesh.vertices.size());
  std::vector<Vec3> bitangent_sum(mesh.vertices.size());
  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const auto i0 = mesh.indices[i], i1 = mesh.indices[i + 1], i2 = mesh.indices[i + 2];
    const auto& v0 = mesh.vertices[i0]; const auto& v1 = mesh.vertices[i1]; const auto& v2 = mesh.vertices[i2];
    const Vec3 e1 = sub(position_of(v1), position_of(v0));
    const Vec3 e2 = sub(position_of(v2), position_of(v0));
    const float du1 = v1.uv[0] - v0.uv[0], dv1 = v1.uv[1] - v0.uv[1];
    const float du2 = v2.uv[0] - v0.uv[0], dv2 = v2.uv[1] - v0.uv[1];
    const float det = du1 * dv2 - dv1 * du2;
    if (std::abs(det) <= 1.0e-12f) continue;
    const float inv = 1.0f / det;
    const Vec3 t = mul(sub(mul(e1, dv2), mul(e2, dv1)), inv);
    const Vec3 b = mul(sub(mul(e2, du1), mul(e1, du2)), inv);
    for (const auto index : {i0, i1, i2}) { tangent_sum[index] = add(tangent_sum[index], t); bitangent_sum[index] = add(bitangent_sum[index], b); }
  }
  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const Vec3 n = normal_of(mesh.vertices[i]);
    Vec3 t = sub(tangent_sum[i], mul(n, dot(n, tangent_sum[i])));
    t = normalize(t);
    const float handedness = dot(cross(n, t), bitangent_sum[i]) < 0.0f ? -1.0f : 1.0f;
    mesh.vertices[i].tangent = {t.x, t.y, t.z, handedness};
  }
}

void assign_regions(RMeshV0& mesh) {
  std::array<float, 3> mn = mesh.vertices.front().position, mx = mesh.vertices.front().position;
  for (const auto& v : mesh.vertices) for (std::size_t i = 0; i < 3; ++i) { mn[i] = std::min(mn[i], v.position[i]); mx[i] = std::max(mx[i], v.position[i]); }
  const float height = std::max(1.0e-6f, mx[1] - mn[1]);
  const float half_width = std::max(1.0e-6f, 0.5f * (mx[0] - mn[0]));
  const float center_x = 0.5f * (mx[0] + mn[0]);
  for (auto& v : mesh.vertices) {
    const float y = (v.position[1] - mn[1]) / height;
    const float x = std::abs(v.position[0] - center_x) / half_width;
    rengine::lsg::BodyRegion region = rengine::lsg::BodyRegion::abdomen;
    if (y > 0.88f) region = rengine::lsg::BodyRegion::head;
    else if (y > 0.81f) region = rengine::lsg::BodyRegion::neck;
    else if (x > 0.55f && y > 0.66f) region = rengine::lsg::BodyRegion::upper_arm;
    else if (x > 0.72f && y > 0.52f) region = rengine::lsg::BodyRegion::forearm;
    else if (x > 0.88f && y > 0.42f) region = rengine::lsg::BodyRegion::hand;
    else if (y > 0.62f) region = v.position[2] < 0.0f ? rengine::lsg::BodyRegion::back : rengine::lsg::BodyRegion::chest;
    else if (y > 0.48f) region = rengine::lsg::BodyRegion::abdomen;
    else if (y > 0.26f) region = rengine::lsg::BodyRegion::thigh;
    else if (y > 0.08f) region = rengine::lsg::BodyRegion::lower_leg;
    else region = rengine::lsg::BodyRegion::foot;
    v.region_id = static_cast<std::uint8_t>(region);
  }
}

bool append_primitive(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive, RMeshV0& out, std::string& error) {
  if (primitive.type != fastgltf::PrimitiveType::Triangles) { error = "only triangle-list glTF primitives are accepted in rmesh v0"; return false; }
  const auto position_it = primitive.findAttribute("POSITION");
  const auto uv_it = primitive.findAttribute("TEXCOORD_0");
  if (position_it == primitive.attributes.end()) { error = "primitive has no POSITION"; return false; }
  if (uv_it == primitive.attributes.end()) { error = "primitive has no TEXCOORD_0; v0 requires validated UVs"; return false; }
  if (!primitive.indicesAccessor.has_value()) { error = "primitive has no generated/index accessor"; return false; }
  const auto& positions = asset.accessors[position_it->accessorIndex];
  const auto& uvs = asset.accessors[uv_it->accessorIndex];
  if (positions.count == 0 || positions.count != uvs.count) { error = "POSITION/TEXCOORD_0 accessor count mismatch"; return false; }
  const std::size_t base = out.vertices.size();
  out.vertices.resize(base + positions.count);
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positions, [&](const auto& p, std::size_t i) { out.vertices[base + i].position = {p[0], p[1], p[2]}; });
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, uvs, [&](const auto& uv, std::size_t i) { out.vertices[base + i].uv = {uv[0], uv[1]}; });

  const auto joints_it = primitive.findAttribute("JOINTS_0");
  const auto weights_it = primitive.findAttribute("WEIGHTS_0");
  if ((joints_it == primitive.attributes.end()) != (weights_it == primitive.attributes.end())) { error = "JOINTS_0 and WEIGHTS_0 must appear together"; return false; }
  if (joints_it != primitive.attributes.end()) {
    const auto& joints = asset.accessors[joints_it->accessorIndex];
    const auto& weights = asset.accessors[weights_it->accessorIndex];
    if (joints.count != positions.count || weights.count != positions.count) { error = "skin accessor count mismatch"; return false; }
    fastgltf::iterateAccessorWithIndex<fastgltf::math::uvec4>(asset, joints, [&](const auto& value, std::size_t i) {
      for (std::size_t c = 0; c < 4; ++c) out.vertices[base + i].joints[c] = static_cast<std::uint16_t>(std::min(value[c], 65535u));
    });
    fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, weights, [&](const auto& value, std::size_t i) { out.vertices[base + i].weights_unorm = quantize_weights(value); });
    out.flags |= rengine::lsg::rmesh_has_skin;
  }

  const auto& indices = asset.accessors[*primitive.indicesAccessor];
  const std::size_t first = out.indices.size();
  out.indices.resize(first + indices.count);
  fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset, indices, [&](std::uint32_t value, std::size_t i) {
    out.indices[first + i] = static_cast<std::uint32_t>(base + value);
  });
  return true;
}

bool write_file(const std::filesystem::path& path, const std::vector<std::byte>& bytes) {
  std::ofstream stream(path, std::ios::binary);
  if (!stream) return false;
  stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return static_cast<bool>(stream);
}
}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage: lsg_prepare_mesh <human.glb|human.gltf> <human.rmesh>\n";
    return 2;
  }
  const std::filesystem::path input = argv[1], output = argv[2];
  auto source = fastgltf::MappedGltfFile::FromPath(input);
  if (!source) { std::cerr << "failed to open glTF: " << fastgltf::getErrorMessage(source.error()) << '\n'; return 3; }
  fastgltf::Parser parser;
  constexpr auto options = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices;
  auto loaded = parser.loadGltf(source.get(), input.parent_path(), options);
  if (!loaded) { std::cerr << "failed to parse glTF: " << fastgltf::getErrorMessage(loaded.error()) << '\n'; return 4; }
  if (const auto validation = fastgltf::validate(loaded.get()); validation != fastgltf::Error::None) { std::cerr << "glTF validation failed: " << fastgltf::getErrorMessage(validation) << '\n'; return 5; }
  const auto& asset = loaded.get();
  if (asset.meshes.empty()) { std::cerr << "glTF contains no meshes\n"; return 6; }

  RMeshV0 mesh{};
  mesh.flags = rengine::lsg::rmesh_has_uv | rengine::lsg::rmesh_has_tangents | rengine::lsg::rmesh_has_regions;
  std::size_t max_joints = 0;
  for (const auto& skin : asset.skins) max_joints = std::max(max_joints, skin.joints.size());
  mesh.joint_count = static_cast<std::uint16_t>(std::min<std::size_t>(max_joints, 65535u));
  std::string error;
  for (const auto& source_mesh : asset.meshes) for (const auto& primitive : source_mesh.primitives) {
    if (!append_primitive(asset, primitive, mesh, error)) { std::cerr << "mesh preparation failed: " << error << '\n'; return 7; }
  }
  regenerate_normals(mesh);
  regenerate_tangents(mesh);
  assign_regions(mesh);
  if (!rengine::lsg::validate_rmesh(mesh, error)) { std::cerr << "rmesh validation failed: " << error << '\n'; return 8; }
  const auto bytes = rengine::lsg::encode_rmesh(mesh);
  if (bytes.empty() || !write_file(output, bytes)) { std::cerr << "failed to write rmesh\n"; return 9; }
  std::cout << "RMS0 PASS: vertices=" << mesh.vertices.size() << " indices=" << mesh.indices.size()
            << " joints=" << mesh.joint_count << " bytes=" << bytes.size() << '\n';
  return 0;
}
