#include "dmc_rengine/formats/so/volume_table.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>

namespace dmc::rengine::formats::so::volume_table {
namespace {

[[nodiscard]] bool has_error(const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const ParseDiagnostic& diagnostic) {
        return diagnostic.severity == ParseSeverity::error;
    });
}

[[nodiscard]] bool read_vec4(const binary::Reader& reader, const std::size_t offset, Vec4& out) noexcept {
    const auto x = reader.f32_le(offset);
    const auto y = reader.f32_le(offset + 4U);
    const auto z = reader.f32_le(offset + 8U);
    const auto w = reader.f32_le(offset + 12U);
    if (!x || !y || !z || !w) {
        return false;
    }
    out = Vec4{*x, *y, *z, *w};
    return true;
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult parse(const std::span<const std::byte> bytes) {
    ParseResult result;
    if (bytes.empty() || (bytes.size() % record_size) != 0U) {
        return result;
    }

    result.recognized = true;
    const binary::Reader reader(bytes);
    result.records.reserve(bytes.size() / record_size);
    for (std::size_t offset = 0U; offset < bytes.size(); offset += record_size) {
        const auto type = reader.u32_le(offset);
        const auto prefix = reader.slice(offset + 4U, 12U);
        Record record;
        if (!type || !prefix ||
            !read_vec4(reader, offset + 0x10U, record.vector0) ||
            !read_vec4(reader, offset + 0x20U, record.vector1) ||
            !read_vec4(reader, offset + 0x30U, record.vector2) ||
            !read_vec4(reader, offset + 0x40U, record.vector3)) {
            result.diagnostics.push_back(ParseDiagnostic{
                ParseSeverity::error,
                "so.volume_table.record_truncated",
                "SO 0x50 volume record is truncated",
                static_cast<std::uint64_t>(offset),
            });
            return result;
        }

        record.type = *type;
        for (std::size_t index = 0U; index < record.prefix_unknown.size(); ++index) {
            record.prefix_unknown[index] = (*prefix)[index];
        }
        result.records.push_back(record);
    }
    return result;
}

} // namespace dmc::rengine::formats::so::volume_table
