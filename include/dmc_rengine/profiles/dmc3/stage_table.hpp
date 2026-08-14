#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class StageResourceRole {
    script,
    room_config,
    room_effects,
    room_sound,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageResourceRole role) noexcept {
    switch (role) {
    case StageResourceRole::script: return "script";
    case StageResourceRole::room_config: return "room-config";
    case StageResourceRole::room_effects: return "room-effects";
    case StageResourceRole::room_sound: return "room-sound";
    }
    return "script";
}

// Compatibility descriptor for the 110-row Bank-A region. Wave 2 corrected
// the semantic cell boundary: each 0x10 cell begins with kind16 and stores its
// path pointer at +0x08. The original Phase-12 path matrix was real, but it was
// read beginning at the first path field rather than the true descriptor base.
struct StageResourceTableDescriptor final {
    std::string id;
    std::string evidence_packet_id;
    std::string artifact_sha256;
    std::uint64_t artifact_size{};
    std::uint64_t file_offset{};
    std::uint32_t rva{};
    std::uint64_t va{};
    std::uint32_t row_count{};
    std::uint32_t cell_stride{};
    std::uint32_t kind16_offset{};
    std::uint32_t path_pointer_offset{};
    std::array<StageResourceRole, 4> columns{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::uint32_t entry_count() const noexcept;
    [[nodiscard]] std::uint64_t table_size_bytes() const noexcept;
    [[nodiscard]] std::optional<StageResourceRole> role_for_column(
        std::size_t column) const noexcept;
};

// Evidence-only metadata for the larger Stage descriptor universe discovered
// by Wave 2. This deliberately records the observed bank topology and numeric
// Stage IDs without pretending that the still-open selector/fallback ABI has
// been fully reconstructed.
struct StageDescriptorBankMetadata final {
    std::string id;
    std::uint64_t va{};
    std::uint32_t row_count{};
    std::vector<std::uint16_t> numeric_stage_ids;

    [[nodiscard]] bool valid() const noexcept;
};

struct StageDescriptorUniverseMetadata final {
    std::string evidence_packet_id;
    std::array<StageDescriptorBankMetadata, 2> banks;
    std::uint64_t selector_table_va{};
    std::uint32_t selector_entry_count{};
    std::uint64_t group_base_table_va{};
    std::uint32_t group_base_count{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::uint32_t observed_descriptor_count() const noexcept;
};

// Corrected Bank-A compatibility descriptor. The full Stage descriptor
// universe is described separately by wave2_stage_descriptor_universe().
[[nodiscard]] const StageResourceTableDescriptor&
wave2_stage_resource_bank_a() noexcept;

// Legacy name retained while the 110-row StageCatalog stack is reconciled.
// It aliases the corrected Wave-2 Bank-A descriptor and must not be interpreted
// as the complete DMC3 Stage descriptor universe.
[[nodiscard]] const StageResourceTableDescriptor&
phase12_stage_resource_table() noexcept;

[[nodiscard]] const StageDescriptorUniverseMetadata&
wave2_stage_descriptor_universe() noexcept;

} // namespace dmc::rengine::profiles::dmc3
