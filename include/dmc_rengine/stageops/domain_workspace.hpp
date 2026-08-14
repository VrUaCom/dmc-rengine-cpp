#pragma once

#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/stageops/assembly_workspace.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::stageops {

enum class StageDomainKind {
    hits_collision,
    dca_records,
    lighting_records,
    stage_script_tokens,
    stage_set_directive_token,
    stage_set_value_token,
    door_token,
    box_in_token,
    next_room_token,
};

[[nodiscard]] constexpr std::string_view to_string(
    StageDomainKind kind) noexcept {
    switch (kind) {
    case StageDomainKind::hits_collision: return "hits-collision";
    case StageDomainKind::dca_records: return "dca-records";
    case StageDomainKind::lighting_records: return "lighting-records";
    case StageDomainKind::stage_script_tokens: return "stage-script-tokens";
    case StageDomainKind::stage_set_directive_token:
        return "stage-set-directive-token";
    case StageDomainKind::stage_set_value_token:
        return "stage-set-value-token";
    case StageDomainKind::door_token: return "door-token";
    case StageDomainKind::box_in_token: return "box-in-token";
    case StageDomainKind::next_room_token: return "next-room-token";
    }
    return "stage-script-tokens";
}

struct StageDomainObject final {
    std::string id;
    StageDomainKind kind{StageDomainKind::stage_script_tokens};
    std::string resource_id;
    std::string parser_id;
    integration::ParsedByteSource byte_source{
        integration::ParsedByteSource::immutable_source};
    std::uint64_t byte_revision{};

    // False means the retained typed parser result no longer describes the
    // active WorkingCopy bytes. The object stays visible for diagnostics but
    // must not be treated as current scene state.
    bool current_for_active_bytes{false};
    std::map<std::string, std::string, std::less<>> attributes;

    [[nodiscard]] bool valid() const noexcept {
        return !id.empty() && !resource_id.empty() && !parser_id.empty();
    }
};

struct StageDomainWorkspace final {
    StageAssemblyIdentity identity;
    std::uint64_t source_stage_revision{};
    std::vector<StageDomainObject> objects;
    std::size_t missing_project_session_count{};
    std::size_t missing_typed_result_count{};
    std::size_t unrecognized_typed_result_count{};
    std::size_t stale_typed_result_count{};

    [[nodiscard]] bool valid() const noexcept {
        return identity.valid();
    }

    [[nodiscard]] bool current_for_active_bytes() const noexcept {
        return valid() && stale_typed_result_count == 0U;
    }

    [[nodiscard]] std::vector<const StageDomainObject*> by_kind(
        StageDomainKind kind) const;
};

// Builds Stage Ops domain projections strictly from the canonical typed parser
// results retained by ProjectWorkspace. It never reparses bytes and never
// discovers resources outside StageAssemblyWorkspace.
//
// Stage TXT token objects are structural lexical markers only. A DOOR token is
// not an original runtime Door object, and a StageSet value token is not a
// recovered StageSet instance until separate executable/runtime evidence links
// that lexical marker to a recovered-game consumer/object lifecycle.
class StageDomainAssembler final {
public:
    [[nodiscard]] static StageDomainWorkspace assemble(
        const StageAssemblyWorkspace& assembly,
        const integration::ProjectWorkspace& project,
        std::uint64_t source_stage_revision = 0U);
};

} // namespace dmc::rengine::stageops
