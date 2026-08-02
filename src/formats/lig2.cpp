#include "dmc_rengine/formats/lig2.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::formats::lig2 {
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
    if (bytes.size() < header_size) {
        add_diagnostic(
            result,
            ParseSeverity::error,
            "lig2.truncated-header",
            "The candidate LIG2 resource is shorter than the confirmed 0x20-byte header.",
            bytes.size());
        return result;
    }
    result.recognized = true;

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
            "lig2.trailing-bytes",
            "Bytes remain after the last complete 0x30-byte LIG2 record and are preserved as unknown.",
            static_cast<std::uint64_t>(
                header_size + complete_records * record_size));
    }
    if (complete_records != expected_dmc3_record_count) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "lig2.unexpected-record-count",
            "The confirmed DMC3 corpus contains 48 records; this candidate has a different complete-record count and remains readable but not corpus-confirmed.",
            header_size);
    }
    return result;
}

} // namespace dmc::rengine::formats::lig2
