#include "dmc_rengine/formats/hits.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <string>

namespace dmc::rengine::formats::hits {
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

    if (!reader.matches(0U, "HITS$")) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "hits.unrecognized",
            "The resource does not begin with the HITS$ magic.",
            0U);
        return result;
    }
    result.recognized = true;

    constexpr std::size_t magic_size = 5U;
    std::size_t offset = magic_size;
    while (offset < bytes.size()) {
        const auto marker = reader.u32_le(offset);
        if (!marker.has_value()) {
            break;
        }

        if (*marker != record_marker) {
            ++offset;
            continue;
        }

        if (!reader.contains(offset, record_size)) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.truncated_record",
                "A HITS record marker is present but the 56-byte record is truncated.",
                static_cast<std::uint64_t>(offset));
            break;
        }

        Record record{
            .offset = static_cast<std::uint64_t>(offset),
            .marker = *marker,
            .values = {},
        };

        auto complete = true;
        for (std::size_t index = 0; index < value_count; ++index) {
            const auto value = reader.f32_le(offset + 4U + index * 4U);
            if (!value.has_value()) {
                complete = false;
                break;
            }
            record.values[index] = *value;
        }

        if (!complete) {
            add_diagnostic(
                result,
                ParseSeverity::error,
                "hits.invalid_record",
                "A HITS record could not be decoded completely.",
                static_cast<std::uint64_t>(offset));
            break;
        }

        result.records.push_back(record);
        offset += record_size;
    }

    if (result.records.empty()) {
        add_diagnostic(
            result,
            ParseSeverity::warning,
            "hits.no_records",
            "The HITS$ magic was recognized, but no complete confirmed record was found.",
            magic_size);
    }

    return result;
}

} // namespace dmc::rengine::formats::hits
