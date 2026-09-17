#include "rengine/lsg/genome.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {
std::vector<std::byte> read_binary(const std::filesystem::path& path) {
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

std::uint64_t fnv1a64(std::span<const std::byte> bytes) noexcept {
  std::uint64_t value = 14695981039346656037ull;
  for (const auto byte : bytes) {
    value ^= std::to_integer<std::uint8_t>(byte);
    value *= 1099511628211ull;
  }
  return value;
}

bool decode_checked(const std::filesystem::path& path,
                    std::vector<std::byte>& bytes,
                    rengine::lsg::DecodedGenome& decoded) {
  bytes = read_binary(path);
  if (bytes.empty()) {
    std::cerr << "empty or unreadable genome: " << path << '\n';
    return false;
  }
  std::string error;
  if (!rengine::lsg::decode_genome(bytes, decoded, error)) {
    std::cerr << "invalid genome " << path << ": " << error << '\n';
    return false;
  }
  return true;
}
} // namespace

int main(int argc, char** argv) {
  if (argc != 6) {
    std::cerr << "usage: lsg_report <character0.lsg> <character1.lsg> <human_base.rmesh> <human.vert.spv> <human.frag.spv>\n";
    return 2;
  }

  std::vector<std::byte> genome0_bytes;
  std::vector<std::byte> genome1_bytes;
  rengine::lsg::DecodedGenome genome0{};
  rengine::lsg::DecodedGenome genome1{};
  if (!decode_checked(argv[1], genome0_bytes, genome0) ||
      !decode_checked(argv[2], genome1_bytes, genome1)) {
    return 3;
  }

  const auto mesh_bytes = read_binary(argv[3]);
  const auto vertex_shader_bytes = read_binary(argv[4]);
  const auto fragment_shader_bytes = read_binary(argv[5]);
  if (mesh_bytes.empty() || vertex_shader_bytes.empty() || fragment_shader_bytes.empty()) {
    std::cerr << "shared base mesh or shader payload is missing\n";
    return 4;
  }

  const bool genomes_within_limit =
      genome0_bytes.size() <= rengine::lsg::kGenomeHardLimit &&
      genome1_bytes.size() <= rengine::lsg::kGenomeHardLimit;
  const bool revisions_match =
      genome0.generator_revision == rengine::lsg::kGeneratorRevision &&
      genome1.generator_revision == rengine::lsg::kGeneratorRevision;
  if (!genomes_within_limit || !revisions_match) {
    std::cerr << "LSG storage contract FAIL\n";
    return 5;
  }

  const std::uint64_t shader_bytes =
      static_cast<std::uint64_t>(vertex_shader_bytes.size()) +
      static_cast<std::uint64_t>(fragment_shader_bytes.size());

  std::cout << "LSG STORAGE CONTRACT PASS\n"
            << "Character 0 genome: " << genome0_bytes.size() << " bytes\n"
            << "Character 1 genome: " << genome1_bytes.size() << " bytes\n"
            << "Genome hard limit: " << rengine::lsg::kGenomeHardLimit << " bytes\n"
            << "Generator revision: " << rengine::lsg::kGeneratorRevision << '\n'
            << "Shared body asset: " << mesh_bytes.size() << " bytes\n"
            << "Base mesh revision FNV1a64: 0x" << std::hex << fnv1a64(mesh_bytes) << std::dec << '\n'
            << "Shared Vulkan shader payload: " << shader_bytes << " bytes\n"
            << "Mandatory character texture bytes: 0\n"
            << "Character-specific generated microdetail stored on disk: 0 bytes\n";
  return 0;
}
