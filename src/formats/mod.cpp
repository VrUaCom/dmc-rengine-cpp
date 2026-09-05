#include "dmc_rengine/formats/mod.hpp"

#include "dmc_rengine/binary/reader.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <utility>

namespace dmc::rengine::formats::mod {
namespace {

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
    const std::uint64_t offset,
    const std::uint64_t length) noexcept {
    if (offset > static_cast<std::uint64_t>(reader.size())) return false;
    const auto remaining =
        static_cast<std::uint64_t>(reader.size()) - offset;
    return length <= remaining;
}

[[nodiscard]] bool add_offset(
    const std::uint64_t base,
    const std::uint64_t relative,
    std::uint64_t& result) noexcept {
    if (relative > std::numeric_limits<std::uint64_t>::max() - base) {
        return false;
    }
    result = base + relative;
    return true;
}

[[nodiscard]] bool aligned_16(const std::uint64_t offset) noexcept {
    return (offset & 0x0FU) == 0U;
}

[[nodiscard]] std::int16_t as_i16(const std::uint16_t raw) noexcept {
    if (raw <= 0x7FFFU) return static_cast<std::int16_t>(raw);
    return static_cast<std::int16_t>(
        static_cast<std::int32_t>(raw) - 0x10000);
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult Parser::parse(const std::span<const std::byte> bytes) {
    ParseResult out;
    const binary::Reader reader(bytes);

    if (!reader.matches(0U, "MOD ")) return out;
    out.recognized = true;

    if (bytes.size() < header_size) {
        diag(out, ParseSeverity::error,
             "mod.truncated-header",
             "MOD payload is shorter than the confirmed 0x40-byte header.",
             bytes.size());
        return out;
    }

    out.document.source_bytes.assign(bytes.begin(), bytes.end());
    auto& header = out.document.header;

    const auto version = reader.f32_le(0x04U);
    const auto outer_count = reader.u8(0x10U);
    const auto transform_count = reader.u8(0x11U);
    const auto document_offset = reader.u64_le(0x20U);
    if (!version || !outer_count || !transform_count || !document_offset) {
        diag(out, ParseSeverity::error,
             "mod.header-fields",
             "MOD header fields are truncated.", 0x04U);
        return out;
    }

    header.version = *version;
    header.outer_record_count = *outer_count;
    header.transform_domain_count = *transform_count;
    header.document_offset = *document_offset;

    if (!std::isfinite(header.version) ||
        std::fabs(header.version - 1.01F) > 0.0001F) {
        diag(out, ParseSeverity::warning,
             "mod.unconfirmed-version",
             "Recovered DMC3-HD MOD corpus uses version 1.01; raw version is preserved.",
             0x04U);
    }
    if (header.transform_domain_count == 0U) {
        diag(out, ParseSeverity::warning,
             "mod.zero-transform-domain",
             "MOD transform-domain count is zero; skin-to-node validation is unavailable.",
             0x11U);
    }
    if (header.document_offset > bytes.size()) {
        diag(out, ParseSeverity::error,
             "mod.document-out-of-bounds",
             "MOD document pointer lies outside the payload.", 0x20U);
    }

    const auto outer_bytes =
        static_cast<std::uint64_t>(header.outer_record_count) *
        outer_record_size;
    if (!span_in_bounds(reader, header_size, outer_bytes)) {
        diag(out, ParseSeverity::error,
             "mod.outer-table-out-of-bounds",
             "MOD outer-record table exceeds the payload.", header_size);
        return out;
    }

    out.document.outer_models.reserve(header.outer_record_count);
    for (std::size_t outer_index = 0U;
         outer_index < header.outer_record_count;
         ++outer_index) {
        OuterModel outer;
        outer.record_offset =
            header_size +
            static_cast<std::uint64_t>(outer_index) * outer_record_size;

        const auto inner_count = reader.u8(
            static_cast<std::size_t>(outer.record_offset + 0x00U));
        const auto aggregate = reader.u16_le(
            static_cast<std::size_t>(outer.record_offset + 0x02U));
        const auto inner_table = reader.u64_le(
            static_cast<std::size_t>(outer.record_offset + 0x08U));
        if (!inner_count || !aggregate || !inner_table) {
            diag(out, ParseSeverity::error,
                 "mod.outer-record-truncated",
                 "MOD outer record is truncated.", outer.record_offset);
            return out;
        }

        outer.inner_record_count = *inner_count;
        outer.aggregate_element_count = *aggregate;
        outer.inner_table_offset = *inner_table;

        const auto inner_bytes =
            static_cast<std::uint64_t>(outer.inner_record_count) *
            inner_record_size;
        if (!span_in_bounds(reader, outer.inner_table_offset, inner_bytes)) {
            diag(out, ParseSeverity::error,
                 "mod.inner-table-out-of-bounds",
                 "MOD inner-mesh table exceeds the payload.",
                 outer.inner_table_offset);
            out.document.outer_models.push_back(std::move(outer));
            continue;
        }

        outer.meshes.reserve(outer.inner_record_count);
        std::uint64_t aggregate_sum = 0U;
        for (std::size_t inner_index = 0U;
             inner_index < outer.inner_record_count;
             ++inner_index) {
            InnerMesh mesh;
            mesh.record_offset =
                outer.inner_table_offset +
                static_cast<std::uint64_t>(inner_index) * inner_record_size;

            const auto count = reader.u16_le(
                static_cast<std::size_t>(mesh.record_offset + 0x00U));
            const auto positions = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x10U));
            const auto normals = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x18U));
            const auto uv = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x20U));
            const auto blend = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x28U));
            const auto control = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x30U));
            const auto reserved38 = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x38U));
            const auto generated_rel = reader.u64_le(
                static_cast<std::size_t>(mesh.record_offset + 0x40U));
            const auto generated_count = reader.u32_le(
                static_cast<std::size_t>(mesh.record_offset + 0x48U));
            const auto reserved4c = reader.u32_le(
                static_cast<std::size_t>(mesh.record_offset + 0x4CU));
            if (!count || !positions || !normals || !uv || !blend ||
                !control || !reserved38 || !generated_rel ||
                !generated_count || !reserved4c) {
                diag(out, ParseSeverity::error,
                     "mod.inner-record-truncated",
                     "MOD inner mesh record is truncated.", mesh.record_offset);
                outer.meshes.push_back(std::move(mesh));
                continue;
            }

            mesh.element_count = *count;
            mesh.positions_offset = *positions;
            mesh.normals_offset = *normals;
            mesh.uv_offset = *uv;
            mesh.blend_indices_offset = *blend;
            mesh.control_offset = *control;
            mesh.reserved38 = *reserved38;
            mesh.generated_workspace_relative_offset = *generated_rel;
            mesh.generated_topology_count = *generated_count;
            mesh.reserved4c = *reserved4c;
            aggregate_sum += mesh.element_count;

            if (!add_offset(mesh.record_offset,
                            mesh.generated_workspace_relative_offset,
                            mesh.generated_workspace_offset) ||
                mesh.generated_workspace_offset > bytes.size()) {
                diag(out, ParseSeverity::error,
                     "mod.generated-workspace-out-of-bounds",
                     "MOD generated-topology workspace pointer is invalid.",
                     mesh.record_offset + 0x40U);
            }
            if (mesh.reserved38 != 0U) {
                diag(out, ParseSeverity::warning,
                     "mod.inner-38-nonzero",
                     "MOD inner +0x38 is non-zero outside the bound retail corpus.",
                     mesh.record_offset + 0x38U);
            }
            if (mesh.generated_topology_count != 0U) {
                diag(out, ParseSeverity::warning,
                     "mod.generated-count-nonzero",
                     "MOD generated-topology count is non-zero in serialized input.",
                     mesh.record_offset + 0x48U);
            }
            if (mesh.reserved4c != 0U) {
                diag(out, ParseSeverity::warning,
                     "mod.inner-4c-nonzero",
                     "MOD inner +0x4C is non-zero; raw value is preserved.",
                     mesh.record_offset + 0x4CU);
            }

            const auto element_count =
                static_cast<std::uint64_t>(mesh.element_count);
            const std::array stream_ranges{
                std::pair{mesh.positions_offset,
                          element_count * model_family::MeshCoreAbi::position_stride},
                std::pair{mesh.normals_offset,
                          element_count * model_family::MeshCoreAbi::normal_stride},
                std::pair{mesh.uv_offset,
                          element_count * model_family::MeshCoreAbi::uv_stride},
                std::pair{mesh.blend_indices_offset, element_count * 4U},
                std::pair{mesh.control_offset, element_count * 2U},
            };

            const bool streams_in_bounds = std::all_of(
                stream_ranges.begin(), stream_ranges.end(),
                [&](const auto& range) {
                    return span_in_bounds(reader, range.first, range.second);
                });
            if (!streams_in_bounds) {
                diag(out, ParseSeverity::error,
                     "mod.mesh-stream-out-of-bounds",
                     "MOD source stream exceeds the payload.", mesh.record_offset);
                outer.meshes.push_back(std::move(mesh));
                continue;
            }

            for (const auto& range : stream_ranges) {
                if (range.second != 0U && !aligned_16(range.first)) {
                    diag(out, ParseSeverity::warning,
                         "mod.stream-misaligned",
                         "Recovered MOD source streams are 0x10-aligned; this stream is not.",
                         range.first);
                }
            }

            mesh.positions.resize(mesh.element_count);
            mesh.normals.resize(mesh.element_count);
            mesh.uvs.resize(mesh.element_count);
            mesh.blend_indices.resize(mesh.element_count);
            mesh.control_words.resize(mesh.element_count);
            if (header.transform_domain_count != 0U) {
                mesh.skin.resize(mesh.element_count);
            }

            std::size_t non_finite_vectors = 0U;
            for (std::size_t element = 0U;
                 element < mesh.element_count;
                 ++element) {
                const auto p = static_cast<std::size_t>(
                    mesh.positions_offset + element *
                    model_family::MeshCoreAbi::position_stride);
                const auto n = static_cast<std::size_t>(
                    mesh.normals_offset + element *
                    model_family::MeshCoreAbi::normal_stride);
                const auto t = static_cast<std::size_t>(
                    mesh.uv_offset + element * model_family::MeshCoreAbi::uv_stride);
                const auto b = static_cast<std::size_t>(
                    mesh.blend_indices_offset + element * 4U);
                const auto c = static_cast<std::size_t>(
                    mesh.control_offset + element * 2U);

                auto& position = mesh.positions[element];
                position.x = *reader.f32_le(p + 0U);
                position.y = *reader.f32_le(p + 4U);
                position.z = *reader.f32_le(p + 8U);

                auto& normal = mesh.normals[element];
                normal.x = *reader.f32_le(n + 0U);
                normal.y = *reader.f32_le(n + 4U);
                normal.z = *reader.f32_le(n + 8U);

                if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
                    !std::isfinite(position.z) || !std::isfinite(normal.x) ||
                    !std::isfinite(normal.y) || !std::isfinite(normal.z)) {
                    ++non_finite_vectors;
                }

                mesh.uvs[element].u = as_i16(*reader.u16_le(t + 0U));
                mesh.uvs[element].v = as_i16(*reader.u16_le(t + 2U));

                auto& lanes = mesh.blend_indices[element].lanes;
                lanes[0] = *reader.u8(b + 0U);
                lanes[1] = *reader.u8(b + 1U);
                lanes[2] = *reader.u8(b + 2U);
                lanes[3] = *reader.u8(b + 3U);
                if (lanes[0] != 0U) ++mesh.reserved_blend_lane_nonzero;

                mesh.control_words[element] = *reader.u16_le(c);
                if (header.transform_domain_count != 0U) {
                    mesh.skin[element] = decode_vertex_skin(
                        lanes,
                        mesh.control_words[element],
                        header.transform_domain_count);
                    if (!mesh.skin[element].ok()) ++mesh.skin_decode_failures;
                }
            }

            if (non_finite_vectors != 0U) {
                diag(out, ParseSeverity::warning,
                     "mod.non-finite-vector",
                     "MOD position/normal stream contains non-finite values.",
                     mesh.record_offset);
            }
            if (mesh.reserved_blend_lane_nonzero != 0U) {
                diag(out, ParseSeverity::warning,
                     "mod.blend-lane0-nonzero",
                     "MOD BLENDINDICES lane 0 is non-zero outside the current corpus invariant.",
                     mesh.blend_indices_offset);
            }
            if (mesh.skin_decode_failures != 0U) {
                diag(out, ParseSeverity::warning,
                     "mod.skin-invariant-mismatch",
                     "One or more MOD vertices do not satisfy the recovered three-influence skin encoding; raw streams remain available.",
                     mesh.control_offset);
            }

            outer.meshes.push_back(std::move(mesh));
        }

        if (aggregate_sum != outer.aggregate_element_count) {
            diag(out, ParseSeverity::error,
                 "mod.aggregate-element-count-mismatch",
                 "MOD outer +0x02 does not equal the sum of child inner element counts.",
                 outer.record_offset + 0x02U);
        }
        out.document.outer_models.push_back(std::move(outer));
    }

    out.document.transform_domain = transform_domain::parse(bytes);
    if (out.document.transform_domain.recognized) {
        if (!out.document.transform_domain.ok()) {
            diag(out, ParseSeverity::warning,
                 "mod.transform-domain-parse-incomplete",
                 "MOD mesh streams parsed, but transform-domain parsing is incomplete.",
                 header.document_offset);
        } else if (!out.document.transform_domain.permutation_is_complete) {
            diag(out, ParseSeverity::warning,
                 "mod.transform-permutation-incomplete",
                 "MOD transform-domain permutation is not complete.",
                 header.document_offset);
        } else if (!out.document.transform_domain.hierarchy_candidate_is_acyclic) {
            diag(out, ParseSeverity::warning,
                 "mod.transform-hierarchy-candidate-invalid",
                 "MOD derived hierarchy candidate is not acyclic.",
                 header.document_offset);
        }
    }

    return out;
}

} // namespace dmc::rengine::formats::mod
