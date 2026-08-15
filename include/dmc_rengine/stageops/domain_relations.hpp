#pragma once

#include "dmc_rengine/stageops/domain_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

enum class StageDomainRelationKind {
    contains_marker,
    lexical_next_marker,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageDomainRelationKind kind) noexcept {
    switch (kind) {
    case StageDomainRelationKind::contains_marker: return "contains-marker";
    case StageDomainRelationKind::lexical_next_marker:
        return "lexical-next-marker";
    }
    return "contains-marker";
}

enum class StageDomainRelationAuthority {
    structural_parser_fact,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageDomainRelationAuthority authority) noexcept {
    switch (authority) {
    case StageDomainRelationAuthority::structural_parser_fact:
        return "structural-parser-fact";
    }
    return "structural-parser-fact";
}

struct StageDomainRelation final {
    std::string id;
    std::string from_domain_id;
    std::string to_domain_id;
    StageDomainRelationKind kind{StageDomainRelationKind::contains_marker};
    StageDomainRelationAuthority authority{
        StageDomainRelationAuthority::structural_parser_fact};
    std::string resource_id;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() &&
            !from_domain_id.empty() &&
            !to_domain_id.empty() &&
            from_domain_id != to_domain_id &&
            !resource_id.empty();
    }
};

struct StageDomainRelationWorkspace final {
    StageAssemblyIdentity identity;
    std::uint64_t source_stage_revision{};
    std::vector<StageDomainRelation> relations;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] std::vector<const StageDomainRelation*> by_kind(
        StageDomainRelationKind kind) const;
};

// Builds only relationships already implied by Stage Ops structural domain
// identity/provenance. It does not parse bytes, infer gameplay grammar, or claim
// original DMC runtime control flow.
class StageDomainRelationAssembler final {
public:
    [[nodiscard]] static StageDomainRelationWorkspace assemble(
        const StageDomainWorkspace& domains);
};

} // namespace dmc::rengine::stageops
