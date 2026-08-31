#pragma once

#include "dmc_rengine/gdspaces/profile.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

struct ResourcePayload;

struct ResourceClassification final {
    std::string format{"unknown"};
    GameProfile profile{GameProfile::unknown};
    bool container{false};
    bool magic_confirmed{false};
    bool structural_confirmed{false};
    // Sealed evidence from the recovered three-byte DMC3 registry/content
    // probe (0x1402DB1F0).
    bool runtime_content_tag_confirmed{false};
    // Sealed evidence from the separate recovered four-byte family-mask probe
    // (0x1402FD650). Kept separate because byte 3 is significant there and MCV
    // is recognized only by that path.
    bool runtime_family_mask_confirmed{false};
};

class ResourceClassifier final {
public:
    [[nodiscard]] static ResourceClassification classify(
        std::string_view logical_path,
        std::span<const std::byte> bytes = {});

    [[nodiscard]] static ResourceClassification classify(
        const ResourcePayload& payload,
        std::string_view naming_hint = {});

    [[nodiscard]] static GameProfile profile_from_path(
        std::string_view logical_path);

    [[nodiscard]] static bool is_container_format(
        std::string_view format) noexcept;
};

} // namespace dmc::rengine::gdspaces
