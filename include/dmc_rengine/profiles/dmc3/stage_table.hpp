#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

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

struct StageResourceTableDescriptor final {
    std::string id;
    std::string evidence_packet_id;
    std::string artifact_sha256;
    std::uint64_t file_offset{};
    std::uint32_t rva{};
    std::uint64_t va{};
    std::uint32_t row_count{};
    std::array<StageResourceRole, 4> columns{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::uint32_t entry_count() const noexcept;
    [[nodiscard]] std::optional<StageResourceRole> role_for_column(
        std::size_t column) const noexcept;
};

[[nodiscard]] const StageResourceTableDescriptor&
phase12_stage_resource_table() noexcept;

} // namespace dmc::rengine::profiles::dmc3
