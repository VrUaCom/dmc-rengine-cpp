#include "dmc_rengine/formats/dmc3_ptx_envelope.hpp"

#include <limits>

namespace dmc::rengine::formats {
namespace {

[[nodiscard]] std::uint32_t read_u32_le(std::span<const std::byte> bytes,
                                        const std::size_t offset) noexcept {
  return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset])) |
         (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 1])) << 8U) |
         (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 2])) << 16U) |
         (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 3])) << 24U);
}

[[nodiscard]] Dmc3PtxEnvelopeParseResult fail(std::string error) {
  Dmc3PtxEnvelopeParseResult result;
  result.error = std::move(error);
  return result;
}

}  // namespace

Dmc3PtxEnvelopeParseResult Dmc3PtxEnvelopeParser::parse(const std::span<const std::byte> bytes) {
  if (bytes.size() < kHeaderSize) {
    return fail("ptx_envelope_header_truncated");
  }

  const auto texture_count = read_u32_le(bytes, 0);
  constexpr auto kCountWidth = sizeof(std::uint32_t);
  constexpr auto kHeaderPayloadBytes = kHeaderSize - kCountTableOffset;
  constexpr auto kMaxHeaderCount = kHeaderPayloadBytes / kCountWidth;
  if (texture_count > kMaxHeaderCount) {
    return fail("ptx_envelope_count_table_overflow");
  }

  Dmc3PtxEnvelope envelope;
  envelope.texture_count = texture_count;
  envelope.count_table_offset = kCountTableOffset;
  envelope.count_table_size = static_cast<std::size_t>(texture_count) * kCountWidth;
  envelope.opaque_header_offset = kCountTableOffset + envelope.count_table_size;
  envelope.opaque_header_size = kHeaderSize - envelope.opaque_header_offset;
  envelope.block_counts.reserve(texture_count);
  envelope.entries.reserve(texture_count);

  std::size_t entry_offset = kHeaderSize;
  for (std::uint32_t index = 0; index < texture_count; ++index) {
    const auto block_table_offset = kCountTableOffset + static_cast<std::size_t>(index) * kCountWidth;
    const auto block_count = read_u32_le(bytes, block_table_offset);
    envelope.block_counts.push_back(block_count);

    // The recovered runtime advances by blockCount << 11. A zero block count would make
    // physical entry ownership non-progressing, so the bounded materialization parser
    // rejects it until a real retail counterexample proves alias semantics.
    if (block_count == 0) {
      return fail("ptx_envelope_block_count_zero");
    }

    if (block_count > std::numeric_limits<std::size_t>::max() / kSectorSize) {
      return fail("ptx_envelope_span_overflow");
    }
    const auto span_size = static_cast<std::size_t>(block_count) * kSectorSize;
    if (entry_offset > bytes.size() || span_size > bytes.size() - entry_offset) {
      return fail("ptx_envelope_entry_out_of_bounds");
    }
    if (span_size < 0x0C) {
      return fail("ptx_envelope_entry_too_small");
    }

    if (read_u32_le(bytes, entry_offset) != kTim2Magic) {
      return fail("ptx_envelope_entry_magic_mismatch");
    }

    const auto tim2_data_offset = read_u32_le(bytes, entry_offset + 0x08);
    if (tim2_data_offset >= span_size) {
      return fail("ptx_envelope_tim2_data_offset_out_of_bounds");
    }

    envelope.entries.push_back(Dmc3PtxEnvelopeEntry{
        index,
        block_count,
        entry_offset,
        span_size,
        tim2_data_offset,
    });

    entry_offset += span_size;
  }

  envelope.consumed_size = entry_offset;
  envelope.trailing_size = bytes.size() - entry_offset;

  Dmc3PtxEnvelopeParseResult result;
  result.ok = true;
  result.envelope = std::move(envelope);
  return result;
}

}  // namespace dmc::rengine::formats
