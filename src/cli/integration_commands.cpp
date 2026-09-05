#include "integration_commands.hpp"

#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/integration/tool_registry.hpp"
#include "dmc_rengine/integration/workspace_manifest.hpp"

#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] std::optional<gdspaces::ResourcePayload> load_local_file(
    const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << "inspect-workspace: not a readable file: "
                  << input_path.string() << '\n';
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << "inspect-workspace: file size is unsupported\n";
        return std::nullopt;
    }

    gdspaces::SourceRegistry registry;
    constexpr std::string_view source_id = "workspace-inspect";
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string(source_id), absolute.parent_path(), false))) {
        std::cerr << "inspect-workspace: failed to mount parent directory\n";
        return std::nullopt;
    }

    const gdspaces::ResourceId resource{
        .source_id = std::string(source_id),
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };

    auto payload = registry.read(resource);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << "inspect-workspace: GDSpaces could not read the resource\n";
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

} // namespace

int run_list_tools() {
    const integration::ToolRegistry registry;
    for (const auto& tool : registry.tools()) {
        std::cout << gdspaces::to_string(tool.target)
                  << " | " << tool.product_name
                  << " | " << tool.lore_name << '\n';
        for (const auto capability : tool.capabilities) {
            std::cout << "  - " << integration::to_string(capability) << '\n';
        }
    }
    return 0;
}

int run_list_formats() {
    const integration::FormatIntegrationRegistry registry;
    for (const auto& format : registry.formats()) {
        std::cout << format.format
                  << " maturity=" << integration::to_string(format.maturity)
                  << " write=" << integration::to_string(format.write_policy)
                  << " parser="
                  << (format.parser_id.empty() ? "pending" : format.parser_id)
                  << " binary-adapter="
                  << (format.binary_adapter ? "yes" : "no");
        if (format.stage_category.has_value()) {
            std::cout << " stage-category="
                      << gdspaces::to_string(*format.stage_category);
        }
        std::cout << '\n';
        for (const auto& limitation : format.limitations) {
            std::cout << "  [limit] " << limitation << '\n';
        }
    }
    return 0;
}

int run_integration_status() {
    const integration::ToolRegistry tools;
    const integration::FormatIntegrationRegistry formats;

    std::size_t structural = 0U;
    std::size_t semantic = 0U;
    std::size_t editable = 0U;
    std::size_t exportable = 0U;
    std::size_t read_only = 0U;
    std::size_t working_copy = 0U;
    std::size_t guarded_export = 0U;

    for (const auto& format : formats.formats()) {
        switch (format.maturity) {
        case integration::IntegrationMaturity::recognized: break;
        case integration::IntegrationMaturity::structural: ++structural; break;
        case integration::IntegrationMaturity::semantic: ++semantic; break;
        case integration::IntegrationMaturity::editable: ++editable; break;
        case integration::IntegrationMaturity::exportable: ++exportable; break;
        }
        switch (format.write_policy) {
        case integration::ResourceWritePolicy::read_only: ++read_only; break;
        case integration::ResourceWritePolicy::working_copy_only:
            ++working_copy;
            break;
        case integration::ResourceWritePolicy::guarded_export:
            ++guarded_export;
            break;
        }
    }

    std::cout
        << "DMC Rengine integration status\n"
        << "Tools: " << tools.tools().size() << '\n'
        << "Formats: " << formats.formats().size() << '\n'
        << "Structural formats: " << structural << '\n'
        << "Semantic formats: " << semantic << '\n'
        << "Editable formats: " << editable << '\n'
        << "Exportable formats: " << exportable << '\n'
        << "Read-only policies: " << read_only << '\n'
        << "Working-copy-only policies: " << working_copy << '\n'
        << "Guarded-export policies: " << guarded_export << '\n'
        << "Invariant: GDSpaces owns all source access and canonical identity.\n"
        << "Invariant: resource readers dispatch through NativeReaderModuleRegistry.\n"
        << "Invariant: PAC/PNST dispatch through ContainerParserRegistry.\n"
        << "Invariant: NBZ remains a source/materialization adapter.\n";
    return 0;
}

int run_inspect_workspace(
    const std::filesystem::path& path,
    bool stage_context,
    bool menu_context) {
    auto payload = load_local_file(path);
    if (!payload.has_value()) {
        return 2;
    }

    integration::ProjectWorkspace project;
    const auto resource_id = payload->resource.id;
    if (!project.create_session(
            std::move(*payload),
            integration::WorkspaceContext{
                .stage_context = stage_context,
                .menu_context = menu_context,
                .evidence_context = true,
            })) {
        std::cerr << "inspect-workspace: workspace creation failed\n";
        return 3;
    }

    const auto analysis = integration::ResourceAnalyzer::analyze(project, resource_id);
    if (analysis.parser_available && !analysis.ok()) {
        for (const auto& diagnostic : analysis.diagnostics) {
            std::cerr << "  ["
                      << gdspaces::to_string(diagnostic.severity)
                      << "] " << diagnostic.code << ": "
                      << diagnostic.message << '\n';
        }
    }

    const auto* workspace = project.find_session(resource_id);
    if (workspace == nullptr) {
        std::cerr << "inspect-workspace: canonical session disappeared after analysis\n";
        return 4;
    }

    const auto manifest = integration::workspace_manifest_json(*workspace);
    if (manifest.empty()) {
        std::cerr << "inspect-workspace: manifest generation failed\n";
        return 5;
    }
    std::cout << manifest;
    return 0;
}

} // namespace dmc::rengine::cli