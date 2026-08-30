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
    // True only when sealed semantic authority comes from a recovered runtime
    // content-tag comparison. Keep this distinct from both generic magic and a
    // structural parser so downstream code cannot launder instruction-backed
    // evidence into a stronger/different proof category.
    bool runtime_content_tag_confirmed{false};
};

class ResourceClassifier final {
public:
    // Raw probe API. Magic/structural byte signatures outrank path extension.
    [[nodiscard]] static ResourceClassification classify(
        std::string_view logical_path,
        std::span<const std::byte> bytes = {});

    // Materialized-resource API. Valid sealed semantic evidence outranks any
    // presentation/name hint. If semantic evidence exists but no longer matches
    // current bytes, the hint is deliberately ignored so a display suffix
    // cannot launder stale evidence into semantic authority.
    [[nodiscard]] static ResourceClassification classify(
        const ResourcePayload& payload,
        std::string_view naming_hint = {});

    [[nodiscard]] static GameProfile profile_from_path(
        std::string_view logical_path);

    [[nodiscard]] static bool is_container_format(
        std::string_view format) noexcept;
};

} // namespace dmc::rengine::gdspaces
