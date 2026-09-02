#pragma once

#include "dmc_rengine/gdspaces/profile.hpp"

#include <cstddef>
#include <cstdint>
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

    // True when this format came from sealed semantic evidence that still
    // validates against the exact current bytes.
    //
    // Distinct from `byte_derived`, which says the raw probe read the payload.
    // This says something stronger and narrower: a recovered semantic record
    // was checked against this resource's identity and digest and agreed. A
    // `.ukn` that reads as `.hits` earns this; a `.pac` recognized by its
    // extension earns neither.
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
    // `path_names_the_resource` is false when the caller synthesized the name
    // itself — a relative-slot container stores no names, so `slot_0001.bin`
    // is the parser's placeholder, not evidence. Classifying by that suffix
    // would be reading back our own guess as if it were a fact.
    [[nodiscard]] static ResourceClassification classify(
        std::string_view logical_path,
        std::span<const std::byte> bytes = {},
        bool path_names_the_resource = true);

    // Materialized-resource API. Valid sealed semantic evidence outranks any
    // presentation or name hint. Where semantic evidence exists but no longer
    // matches the current bytes the hint is deliberately ignored, so a display
    // suffix cannot launder stale evidence into semantic authority.
    [[nodiscard]] static ResourceClassification classify(
        const ResourcePayload& payload,
        std::string_view naming_hint = {});

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
