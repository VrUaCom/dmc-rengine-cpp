#include "dmc_rengine/formats/dca.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::formats::dca {
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

} // namespace

bool ScanResult::ok() const noexcept {
    return recognized && std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

ScanResult RecordScanner::scan(std::span<const std::byte> bytes) {
    ScanResult result;
    const binary::Reader reader(bytes);
    if (!reader.matches(0U, std::string_view{"DCA\0", 4U})) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dca.unrecognized",
            "The resource does not begin with the DCA\\0 magic.",
            0U);
        return result;
    }
    result.recognized = true;

    if (bytes.size() < header_size) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "dca.truncated-header",
            "The DCA resource is shorter than the confirmed 0x10-byte header.",
            bytes.size());
        return result;
    }

    const auto payload_size = bytes.size() - header_size;
    const auto complete_records = payload_size / record_size;
    const auto trailing = payload_size % record_size;
    result.records.reserve(complete_records);
    for (std::size_t index = 0; index < complete_records; ++index) {
        result.records.push_back(Record{
            .offset = static_cast<std::uint64_t>(
                header_size + index * record_size),
            .size = record_size,
        });
    }

    if (trailing != 0U) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dca.trailing-bytes",
            "Bytes remain after the last complete 0x410-byte DCA record and are preserved as unknown.",
            static_cast<std::uint64_t>(
                header_size + complete_records * record_size));
    }
    if (result.records.empty()) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "dca.no-records",
            "The DCA magic and header were recognized, but no complete record is present.",
            header_size);
    }
    return result;
}

} // namespace dmc::rengine::formats::dca
