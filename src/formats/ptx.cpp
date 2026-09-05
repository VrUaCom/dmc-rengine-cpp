#include "dmc_rengine/formats/ptx.hpp"

#include "dmc_rengine/profiles/dmc3/texture_slot_expander.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::formats::ptx {
namespace {

void add_diagnostic(
    ScanResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset = 0U) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

[[nodiscard]] std::string status_code(
    profiles::dmc3::TextureSlotFramingStatus status) {
    return "ptx." + std::string(profiles::dmc3::to_string(status));
}

} // namespace

bool ScanResult::ok() const noexcept {
    return recognized && framing.ok() &&
        framing.document.kind ==
            profiles::dmc3::TextureSlotFramingKind::texture_bundle &&
        std::none_of(
            diagnostics.begin(), diagnostics.end(),
            [](const ParseDiagnostic& diagnostic) {
                return diagnostic.severity == ParseSeverity::error;
            });
}

ScanResult Reader::scan(
    std::span<const std::byte> bytes,
    profiles::dmc3::TextureSlotFramingSafety safety) {
    ScanResult result;
    result.framing = profiles::dmc3::TextureSlotFramingParser::parse(
        bytes, safety);

    if (!result.framing.ok()) {
        if (result.framing.status ==
            profiles::dmc3::TextureSlotFramingStatus::not_recognized) {
            add_diagnostic(
                result,
                ParseSeverity::warning,
                "ptx.unrecognized",
                "The resource does not expose the evidenced DMC3 PTX texture-bundle framing.");
            return result;
        }

        // A non-recognition status after entering the framing parser is a
        // structural failure, not proof of a valid PTX identity. Keep the
        // reader fail-closed while surfacing the exact evidence-backed reason.
        add_diagnostic(
            result,
            ParseSeverity::error,
            status_code(result.framing.status),
            result.framing.detail.empty()
                ? "The texture bundle failed structural PTX validation."
                : std::string(result.framing.detail));
        return result;
    }

    if (result.framing.document.kind !=
        profiles::dmc3::TextureSlotFramingKind::texture_bundle) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "ptx.wrapped-dds-not-bundle",
            "The resource is a validated descriptor-plus-DDS texture slot, not a PTX texture bundle.");
        return result;
    }

    result.recognized = true;
    return result;
}

gdspaces::ContainerExpansion Reader::expand_dds_children(
    const gdspaces::ResourcePayload& parent,
    profiles::dmc3::TextureSlotFramingSafety safety) {
    auto expansion = profiles::dmc3::TextureSlotExpander::expand(
        parent, safety);

    // The canonical backend also supports a single wrapped DDS slot. The PTX
    // module is intentionally narrower: PTX means the validated bundle framing.
    const auto scan = Reader::scan(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        safety);
    if (!scan.ok()) {
        expansion.children.clear();
    }
    expansion.parser_format = "PTX";
    return expansion;
}

} // namespace dmc::rengine::formats::ptx
