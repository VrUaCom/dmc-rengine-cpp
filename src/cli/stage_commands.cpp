#include "stage_commands.hpp"

#include "dmc_rengine/evidence/json_import.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/integration/stage_workspace_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/stage_resources.hpp"
#include "dmc_rengine/profiles/dmc3/stage_workspace_builder.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] std::optional<evidence::EvidencePacket> load_evidence_packet(
    const std::filesystem::path& path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << "build-stage-workspace: evidence file is not readable: "
                  << path.string() << '\n';
        return std::nullopt;
    }

    gdspaces::SourceRegistry sources;
    constexpr auto source_id = "stage-evidence";
    if (!sources.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            source_id, absolute.parent_path(), false))) {
        std::cerr << "build-stage-workspace: failed to mount evidence directory\n";
        return std::nullopt;
    }

    const auto resources = sources.enumerate_all();
    const auto iterator = std::find_if(
        resources.begin(), resources.end(),
        [&absolute](const gdspaces::ResourceRef& resource) {
            return resource.id.logical_path == absolute.filename().generic_string();
        });
    if (iterator == resources.end()) {
        std::cerr << "build-stage-workspace: evidence resource was not enumerated\n";
        return std::nullopt;
    }

    const auto payload = sources.read(iterator->id);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << "build-stage-workspace: GDSpaces could not read evidence\n";
        return std::nullopt;
    }

    std::string json;
    if (!payload->bytes.empty()) {
        json.assign(
            reinterpret_cast<const char*>(payload->bytes.data()),
            payload->bytes.size());
    }
    const auto imported = evidence::evidence_packet_from_json(json);
    if (!imported.ok()) {
        std::cerr << "build-stage-workspace: Evidence Packet is invalid\n";
        for (const auto& diagnostic : imported.diagnostics) {
            std::cerr << "  " << diagnostic.path << ": "
                      << diagnostic.message << '\n';
        }
        return std::nullopt;
    }
    return imported.packet;
}

} // namespace

int run_build_stage_workspace(
    std::string_view stage_id,
    const std::filesystem::path& root,
    const std::optional<std::filesystem::path>& evidence_path) {
    if (stage_id != "st001") {
        std::cerr
            << "build-stage-workspace: only the evidence-backed st001 plan is currently exposed\n";
        return 1;
    }

    std::error_code error;
    const auto absolute_root = std::filesystem::absolute(root, error);
    if (error || !std::filesystem::is_directory(absolute_root, error) || error) {
        std::cerr << "build-stage-workspace: not a readable directory: "
                  << root.string() << '\n';
        return 2;
    }

    gdspaces::SourceRegistry sources;
    if (!sources.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            "stage-workspace", absolute_root, true))) {
        std::cerr << "build-stage-workspace: failed to mount stage directory\n";
        return 3;
    }

    std::vector<gdspaces::ResourcePayload> payloads;
    for (const auto& resource : sources.enumerate_all()) {
        auto payload = sources.read(resource.id);
        if (!payload.has_value() || !payload->readable()) {
            std::cerr << "[warning] skipped unreadable resource: "
                      << resource.id.logical_path << '\n';
            continue;
        }
        payloads.push_back(std::move(*payload));
    }
    if (payloads.empty()) {
        std::cerr << "build-stage-workspace: no readable resources found\n";
        return 4;
    }

    std::optional<evidence::EvidencePacket> evidence_packet;
    if (evidence_path.has_value()) {
        evidence_packet = load_evidence_packet(*evidence_path);
        if (!evidence_packet.has_value()) {
            return 5;
        }
    }

    auto result = profiles::dmc3::StageWorkspaceBuilder::build(
        profiles::dmc3::phase12_st001_resource_plan(),
        std::move(payloads),
        evidence_packet.has_value() ? &*evidence_packet : nullptr);

    for (const auto& diagnostic : result.diagnostics) {
        std::cerr << '[' << gdspaces::to_string(diagnostic.severity) << "] "
                  << diagnostic.code << ": " << diagnostic.message << '\n';
    }

    const auto manifest = integration::stage_workspace_manifest_json(
        result.project, "st001");
    if (manifest.empty()) {
        std::cerr << "build-stage-workspace: no Stage Workspace Manifest was produced\n";
        return 6;
    }
    std::cout << manifest;
    return result.complete() ? 0 : 7;
}

} // namespace dmc::rengine::cli
