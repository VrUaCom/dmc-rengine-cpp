#pragma once

#include "dmc_rengine/formats/mod_writer.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/nested_relative_slot_reintegrator.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class ModAuthoredChildStatus : std::uint8_t {
    ok,
    invalid_source,
    writer_failed,
    receipt_invalid,
    source_hash_mismatch,
    output_hash_mismatch,
    size_changed,
    source_reparse_failed,
    output_reparse_failed,
    invalid_authored_image,
};

[[nodiscard]] constexpr std::string_view to_string(
    ModAuthoredChildStatus status) noexcept {
    switch (status) {
    case ModAuthoredChildStatus::ok: return "ok";
    case ModAuthoredChildStatus::invalid_source: return "invalid-source";
    case ModAuthoredChildStatus::writer_failed: return "writer-failed";
    case ModAuthoredChildStatus::receipt_invalid: return "receipt-invalid";
    case ModAuthoredChildStatus::source_hash_mismatch: return "source-hash-mismatch";
    case ModAuthoredChildStatus::output_hash_mismatch: return "output-hash-mismatch";
    case ModAuthoredChildStatus::size_changed: return "size-changed";
    case ModAuthoredChildStatus::source_reparse_failed: return "source-reparse-failed";
    case ModAuthoredChildStatus::output_reparse_failed: return "output-reparse-failed";
    case ModAuthoredChildStatus::invalid_authored_image: return "invalid-authored-image";
    }
    return "invalid-authored-image";
}

struct ModAuthoredChildResult final {
    ModAuthoredChildStatus status{ModAuthoredChildStatus::invalid_source};
    std::optional<AuthoredChildImage> image;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == ModAuthoredChildStatus::ok && image.has_value() &&
            image->valid();
    }
};

class ModAuthoredChildBridge final {
public:
    // Converts only a successful canonical MOD preserve-layout writer result
    // into the generic reintegration envelope. This is the trust boundary
    // between format-specific writer evidence and PAC/PNST child replacement.
    //
    // revision remains caller-supplied because the MOD writer currently edits
    // typed IR directly rather than a gdspaces::WorkingCopy. A value of 0 is
    // therefore valid and means no WorkingCopy revision authority is claimed.
    [[nodiscard]] static ModAuthoredChildResult build(
        const gdspaces::ResourcePayload& source,
        const formats::mod::WriteResult& written,
        std::uint64_t revision = 0U);
};

} // namespace dmc::rengine::profiles::dmc3
