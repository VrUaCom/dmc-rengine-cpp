#include "rengine/lsg/anatomical_field.hpp"
#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/rmesh.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {
using namespace rengine::lsg;

std::vector<std::byte> read_file(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) return {};
  const auto end = stream.tellg();
  if (end <= 0) return {};
  const auto size = static_cast<std::size_t>(end);
  std::vector<std::byte> bytes(size);
  stream.seekg(0, std::ios::beg);
  stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
  return stream ? bytes : std::vector<std::byte>{};
}

float distance(AnatomicalPoint a, AnatomicalPoint b) {
  const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

struct AuditResult {
  float maximum_rest_edge_m{};
  float maximum_stretch{1.0f};
  std::uint64_t mixed_region_triangles{};
  std::uint64_t edges_over_110{};
  std::uint64_t edges_over_125{};
  std::uint64_t edges_over_150{};
};

AuditResult audit_profile(const RMeshV0& mesh,
                          const DerivedCharacterParameters& parameters,
                          const std::array<float, 3>& center,
                          float meters_per_unit) {
  AuditResult out{};
  const auto point = [&](std::uint32_t index) {
    const auto& p = mesh.vertices[index].position;
    return AnatomicalPoint{
        (p[0] - center[0]) * meters_per_unit,
        (p[1] - center[1]) * meters_per_unit,
        (p[2] - center[2]) * meters_per_unit};
  };

  for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
    const std::uint32_t tri[3] = {mesh.indices[i], mesh.indices[i + 1], mesh.indices[i + 2]};
    const auto r0 = mesh.vertices[tri[0]].region_id;
    const auto r1 = mesh.vertices[tri[1]].region_id;
    const auto r2 = mesh.vertices[tri[2]].region_id;
    if (r0 != r1 || r1 != r2) ++out.mixed_region_triangles;

    for (int edge = 0; edge < 3; ++edge) {
      const auto a = point(tri[edge]);
      const auto b = point(tri[(edge + 1) % 3]);
      const float rest = distance(a, b);
      if (!(rest > 1.0e-5f) || !std::isfinite(rest)) continue;
      out.maximum_rest_edge_m = std::max(out.maximum_rest_edge_m, rest);
      const auto da = deform_anatomy_rest(a, parameters);
      const auto db = deform_anatomy_rest(b, parameters);
      const float stretch = distance(da, db) / rest;
      if (!std::isfinite(stretch)) {
        out.maximum_stretch = std::numeric_limits<float>::infinity();
        continue;
      }
      out.maximum_stretch = std::max(out.maximum_stretch, stretch);
      if (stretch > 1.10f) ++out.edges_over_110;
      if (stretch > 1.25f) ++out.edges_over_125;
      if (stretch > 1.50f) ++out.edges_over_150;
    }
  }
  return out;
}

} // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: lsg_mesh_audit <human.rmesh>\n";
    return 2;
  }

  const auto bytes = read_file(argv[1]);
  if (bytes.empty()) {
    std::cerr << "failed to read RMS0\n";
    return 3;
  }

  RMeshV0 mesh{};
  std::string error;
  if (!decode_rmesh(bytes, mesh, error)) {
    std::cerr << "RMS0 decode failed: " << error << '\n';
    return 4;
  }

  std::array<float, 3> minimum = mesh.vertices.front().position;
  std::array<float, 3> maximum = mesh.vertices.front().position;
  for (const auto& vertex : mesh.vertices) {
    for (std::size_t c = 0; c < 3; ++c) {
      minimum[c] = std::min(minimum[c], vertex.position[c]);
      maximum[c] = std::max(maximum[c], vertex.position[c]);
    }
  }
  const float source_height = maximum[1] - minimum[1];
  if (!(source_height > 1.0e-6f)) {
    std::cerr << "RMS0 has invalid height\n";
    return 5;
  }
  std::array<float, 3> center{};
  for (std::size_t c = 0; c < 3; ++c) center[c] = 0.5f * (minimum[c] + maximum[c]);
  const float meters_per_unit = 1.75f / source_height;

  float overall_max_stretch = 1.0f;
  std::uint64_t overall_edges_over_150 = 0;
  for (std::uint32_t profile = 0; profile < 2; ++profile) {
    const auto parameters = derive_character_parameters(builtin_profile(profile));
    const auto result = audit_profile(mesh, parameters, center, meters_per_unit);
    overall_max_stretch = std::max(overall_max_stretch, result.maximum_stretch);
    overall_edges_over_150 += result.edges_over_150;
    std::cout << "profile=" << profile
              << " triangles=" << mesh.indices.size() / 3u
              << " mixed_region_triangles=" << result.mixed_region_triangles
              << " max_rest_edge_m=" << result.maximum_rest_edge_m
              << " max_deformation_stretch=" << result.maximum_stretch
              << " edges_gt_1.10=" << result.edges_over_110
              << " edges_gt_1.25=" << result.edges_over_125
              << " edges_gt_1.50=" << result.edges_over_150 << '\n';
  }

  if (!std::isfinite(overall_max_stretch) || overall_max_stretch > 1.50f || overall_edges_over_150 != 0u) {
    std::cerr << "LSG MESH CONTINUITY FAIL: catastrophic deformation stretch detected\n";
    return 6;
  }

  std::cout << "LSG MESH CONTINUITY PASS: max_stretch=" << overall_max_stretch
            << " threshold=1.50\n";
  return 0;
}
