#include "rengine/lsg/rmesh.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

bool read_file(const std::filesystem::path& path, std::vector<std::byte>& bytes) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) return false;
  const auto end = stream.tellg();
  if (end <= 0) return false;
  bytes.resize(static_cast<std::size_t>(end));
  stream.seekg(0, std::ios::beg);
  stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return static_cast<bool>(stream);
}

bool write_file(const std::filesystem::path& path, const std::vector<std::byte>& bytes) {
  std::ofstream stream(path, std::ios::binary);
  if (!stream) return false;
  stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return static_cast<bool>(stream);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: lsg_merge_debug_mesh <body.rmesh> <joints.rmesh> <output.rmesh>\n";
    return 2;
  }

  std::vector<std::byte> body_bytes, joint_bytes;
  if (!read_file(argv[1], body_bytes) || !read_file(argv[2], joint_bytes)) {
    std::cerr << "RMS0 debug merge failed: cannot read input mesh\n";
    return 3;
  }

  rengine::lsg::RMeshV0 body{}, joints{};
  std::string error;
  if (!rengine::lsg::decode_rmesh(body_bytes, body, error)) {
    std::cerr << "RMS0 debug merge failed: invalid body mesh: " << error << '\n';
    return 4;
  }
  if (!rengine::lsg::decode_rmesh(joint_bytes, joints, error)) {
    std::cerr << "RMS0 debug merge failed: invalid joint mesh: " << error << '\n';
    return 5;
  }

  if (body.vertices.size() + joints.vertices.size() > std::numeric_limits<std::uint32_t>::max() ||
      body.indices.size() + joints.indices.size() > std::numeric_limits<std::uint32_t>::max()) {
    std::cerr << "RMS0 debug merge failed: merged mesh exceeds v0 element limits\n";
    return 6;
  }

  const auto body_vertex_count = static_cast<std::uint32_t>(body.vertices.size());
  const auto body_index_count = body.indices.size();
  const auto joint_vertex_count = joints.vertices.size();
  const auto joint_index_count = joints.indices.size();

  for (auto& vertex : joints.vertices) {
    vertex.region_id = 255u;  // Reserved RMS0 diagnostic region: MakeHuman joint marker geometry.
  }

  body.vertices.insert(body.vertices.end(), joints.vertices.begin(), joints.vertices.end());
  body.indices.reserve(body.indices.size() + joints.indices.size());
  for (const auto index : joints.indices) {
    if (index > std::numeric_limits<std::uint32_t>::max() - body_vertex_count) {
      std::cerr << "RMS0 debug merge failed: index overflow\n";
      return 7;
    }
    body.indices.push_back(body_vertex_count + index);
  }
  body.flags |= joints.flags | rengine::lsg::rmesh_has_regions;
  body.joint_count = static_cast<std::uint16_t>(
      std::max<std::uint32_t>(body.joint_count, joints.joint_count));

  if (!rengine::lsg::validate_rmesh(body, error)) {
    std::cerr << "RMS0 debug merge failed validation: " << error << '\n';
    return 8;
  }
  const auto output = rengine::lsg::encode_rmesh(body);
  if (output.empty() || !write_file(argv[3], output)) {
    std::cerr << "RMS0 debug merge failed: cannot write output\n";
    return 9;
  }

  std::cout << "RMS0 DEBUG MERGE PASS: body_vertices=" << body_vertex_count
            << " body_triangles=" << body_index_count / 3u
            << " joint_vertices=" << joint_vertex_count
            << " joint_triangles=" << joint_index_count / 3u
            << " total_vertices=" << body.vertices.size()
            << " total_triangles=" << body.indices.size() / 3u
            << " bytes=" << output.size() << '\n';
  return 0;
}
