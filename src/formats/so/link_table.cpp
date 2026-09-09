#include "dmc_rengine/formats/so/link_table.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>

namespace dmc::rengine::formats::so::link_table {
namespace {

[[nodiscard]] bool has_error(const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const ParseDiagnostic& diagnostic) {
        return diagnostic.severity == ParseSeverity::error;
    });
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

// A size that divides by four is not an identity. Recognition is claimed only
// once the two invariants the corpus actually shows are present, because
// claiming it on the size alone made this reader accept 305 of a 306-payload
// extraction — and a reader that recognizes everything tells a classifier
// nothing. Neither invariant is sufficient alone: the leading word leaves one
// effect record and the reserved byte leaves the ten effect-M companions.
bool recognizes(const std::span<const std::byte> bytes) noexcept {
    if (bytes.size() < leading_word_size + record_size ||
        (bytes.size() % record_size) != 0U) {
        return false;
    }

    const binary::Reader reader(bytes);
    const auto leading = reader.u32_le(0U);
    if (!leading || *leading != leading_word_value) {
        return false;
    }
    for (std::size_t offset = 0U; offset < bytes.size(); offset += record_size) {
        const auto reserved = reader.u8(offset + reserved_field_offset);
        if (!reserved || *reserved != 0U) {
            return false;
        }
    }
    return true;
}

ParseResult parse(const std::span<const std::byte> bytes) {
    ParseResult result;
    if (!recognizes(bytes)) {
        return result;
    }

    const binary::Reader reader(bytes);
    result.recognized = true;
    result.records.reserve(bytes.size() / record_size);
    for (std::size_t offset = 0U; offset < bytes.size(); offset += record_size) {
        const auto field0 = reader.u8(offset);
        const auto field1 = reader.u8(offset + 1U);
        const auto field2 = reader.u8(offset + 2U);
        const auto field3 = reader.u8(offset + 3U);
        if (!field0 || !field1 || !field2 || !field3) {
            result.diagnostics.push_back(ParseDiagnostic{
                ParseSeverity::error,
                "so.link_table.record_truncated",
                "SO compact link-table record is truncated",
                static_cast<std::uint64_t>(offset),
            });
            return result;
        }
        result.records.push_back(Record{*field0, *field1, *field2, *field3});
    }
    return result;
}

} // namespace dmc::rengine::formats::so::link_table
