#pragma once

#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::gdspaces {

// One read-only view over all naming domains attached to a physical child.
// None of these presentation/evidence fields participate in ResourceId or
// write targeting; physical_slot_index remains the physical locator.
struct ResourceNamingIdentity final {
    ResourceId resource_id;
    std::uint32_t physical_slot_index{};
    bool populated{false};

    // Legacy extraction namespace: .index entry N -> N-th populated payload.
    std::optional<std::size_t> extracted_ordinal;
    std::optional<std::string> external_index_raw_label;
    std::optional<std::string> external_index_name;
    bool external_index_folder{false};

    // Embedded slot-0 alias namespace. This is semantic/name evidence and is
    // not automatically promoted to an original on-disk filename.
    std::optional<std::string> embedded_alias;

    // A distinct name stored by an enclosing physical container (for example
    // the recovered DMC3 effect manifest). It is neither `.index` nor embedded
    // alias authority. The sealed evidence is retained alongside the label.
    std::optional<std::string> enclosing_container_stored_name;
    std::optional<EnclosingContainerNameEvidence>
        enclosing_container_stored_name_evidence;

    // Semantic/presentation namespace. If a sealed structural observation was
    // used, the exact evidence snapshot is retained rather than collapsing it
    // into the semantic_format string.
    std::string semantic_format;
    std::string canonical_extension;
    std::optional<ResourceSemanticEvidence> semantic_format_evidence;
    std::string canonical_display_name;

    // Canonical terminology used by the L1 naming model. The retained member
    // names above are compatibility storage only; these accessors state the
    // semantic meaning unambiguously without introducing duplicate authority.
    [[nodiscard]] const std::optional<std::string>&
    external_index_normalized_name() const noexcept {
        return external_index_name;
    }
    [[nodiscard]] bool external_index_folder_marker() const noexcept {
        return external_index_folder;
    }

    [[nodiscard]] bool valid() const noexcept;
};

struct ResourceNamingIdentityBuildResult final {
    std::optional<ResourceNamingIdentity> identity;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

struct ContainerNamingIdentitySnapshot final {
    ResourceId parent_resource;
    std::optional<ContainerIndexNamingEvidence> external_index_evidence;
    std::vector<ResourceNamingIdentity> children;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class ResourceNamingIdentityBuilder final {
public:
    [[nodiscard]] static ResourceNamingIdentityBuildResult build(
        const ContainerChild& child);

    [[nodiscard]] static ContainerNamingIdentitySnapshot build(
        const ContainerExpansion& expansion);
};

} // namespace dmc::rengine::gdspaces
