#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::spider {

inline constexpr std::string_view k_l2_runtime_mapping_v1_schema =
    "dmc-rengine.gdspaces-l2-runtime-mapping.v1";

struct L2RuntimeMappingAnchor final {
    std::string id;
    std::uint64_t rva{};
    std::uint64_t runtime_va{};
    std::uint64_t size{};
    std::string window_sha256;
};

struct L2RuntimeMappingPacketV1 final {
    std::string protected_artifact_sha256;
    std::uint64_t protected_artifact_size{};
    std::string canonical_analysis_artifact_sha256;
    std::uint64_t preferred_image_base{};
    std::uint32_t pid{};
    std::uint64_t module_base{};
    std::string image_name;
    std::vector<L2RuntimeMappingAnchor> anchors;
};

struct L2RuntimeMappingBuildResult final {
    std::optional<L2RuntimeMappingPacketV1> packet;
    std::vector<std::string> errors;

    [[nodiscard]] bool ok() const noexcept {
        return packet.has_value() && errors.empty();
    }
};

// Native replacement for verify_l2_runtime_mapping_packet.py. Inputs are the
// exact metadata-only JSON child receipts emitted by the process-window v1
// capture surface. Raw bytes are rejected and no proprietary process bytes are
// read by this validator.
[[nodiscard]] L2RuntimeMappingBuildResult build_l2_runtime_mapping_v1(
    std::span<const std::string_view> receipt_jsons);

[[nodiscard]] std::string l2_runtime_mapping_v1_to_json(
    const L2RuntimeMappingPacketV1& packet);

} // namespace dmc::rengine::spider
