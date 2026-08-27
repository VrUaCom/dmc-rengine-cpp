#pragma once

#include "dmc_rengine/gdspaces/profile.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

struct ResourceClassification final {
    std::string format{"unknown"};
    GameProfile profile{GameProfile::unknown};
    bool container{false};
    bool magic_confirmed{false};

    // True when the format was decided by reading the payload rather than by
    // trusting the path. A magic signature is one way to earn this; a record
    // that is structurally text is another, and that one carries no magic.
    bool byte_derived{false};

    // The animation registry's type code for this resource, or -1.
    //
    // Animation is typed by a second registry that asks the name and never the
    // bytes, so this is a claim about the name in every case but one. `mot` is
    // the exception: its structure is recovered, so a motion found inside a
    // container with no name at all still lands here.
    std::int32_t animation_type{-1};
    // True where this project can read the kind and not merely name it.
    bool animation_structure_recovered{false};
};

class ResourceClassifier final {
public:
    // `path_names_the_resource` is false when the caller synthesized the name
    // itself — a relative-slot container stores no names, so `slot_0001.bin`
    // is the parser's placeholder, not evidence. Classifying by that suffix
    // would be reading back our own guess as if it were a fact.
    [[nodiscard]] static ResourceClassification classify(
        std::string_view logical_path,
        std::span<const std::byte> bytes = {},
        bool path_names_the_resource = true);

    [[nodiscard]] static GameProfile profile_from_path(
        std::string_view logical_path);

    // The formats whose container-ness is a claim about bytes rather than
    // about a name. See the note in `classify`.
    [[nodiscard]] static bool is_structural_container_format(
        std::string_view format) noexcept;

    [[nodiscard]] static bool is_container_format(
        std::string_view format) noexcept;
};

} // namespace dmc::rengine::gdspaces
