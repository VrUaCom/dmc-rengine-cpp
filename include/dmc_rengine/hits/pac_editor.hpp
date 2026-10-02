#pragma once

#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::hits::pac_editor {

enum class ReplaceStatus : std::uint8_t {
    ok = 0,
    invalid_parent,
    target_slot_not_found,
    target_slot_not_hits,
    invalid_replacement_hits,
    reflow_failed,
    reopen_failed,
    replacement_mismatch,
};

[[nodiscard]] constexpr const char* to_string(
    ReplaceStatus status) noexcept {
    switch (status) {
    case ReplaceStatus::ok: return "ok";
    case ReplaceStatus::invalid_parent: return "invalid-parent";
    case ReplaceStatus::target_slot_not_found: return "target-slot-not-found";
    case ReplaceStatus::target_slot_not_hits: return "target-slot-not-hits";
    case ReplaceStatus::invalid_replacement_hits: return "invalid-replacement-hits";
    case ReplaceStatus::reflow_failed: return "reflow-failed";
    case ReplaceStatus::reopen_failed: return "reopen-failed";
    case ReplaceStatus::replacement_mismatch: return "replacement-mismatch";
    }
    return "invalid-parent";
}

struct ReplaceResult final {
    ReplaceStatus status{ReplaceStatus::invalid_parent};
    std::uint32_t slot_index{};
    std::vector<std::byte> bytes;
    std::optional<profiles::dmc3::RelativeSlotPackedReflowReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == ReplaceStatus::ok &&
            receipt.has_value() &&
            !bytes.empty();
    }
};

class PacHitsWriter final {
public:
    // Replaces exactly one populated PAC child whose current payload parses as
    // canonical HITS. Slot identity is physical and explicit; no filename or
    // ordinal inference is performed.
    [[nodiscard]] static ReplaceResult replace_slot(
        const gdspaces::ResourcePayload& parent_pac,
        std::uint32_t slot_index,
        std::span<const std::byte> replacement_hits,
        std::uint64_t revision = 1U);
};

} // namespace dmc::rengine::hits::pac_editor
