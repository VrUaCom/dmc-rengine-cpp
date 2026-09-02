#pragma once

#include "dmc_rengine/formats/container.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include <cstdint>
#include <span>
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

// Parent-level external .index observation retained after reconciliation.
// `directive` is presentation/extraction metadata (currently "" or "PNST");
// it never controls physical slot identity or write targeting.
struct ContainerIndexNamingEvidence final {
    ResourceId manifest_resource;
    std::string manifest_sha256;
    std::string directive;
    std::size_t entry_count{};
};

struct ContainerExpansion final {
    ResourceRef parent;
    std::string parser_format;
    std::vector<ContainerChild> children;
    std::vector<Diagnostic> diagnostics;
    SlotNumberingSummary numbering;
    std::optional<ContainerIndexNamingEvidence> external_index_evidence;

    [[nodiscard]] bool usable() const noexcept;
};

// What a caller knows that the container being expanded does not.
//
// Some containers are named by their *enclosing* one — an effect pack writes
// its record names in the outer container's slot 0 and they name the slots of
// the inner container in slot 1. The expander sees one container at a time and
// cannot reach across, so the caller, which holds the enclosing bytes, brings
// them.
//
// This exists because the alternative was worse. The rule lived in one
// application's session layer, so a phone showed `V 922` and every other
// consumer of this library showed `slot_0000.bin` for the same slot. A naming
// rule that only one caller has is not a naming rule.
struct ContainerNamingContext final {
    // The container that encloses the one being expanded, if any.
    std::span<const std::byte> enclosing_container;
    // Which slot of that enclosing container holds what is being expanded.
    std::uint32_t slot_index_within_enclosing{};

    [[nodiscard]] bool empty() const noexcept {
        return enclosing_container.empty();
    }
};

class ContainerExpander final {
public:
    [[nodiscard]] static ContainerExpansion expand(
        const ResourcePayload& parent,
        const formats::ContainerParseResult& parsed,
        const ContainerNamingContext& naming = {});

    static void connect_graph(
        const ContainerExpansion& expansion,
        ResourceGraph& graph);
};

} // namespace dmc::rengine::gdspaces
