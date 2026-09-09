#pragma once

#include "dmc_rengine/formats/mod.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::formats::mod::writer {

enum class RebuildStatus : std::uint8_t {
    ok,
    missing_source_bytes,
    source_image_mismatch,
    source_parse_failed,
    typed_ir_diverged,
    output_parse_failed,
    byte_parity_failed,
};

[[nodiscard]] constexpr std::string_view to_string(
    RebuildStatus status) noexcept {
    switch (status) {
    case RebuildStatus::ok: return "ok";
    case RebuildStatus::missing_source_bytes: return "missing-source-bytes";
    case RebuildStatus::source_image_mismatch: return "source-image-mismatch";
    case RebuildStatus::source_parse_failed: return "source-parse-failed";
    case RebuildStatus::typed_ir_diverged: return "typed-ir-diverged";
    case RebuildStatus::output_parse_failed: return "output-parse-failed";
    case RebuildStatus::byte_parity_failed: return "byte-parity-failed";
    }
    return "missing-source-bytes";
}

struct NoEditRebuildReceipt final {
    std::string source_sha256;
    std::string output_sha256;
    std::uint64_t byte_count{};
    std::uint64_t mesh_count{};
    std::uint8_t outer_record_count{};
    std::uint8_t transform_domain_count{};
    bool source_image_matches_document{false};
    bool typed_ir_matches_source{false};
    bool output_reparse_ok{false};
    bool byte_identical{false};

    [[nodiscard]] bool valid() const noexcept;
};

struct RebuildResult final {
    RebuildStatus status{RebuildStatus::missing_source_bytes};
    std::vector<std::byte> bytes;
    std::optional<NoEditRebuildReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RebuildStatus::ok && receipt.has_value() &&
            receipt->valid();
    }
};

class PreserveSourceWriter final {
public:
    // First MOD authoring gate only. The immutable caller-supplied source image
    // remains layout authority. No field mutation or layout synthesis is
    // performed here. The writer fails closed if the parsed serializable IR no
    // longer describes that exact source image.
    [[nodiscard]] static RebuildResult rebuild_no_edit(
        std::span<const std::byte> immutable_source,
        const Document& document);
};

} // namespace dmc::rengine::formats::mod::writer
