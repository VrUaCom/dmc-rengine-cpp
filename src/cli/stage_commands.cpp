#include "stage_commands.hpp"

#include "dmc_rengine/evidence/json_import.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/integration/stage_workspace_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/stage_catalog.hpp"
#include "dmc_rengine/profiles/dmc3/stage_workspace_builder.hpp"

#include <charconv>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] std::optional<gdspaces::ResourcePayload> load_local_file(
    const std::filesystem::path& input_path,
    std::string source_id,
    std::string_view operation) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << operation << ": not a readable file: "
                  << input_path.string() << '\n';
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << operation << ": file size is unsupported\n";
        return std::nullopt;
    }

    gdspaces::SourceRegistry sources;
    const auto source_id_copy = source_id;
    if (!sources.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::move(source_id), absolute.parent_path(), false))) {
        std::cerr << operation << ": failed to mount parent directory\n";
        return std::nullopt;
    }

    const gdspaces::ResourceId resource{
        .source_id = source_id_copy,
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };
    auto payload = sources.read(resource);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << operation << ": GDSpaces could not read the resource\n";
        if (payload.has_value()) {
            for (const auto& diagnostic : payload->diagnostics) {
                std::cerr << "  ["
                          << gdspaces::to_string(diagnostic.severity)
                          << "] " << diagnostic.code << ": "
                          << diagnostic.message << '\n';
            }
        }
        return std::nullopt;
    }
    return payload;
}

[[nodiscard]] std::optional<evidence::EvidencePacket> load_evidence_packet(
    const std::filesystem::path& path) {
    const auto payload = load_local_file(path, "stage-evidence", "build-stage-workspace");
    if (!payload.has_value()) {
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

[[nodiscard]] std::optional<profiles::dmc3::StageCatalogLoadResult>
load_stage_catalog(const std::filesystem::path& executable_path) {
    constexpr std::string_view operation = "stage-catalog";
    const auto payload = load_local_file(
        executable_path, "stage-catalog-exe", operation);
    if (!payload.has_value()) {
        return std::nullopt;
    }

    const auto bytes = std::span<const std::byte>{payload->bytes};
    auto loaded = profiles::dmc3::StageCatalogLoader::load_canonical(bytes);
    for (const auto& diagnostic : loaded.catalog.diagnostics) {
        std::cerr << "stage-catalog ["
                  << gdspaces::to_string(diagnostic.severity) << "] "
                  << diagnostic.code << ": " << diagnostic.message << '\n';
    }
    if (!loaded.bank_a_complete()) {
        std::cerr << "stage-catalog: canonical Wave-2 Bank-A compatibility load failed; sha256="
                  << loaded.artifact_sha256 << '\n';
        return std::nullopt;
    }
    return loaded;
}

[[nodiscard]] std::optional<std::uint32_t> parse_row_index(
    std::string_view text) noexcept {
    std::uint32_t value{};
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto [end, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || end != last) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] std::optional<std::vector<gdspaces::ResourcePayload>>
load_workspace_payloads(const std::filesystem::path& root) {
    std::error_code error;
    const auto absolute_root = std::filesystem::absolute(root, error);
    if (error || !std::filesystem::is_directory(absolute_root, error) || error) {
        std::cerr << "build-stage-workspace: not a readable directory: "
                  << root.string() << '\n';
        return std::nullopt;
    }

    gdspaces::SourceRegistry sources;
    if (!sources.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            "stage-workspace", absolute_root, true))) {
        std::cerr << "build-stage-workspace: failed to mount stage directory\n";
        return std::nullopt;
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
        return std::nullopt;
    }
    return payloads;
}

int run_list_stage_catalog(const std::filesystem::path& executable_path) {
    const auto loaded = load_stage_catalog(executable_path);
    if (!loaded.has_value()) {
        return 2;
    }

    const auto& catalog = loaded->catalog;
    const auto& universe = profiles::dmc3::wave2_stage_descriptor_universe();
    std::cout << "DMC3 Stage Catalog — Wave 2 reconciled coverage\n"
              << "artifact_sha256=" << loaded->artifact_sha256 << '\n'
              << "coverage=" << profiles::dmc3::to_string(catalog.coverage) << '\n'
              << "bank_a_entries=" << catalog.entries.size() << '\n'
              << "observed_descriptor_universe="
              << universe.observed_descriptor_count() << '\n'
              << "selector_entries=" << universe.selector_entry_count << '\n'
              << "full_selector_universe_complete="
              << (loaded->full_stage_universe_complete() ? "true" : "false") << '\n'
              << "table_id=" << catalog.table_id << '\n'
              << "repeated_exact_references="
              << catalog.repeated_references.size() << '\n';

    for (const auto& entry : catalog.entries) {
        std::cout << "bank_a_row=" << entry.row_index
                  << " id=" << entry.catalog_entry_id
                  << " numeric_stage_id=";
        if (entry.numeric_stage_id.has_value()) {
            std::cout << *entry.numeric_stage_id;
        } else {
            std::cout << "unresolved";
        }
        std::cout << " semantic_stage="
                  << (entry.semantic_stage_id.has_value()
                        ? *entry.semantic_stage_id : "unresolved")
                  << '\n';
        for (const auto& cell : entry.observation.cells) {
            std::cout << "  " << profiles::dmc3::to_string(cell.role)
                      << " kind16=";
            if (cell.kind16.has_value()) {
                std::cout << *cell.kind16;
            } else {
                std::cout << "unobserved";
            }
            std::cout << " path=" << cell.logical_path << '\n';
        }
    }

    if (!catalog.repeated_references.empty()) {
        std::cout << "Repeated exact EXE references (structural only):\n";
        for (const auto& repeated : catalog.repeated_references) {
            std::cout << "  " << repeated.logical_path << " <-";
            for (const auto& use : repeated.uses) {
                std::cout << " bank_a_row=" << use.row_index
                          << ":" << profiles::dmc3::to_string(use.role);
            }
            std::cout << '\n';
        }
    }
    return 0;
}

int run_build_stage_workspace(
    const std::filesystem::path& executable_path,
    std::uint32_t row_index,
    const std::filesystem::path& root,
    const std::optional<std::filesystem::path>& evidence_path) {
    const auto loaded = load_stage_catalog(executable_path);
    if (!loaded.has_value()) {
        return 2;
    }

    const auto* entry = loaded->catalog.find(row_index);
    if (entry == nullptr) {
        std::cerr << "build-stage-workspace: Bank-A row index is outside current compatibility coverage: "
                  << row_index << '\n';
        return 3;
    }

    auto payloads = load_workspace_payloads(root);
    if (!payloads.has_value()) {
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
        entry->resource_plan(),
        std::move(*payloads),
        evidence_packet.has_value() ? &*evidence_packet : nullptr);

    for (const auto& diagnostic : result.diagnostics) {
        std::cerr << '[' << gdspaces::to_string(diagnostic.severity) << "] "
                  << diagnostic.code << ": " << diagnostic.message << '\n';
    }

    const auto manifest = integration::stage_workspace_manifest_json(
        result.project, entry->catalog_entry_id);
    if (manifest.empty()) {
        std::cerr << "build-stage-workspace: no Stage Workspace Manifest was produced\n";
        return 6;
    }
    std::cout << manifest;
    return result.complete() ? 0 : 7;
}

} // namespace

void print_stage_help() {
    std::cout
        << "  list-stage-catalog <dmc3.exe>\n"
        << "                             Read Wave-2-corrected Bank-A coverage (110 of 189 observed descriptors)\n"
        << "  build-stage-workspace <dmc3.exe> <bank-a-row-index> <resource-root> [evidence.json]\n"
        << "                             Build from one exact Bank-A compatibility entry; selector-derived full coverage is pending\n";
}

int try_run_stage_command(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }

    const std::string_view command{argv[1]};
    if (command == "list-stage-catalog") {
        if (argc != 3) {
            std::cerr << "list-stage-catalog: usage: list-stage-catalog <dmc3.exe>\n";
            return 1;
        }
        return run_list_stage_catalog(std::filesystem::path{argv[2]});
    }

    if (command == "build-stage-workspace") {
        if (argc < 5 || argc > 6) {
            std::cerr
                << "build-stage-workspace: usage: build-stage-workspace <dmc3.exe> <bank-a-row-index> <resource-root> [evidence.json]\n";
            return 1;
        }
        const auto row_index = parse_row_index(argv[3]);
        if (!row_index.has_value()) {
            std::cerr << "build-stage-workspace: invalid Bank-A row index: "
                      << argv[3] << '\n';
            return 1;
        }
        const std::optional<std::filesystem::path> evidence_path =
            argc == 6
                ? std::optional<std::filesystem::path>{std::filesystem::path{argv[5]}}
                : std::nullopt;
        return run_build_stage_workspace(
            std::filesystem::path{argv[2]},
            *row_index,
            std::filesystem::path{argv[4]},
            evidence_path);
    }

    return -1;
}

} // namespace dmc::rengine::cli
