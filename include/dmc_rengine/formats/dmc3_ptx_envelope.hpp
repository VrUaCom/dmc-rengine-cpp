#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats {

enum class Dmc3PtxEntryRepresentation : std::uint8_t {
  tim2,
  serialized_gfx_texture_dds,
};

struct Dmc3Tm2DdsBridge {
  std::uint32_t dds_relative_offset{0};
  std::uint32_t dds_byte_size{0};
  std::size_t dds_offset{0};
  std::uint16_t width{0};
  std::uint16_t height{0};
  std::array<std::byte, 0x18> runtime_texture_metadata{};
  bool has_embedded_dds{false};
};

struct Dmc3SerializedGfxTextureDdsBridge {
  std::uint64_t vtable_placeholder{0};
  std::uint64_t descriptor_relative_delta{0};
  std::size_t descriptor_offset{0};
  std::uint32_t dds_byte_size{0};
  std::uint64_t dds_relative_delta{0};
  std::size_t dds_offset{0};
  std::uint16_t width{0};
  std::uint16_t height{0};
  bool has_embedded_dds{false};
};

struct Dmc3PtxEnvelopeEntry {
  std::uint32_t index{0};
  std::uint32_t block_count{0};
  std::size_t offset{0};
  std::size_t span_size{0};
  Dmc3PtxEntryRepresentation representation{Dmc3PtxEntryRepresentation::tim2};
  Dmc3Tm2DdsBridge texture;
  Dmc3SerializedGfxTextureDdsBridge serialized_texture;
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

  static constexpr std::size_t kTm2DdsRelativeOffsetField = 0x08;
  static constexpr std::size_t kTm2DdsByteSizeField = 0x3C;
  static constexpr std::size_t kTm2RuntimeTextureMetadataOffset = 0x50;
  static constexpr std::size_t kTm2RuntimeTextureMetadataSize = 0x18;
  static constexpr std::size_t kTm2WidthField = 0x58;
  static constexpr std::size_t kTm2HeightField = 0x5A;

  // Canonical EXE 0x140046510 materializes the non-TM2 representation in place:
  // it writes the gfxTexture vtable over a zero qword at +0x00, then resolves the
  // qword fields at +0x20 and descriptor+0x08 as address(field) + serialized delta.
  static constexpr std::size_t kSerializedGfxVtablePlaceholderField = 0x00;
  static constexpr std::size_t kSerializedGfxWidthField = 0x10;
  static constexpr std::size_t kSerializedGfxHeightField = 0x12;
  static constexpr std::size_t kSerializedGfxDescriptorPointerField = 0x20;
  static constexpr std::size_t kSerializedGfxDescriptorDdsByteSizeField = 0x04;
  static constexpr std::size_t kSerializedGfxDescriptorDdsPointerField = 0x08;
  static constexpr std::size_t kSerializedGfxDescriptorMinimumBytes = 0x10;

  static constexpr std::uint32_t kTim2Magic = 0x00324D54U;  // "TM2\0", little-endian.
  static constexpr std::uint32_t kDdsMagic = 0x20534444U;   // "DDS ", little-endian.
  static constexpr std::uint32_t kDdsHeaderSize = 0x7CU;
  static constexpr std::uint32_t kDdsPixelFormatSize = 0x20U;
  static constexpr std::size_t kDdsMinimumBytes = 0x80;
  static constexpr std::size_t kDdsPixelFormatSizeField = 0x4C;

  [[nodiscard]] static Dmc3PtxEnvelopeParseResult parse(std::span<const std::byte> bytes);
};

}  // namespace dmc::rengine::formats
