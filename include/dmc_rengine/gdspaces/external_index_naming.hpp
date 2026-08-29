#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/index_sidecar_manifest.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ExternalIndexNamingResult final {
    /// Slots that took a name from the sidecar.
    std::size_t named_slots{};
    /// Named slots whose payload independently agrees with the name's suffix.
    std::size_t corroborated_slots{};
    /// Sidecar lines naming a slot this container does not have.
    std::size_t lines_without_a_slot{};
    /// Populated slots the sidecar says nothing about.
    std::size_t slots_without_a_line{};
    /// True when the sidecar's directive names this container's parser format.
    bool directive_matches_parser{false};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool applied() const noexcept;
};

/**
 * Applies an extraction tool's external `.index` to a container's slot names.
 *
 * Every unpacked folder in the wild carries one of these, which makes them the
 * largest supply of real slot names available anywhere — and this project has
 * been printing `slot_0000` beside them. They are not the game's: the literal
 * `.index` occurs zero times in the executable, so the runtime cannot name such
 * a file. Whatever tool did the extraction wrote down what it called each slot.
 *
 * That provenance is the whole reason this rule lives here rather than in one
 * application. A name from a sidecar is evidence of what a tool decided, never
 * of what the container stores, and it is attributed as `external_index` so a
 * reader is never invited to confuse the two. Putting the rule in a session
 * layer once already produced a phone that showed one name and every other
 * consumer of this library that showed another for the same slot.
 *
 * The mapping is the sidecar's own: line 0 is the container directive, so line
 * *i* names slot *i - 1*. Nothing is overwritten that the container itself
 * named — a name the container stores outranks a name a tool chose.
 */
class ExternalIndexNaming final {
public:
    [[nodiscard]] static ExternalIndexNamingResult apply(
        ContainerExpansion& expansion,
        const IndexSidecarManifest::Document& sidecar);

    /// Parses the bytes and applies them in one step, so a caller cannot
    /// accidentally apply a sidecar this project rendered itself.
    [[nodiscard]] static ExternalIndexNamingResult apply_bytes(
        ContainerExpansion& expansion,
        std::span<const std::byte> sidecar_bytes);
};

} // namespace dmc::rengine::gdspaces
