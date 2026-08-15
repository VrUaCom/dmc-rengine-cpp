#include "dmc_rengine/stageops/domain_relations.hpp"

#include "dmc_rengine/stageops/domain_source_span.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::stageops {
namespace {

struct ScriptGroup final {
    const StageDomainObject* aggregate{nullptr};
    std::vector<const StageDomainObject*> markers;
};

[[nodiscard]] bool script_marker(StageDomainKind kind) noexcept {
    switch (kind) {
    case StageDomainKind::stage_set_directive_token:
    case StageDomainKind::stage_set_value_token:
    case StageDomainKind::door_token:
    case StageDomainKind::box_in_token:
    case StageDomainKind::next_room_token:
        return true;
    case StageDomainKind::hits_collision:
    case StageDomainKind::dca_records:
    case StageDomainKind::lighting_records:
    case StageDomainKind::stage_script_tokens:
        return false;
    }
    return false;
}

[[nodiscard]] std::string relation_id(
    StageDomainRelationKind kind,
    std::string_view from,
    std::string_view to) {
    return "stage-domain-relation:" + std::string{to_string(kind)} + ":" +
        std::string{from} + "->" + std::string{to};
}

void add_relation(
    StageDomainRelationWorkspace& workspace,
    StageDomainRelationKind kind,
    const StageDomainObject& from,
    const StageDomainObject& to) {
    workspace.relations.push_back(StageDomainRelation{
        .id = relation_id(kind, from.id, to.id),
        .from_domain_id = from.id,
        .to_domain_id = to.id,
        .kind = kind,
        .authority = StageDomainRelationAuthority::structural_parser_fact,
        .resource_id = from.resource_id,
    });
}

} // namespace

bool StageDomainRelationWorkspace::valid() const noexcept {
    if (!identity.valid()) {
        return false;
    }

    std::set<std::string, std::less<>> ids;
    for (const auto& relation : relations) {
        if (!relation.valid() || !ids.insert(relation.id).second) {
            return false;
        }
    }
    return true;
}

std::vector<const StageDomainRelation*> StageDomainRelationWorkspace::by_kind(
    StageDomainRelationKind kind) const {
    std::vector<const StageDomainRelation*> result;
    for (const auto& relation : relations) {
        if (relation.kind == kind) {
            result.push_back(&relation);
        }
    }
    return result;
}

StageDomainRelationWorkspace StageDomainRelationAssembler::assemble(
    const StageDomainWorkspace& domains) {
    StageDomainRelationWorkspace workspace;
    if (!domains.valid()) {
        return workspace;
    }

    workspace.identity = domains.identity;
    workspace.source_stage_revision = domains.source_stage_revision;

    std::map<std::string, ScriptGroup, std::less<>> scripts;
    for (const auto& object : domains.objects) {
        if (!object.valid()) {
            continue;
        }
        auto& group = scripts[object.resource_id];
        if (object.kind == StageDomainKind::stage_script_tokens) {
            group.aggregate = &object;
        } else if (script_marker(object.kind)) {
            group.markers.push_back(&object);
        }
    }

    for (auto& [resource_id, group] : scripts) {
        (void)resource_id;
        if (group.aggregate == nullptr || group.markers.empty()) {
            continue;
        }

        // Source order is derived from the exact parser-provenance byte span,
        // never from vector or lexicographic object-ID order.
        std::sort(
            group.markers.begin(), group.markers.end(),
            [](const StageDomainObject* left, const StageDomainObject* right) {
                const auto left_span = domain_source_span(*left);
                const auto right_span = domain_source_span(*right);
                if (left_span.has_value() && right_span.has_value() &&
                    left_span->offset != right_span->offset) {
                    return left_span->offset < right_span->offset;
                }
                return left->id < right->id;
            });

        for (const auto* marker : group.markers) {
            add_relation(
                workspace,
                StageDomainRelationKind::contains_marker,
                *group.aggregate,
                *marker);
        }
        for (std::size_t index = 1U; index < group.markers.size(); ++index) {
            add_relation(
                workspace,
                StageDomainRelationKind::lexical_next_marker,
                *group.markers[index - 1U],
                *group.markers[index]);
        }
    }

    std::sort(
        workspace.relations.begin(), workspace.relations.end(),
        [](const StageDomainRelation& left, const StageDomainRelation& right) {
            return left.id < right.id;
        });

    return workspace.valid()
        ? workspace
        : StageDomainRelationWorkspace{};
}

} // namespace dmc::rengine::stageops
