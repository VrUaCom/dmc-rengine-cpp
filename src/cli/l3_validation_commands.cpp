#include "l3_validation_commands.hpp"

#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/validation/l3_lifecycle_trace.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::cli {
namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace validation = dmc::rengine::validation;

[[nodiscard]] std::optional<std::string> read_trace_text(
    const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        std::cerr << "validate-l3-lifecycle: not a readable file: "
                  << input_path.string() << '\n';
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << "validate-l3-lifecycle: file size is unsupported\n";
        return std::nullopt;
    }

    gdspaces::SourceRegistry registry;
    constexpr std::string_view source_id = "l3-lifecycle-validation";
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, absolute.parent_path(), false))) {
        std::cerr << "validate-l3-lifecycle: failed to mount trace directory through GDSpaces\n";
        return std::nullopt;
    }

    const gdspaces::ResourceId id{
        .source_id = std::string{source_id},
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };
    const auto payload = registry.read(id);
    if (!payload.has_value() || !payload->readable()) {
        std::cerr << "validate-l3-lifecycle: GDSpaces could not read the trace\n";
        return std::nullopt;
    }

    std::string text;
    if (!payload->bytes.empty()) {
        text.assign(
            reinterpret_cast<const char*>(payload->bytes.data()),
            payload->bytes.size());
    }
    return text;
}

int run_validate_l3_lifecycle(
    const std::filesystem::path& path,
    bool require_promotable) {
    const auto text = read_trace_text(path);
    if (!text.has_value()) return 2;

    const auto result = validation::l3_lifecycle_trace_from_json(*text);
    if (!result.ok()) {
        std::cerr << "Layer-3 lifecycle trace validation failed:\n";
        for (const auto& diagnostic : result.diagnostics) {
            std::cerr << "- " << diagnostic.path << ": "
                      << diagnostic.message << '\n';
        }
        return 3;
    }

    const auto& trace = *result.trace;
    std::cout << "Layer-3 lifecycle trace: valid\n"
              << "Schema: " << trace.schema << '\n'
              << "Scope: " << validation::to_string(trace.scope) << '\n'
              << "Status: " << validation::to_string(trace.status) << '\n'
              << "Run: " << trace.run.id << '\n'
              << "Authority role: " << trace.authority.role << '\n'
              << "Executable SHA-256: " << trace.authority.exe_sha256 << '\n'
              << "Resource: " << trace.resource.logical_identity << '\n'
              << "Selected provider: "
              << trace.resource.selected_provider_identity << '\n'
              << "Materialized SHA-256: "
              << trace.resource.materialized_sha256 << '\n'
              << "Events: " << trace.events.size() << '\n'
              << "Original-process claim: "
              << (trace.run.original_process ? "yes" : "no") << '\n'
              << "Promotion content candidate: "
              << (result.promotion_eligible.content_candidate() ? "yes" : "no")
              << '\n'
              << "Trusted origin bound: no\n"
              << "Promotion eligible: "
              << (result.promotion_eligible.eligible() ? "yes" : "no") << '\n';

    if (require_promotable && !result.promotion_eligible.eligible()) {
        std::cerr
            << "validate-l3-lifecycle: manual schema-v1 JSON import cannot satisfy "
               "the Level-E promotion boundary because original-process origin is "
               "self-asserted; a trusted instrumentation/publisher binding is required\n";
        return 4;
    }
    return 0;
}

} // namespace

void print_l3_validation_help() {
    std::cout
        << "  validate-l3-lifecycle <trace.json> [--require-promotable]\n"
        << "                            Validate a GDSpaces L3 original-runtime trace\n";
}

int try_run_l3_validation_command(int argc, char** argv) {
    if (argc < 2 || std::string_view{argv[1]} != "validate-l3-lifecycle") {
        return -1;
    }
    if (argc < 3) {
        std::cerr << "validate-l3-lifecycle: missing trace path\n";
        return 1;
    }

    auto require_promotable = false;
    if (argc == 4) {
        if (std::string_view{argv[3]} != "--require-promotable") {
            std::cerr << "validate-l3-lifecycle: unknown option: " << argv[3] << '\n';
            return 1;
        }
        require_promotable = true;
    } else if (argc > 4) {
        std::cerr << "validate-l3-lifecycle: too many arguments\n";
        return 1;
    }

    return run_validate_l3_lifecycle(
        std::filesystem::path{argv[2]}, require_promotable);
}

} // namespace dmc::rengine::cli
