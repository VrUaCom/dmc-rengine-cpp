#include "hits_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/hits/spatial_compare.hpp"
#include "dmc_rengine/hits/spatial_match.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::cli {
namespace {

using gdspaces::LocalDirectorySource;
using gdspaces::ResourceId;
using gdspaces::ResourcePayload;
using gdspaces::SourceRegistry;

[[nodiscard]] std::optional<ResourcePayload> load_local_file(
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

    SourceRegistry registry;
    const auto source_id_copy = source_id;
    if (!registry.mount(std::make_unique<LocalDirectorySource>(
            std::move(source_id), absolute.parent_path(), false))) {
        std::cerr << operation << ": failed to mount parent directory\n";
        return std::nullopt;
    }

    const ResourceId resource{
        .source_id = source_id_copy,
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };
    auto payload = registry.read(resource);
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

void print_parse_diagnostics(
    std::string_view side,
    const formats::hits::ScanResult& scan) {
    for (const auto& diagnostic : scan.diagnostics) {
        std::cerr << side << " ["
                  << formats::to_string(diagnostic.severity)
                  << "] " << diagnostic.code << ": "
                  << diagnostic.message
                  << " offset=" << diagnostic.offset << '\n';
    }
}

int run_compare_hits_spatial(
    const std::filesystem::path& original_path,
    const std::filesystem::path& candidate_path,
    const std::filesystem::path& report_path) {
    constexpr std::string_view operation = "compare-hits-spatial";
    const auto original_payload = load_local_file(
        original_path, "hits-original", operation);
    const auto candidate_payload = load_local_file(
        candidate_path, "hits-candidate", operation);
    if (!original_payload.has_value() || !candidate_payload.has_value()) {
        return 2;
    }

    const auto original_bytes = std::span<const std::byte>{
        original_payload->bytes};
    const auto candidate_bytes = std::span<const std::byte>{
        candidate_payload->bytes};
    const auto original_sha256 = core::Sha256::compute(
        original_bytes).hex();
    const auto candidate_sha256 = core::Sha256::compute(
        candidate_bytes).hex();
    const auto original_scan = formats::hits::RecordScanner::scan(
        original_bytes);
    const auto candidate_scan = formats::hits::RecordScanner::scan(
        candidate_bytes);
    if (!original_scan.ok() || !candidate_scan.ok()) {
        print_parse_diagnostics("original", original_scan);
        print_parse_diagnostics("candidate", candidate_scan);
        return 3;
    }

    const auto matched = hits::spatial_match::match_unique_geometry(
        original_scan, candidate_scan);
    if (!matched.ok()) {
        std::cerr
            << "Automatic geometry mapping failed: "
            << hits::spatial_match::to_string(matched.status) << '\n';
        for (const auto& diagnostic : matched.diagnostics) {
            std::cerr << "- ["
                      << formats::to_string(diagnostic.severity)
                      << "] " << diagnostic.code << ": "
                      << diagnostic.message
                      << " offset=" << diagnostic.offset << '\n';
        }
        return 4;
    }

    const auto report = hits::spatial_compare::compare(
        original_scan,
        candidate_scan,
        matched.mappings,
        "hits-spatial-corpus-comparison",
        "sha256:" + original_sha256,
        "sha256:" + candidate_sha256,
        "unspecified-external-candidate");
    if (!report.comparable()) {
        std::cerr << "Spatial comparison failed: "
                  << hits::spatial_compare::to_string(report.status) << '\n';
        for (const auto& diagnostic : report.diagnostics) {
            std::cerr << "- ["
                      << formats::to_string(diagnostic.severity)
                      << "] " << diagnostic.code << ": "
                      << diagnostic.message
                      << " offset=" << diagnostic.offset << '\n';
        }
        return 5;
    }

    const auto json = hits::spatial_compare::report_json(report);
    std::error_code error;
    const auto absolute_report = std::filesystem::absolute(
        report_path, error);
    if (error) {
        std::cerr << operation << ": invalid report path: "
                  << report_path.string() << '\n';
        return 6;
    }
    std::ofstream output(
        absolute_report,
        std::ios::binary | std::ios::trunc);
    if (!output) {
        std::cerr << operation << ": cannot create report: "
                  << absolute_report.string() << '\n';
        return 6;
    }
    output.write(
        json.data(),
        static_cast<std::streamsize>(json.size()));
    output.close();
    if (!output) {
        std::cerr << operation << ": failed to write report: "
                  << absolute_report.string() << '\n';
        return 6;
    }

    std::cout
        << "HITS spatial comparison: "
        << (report.exact() ? "exact" : "different") << '\n'
        << "Original SHA-256: " << original_sha256 << '\n'
        << "Candidate SHA-256: " << candidate_sha256 << '\n'
        << "Triangles mapped: " << matched.mappings.size() << '\n'
        << "Original references: "
        << report.metrics.original_reference_count << '\n'
        << "Candidate references: "
        << report.metrics.candidate_reference_count << '\n'
        << "Shared references: "
        << report.metrics.shared_reference_count << '\n'
        << "Missing references: "
        << report.metrics.missing_reference_count << '\n'
        << "Extra references: "
        << report.metrics.extra_reference_count << '\n'
        << "Exact cells: " << report.metrics.exact_cell_count
        << '/' << report.cells.size() << '\n'
        << "Exact surfaces: " << report.metrics.exact_surface_count
        << '/' << report.surfaces.size() << '\n'
        << "Precision: " << report.metrics.precision << '\n'
        << "Recall: " << report.metrics.recall << '\n'
        << "Jaccard: " << report.metrics.jaccard << '\n'
        << "Report: " << absolute_report.string() << '\n';
    return 0;
}

} // namespace

void print_hits_help() {
    std::cout
        << "  compare-hits-spatial <original> <candidate> [report.json]\n"
        << "                            Compare bit-exact unique geometry and spatial ownership\n";
}

int try_run_hits_command(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }
    const std::string_view command{argv[1]};
    if (command != "compare-hits-spatial") {
        return -1;
    }
    if (argc < 4) {
        std::cerr
            << "compare-hits-spatial: expected <original> <candidate> [report.json]\n";
        return 1;
    }

    const std::filesystem::path candidate_path{argv[3]};
    const auto report_path = argc >= 5
        ? std::filesystem::path{argv[4]}
        : std::filesystem::path{
              candidate_path.string() +
              ".hits-spatial-comparison.json"};
    return run_compare_hits_spatial(
        std::filesystem::path{argv[2]},
        candidate_path,
        report_path);
}

} // namespace dmc::rengine::cli
