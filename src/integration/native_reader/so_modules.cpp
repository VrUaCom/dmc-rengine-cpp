#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/so.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"

#include <span>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

// The three SO payloads sit in adjacent slots of one archive and are read by
// three separate structural readers. Each reports geometry — block extents,
// record counts, the node a volume hangs off — and none reports semantics, so
// they publish to the binary inspector rather than to a scene.
template <typename Parse>
void analyze_so(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report,
    Parse&& parse) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    const auto parsed = parse(bytes);
    report.recognized = parsed.ok();
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, parsed.diagnostics);
}

void analyze_so_graph(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    analyze_so(project, session, report, [](auto bytes) {
        return formats::so::graph::parse(bytes);
    });
}

void analyze_so_volume(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    analyze_so(project, session, report, [](auto bytes) {
        return formats::so::volume_table::parse(bytes);
    });
}

void analyze_so_link(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    analyze_so(project, session, report, [](auto bytes) {
        return formats::so::link_table::parse(bytes);
    });
}

} // namespace

NativeReaderModule so_graph() {
    return NativeReaderModule{
        .parser_id = "formats.so-graph-structural-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_so_graph,
    };
}

NativeReaderModule so_volume() {
    return NativeReaderModule{
        .parser_id = "formats.so-volume-structural-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_so_volume,
    };
}

NativeReaderModule so_link() {
    return NativeReaderModule{
        .parser_id = "formats.so-link-structural-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_so_link,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
