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

[[nodiscard]] std::string status_code(
    profiles::dmc3::Dmc3DdsStatus status) {
    return "dds." + std::string(profiles::dmc3::to_string(status));
}

} // namespace

bool ScanResult::ok() const noexcept {
    return recognized && profile.ok() && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

ScanResult Reader::scan(std::span<const std::byte> bytes) {
    ScanResult result;
    const binary::Reader reader(bytes);
    if (!reader.matches(0U, "DDS ")) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dds.unrecognized",
            "The resource does not begin with the DDS magic.",
            0U);
        return result;
    }
    result.recognized = true;
    result.profile = profiles::dmc3::Dmc3DdsProfile::parse(bytes);
    if (!result.profile.ok()) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            status_code(result.profile.status),
            result.profile.detail.empty()
                ? "The DDS resource does not satisfy the confirmed DMC3 HD DDS profile."
                : std::string(result.profile.detail),
            0U);
    }
    return result;
}

} // namespace dmc::rengine::formats::dds
