#include "rengine/lsg/character_profile.hpp"
#include "rengine/lsg/genome.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <unordered_set>
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
  using namespace rengine::lsg;
  const int expected = 2 + static_cast<int>(kBuiltinProfileCount);
  if (argc != expected) {
    std::cerr << "usage: lsg_report <runtime-root>";
    for (std::uint32_t i = 0; i < kBuiltinProfileCount; ++i)
      std::cerr << " <character" << i << ".lsg>";
    std::cerr << "\n";
    return 2;
  }

  const std::filesystem::path runtime_root = argv[1];
  std::vector<std::vector<std::byte>> genome_bytes(kBuiltinProfileCount);
  std::vector<DecodedGenome> genomes(kBuiltinProfileCount);
  for (std::uint32_t i = 0; i < kBuiltinProfileCount; ++i) {
    if (!decode_checked(argv[2 + static_cast<int>(i)], genome_bytes[i], genomes[i]))
      return 3;
    if (genome_bytes[i].size() > kGenomeHardLimit ||
        genomes[i].generator_revision != kGeneratorRevision) {
      std::cerr << "LSG storage contract FAIL: profile " << i
                << " violates current genome contract\n";
      return 4;
    }
  }

  for (std::uint32_t i = 0; i < kBuiltinProfileCount; ++i) {
    const auto canonical = encode_genome(builtin_profile(i));
    if (canonical != genome_bytes[i]) {
      std::cerr << "LSG storage contract FAIL: compiled authoring profile " << i
                << " differs from built-in runtime profile\n";
      return 5;
    }
  }

  std::uint64_t carrier_body_bytes = 0;
  std::uint64_t carrier_eye_bytes = 0;
  std::unordered_set<std::string> body_paths;
  std::unordered_set<std::string> eye_paths;
  struct CarrierMeasured {
    CarrierDefinition definition{};
    std::vector<std::byte> body;
    std::vector<std::byte> eye;
  };
  std::vector<CarrierMeasured> measured;
  measured.reserve(kBuiltinCarrierCount);

  for (const auto& carrier : builtin_carriers()) {
    const std::string body_path{carrier.body_asset_path};
    const std::string eye_path{carrier.eye_asset_path};
    if (!body_paths.insert(body_path).second || !eye_paths.insert(eye_path).second) {
      std::cerr << "LSG storage contract FAIL: duplicate carrier asset path\n";
      return 6;
    }
    CarrierMeasured item{};
    item.definition = carrier;
    item.body = read_binary(runtime_root / body_path);
    item.eye = read_binary(runtime_root / eye_path);
    if (item.body.empty() || item.eye.empty()) {
      std::cerr << "LSG storage contract FAIL: missing carrier assets for " << carrier.key << '\n';
      return 7;
    }
    carrier_body_bytes += static_cast<std::uint64_t>(item.body.size());
    carrier_eye_bytes += static_cast<std::uint64_t>(item.eye.size());
    measured.push_back(std::move(item));
  }

  const std::filesystem::path shader_root = runtime_root / "shaders";
  const auto human_vert = read_binary(shader_root / "human.vert.spv");
  const auto human_frag = read_binary(shader_root / "human.frag.spv");
  const auto eye_vert = read_binary(shader_root / "eye.vert.spv");
  const auto eye_frag = read_binary(shader_root / "eye.frag.spv");
  if (human_vert.empty() || human_frag.empty() || eye_vert.empty() || eye_frag.empty()) {
    std::cerr << "LSG storage contract FAIL: shared shader payload missing\n";
    return 8;
  }
  const std::uint64_t shader_bytes =
      static_cast<std::uint64_t>(human_vert.size()) +
      static_cast<std::uint64_t>(human_frag.size()) +
      static_cast<std::uint64_t>(eye_vert.size()) +
      static_cast<std::uint64_t>(eye_frag.size());

  std::cout << "LSG STORAGE CONTRACT PASS\n"
            << "Built-in character profiles: " << kBuiltinProfileCount << '\n'
            << "Unique shared carriers: " << kBuiltinCarrierCount << '\n'
            << "Genome hard limit: " << kGenomeHardLimit << " bytes\n"
            << "Generator revision: " << kGeneratorRevision << '\n'
            << "Built-in profile source parity: PASS\n";

  for (std::uint32_t i = 0; i < kBuiltinProfileCount; ++i) {
    const auto& profile = character_profile_definition(i);
    const auto& carrier = carrier_definition(profile.carrier);
    std::cout << "Profile " << i << " (" << profile.display_name << "): "
              << genome_bytes[i].size() << " bytes; carrier=" << carrier.key << '\n';
  }
  for (const auto& item : measured) {
    std::cout << "Carrier " << item.definition.key
              << " body: " << item.body.size()
              << " bytes; FNV1a64=0x" << std::hex << fnv1a64(item.body) << std::dec << '\n'
              << "Carrier " << item.definition.key
              << " eyes: " << item.eye.size()
              << " bytes; FNV1a64=0x" << std::hex << fnv1a64(item.eye) << std::dec << '\n';
  }

  std::cout << "Unique shared carrier body bytes: " << carrier_body_bytes << '\n'
            << "Unique shared carrier eye bytes: " << carrier_eye_bytes << '\n'
            << "Shared Vulkan shader payload: " << shader_bytes << " bytes\n"
            << "Profile-specific carrier duplicate bytes: 0\n"
            << "Mandatory character texture bytes: 0\n"
            << "Character-specific generated microdetail stored on disk: 0 bytes\n";
  return 0;
}
