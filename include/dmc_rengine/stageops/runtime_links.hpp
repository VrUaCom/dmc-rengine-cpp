#pragma once

#include "dmc_rengine/stageops/domain_workspace.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

// This authority describes the recovered-runtime side of a link. It is not the
// same thing as Stage Ops structural-domain authority and never upgrades a
// lexical/parser fact by itself.
enum class StageRuntimeLinkAuthority {
    direct_reconstructed,
    disassembly_complete_corpus_pending,
    executable_candidate,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageRuntimeLinkAuthority authority) noexcept {
    switch (authority) {
    case StageRuntimeLinkAuthority::direct_reconstructed:
        return "direct-reconstructed";
    case StageRuntimeLinkAuthority::disassembly_complete_corpus_pending:
        return "disassembly-complete-corpus-pending";
    case StageRuntimeLinkAuthority::executable_candidate:
        return "executable-candidate";
    }
    return "executable-candidate";
}

enum class StageRuntimeLinkKind {
    parsed_by_runtime_consumer,
    classified_by_runtime_function,
    consumed_by_runtime_function,
    constructs_runtime_object,
    participates_in_runtime_dependency,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageRuntimeLinkKind kind) noexcept {
    switch (kind) {
    case StageRuntimeLinkKind::parsed_by_runtime_consumer:
        return "parsed-by-runtime-consumer";
    case StageRuntimeLinkKind::classified_by_runtime_function:
        return "classified-by-runtime-function";
    case StageRuntimeLinkKind::consumed_by_runtime_function:
        return "consumed-by-runtime-function";
    case StageRuntimeLinkKind::constructs_runtime_object:
        return "constructs-runtime-object";
    case StageRuntimeLinkKind::participates_in_runtime_dependency:
        return "participates-in-runtime-dependency";
    }
    return "consumed-by-runtime-function";
}

// Opaque stable identity owned by the Recovered Game Source Tree / Reverse
// evidence layer. Generic Stage Ops deliberately does not embed DMC3 VAs or
// duplicate recovered function bodies.
struct RecoveredRuntimeIdentity final {
    std::string id;
    std::string source_tree_path;
    std::string symbol;
    std::string executable_artifact_id;
    std::vector<std::string> evidence_ids;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() &&
            !source_tree_path.empty() &&
            !symbol.empty() &&
            !executable_artifact_id.empty() &&
            !evidence_ids.empty();
    }
};

// One explicit evidence-backed relation between a Stage Ops-owned domain object
// and recovered vanilla runtime identity. The domain object remains product-side
// state; the recovered identity remains recovered-game/evidence-side state.
struct StageRuntimeLink final {
    std::string id;
    std::string domain_object_id;
    RecoveredRuntimeIdentity runtime;
    StageRuntimeLinkKind kind{StageRuntimeLinkKind::consumed_by_runtime_function};
    StageRuntimeLinkAuthority authority{
        StageRuntimeLinkAuthority::executable_candidate};
    std::string claim_id;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() &&
            !domain_object_id.empty() &&
            runtime.valid() &&
            !claim_id.empty();
    }
};

struct StageRuntimeLinkWorkspace final {
    StageAssemblyIdentity identity;
    std::uint64_t source_stage_revision{};
    std::vector<StageRuntimeLink> links;

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] std::vector<const StageRuntimeLink*> for_domain(
        std::string_view domain_object_id) const;
};

// Generic Stage Ops accepts only explicit links supplied by a profile/reverse
// integration layer. It never scans token values, filenames or graph labels to
// invent runtime semantics.
class StageRuntimeLinkBuilder final {
public:
    [[nodiscard]] static StageRuntimeLinkWorkspace build(
        const StageDomainWorkspace& domains,
        std::vector<StageRuntimeLink> explicit_links);
};

} // namespace dmc::rengine::stageops
