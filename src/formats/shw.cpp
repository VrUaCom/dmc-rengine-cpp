#include "dmc_rengine/formats/shw.hpp"

#include "dmc_rengine/binary/reader.hpp"
#include "dmc_rengine/profiles/dmc3/shw_contract.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

namespace dmc::rengine::formats::shw {
namespace {

using Contract = profiles::dmc3::ShwContract;

[[nodiscard]] bool has_error(
    const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [](const ParseDiagnostic& diagnostic) {
            return diagnostic.severity == ParseSeverity::error;
        });
}

void diag(ParseResult& out,
          ParseSeverity severity,
          std::string code,
          std::string message,
          std::uint64_t offset) {
    out.diagnostics.push_back(ParseDiagnostic{
        severity,
        std::move(code),
        std::move(message),
        offset,
    });
}

[[nodiscard]] bool span_in_bounds(
    const binary::Reader& reader,
    std::uint64_t offset,
    std::uint64_t length) noexcept {
    const auto total = static_cast<std::uint64_t>(reader.size());
    return offset <= total && length <= total - offset;
}

[[nodiscard]] bool aligned_16(std::uint64_t value) noexcept {
    return (value & 0x0FU) == 0U;
}

[[nodiscard]] std::uint64_t align_16(std::uint64_t value) noexcept {
    return (value + 0x0FU) & ~std::uint64_t{0x0FU};
}

[[nodiscard]] std::size_t shared_vertex_count(
    const Triangle& lhs,
    const Triangle& rhs) noexcept {
    std::size_t shared = 0U;
    for (const auto left : lhs.vertices) {
        if (std::find(rhs.vertices.begin(), rhs.vertices.end(), left) !=
            rhs.vertices.end()) {
            ++shared;
        }
    }
    return shared;
}

void validate_closed_hull_semantics(ParseResult& out, const Hull& hull) {
    if (hull.vertex_count >= 4U) {
        const auto expected =
            2U * static_cast<std::uint32_t>(hull.vertex_count) - 4U;
        if (static_cast<std::uint32_t>(hull.triangle_count) != expected) {
            diag(out, ParseSeverity::warning,
                 "shw.nonclosed-euler-count",
                 "The bound DMC3 SHW payload uses closed triangulated hulls with triangle_count = 2*vertex_count-4; this hull differs and is preserved for variant review.",
                 hull.record_offset + Contract::triangle_count_offset);
        }
    }

    if (hull.triangles.size() != hull.adjacency.size()) return;

    for (std::size_t index = 0U; index < hull.triangles.size(); ++index) {
        std::vector<std::uint16_t> expected;
        for (std::size_t other = 0U; other < hull.triangles.size(); ++other) {
            if (other == index) continue;
            if (shared_vertex_count(hull.triangles[index], hull.triangles[other]) == 2U) {
                expected.push_back(static_cast<std::uint16_t>(other));
            }
        }

        auto recorded = hull.adjacency[index].neighbors;
        std::sort(expected.begin(), expected.end());
        std::sort(recorded.begin(), recorded.end());

        const bool exact_three = expected.size() == 3U;
        const bool same = exact_three &&
            std::equal(expected.begin(), expected.end(), recorded.begin(), recorded.end());
        if (!same) {
            diag(out, ParseSeverity::warning,
                 "shw.adjacency-not-complete-edge-neighborhood",
                 "The bound DMC3 SHW payload stores exactly the three edge-neighbour triangles for every face; this hull differs and is preserved for variant review.",
                 hull.adjacency_offset +
                     static_cast<std::uint64_t>(index) * Contract::adjacency_record_size);
        }
    }
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult Parser::parse(const std::span<const std::byte> bytes) {
    ParseResult out;
    const binary::Reader reader(bytes);

    if (!reader.matches(0U, Contract::magic)) return out;
    out.recognized = true;

    if (bytes.size() < Contract::header_size) {
        diag(out, ParseSeverity::error,
             "shw.truncated-header",
             "SHW payload is shorter than the confirmed 0x20-byte header.",
             bytes.size());
        return out;
    }

    out.document.source_bytes.assign(bytes.begin(), bytes.end());

    const auto version = reader.f32_le(Contract::version_offset);
    const auto hull_count = reader.u8(Contract::hull_count_offset);
    if (!version || !hull_count) {
        diag(out, ParseSeverity::error,
             "shw.header-fields",
             "SHW header fields are truncated.",
             Contract::version_offset);
        return out;
    }

    out.document.header.version = *version;
    out.document.header.hull_count = *hull_count;

    if (!std::isfinite(*version) || std::fabs(*version - Contract::bound_version) > 0.0001F) {
        diag(out, ParseSeverity::warning,
             "shw.unconfirmed-version",
             "The hash-bound DMC3 SHW payload uses version 0.5; the raw version is preserved for variant review.",
             Contract::version_offset);
    }

    const auto table_bytes =
        static_cast<std::uint64_t>(*hull_count) * Contract::hull_record_size;
    if (!span_in_bounds(reader, Contract::hull_table_offset, table_bytes)) {
        diag(out, ParseSeverity::error,
             "shw.hull-table-out-of-bounds",
             "SHW hull-record table exceeds the payload.",
             Contract::hull_table_offset);
        return out;
    }

    out.document.hulls.reserve(*hull_count);
    for (std::size_t hull_index = 0U; hull_index < *hull_count; ++hull_index) {
        Hull hull;
        hull.record_offset = Contract::hull_table_offset +
            static_cast<std::uint64_t>(hull_index) * Contract::hull_record_size;
        const auto record = static_cast<std::size_t>(hull.record_offset);

        const auto vertex_count = reader.u16_le(record + Contract::vertex_count_offset);
        const auto triangle_count = reader.u16_le(record + Contract::triangle_count_offset);
        const auto triangle_offset = reader.u64_le(record + Contract::triangle_pointer_offset);
        const auto adjacency_offset = reader.u64_le(record + Contract::adjacency_pointer_offset);
        const auto vertex_offset = reader.u64_le(record + Contract::vertex_pointer_offset);
        const auto selector_offset = reader.u64_le(record + Contract::selector_pointer_offset);
        if (!vertex_count || !triangle_count || !triangle_offset ||
            !adjacency_offset || !vertex_offset || !selector_offset) {
            diag(out, ParseSeverity::error,
                 "shw.hull-record-truncated",
                 "SHW hull record is truncated.", hull.record_offset);
            return out;
        }

        hull.vertex_count = *vertex_count;
        hull.triangle_count = *triangle_count;
        hull.triangle_offset = *triangle_offset;
        hull.adjacency_offset = *adjacency_offset;
        hull.vertex_offset = *vertex_offset;
        hull.selector_offset = *selector_offset;

        const auto triangle_bytes =
            static_cast<std::uint64_t>(hull.triangle_count) * Contract::triangle_record_size;
        const auto adjacency_bytes =
            static_cast<std::uint64_t>(hull.triangle_count) * Contract::adjacency_record_size;
        const auto vertex_bytes =
            static_cast<std::uint64_t>(hull.vertex_count) * Contract::vertex_record_size;
        const auto selector_bytes = static_cast<std::uint64_t>(hull.vertex_count);

        for (const auto& [offset, length, code, message] :
             std::array{
                 std::tuple{hull.triangle_offset, triangle_bytes,
                            "shw.triangle-span-out-of-bounds",
                            "SHW triangle span exceeds the payload."},
                 std::tuple{hull.adjacency_offset, adjacency_bytes,
                            "shw.adjacency-span-out-of-bounds",
                            "SHW adjacency span exceeds the payload."},
                 std::tuple{hull.vertex_offset, vertex_bytes,
                            "shw.vertex-span-out-of-bounds",
                            "SHW vertex span exceeds the payload."},
                 std::tuple{hull.selector_offset, selector_bytes,
                            "shw.selector-span-out-of-bounds",
                            "SHW transform-selector span exceeds the payload."}}) {
            if (!span_in_bounds(reader, offset, length)) {
                diag(out, ParseSeverity::error, code, message, offset);
            }
        }
        if (has_error(out.diagnostics)) {
            out.document.hulls.push_back(std::move(hull));
            continue;
        }

        for (const auto offset : {
                 hull.triangle_offset,
                 hull.adjacency_offset,
                 hull.vertex_offset,
                 hull.selector_offset}) {
            if (!aligned_16(offset)) {
                diag(out, ParseSeverity::warning,
                     "shw.unconfirmed-stream-alignment",
                     "The bound DMC3 SHW payload aligns all four hull streams to 0x10; this offset differs.",
                     offset);
            }
        }

        hull.triangles.reserve(hull.triangle_count);
        for (std::size_t triangle_index = 0U;
             triangle_index < hull.triangle_count;
             ++triangle_index) {
            const auto offset = hull.triangle_offset +
                static_cast<std::uint64_t>(triangle_index) * Contract::triangle_record_size;
            Triangle triangle;
            bool fields_ok = true;
            for (std::size_t lane = 0U; lane < 3U; ++lane) {
                const auto value = reader.u32_le(
                    static_cast<std::size_t>(offset + lane * 4U));
                if (!value) {
                    fields_ok = false;
                    break;
                }
                triangle.vertices[lane] = *value;
            }
            const auto reserved = reader.u32_le(static_cast<std::size_t>(offset + 0x0CU));
            if (!fields_ok || !reserved) {
                diag(out, ParseSeverity::error,
                     "shw.triangle-truncated",
                     "SHW triangle record is truncated.", offset);
                break;
            }
            triangle.reserved = *reserved;
            if (triangle.reserved != 0U) {
                diag(out, ParseSeverity::warning,
                     "shw.triangle-reserved-nonzero",
                     "The fourth u32 triangle lane is zero in the bound payload; raw value is preserved.",
                     offset + 0x0CU);
            }
            for (const auto vertex : triangle.vertices) {
                if (vertex >= hull.vertex_count) {
                    diag(out, ParseSeverity::error,
                         "shw.triangle-vertex-out-of-range",
                         "SHW triangle references a vertex outside this hull.",
                         offset);
                    break;
                }
            }
            hull.triangles.push_back(triangle);
        }

        hull.adjacency.reserve(hull.triangle_count);
        for (std::size_t adjacency_index = 0U;
             adjacency_index < hull.triangle_count;
             ++adjacency_index) {
            const auto offset = hull.adjacency_offset +
                static_cast<std::uint64_t>(adjacency_index) * Contract::adjacency_record_size;
            Adjacency adjacency;
            bool fields_ok = true;
            for (std::size_t lane = 0U; lane < 3U; ++lane) {
                const auto value = reader.u16_le(
                    static_cast<std::size_t>(offset + lane * 2U));
                if (!value) {
                    fields_ok = false;
                    break;
                }
                adjacency.neighbors[lane] = *value;
            }
            const auto reserved = reader.u16_le(static_cast<std::size_t>(offset + 0x06U));
            if (!fields_ok || !reserved) {
                diag(out, ParseSeverity::error,
                     "shw.adjacency-truncated",
                     "SHW adjacency record is truncated.", offset);
                break;
            }
            adjacency.reserved = *reserved;
            if (adjacency.reserved != 0U) {
                diag(out, ParseSeverity::warning,
                     "shw.adjacency-reserved-nonzero",
                     "The fourth u16 adjacency lane is zero in the bound payload; raw value is preserved.",
                     offset + 0x06U);
            }
            for (const auto neighbor : adjacency.neighbors) {
                if (neighbor >= hull.triangle_count) {
                    diag(out, ParseSeverity::error,
                         "shw.adjacency-out-of-range",
                         "SHW adjacency references a triangle outside this hull.",
                         offset);
                    break;
                }
            }
            hull.adjacency.push_back(adjacency);
        }

        hull.vertices.reserve(hull.vertex_count);
        for (std::size_t vertex_index = 0U;
             vertex_index < hull.vertex_count;
             ++vertex_index) {
            const auto offset = hull.vertex_offset +
                static_cast<std::uint64_t>(vertex_index) * Contract::vertex_record_size;
            const auto x = reader.f32_le(static_cast<std::size_t>(offset + 0x00U));
            const auto y = reader.f32_le(static_cast<std::size_t>(offset + 0x04U));
            const auto z = reader.f32_le(static_cast<std::size_t>(offset + 0x08U));
            const auto w = reader.f32_le(static_cast<std::size_t>(offset + 0x0CU));
            if (!x || !y || !z || !w) {
                diag(out, ParseSeverity::error,
                     "shw.vertex-truncated",
                     "SHW float4 vertex is truncated.", offset);
                break;
            }
            const Vec4f vertex{*x, *y, *z, *w};
            if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) ||
                !std::isfinite(vertex.z) || !std::isfinite(vertex.w)) {
                diag(out, ParseSeverity::error,
                     "shw.vertex-nonfinite",
                     "SHW vertex contains a non-finite component.", offset);
            }
            if (std::isfinite(vertex.w) && std::fabs(vertex.w - 1.0F) > 0.0001F) {
                diag(out, ParseSeverity::warning,
                     "shw.unconfirmed-vertex-w",
                     "All vertices in the bound SHW payload use w=1.0; raw value is preserved for variant review.",
                     offset + 0x0CU);
            }
            hull.vertices.push_back(vertex);
        }

        hull.transform_selectors.reserve(hull.vertex_count);
        for (std::size_t selector_index = 0U;
             selector_index < hull.vertex_count;
             ++selector_index) {
            const auto offset = hull.selector_offset + selector_index;
            const auto selector = reader.u8(static_cast<std::size_t>(offset));
            if (!selector) {
                diag(out, ParseSeverity::error,
                     "shw.selector-truncated",
                     "SHW per-vertex transform selector is truncated.", offset);
                break;
            }
            hull.transform_selectors.push_back(*selector);
        }

        if (span_in_bounds(reader, hull.selector_offset, selector_bytes)) {
            const auto selector_end = hull.selector_offset + selector_bytes;
            const auto padded_end = align_16(selector_end);
            if (padded_end <= reader.size()) {
                for (auto offset = selector_end; offset < padded_end; ++offset) {
                    const auto value = reader.u8(static_cast<std::size_t>(offset));
                    if (value && *value != 0U) {
                        diag(out, ParseSeverity::warning,
                             "shw.selector-padding-nonzero",
                             "Selector padding is zero in the bound DMC3 SHW payload; non-zero raw padding is preserved.",
                             offset);
                        break;
                    }
                }
            }
        }

        if (!has_error(out.diagnostics)) {
            validate_closed_hull_semantics(out, hull);
        }
        out.document.hulls.push_back(std::move(hull));
    }

    return out;
}

} // namespace dmc::rengine::formats::shw
