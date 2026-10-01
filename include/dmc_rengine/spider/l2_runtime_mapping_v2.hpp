#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::spider {

inline constexpr std::string_view k_l2_runtime_mapping_v2_schema =
    "dmc-rengine.gdspaces-l2-runtime-mapping.v2";

struct L2RuntimeMappingV2Authority final {
    std::string canonical_analysis_sha256{
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082"};
    std::uint64_t canonical_analysis_size{6356432U};
    std::uint64_t preferred_image_base{0x140000000ULL};
};

struct L2RuntimeMappingAnchorV2 final {
    std::string id;
    std::uint64_t rva{};
    std::uint64_t runtime_va{};
    std::uint64_t size{};
    std::string window_sha256;
    std::string canonical_window_sha256;
};

struct L2RuntimeMappingPacketV2 final {
    std::string protected_artifact_sha256;
    std::uint64_t protected_artifact_size{};
    std::string canonical_analysis_artifact_sha256;
    std::uint64_t canonical_analysis_artifact_size{};
    std::uint64_t preferred_image_base{};
    std::uint64_t pid{};
    std::uint64_t process_creation_filetime{};
    std::uint64_t module_base{};
    std::string image_name;
    std::vector<L2RuntimeMappingAnchorV2> anchors;
};

struct L2RuntimeMappingBuildResultV2 final {
    std::optional<L2RuntimeMappingPacketV2> packet;
    std::vector<std::string> errors;

    [[nodiscard]] bool ok() const noexcept {
        return packet.has_value() && errors.empty();
    }
};

// Process-instance-bound promotion surface. Canonical window authority is
// derived directly from the exact canonical PE bytes using the canonical PE
// reader; child diagnostic expectation fields can never establish authority.
[[nodiscard]] L2RuntimeMappingBuildResultV2 build_l2_runtime_mapping_v2(
    std::span<const std::string_view> receipt_jsons,
    std::span<const std::byte> canonical_exe,
    const L2RuntimeMappingV2Authority& authority = {});

[[nodiscard]] std::string l2_runtime_mapping_v2_to_json(
    const L2RuntimeMappingPacketV2& packet);

} // namespace dmc::rengine::spider
