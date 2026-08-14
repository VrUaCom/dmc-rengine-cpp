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

// One exact contiguous bank of 0x40-byte StageResourceDescriptor records.
// Each descriptor has four 0x10 cells: script/config/effect/sound. Wave 2/3
// direct code evidence establishes kind16 @ +0x00 and path pointer @ +0x08.
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
    [[nodiscard]] std::uint64_t row_size_bytes() const noexcept;
    [[nodiscard]] std::uint64_t table_size_bytes() const noexcept;
    [[nodiscard]] std::optional<StageResourceRole> role_for_column(
        std::size_t column) const noexcept;
};

// Evidence metadata for the complete observed Stage descriptor universe.
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

// Corrected Bank-A compatibility descriptor.
[[nodiscard]] const StageResourceTableDescriptor&
wave2_stage_resource_bank_a() noexcept;

// Legacy name retained for existing Bank-A callers.
[[nodiscard]] const StageResourceTableDescriptor&
phase12_stage_resource_table() noexcept;

// Wave-3 promotion of the second executable descriptor bank from metadata into
// the same bounded PE-backed reader contract used for Bank A.
[[nodiscard]] const StageResourceTableDescriptor&
wave3_stage_resource_bank_b() noexcept;

[[nodiscard]] const std::array<StageResourceTableDescriptor, 2>&
wave3_stage_resource_banks() noexcept;

[[nodiscard]] const StageDescriptorUniverseMetadata&
wave2_stage_descriptor_universe() noexcept;

// Map a row in either recovered descriptor bank to its primary observed numeric
// ST identity. This is descriptor identity, not mission/room semantics.
[[nodiscard]] std::optional<std::uint16_t> runtime_stage_id_for_table_row(
    const StageResourceTableDescriptor& descriptor,
    std::uint32_t row_index) noexcept;

} // namespace dmc::rengine::profiles::dmc3
