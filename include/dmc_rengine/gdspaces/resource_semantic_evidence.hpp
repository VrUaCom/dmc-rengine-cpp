#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

class ContainerNamingReconciler;

enum class ResourceSemanticEvidenceKind : std::uint8_t {
    embedded_name_list,
    magic_confirmed_format,
    profile_structural_format,
    // Profile-specific evidence that comes from an instruction-backed runtime
    // content probe rather than a structural parser. Keeping this distinct is
    // essential provenance: "MOD" is evidence because dmc3.exe compares those
    // exact three bytes, not because a MOD document parser proved its shape.
    profile_runtime_content_tag,
};

// Read-only semantic evidence attached to a materialized resource. This is
// presentation-independent and is never ResourceId/write authority. Only the
// naming reconciler can persist evidence produced by a sealed byte/structural
// observation, so a display suffix cannot manufacture semantic authority.
class ResourceSemanticEvidence final {
public:
    [[nodiscard]] ResourceSemanticEvidenceKind kind() const noexcept;
    [[nodiscard]] const ResourceId& authority_resource() const noexcept;
    [[nodiscard]] std::string_view authority_sha256() const noexcept;
    [[nodiscard]] std::string_view semantic_format() const noexcept;
    [[nodiscard]] std::string_view canonical_extension() const noexcept;
    [[nodiscard]] std::uint32_t physical_slot_index() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class ContainerNamingReconciler;

    ResourceSemanticEvidence(
        ResourceSemanticEvidenceKind kind,
        ResourceId authority_resource,
        std::string authority_sha256,
        std::string semantic_format,
        std::string canonical_extension,
        std::uint32_t physical_slot_index);

    ResourceSemanticEvidenceKind kind_{
        ResourceSemanticEvidenceKind::embedded_name_list};
    ResourceId authority_resource_;
    std::string authority_sha256_;
    std::string semantic_format_;
    std::string canonical_extension_;
    std::uint32_t physical_slot_index_{};
};

} // namespace dmc::rengine::gdspaces
