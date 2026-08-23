#include "dmc_rengine/formats/dmc3_ptx_envelope.hpp"

#include <limits>
#include <utility>

namespace dmc::rengine::formats {
namespace {

[[nodiscard]] std::uint16_t read_u16_le(std::span<const std::byte> bytes,
                                        const std::size_t offset) noexcept {
  const auto low = static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset]));
  const auto high = static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(bytes[offset + 1]));
  return static_cast<std::uint16_t>(low | static_cast<std::uint16_t>(high << 8U));
}

[[nodiscard]] std::uint32_t read_u32_le(std::span<const std::byte> bytes,
                                        const std::size_t offset) noexcept {
  return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset])) |
         (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 1])) << 8U) |
         (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 2])) << 16U) |
         (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(bytes[offset + 3])) << 24U);
}

[[nodiscard]] std::uint64_t read_u64_le(std::span<const std::byte> bytes,
                                        const std::size_t offset) noexcept {
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < sizeof(std::uint64_t); ++index) {
    value |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(bytes[offset + index]))
             << (index * 8U);
  }
  return value;
}

[[nodiscard]] Dmc3PtxEnvelopeParseResult fail(std::string error) {
  Dmc3PtxEnvelopeParseResult result;
  result.error = std::move(error);
  return result;
}

[[nodiscard]] bool resolve_serialized_delta(const std::size_t field_offset,
                                            const std::uint64_t raw_delta,
                                            const std::size_t entry_offset,
                                            const std::size_t span_size,
                                            std::size_t& resolved_offset) noexcept {
  if (raw_delta > std::numeric_limits<std::size_t>::max()) {
    return false;
  }
  const auto delta = static_cast<std::size_t>(raw_delta);
  if (field_offset > std::numeric_limits<std::size_t>::max() - delta) {
    return false;
  }
  resolved_offset = field_offset + delta;
  if (resolved_offset < entry_offset) {
    return false;
  }
  return resolved_offset - entry_offset < span_size;
}

[[nodiscard]] std::string validate_embedded_dds(const std::span<const std::byte> bytes,
                                                const std::size_t dds_offset,
                                                const std::size_t dds_byte_size,
                                                const std::size_t entry_offset,
                                                const std::size_t span_size) {
  if (dds_offset < entry_offset || dds_offset - entry_offset >= span_size) {
    return "ptx_envelope_dds_offset_out_of_bounds";
  }
  const auto dds_relative = dds_offset - entry_offset;
  if (dds_byte_size > span_size - dds_relative) {
    return "ptx_envelope_dds_range_out_of_bounds";
  }
  if (dds_byte_size < Dmc3PtxEnvelopeParser::kDdsMinimumBytes) {
    return "ptx_envelope_dds_buffer_too_small";
  }
  if (read_u32_le(bytes, dds_offset) != Dmc3PtxEnvelopeParser::kDdsMagic) {
    return "ptx_envelope_dds_magic_mismatch";
  }
  if (read_u32_le(bytes, dds_offset + 0x04U) != Dmc3PtxEnvelopeParser::kDdsHeaderSize) {
    return "ptx_envelope_dds_header_size_mismatch";
  }
  if (read_u32_le(bytes, dds_offset + Dmc3PtxEnvelopeParser::kDdsPixelFormatSizeField) !=
      Dmc3PtxEnvelopeParser::kDdsPixelFormatSize) {
    return "ptx_envelope_dds_pixel_format_size_mismatch";
  }
  return {};
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

    const bool is_final_entry = index + 1U == texture_count;
    bool terminal_span_to_eof = false;
    std::size_t span_size = 0;

    if (block_count == 0U) {
      // The recovered EXE reads blockCount only after parsing the current entry.
      // Therefore a zero final block count is a valid non-advancing terminal shape,
      // while a non-final zero would make the next entry unlocatable. Bound the final
      // shape to the supplied resource EOF and keep non-final zero fail-closed.
      if (!is_final_entry) {
        return fail("ptx_envelope_nonterminal_block_count_zero");
      }
      if (entry_offset > bytes.size()) {
        return fail("ptx_envelope_entry_out_of_bounds");
      }
      span_size = bytes.size() - entry_offset;
      terminal_span_to_eof = true;
    } else {
      if (block_count > std::numeric_limits<std::size_t>::max() / kSectorSize) {
        return fail("ptx_envelope_span_overflow");
      }
      span_size = static_cast<std::size_t>(block_count) * kSectorSize;
      if (entry_offset > bytes.size() || span_size > bytes.size() - entry_offset) {
        return fail("ptx_envelope_entry_out_of_bounds");
      }
    }

    if (span_size < sizeof(std::uint64_t)) {
      return fail("ptx_envelope_entry_metadata_truncated");
    }

    Dmc3PtxEnvelopeEntry entry;
    entry.index = index;
    entry.block_count = block_count;
    entry.offset = entry_offset;
    entry.span_size = span_size;
    entry.terminal_span_to_eof = terminal_span_to_eof;

    if (read_u32_le(bytes, entry_offset) == kTim2Magic) {
      constexpr auto kRequiredTm2MetadataBytes =
          kTm2RuntimeTextureMetadataOffset + kTm2RuntimeTextureMetadataSize;
      if (span_size < kRequiredTm2MetadataBytes) {
        return fail("ptx_envelope_entry_metadata_truncated");
      }

      entry.representation = Dmc3PtxEntryRepresentation::tim2;
      auto& texture = entry.texture;
      texture.dds_relative_offset = read_u32_le(bytes, entry_offset + kTm2DdsRelativeOffsetField);
      texture.dds_byte_size = read_u32_le(bytes, entry_offset + kTm2DdsByteSizeField);
      texture.width = read_u16_le(bytes, entry_offset + kTm2WidthField);
      texture.height = read_u16_le(bytes, entry_offset + kTm2HeightField);

      const auto dds_relative_offset = static_cast<std::size_t>(texture.dds_relative_offset);
      if (dds_relative_offset >= span_size) {
        return fail("ptx_envelope_tm2_dds_offset_out_of_bounds");
      }
      texture.dds_offset = entry_offset + dds_relative_offset;

      for (std::size_t metadata_index = 0;
           metadata_index < texture.runtime_texture_metadata.size();
           ++metadata_index) {
        texture.runtime_texture_metadata[metadata_index] =
            bytes[entry_offset + kTm2RuntimeTextureMetadataOffset + metadata_index];
      }

      texture.has_embedded_dds = texture.dds_byte_size != 0U;
      if (texture.has_embedded_dds) {
        const auto error = validate_embedded_dds(bytes,
                                                 texture.dds_offset,
                                                 texture.dds_byte_size,
                                                 entry_offset,
                                                 span_size);
        if (!error.empty()) {
          return fail(error);
        }
      }
    } else {
      // Canonical dmc3.exe 0x1403365B0 dispatches non-TM2 entries to
      // 0x140046510. That helper materializes a serialized gfxTexture object
      // in place. Source storage uses a zero vtable qword; unknown non-TM2
      // shapes remain fail-closed instead of being accepted as generic bytes.
      const auto vtable_placeholder =
          read_u64_le(bytes, entry_offset + kSerializedGfxVtablePlaceholderField);
      if (vtable_placeholder != 0U) {
        return fail("ptx_envelope_entry_unknown_non_tm2_representation");
      }

      constexpr auto kRequiredSerializedPointerBytes =
          kSerializedGfxDescriptorPointerField + sizeof(std::uint64_t);
      if (span_size < kRequiredSerializedPointerBytes) {
        return fail("ptx_envelope_serialized_gfx_metadata_truncated");
      }

      entry.representation = Dmc3PtxEntryRepresentation::serialized_gfx_texture_dds;
      auto& texture = entry.serialized_texture;
      texture.vtable_placeholder = vtable_placeholder;
      texture.width = read_u16_le(bytes, entry_offset + kSerializedGfxWidthField);
      texture.height = read_u16_le(bytes, entry_offset + kSerializedGfxHeightField);

      const auto descriptor_pointer_field = entry_offset + kSerializedGfxDescriptorPointerField;
      texture.descriptor_relative_delta = read_u64_le(bytes, descriptor_pointer_field);
      if (!resolve_serialized_delta(descriptor_pointer_field,
                                    texture.descriptor_relative_delta,
                                    entry_offset,
                                    span_size,
                                    texture.descriptor_offset)) {
        return fail("ptx_envelope_serialized_gfx_descriptor_offset_out_of_bounds");
      }

      const auto descriptor_relative = texture.descriptor_offset - entry_offset;
      if (kSerializedGfxDescriptorMinimumBytes > span_size - descriptor_relative) {
        return fail("ptx_envelope_serialized_gfx_descriptor_truncated");
      }

      texture.dds_byte_size = read_u32_le(
          bytes, texture.descriptor_offset + kSerializedGfxDescriptorDdsByteSizeField);
      const auto dds_pointer_field =
          texture.descriptor_offset + kSerializedGfxDescriptorDdsPointerField;
      texture.dds_relative_delta = read_u64_le(bytes, dds_pointer_field);
      if (!resolve_serialized_delta(dds_pointer_field,
                                    texture.dds_relative_delta,
                                    entry_offset,
                                    span_size,
                                    texture.dds_offset)) {
        return fail("ptx_envelope_serialized_gfx_dds_offset_out_of_bounds");
      }

      texture.has_embedded_dds = texture.dds_byte_size != 0U;
      if (texture.has_embedded_dds) {
        const auto error = validate_embedded_dds(bytes,
                                                 texture.dds_offset,
                                                 texture.dds_byte_size,
                                                 entry_offset,
                                                 span_size);
        if (!error.empty()) {
          return fail(error);
        }
      }
    }

    envelope.entries.push_back(entry);
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
