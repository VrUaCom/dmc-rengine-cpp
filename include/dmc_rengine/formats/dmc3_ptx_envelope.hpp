#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats {

struct Dmc3PtxEnvelopeEntry {
  std::uint32_t index{0};
  std::uint32_t block_count{0};
  std::size_t offset{0};
  std::size_t span_size{0};
  std::uint32_t tim2_data_offset{0};
  bool terminal_span_to_eof{false};
};

struct Dmc3PtxEnvelope {
  std::uint32_t texture_count{0};
  std::vector<std::uint32_t> block_counts;
  std::vector<Dmc3PtxEnvelopeEntry> entries;

  std::size_t count_table_offset{0};
  std::size_t count_table_size{0};
  std::size_t opaque_header_offset{0};
  std::size_t opaque_header_size{0};
  std::size_t consumed_size{0};
  std::size_t trailing_size{0};
};

struct Dmc3PtxEnvelopeParseResult {
  bool ok{false};
  std::string error;
  Dmc3PtxEnvelope envelope;
};

class Dmc3PtxEnvelopeParser final {
 public:
  static constexpr std::size_t kHeaderSize = 0x800;
  static constexpr std::size_t kSectorSize = 0x800;
  static constexpr std::size_t kCountTableOffset = 0x04;
  static constexpr std::uint32_t kTim2Magic = 0x00324D54U;  // "TM2\0", little-endian.

  [[nodiscard]] static Dmc3PtxEnvelopeParseResult parse(std::span<const std::byte> bytes);
};

}  // namespace dmc::rengine::formats
