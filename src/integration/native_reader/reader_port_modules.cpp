// Native Reader modules for the formats whose readers came over from the DMC
// Native Reader reverses: EFM effect models (the MOD document layout behind
// an "EFM " tag), CLT cloth definitions, TSC UV-scroll tables and EVT event
// scripts, and the nameless character tables (collision shapes, motion
// scripts) read through resource_structure. Each module only reads; identity
// stays with the classifier.
#include "dmc_rengine/integration/native_reader_modules.hpp"

#include "dmc_rengine/formats/evt.hpp"
#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/integration/native_reader_support.hpp"
#include "dmc_rengine/integration/resource_structure.hpp"
#include "dmc_rengine/profiles/dmc3/cloth_chain.hpp"
#include "dmc_rengine/profiles/dmc3/uv_scroll.hpp"

#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::integration::native_reader_modules {
namespace {

[[nodiscard]] std::string_view as_text(std::span<const std::byte> bytes) noexcept {
    return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

void info(ResourceAnalysisReport& report, std::string code, std::string message) {
    native_reader_support::add_report_diagnostic(
        report, gdspaces::DiagnosticSeverity::info, std::move(code), std::move(message));
}

// EFM: the post-load handler 0x1402F7A90 and the ordinary model loader treat
// it as a MOD document; only the leading tag differs. The canonical MOD reader
// keys on "MOD ", so it reads a copy with the tag swapped.
void analyze_efm(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto bytes = std::span<const std::byte>{session.source_payload().bytes};
    if (bytes.size() < 4U || std::memcmp(bytes.data(), "EFM ", 4U) != 0) {
        report.recognized = false;
        native_reader_support::add_report_diagnostic(
            report, gdspaces::DiagnosticSeverity::error, "efm-tag", "The payload does not open with the EFM tag.");
        return;
    }
    std::vector<std::byte> copy(bytes.begin(), bytes.end());
    std::memcpy(copy.data(), "MOD ", 4U);
    const auto parsed = formats::mod::Parser::parse(copy);
    report.recognized = parsed.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, parsed.diagnostics);
    if (parsed.recognized) {
        info(report, "efm-models",
             std::to_string(parsed.document.outer_models.size()) +
                 " model(s) on the MOD document layout (read with the tag swapped).");
    }
}

void analyze_clt(
    ProjectWorkspace&,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto text = as_text(session.source_payload().bytes);
    const bool identity = profiles::dmc3::motion::looks_like_clt(text);
    const auto chains = identity ? profiles::dmc3::motion::parse_clt(text)
                                 : std::vector<profiles::dmc3::motion::ClothParams>{};
    report.recognized = identity && !chains.empty();
    if (!report.recognized) {
        native_reader_support::add_report_diagnostic(
            report, gdspaces::DiagnosticSeverity::error, "clt-blocks",
            "No ClothNo block could be read (parser 0x1402CA345 / 0x1402CA42A grammar).");
        return;
    }
    std::size_t bones = 0U;
    for (const auto& chain : chains) bones += chain.bones.size();
    info(report, "clt-chains",
         std::to_string(chains.size()) + " cloth chain(s), " + std::to_string(bones) +
             " bone(s); gravity, spring, stiffness, wind and damping per chain.");
}

void analyze_tsc(
    ProjectWorkspace&,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto text = as_text(session.source_payload().bytes);
    const bool identity = profiles::dmc3::motion::looks_like_tsc(text);
    const auto records = identity ? profiles::dmc3::motion::parse_tsc(text)
                                  : std::vector<profiles::dmc3::motion::ScrollRecord>{};
    report.recognized = identity;
    if (!identity) {
        native_reader_support::add_report_diagnostic(
            report, gdspaces::DiagnosticSeverity::error, "tsc-tag", "The text does not open with the .TSC tag.");
        return;
    }
    std::string types;
    for (const auto& record : records) {
        if (!types.empty()) types += ", ";
        types += std::to_string(record.type);
    }
    info(report, "tsc-records",
         std::to_string(records.size()) + " scroll record(s) up to '$'" +
             (types.empty() ? std::string{} : " (ScrlType " + types + ")") + ".");
}

void analyze_evt(
    ProjectWorkspace& project,
    const ResourceWorkspaceSession& session,
    ResourceAnalysisReport& report) {
    const auto parsed = formats::evt::Parser::parse(std::span<const std::byte>{session.source_payload().bytes});
    report.recognized = parsed.recognized;
    native_reader_support::append_parser_diagnostics(
        project, report, session.resource().id, parsed.diagnostics);
    if (parsed.recognized) {
        info(report, "evt-commands",
             std::to_string(parsed.document.commands.size()) + " command(s), " +
                 std::to_string(parsed.document.stream_offsets.size()) + " stream(s).");
    }
}

// Formats whose only reader is the structure view: the module reports what
// the view reads and declines what it declines.
void analyze_structure(std::string_view format, std::string_view code,
                       const ResourceWorkspaceSession& session, ResourceAnalysisReport& report) {
    std::string detail;
    const auto view = read_structure(format, std::span<const std::byte>{session.source_payload().bytes},
                                     session.resource().id.logical_path, detail);
    report.recognized = view.has_value();
    if (!view) {
        native_reader_support::add_report_diagnostic(
            report, gdspaces::DiagnosticSeverity::error, std::string{code}, std::move(detail));
        return;
    }
    info(report, std::string{code}, view->summary);
}

void analyze_collision_shapes(ProjectWorkspace&, const ResourceWorkspaceSession& session,
                              ResourceAnalysisReport& report) {
    analyze_structure("collision-shapes", "collision-shapes", session, report);
}

void analyze_motion_script(ProjectWorkspace&, const ResourceWorkspaceSession& session,
                           ResourceAnalysisReport& report) {
    analyze_structure("motion-script", "motion-script-banks", session, report);
}

} // namespace

NativeReaderModule efm() {
    return NativeReaderModule{
        .parser_id = "formats.efm-mod-layout-v1",
        .consumer = gdspaces::ToolTarget::modviz_scene,
        .link_format_evidence = true,
        .analyze = &analyze_efm,
    };
}

NativeReaderModule clt() {
    return NativeReaderModule{
        .parser_id = "profiles.dmc3.clt-cloth-chain-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_clt,
    };
}

NativeReaderModule tsc() {
    return NativeReaderModule{
        .parser_id = "profiles.dmc3.tsc-uv-scroll-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_tsc,
    };
}

NativeReaderModule evt() {
    return NativeReaderModule{
        .parser_id = "formats.evt-structural-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_evt,
    };
}

NativeReaderModule collision_shapes() {
    return NativeReaderModule{
        .parser_id = "profiles.dmc3.collision-shapes-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_collision_shapes,
    };
}

NativeReaderModule motion_script() {
    return NativeReaderModule{
        .parser_id = "profiles.dmc3.motion-script-v1",
        .consumer = gdspaces::ToolTarget::binary_inspector,
        .link_format_evidence = true,
        .analyze = &analyze_motion_script,
    };
}

} // namespace dmc::rengine::integration::native_reader_modules
