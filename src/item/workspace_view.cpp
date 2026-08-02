#include "dmc_rengine/item/workspace_view.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <string>

namespace dmc::rengine::item {
namespace {

struct EvidenceMapping final {
    RuntimeEvidenceKind kind;
    std::string_view record_id;
};

inline constexpr std::array mappings{
    EvidenceMapping{
        .kind = RuntimeEvidenceKind::registry_slot_guard,
        .record_id = "ev-dmc3-item-slot50-guard",
    },
    EvidenceMapping{
        .kind = RuntimeEvidenceKind::game_verified_item_ids,
        .record_id = "ev-dmc3-item-id-runtime-tests",
    },
    EvidenceMapping{
        .kind = RuntimeEvidenceKind::inventory_limit_patch_family,
        .record_id = "ev-dmc3-inventory-limit-patch-family",
    },
};

[[nodiscard]] bool complete_runtime_location(
    const evidence::EvidenceRecord& record) noexcept {
    return std::any_of(
        record.locations.begin(), record.locations.end(),
        [](const evidence::EvidenceLocation& location) {
            const auto has_address = location.file_offset.has_value() ||
                location.rva.has_value() || location.va.has_value();
            return !location.artifact_id.empty() && has_address &&
                   location.size.has_value() && *location.size != 0U;
        });
}

} // namespace

const RuntimeEvidenceView* ItemWorkspaceView::evidence(
    RuntimeEvidenceKind kind) const noexcept {
    const auto iterator = std::find_if(
        runtime_evidence.begin(), runtime_evidence.end(),
        [kind](const RuntimeEvidenceView& item) {
            return item.kind == kind;
        });
    return iterator == runtime_evidence.end() ? nullptr : &*iterator;
}

bool ItemWorkspaceView::has_route(std::string_view route) const noexcept {
    return std::find(routes.begin(), routes.end(), route) != routes.end();
}

ItemWorkspaceView build_workspace_view(
    const integration::ProjectWorkspace& project,
    const gdspaces::ResourceId& item_resource) {
    ItemWorkspaceView view;
    const auto* session = project.find_session(item_resource);
    if (session == nullptr || session->resource().format != "itm") {
        return view;
    }

    view.resource = session->resource();
    view.workspace_status = session->status();
    view.diagnostic_count = session->diagnostics().size();

    if (const auto* format = session->format(); format != nullptr) {
        view.format_maturity = format->maturity;
        view.write_policy = format->write_policy;
    }
    if (const auto* working = session->working_copy(); working != nullptr) {
        view.working_copy_revision = working->revision();
        view.dirty = working->dirty();
    }
    if (const auto* binary = session->binary_document(); binary != nullptr) {
        view.binary_document = true;
        view.binary_coverage_bytes = binary->coverage_bytes();
    }

    view.routes.reserve(session->routes().size());
    for (const auto& route : session->routes()) {
        view.routes.emplace_back(gdspaces::to_string(route.target));
    }
    std::sort(view.routes.begin(), view.routes.end());
    view.routes.erase(
        std::unique(view.routes.begin(), view.routes.end()),
        view.routes.end());

    for (const auto& mapping : mappings) {
        const auto* record = project.evidence().find(mapping.record_id);
        if (record == nullptr) {
            continue;
        }
        view.runtime_evidence.push_back(RuntimeEvidenceView{
            .kind = mapping.kind,
            .record_id = record->id,
            .claim_id = record->claim_id,
            .confidence = record->confidence,
            .summary = record->summary,
            .location_complete = complete_runtime_location(*record),
        });
    }
    return view;
}

} // namespace dmc::rengine::item
