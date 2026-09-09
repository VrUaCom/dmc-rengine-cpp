#pragma once

#include "dmc_rengine/formats/mod.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats::mod {

// First authoring gate for MOD. This mode intentionally preserves the physical
// source layout and refuses structural, companion-sensitive or undecoded-field
// edits. It is not a canonical rebuild-from-scratch authority.
enum class WriteMode : std::uint8_t {
    preserve_layout,
};

struct WriteReceipt final {
    std::string source_sha256;
    std::string output_sha256;
    std::uint64_t byte_count{};
    std::uint64_t modified_byte_count{};
    bool source_image_matches_document{false};
    bool unauthorized_bytes_unchanged{false};
    bool output_reparse_ok{false};
    bool no_edit_byte_identical{false};

    [[nodiscard]] bool valid() const noexcept;
};

struct WriteResult final {
    bool success{false};
    std::vector<std::byte> bytes;
    ParseResult reparsed;
    WriteReceipt receipt;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class Writer final {
public:
    // `immutable_source` is the caller-owned canonical source image. The
    // retained Document::source_bytes must match it exactly before any write is
    // permitted. This prevents simultaneous mutation of source_bytes + typed IR
    // from laundering an edited image into the preserve-layout baseline.
    [[nodiscard]] static WriteResult write(
        std::span<const std::byte> immutable_source,
        const Document& document,
        WriteMode mode = WriteMode::preserve_layout);
};

} // namespace dmc::rengine::formats::mod
