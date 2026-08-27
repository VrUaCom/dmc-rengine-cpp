#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include <cstdint>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ContainerChild final {
    formats::ContainerEntry entry;
    ResourcePayload payload;
    // How the shown name was arrived at, and — where the container carries its
    // own name list — what that list calls this slot.
    //
    // The attributed name never replaces the identity or the display name. A
    // manifest line is a claim the container makes, corroborated at most by
    // the payload agreeing with the extension, and printing it as though it
    // were recovered would be the exact mistake this field exists to avoid.
    SlotNameAttribution name_attribution;
};

// How a container's slots are numbered, and whether the empty ones look
// deliberate.
//
// A run of empty slots reads as damage on a screen. In a model group it is
// not: the populated indices are multiples of ten and the gaps are reserved
// identity space, which the recovered walk already supports — a zero offset is
// a slot that carries nothing, not a container that is broken.
//
// `every_populated_index_on_stride` is the claim worth showing, and it is
// computed from this container rather than assumed from the two that were
// read.
struct SlotNumberingSummary final {
    std::uint32_t declared_slots{};
    std::uint32_t populated_slots{};
    std::uint32_t absent_slots{};
    std::uint32_t stride{};
    bool every_populated_index_on_stride{false};

    // A container with one populated slot cannot demonstrate a stride, so it
    // does not claim one.
    [[nodiscard]] bool stride_is_demonstrated() const noexcept {
        return populated_slots > 1U && every_populated_index_on_stride;
    }
};

struct ContainerExpansion final {
    ResourceRef parent;
    std::string parser_format;
    std::vector<ContainerChild> children;
    std::vector<Diagnostic> diagnostics;
    SlotNumberingSummary numbering;

    [[nodiscard]] bool usable() const noexcept;
};

class ContainerExpander final {
public:
    [[nodiscard]] static ContainerExpansion expand(
        const ResourcePayload& parent,
        const formats::ContainerParseResult& parsed);

    static void connect_graph(
        const ContainerExpansion& expansion,
        ResourceGraph& graph);
};

} // namespace dmc::rengine::gdspaces
