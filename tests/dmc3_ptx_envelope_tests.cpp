#include "dmc_rengine/formats/dmc3_ptx_envelope.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void put_u16(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint16_t value) {
  bytes[offset] = static_cast<std::byte>(value & 0xFFU);
  bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
  bytes[offset] = static_cast<std::byte>(value & 0xFFU);
  bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
  bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
  bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

void put_tm2_dds_entry(std::vector<std::byte>& bytes,
                       const std::size_t entry_offset,
                       const std::size_t span_size,
                       const std::size_t index) {
  namespace formats = dmc::rengine::formats;

  constexpr std::uint32_t kDdsRelativeOffset = 0x80U;
  assert(span_size > kDdsRelativeOffset);
  const auto dds_size = span_size - static_cast<std::size_t>(kDdsRelativeOffset);
  assert(dds_size <= static_cast<std::size_t>(UINT32_MAX));

  put_u32(bytes, entry_offset, formats::Dmc3PtxEnvelopeParser::kTim2Magic);
  put_u32(bytes,
          entry_offset + formats::Dmc3PtxEnvelopeParser::kTm2DdsRelativeOffsetField,
          kDdsRelativeOffset);
  put_u32(bytes,
          entry_offset + formats::Dmc3PtxEnvelopeParser::kTm2DdsByteSizeField,
          static_cast<std::uint32_t>(dds_size));

  for (std::size_t metadata_index = 0;
       metadata_index < formats::Dmc3PtxEnvelopeParser::kTm2RuntimeTextureMetadataSize;
       ++metadata_index) {
    bytes[entry_offset + formats::Dmc3PtxEnvelopeParser::kTm2RuntimeTextureMetadataOffset +
          metadata_index] = static_cast<std::byte>(0xA0U + metadata_index);
  }

  put_u16(bytes,
          entry_offset + formats::Dmc3PtxEnvelopeParser::kTm2WidthField,
          static_cast<std::uint16_t>(64U + index));
  put_u16(bytes,
          entry_offset + formats::Dmc3PtxEnvelopeParser::kTm2HeightField,
          static_cast<std::uint16_t>(32U + index));

  const auto dds_offset = entry_offset + static_cast<std::size_t>(kDdsRelativeOffset);
  put_u32(bytes, dds_offset, formats::Dmc3PtxEnvelopeParser::kDdsMagic);
  put_u32(bytes, dds_offset + 0x04U, formats::Dmc3PtxEnvelopeParser::kDdsHeaderSize);
  put_u32(bytes,
          dds_offset + formats::Dmc3PtxEnvelopeParser::kDdsPixelFormatSizeField,
          formats::Dmc3PtxEnvelopeParser::kDdsPixelFormatSize);
}

std::vector<std::byte> make_ptx(const std::vector<std::uint32_t>& block_counts,
                                const std::size_t trailing_bytes = 0U) {
  namespace formats = dmc::rengine::formats;

  std::size_t total_size = formats::Dmc3PtxEnvelopeParser::kHeaderSize + trailing_bytes;
  for (const auto block_count : block_counts) {
    total_size += static_cast<std::size_t>(block_count) * formats::Dmc3PtxEnvelopeParser::kSectorSize;
  }

  std::vector<std::byte> bytes(total_size, std::byte{0});
  put_u32(bytes, 0U, static_cast<std::uint32_t>(block_counts.size()));

  // Keep the unclaimed header tail visibly non-zero: the parser must preserve
  // it as opaque structure rather than inventing a zero-padding requirement.
  if (block_counts.size() < 0x1F0U) {
    bytes[0x7F0U] = std::byte{0x5A};
  }

  std::size_t entry_offset = formats::Dmc3PtxEnvelopeParser::kHeaderSize;
  for (std::size_t index = 0; index < block_counts.size(); ++index) {
    const auto block_count = block_counts[index];
    put_u32(bytes, formats::Dmc3PtxEnvelopeParser::kCountTableOffset + index * 4U, block_count);

    if (block_count == 0U) {
      // A terminal zero block count is represented by the remaining supplied
      // bytes. Non-final zero is intentionally left without separate storage;
      // the parser rejects it before needing the next entry.
      if (index + 1U == block_counts.size() && trailing_bytes >= 0x100U) {
        put_tm2_dds_entry(bytes, entry_offset, trailing_bytes, index);
      }
      continue;
    }

    const auto span_size =
        static_cast<std::size_t>(block_count) * formats::Dmc3PtxEnvelopeParser::kSectorSize;
    put_tm2_dds_entry(bytes, entry_offset, span_size, index);
    entry_offset += span_size;
  }
  return bytes;
}

}  // namespace

int main() {
  namespace formats = dmc::rengine::formats;

  {
    const auto bytes = make_ptx({1U, 2U}, 37U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bytes);
    assert(result.ok);
    assert(result.error.empty());
    assert(result.envelope.texture_count == 2U);
    assert(result.envelope.block_counts.size() == 2U);
    assert(result.envelope.block_counts[0] == 1U);
    assert(result.envelope.block_counts[1] == 2U);
    assert(result.envelope.count_table_offset == 0x04U);
    assert(result.envelope.count_table_size == 8U);
    assert(result.envelope.opaque_header_offset == 0x0CU);
    assert(result.envelope.opaque_header_size == 0x7F4U);
    assert(result.envelope.entries.size() == 2U);

    const auto& first = result.envelope.entries[0];
    assert(first.offset == 0x800U);
    assert(first.span_size == 0x800U);
    assert(first.texture.dds_relative_offset == 0x80U);
    assert(first.texture.dds_byte_size == 0x780U);
    assert(first.texture.dds_offset == 0x880U);
    assert(first.texture.width == 64U);
    assert(first.texture.height == 32U);
    assert(first.texture.has_embedded_dds);
    assert(first.texture.runtime_texture_metadata[0] == std::byte{0xA0});
    assert(!first.terminal_span_to_eof);

    const auto& second = result.envelope.entries[1];
    assert(second.offset == 0x1000U);
    assert(second.span_size == 0x1000U);
    assert(second.texture.dds_relative_offset == 0x80U);
    assert(second.texture.dds_byte_size == 0xF80U);
    assert(second.texture.dds_offset == 0x1080U);
    assert(second.texture.width == 65U);
    assert(second.texture.height == 33U);
    assert(!second.terminal_span_to_eof);
    assert(result.envelope.consumed_size == 0x2000U);
    assert(result.envelope.trailing_size == 37U);
  }

  {
    const auto compact_final = make_ptx({1U, 0U}, 0x180U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(compact_final);
    assert(result.ok);
    assert(result.envelope.entries.size() == 2U);
    assert(result.envelope.entries[0].offset == 0x800U);
    assert(result.envelope.entries[1].offset == 0x1000U);
    assert(result.envelope.entries[1].block_count == 0U);
    assert(result.envelope.entries[1].span_size == 0x180U);
    assert(result.envelope.entries[1].texture.dds_byte_size == 0x100U);
    assert(result.envelope.entries[1].terminal_span_to_eof);
    assert(result.envelope.trailing_size == 0U);
    assert(result.envelope.consumed_size == compact_final.size());
  }

  {
    auto no_cpu_payload = make_ptx({1U});
    put_u32(no_cpu_payload,
            0x800U + formats::Dmc3PtxEnvelopeParser::kTm2DdsByteSizeField,
            0U);
    no_cpu_payload[0x880U] = std::byte{'X'};
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(no_cpu_payload);
    assert(result.ok);
    assert(!result.envelope.entries[0].texture.has_embedded_dds);
    assert(result.envelope.entries[0].texture.dds_byte_size == 0U);
  }

  {
    const auto empty = make_ptx({});
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(empty);
    assert(result.ok);
    assert(result.envelope.texture_count == 0U);
    assert(result.envelope.entries.empty());
    assert(result.envelope.consumed_size == 0x800U);
  }

  {
    std::vector<std::byte> truncated(0x7FFU, std::byte{0});
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(truncated);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_header_truncated");
  }

  {
    auto too_many = make_ptx({});
    put_u32(too_many, 0U, 512U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(too_many);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_count_table_overflow");
  }

  {
    auto non_progressing = make_ptx({0U, 1U});
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(non_progressing);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_nonterminal_block_count_zero");
  }

  {
    auto bad_magic = make_ptx({1U});
    bad_magic[0x800U] = std::byte{'X'};
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_magic);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_entry_magic_mismatch");
  }

  {
    auto truncated_entry = make_ptx({2U});
    truncated_entry.resize(truncated_entry.size() - 1U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(truncated_entry);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_entry_out_of_bounds");
  }

  {
    auto bad_dds_offset = make_ptx({1U});
    put_u32(bad_dds_offset,
            0x800U + formats::Dmc3PtxEnvelopeParser::kTm2DdsRelativeOffsetField,
            0x800U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_dds_offset);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_tm2_dds_offset_out_of_bounds");
  }

  {
    auto bad_dds_range = make_ptx({1U});
    put_u32(bad_dds_range,
            0x800U + formats::Dmc3PtxEnvelopeParser::kTm2DdsByteSizeField,
            0x781U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_dds_range);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_dds_range_out_of_bounds");
  }

  {
    auto short_dds = make_ptx({1U});
    put_u32(short_dds,
            0x800U + formats::Dmc3PtxEnvelopeParser::kTm2DdsByteSizeField,
            0x7FU);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(short_dds);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_dds_buffer_too_small");
  }

  {
    auto bad_dds_magic = make_ptx({1U});
    bad_dds_magic[0x880U] = std::byte{'X'};
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_dds_magic);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_dds_magic_mismatch");
  }

  {
    auto bad_dds_header = make_ptx({1U});
    put_u32(bad_dds_header, 0x884U, 0x7BU);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_dds_header);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_dds_header_size_mismatch");
  }

  {
    auto bad_pixel_format = make_ptx({1U});
    put_u32(bad_pixel_format,
            0x880U + formats::Dmc3PtxEnvelopeParser::kDdsPixelFormatSizeField,
            0x1FU);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_pixel_format);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_dds_pixel_format_size_mismatch");
  }

  return 0;
}
