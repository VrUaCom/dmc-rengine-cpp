#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "dmc_rengine/codecs/dds_bcn.hpp"

// Texture re-encoding for DMC3 resources: every texture of a DDS, a PTX
// bundle, a single gfxTexture file (.tm2-named) or the PTX / DDS slots of a
// PAC is decoded level by level (the game's own mips are kept) and written
// again in another BC format.
//
// What changes, and only that:
//   DDS      header + payload (legacy FourCC, or DX10 when required/forced);
//   gfxTexture header (0x70):
//     +0x08  0x20000 | mips << 8 | low byte (0x86 / 0x88 by block size in the
//            canonical model family, kept in the single-level families),
//     +0x18  row bytes = logical width * block bytes / 4,
//     +0x38  DDS payload bytes,
//     +0x60  0 / 4 by block size in the canonical family, kept otherwise,
//     +0x64  DDS bytes;
//   PTX      sector spans (a texture keeps its span when it still fits);
//   PAC      offsets of the slots after a slot whose size changed (16-byte
//            aligned, as retail).
// Everything else (other descriptor fields, other slots, nested PACs) is
// copied byte for byte. Fields the PC loader does not read (dmc3.exe
// 0x140046510 / 0x140046AF0 read +0x10/+0x12, +0x20, +0x64, +0x68 and the DDS)
// are kept in their nearest retail class so retail readers still accept
// DXT1 / DXT5 results; see docs/research/dmc3-texture-formats-map-2026-10-01.md.
namespace dmc::rengine::profiles::dmc3 {

enum class ReencodeContainer : std::uint8_t {
    dds,
    wrapped_texture,  // one gfxTexture + DDS, no bundle header (.tm2 files)
    ptx,
    pac,
};

struct TextureReencodeOptions final {
    codecs::dds_bcn::Format format{codecs::dds_bcn::Format::bc7};
    // DX10 header even when the format has a legacy FourCC.
    bool force_dx10{};
    // PAC only: re-encode this slot; -1 = every PTX / DDS / wrapped slot.
    int pac_slot{-1};
    // PAC only: also descend into PAC slots that are PACs themselves (a
    // retail GData.afs holds character PACs, which hold the PTX), rebuilding
    // every level on the way out. Depth is bounded.
    bool nested{false};
};

struct ReencodedTexture final {
    int pac_slot{-1};
    // Enclosing PAC slots, outermost first, when found through `nested`.
    std::vector<std::uint32_t> pac_path;
    std::uint32_t index{};
    codecs::dds_bcn::Format from{};
    codecs::dds_bcn::Format to{};
    bool from_dx10{};
    bool to_dx10{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t mips{};
    std::uint64_t old_bytes{};
    std::uint64_t new_bytes{};
    // Level 0, re-encoded vs the decoded source, all four channels.
    double psnr_db{};
};

struct TextureReencodeResult final {
    bool ok{};
    ReencodeContainer container{};
    std::vector<std::byte> bytes;
    std::vector<ReencodedTexture> textures;
    std::string detail;
};

// Detects DDS / wrapped gfxTexture / PTX / PAC and re-encodes it.
[[nodiscard]] TextureReencodeResult reencode_textures(
    std::span<const std::byte> source, const TextureReencodeOptions& options);

// Building blocks of reencode_textures, for workflows that run the steps
// separately (spider::tarantula texture re-encode).
//
// The texture payload of one PAC slot extent (trailing alignment zeros
// dropped), or an empty span when the slot holds no texture.
[[nodiscard]] std::span<const std::byte> texture_payload(std::span<const std::byte> slot_extent) noexcept;

struct PayloadReencodeResult final {
    bool ok{};
    ReencodeContainer container{};
    std::vector<std::byte> bytes;
    std::vector<ReencodedTexture> textures;
    std::string detail;
};

// Re-encodes one DDS, single gfxTexture or PTX payload; `pac_slot` only tags
// the reports.
[[nodiscard]] PayloadReencodeResult reencode_payload(
    std::span<const std::byte> payload, const TextureReencodeOptions& options, int pac_slot = -1);

// Formats of every texture reencode_textures would touch (same traversal,
// nothing decoded): one entry per texture, PAC slots in order.
[[nodiscard]] std::vector<codecs::dds_bcn::Document> list_textures(std::span<const std::byte> bytes);
// The same, descending into nested PAC slots.
[[nodiscard]] std::vector<codecs::dds_bcn::Document> list_textures(std::span<const std::byte> bytes, bool nested);

// PTX walk with the checks dmc3.exe makes on load (0x140336BB0,
// 0x140046510): count, sector spans, gfxTexture +0x00 = 0, +0x20 = 0x40,
// +0x68 = 8, +0x64 = DDS size, any BC1..BC7 DDS, spans ending at the end.
[[nodiscard]] bool is_texture_bundle(std::span<const std::byte> bytes) noexcept;

// One gfxTexture + DDS with nothing around it (.tm2 files, some PAC slots
// such as id5000.pac slot 23), with the same load-path checks.
[[nodiscard]] bool is_wrapped_texture(std::span<const std::byte> bytes) noexcept;

// PAC slot table: magic "PAC\0", u32 count, u32 offsets (0 = empty slot);
// a slot runs to the next higher offset or the end of the file.
struct PacSlotExtent final {
    std::uint64_t offset{};
    std::uint64_t size{};
};
[[nodiscard]] std::optional<std::vector<PacSlotExtent>> read_pac_slots(std::span<const std::byte> bytes);

// True when the bytes hold something reencode_textures accepts: a DDS, a
// single gfxTexture, a PTX, or a PAC with at least one such slot.
[[nodiscard]] bool holds_textures(std::span<const std::byte> bytes) noexcept;
// The same, descending into nested PAC slots.
[[nodiscard]] bool holds_textures(std::span<const std::byte> bytes, bool nested) noexcept;

// Rebuilds a PAC with some slots replaced; untouched slots are copied
// verbatim, every slot start stays 16-byte aligned.
struct PacSlotReplacement final {
    std::uint32_t slot{};
    std::vector<std::byte> bytes;
};
[[nodiscard]] std::optional<std::vector<std::byte>> replace_pac_slots(
    std::span<const std::byte> pac, std::span<const PacSlotReplacement> replacements);

}  // namespace dmc::rengine::profiles::dmc3
