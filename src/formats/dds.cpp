#include "dmc_rengine/formats/dds.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::formats::dds {
namespace {

void add_diagnostic(
    ScanResult& result,
    ParseSeverity severity,
    std::string code,
    std::string message,
    std::uint64_t offset) {
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = severity,
        .code = std::move(code),
        .message = std::move(message),
        .offset = offset,
    });
}

[[nodiscard]] std::string reader_status_code(codecs::dds_bc::Status status) {
    return "dds.reader." + std::string(codecs::dds_bc::to_string(status));
}

[[nodiscard]] std::string profile_status_code(
    profiles::dmc3::Dmc3DdsStatus status) {
    return "dds.profile." + std::string(profiles::dmc3::to_string(status));
}

} // namespace

bool ScanResult::ok() const noexcept {
    return recognized && reader.ok() && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

ScanResult Reader::scan(std::span<const std::byte> bytes) {
    ScanResult result;
    const binary::Reader binary_reader(bytes);
    if (!binary_reader.matches(0U, "DDS ")) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dds.unrecognized",
            "The resource does not begin with the DDS magic.",
            0U);
        return result;
    }

    result.recognized = true;
    result.reader = codecs::dds_bc::parse(bytes);
    if (!result.reader.ok()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            reader_status_code(result.reader.status),
            result.reader.detail.empty()
                ? "The DDS resource is not a supported bounded 2D DXT1/DXT5 image."
                : std::string(result.reader.detail),
            0U);
        return result;
    }

    // A direct DDS resource must end at the DDS extent. Carrier/framing bytes
    // belong to TextureSlotFramingParser/PTX and must not be silently accepted
    // by the standalone reader.
    if (result.reader.document.total_size != bytes.size()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "dds.reader.trailing-bytes",
            "Standalone DDS contains bytes outside its bounded image extent.",
            result.reader.document.total_size);
        return result;
    }

    // Preserve the strict DMC3 full-chain profile as evidence/authoring
    // metadata, but do not make it a prerequisite for direct read support.
    result.profile = profiles::dmc3::Dmc3DdsProfile::parse(bytes);
    if (!result.profile.ok()) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            profile_status_code(result.profile.status),
            result.profile.detail.empty()
                ? "DDS is readable but does not satisfy the strict canonical DMC3 authoring profile."
                : std::string(result.profile.detail),
            0U);
    }
    return result;
}

} // namespace dmc::rengine::formats::dds
