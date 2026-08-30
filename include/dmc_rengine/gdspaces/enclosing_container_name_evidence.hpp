#pragma once

#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {
class EffectStoredNameEvidenceBuilder;
}

namespace dmc::rengine::gdspaces {

// A name stored by bytes in an enclosing physical container for one of its
// materialized descendants. This is intentionally separate from external
// extractor `.index` evidence and from embedded semantic aliases.
//
// It is naming evidence only: it never participates in ResourceId, byte
// provenance, physical slot selection, write targeting, or container topology.
class EnclosingContainerNameEvidence final {
public:
    [[nodiscard]] const ResourceId& authority_resource() const noexcept;
    [[nodiscard]] std::string_view authority_sha256() const noexcept;
    [[nodiscard]] std::string_view raw_label() const noexcept;
    [[nodiscard]] std::string_view normalized_name() const noexcept;
    [[nodiscard]] std::uint32_t physical_slot_index() const noexcept;
    [[nodiscard]] std::size_t source_line() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    friend class profiles::dmc3::EffectStoredNameEvidenceBuilder;

    EnclosingContainerNameEvidence(
        ResourceId authority_resource,
        std::string authority_sha256,
        std::string raw_label,
        std::string normalized_name,
        std::uint32_t physical_slot_index,
        std::size_t source_line);

    ResourceId authority_resource_;
    std::string authority_sha256_;
    std::string raw_label_;
    std::string normalized_name_;
    std::uint32_t physical_slot_index_{};
    std::size_t source_line_{};
};

} // namespace dmc::rengine::gdspaces
