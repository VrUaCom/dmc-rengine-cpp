#include "dmc_rengine/formats/dmc3_ptx_envelope.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void put_u32(std::vector<std::byte>& bytes, const std::size_t offset, const std::uint32_t value) {
  bytes[offset] = static_cast<std::byte>(value & 0xFFU);
  bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
  bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
  bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
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
      if (index + 1U == block_counts.size() && trailing_bytes >= 0x21U) {
        put_u32(bytes, entry_offset, formats::Dmc3PtxEnvelopeParser::kTim2Magic);
        put_u32(bytes, entry_offset + 0x08U, 0x20U);
      }
      continue;
    }

    put_u32(bytes, entry_offset, formats::Dmc3PtxEnvelopeParser::kTim2Magic);
    put_u32(bytes, entry_offset + 0x08U, 0x20U);
    entry_offset += static_cast<std::size_t>(block_count) * formats::Dmc3PtxEnvelopeParser::kSectorSize;
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
    assert(result.envelope.entries[0].offset == 0x800U);
    assert(result.envelope.entries[0].span_size == 0x800U);
    assert(result.envelope.entries[0].tim2_data_offset == 0x20U);
    assert(!result.envelope.entries[0].terminal_span_to_eof);
    assert(result.envelope.entries[1].offset == 0x1000U);
    assert(result.envelope.entries[1].span_size == 0x1000U);
    assert(!result.envelope.entries[1].terminal_span_to_eof);
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
    assert(result.envelope.entries[1].terminal_span_to_eof);
    assert(result.envelope.trailing_size == 0U);
    assert(result.envelope.consumed_size == compact_final.size());
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
    auto bad_data_offset = make_ptx({1U});
    put_u32(bad_data_offset, 0x808U, 0x800U);
    const auto result = formats::Dmc3PtxEnvelopeParser::parse(bad_data_offset);
    assert(!result.ok);
    assert(result.error == "ptx_envelope_tim2_data_offset_out_of_bounds");
  }

  return 0;
}
