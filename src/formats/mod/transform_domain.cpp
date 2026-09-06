#include "dmc_rengine/formats/mod/transform_domain.hpp"

#include "dmc_rengine/binary/reader.hpp"
#include "dmc_rengine/formats/model_node_domain_core.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace dmc::rengine::formats::mod::transform_domain {
namespace {

[[nodiscard]] bool has_error(const std::vector<ParseDiagnostic>& diagnostics) noexcept {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const ParseDiagnostic& diagnostic) {
        return diagnostic.severity == ParseSeverity::error;
    });
}

void add_diagnostic(std::vector<ParseDiagnostic>& diagnostics,
                    ParseSeverity severity,
                    std::string code,
                    std::string message,
                    const std::size_t offset) {
    diagnostics.push_back(ParseDiagnostic{
        severity,
        std::move(code),
        std::move(message),
        static_cast<std::uint64_t>(offset),
    });
}

void add_error(std::vector<ParseDiagnostic>& diagnostics,
               std::string code,
               std::string message,
               const std::size_t offset) {
    add_diagnostic(diagnostics, ParseSeverity::error,
                   std::move(code), std::move(message), offset);
}

void add_warning(std::vector<ParseDiagnostic>& diagnostics,
                 std::string code,
                 std::string message,
                 const std::size_t offset) {
    add_diagnostic(diagnostics, ParseSeverity::warning,
                   std::move(code), std::move(message), offset);
}

[[nodiscard]] bool resolve_relative(
    const std::size_t base,
    const std::uint32_t relative,
    const std::size_t payload_size,
    std::size_t& resolved) noexcept {
    if (base > payload_size) return false;
    const auto remaining = payload_size - base;
    if (static_cast<std::size_t>(relative) > remaining) return false;
    resolved = base + static_cast<std::size_t>(relative);
    return true;
}

[[nodiscard]] bool finite(const LocalTransformRecord& transform) noexcept {
    return std::isfinite(transform.translation.x) &&
           std::isfinite(transform.translation.y) &&
           std::isfinite(transform.translation.z) &&
           std::isfinite(transform.translation_magnitude) &&
           std::isfinite(transform.rotation_xyz_radians.x) &&
           std::isfinite(transform.rotation_xyz_radians.y) &&
           std::isfinite(transform.rotation_xyz_radians.z) &&
           std::isfinite(transform.reserved1c);
}

} // namespace

bool ParseResult::ok() const noexcept {
    return recognized && !has_error(diagnostics);
}

ParseResult parse(const std::span<const std::byte> bytes) {
    ParseResult result;
    const binary::Reader reader(bytes);
    if (!reader.matches(0U, "MOD ")) {
        return result;
    }
    result.recognized = true;

    const auto domain_count = reader.u8(0x11U);
    const auto document_offset = reader.u64_le(0x20U);
    if (!domain_count || !document_offset || *domain_count == 0U) {
        add_error(result.diagnostics,
                  "mod.transform_domain.header",
                  "MOD transform-domain header is incomplete or has zero domain count",
                  0x11U);
        return result;
    }

    result.raw_domain_count = *domain_count;
    result.document_offset = *document_offset;
    if (*document_offset > static_cast<std::uint64_t>(bytes.size())) {
        add_error(result.diagnostics,
                  "mod.transform_domain.document_range",
                  "MOD document pointer is outside the payload",
                  0x20U);
        return result;
    }

    const auto document = static_cast<std::size_t>(*document_offset);
    if (!reader.contains(document, model_family::NodeDomainCoreAbi::block_header_size)) {
        add_error(result.diagnostics,
                  "mod.transform_domain.document_header",
                  "MOD transform-domain block header is truncated",
                  document);
        return result;
    }

    const auto parent_rel = reader.u32_le(
        document + model_family::NodeDomainCoreAbi::parent_rel_field);
    const auto order_rel = reader.u32_le(
        document + model_family::NodeDomainCoreAbi::order_rel_field);
    const auto adapter_rel = reader.u32_le(
        document + model_family::NodeDomainCoreAbi::adapter_array_rel_field);
    const auto transform_rel = reader.u32_le(
        document + model_family::NodeDomainCoreAbi::transform_rel_field);
    if (!parent_rel || !order_rel || !adapter_rel || !transform_rel) {
        add_error(result.diagnostics,
                  "mod.transform_domain.table_offsets",
                  "MOD transform-domain relative offsets are truncated",
                  document);
        return result;
    }

    result.parent_relative_offset = *parent_rel;
    result.order_relative_offset = *order_rel;
    result.adapter_relative_offset = *adapter_rel;
    result.transform_relative_offset = *transform_rel;

    std::size_t parent_table{};
    std::size_t order_table{};
    std::size_t adapter_table{};
    std::size_t transform_table{};
    if (!resolve_relative(document, *parent_rel, bytes.size(), parent_table) ||
        !resolve_relative(document, *order_rel, bytes.size(), order_table) ||
        !resolve_relative(document, *adapter_rel, bytes.size(), adapter_table) ||
        !resolve_relative(document, *transform_rel, bytes.size(), transform_table)) {
        add_error(result.diagnostics,
                  "mod.transform_domain.table_range",
                  "MOD transform-domain relative pointer escapes the payload",
                  document);
        return result;
    }

    const auto count = static_cast<std::size_t>(*domain_count);
    const auto transform_bytes =
        count * model_family::TransformCoreAbi::record_size;
    if (*parent_rel >= *order_rel || *order_rel >= *adapter_rel ||
        *adapter_rel >= *transform_rel ||
        !reader.contains(parent_table, count) ||
        !reader.contains(order_table, count) ||
        !reader.contains(adapter_table, count) ||
        !reader.contains(transform_table, transform_bytes) ||
        order_table - parent_table < count ||
        adapter_table - order_table < count ||
        transform_table - adapter_table < count) {
        add_error(result.diagnostics,
                  "mod.transform_domain.table_spans",
                  "MOD transform-domain arrays or local-transform records are inconsistent",
                  document);
        return result;
    }

    result.serialized_layout_matches_core =
        *parent_rel == model_family::NodeDomainCoreAbi::expected_parent_rel(count) &&
        *order_rel == model_family::NodeDomainCoreAbi::expected_order_rel(count) &&
        *adapter_rel == model_family::NodeDomainCoreAbi::expected_adapter_array_rel(count) &&
        *transform_rel == model_family::NodeDomainCoreAbi::expected_transform_rel(count);
    if (!result.serialized_layout_matches_core) {
        add_warning(result.diagnostics,
                    "mod.transform_domain.layout_variant",
                    "MOD node-domain offsets differ from the three-payload shared Model Family layout; bounded data remains preserved",
                    document);
    }

    result.reference_table.reserve(count);
    result.permutation_table.reserve(count);
    result.adapter_table.reserve(count);
    for (std::size_t index = 0U; index < count; ++index) {
        const auto reference = reader.u8(parent_table + index);
        const auto permutation = reader.u8(order_table + index);
        const auto adapter = reader.u8(adapter_table + index);
        if (!reference || !permutation || !adapter) {
            add_error(result.diagnostics,
                      "mod.transform_domain.table_truncated",
                      "MOD transform-domain array entry is truncated",
                      parent_table + index);
            return result;
        }
        result.reference_table.push_back(*reference);
        result.permutation_table.push_back(*permutation);
        result.adapter_table.push_back(*adapter);
    }

    std::vector<std::int16_t> inverse(count, static_cast<std::int16_t>(-1));
    result.permutation_is_complete = true;
    for (std::size_t logical_index = 0U; logical_index < count; ++logical_index) {
        const auto physical_index = static_cast<std::size_t>(
            result.permutation_table[logical_index]);
        if (physical_index >= count ||
            inverse[physical_index] != static_cast<std::int16_t>(-1)) {
            result.permutation_is_complete = false;
            break;
        }
        inverse[physical_index] = static_cast<std::int16_t>(logical_index);
    }
    if (!result.permutation_is_complete) {
        add_warning(result.diagnostics,
                    "mod.transform_domain.permutation_incomplete",
                    "MOD transform-domain permutation is incomplete or contains duplicates",
                    order_table);
    } else {
        result.derived_hierarchy_candidate.reserve(count);
        result.hierarchy_candidate_is_acyclic = true;
        for (std::size_t logical_index = 0U; logical_index < count; ++logical_index) {
            const auto raw_reference = result.reference_table[logical_index];
            if (raw_reference == 0xFFU) {
                result.derived_hierarchy_candidate.push_back(
                    static_cast<std::int16_t>(-1));
                continue;
            }

            const auto reference_index = static_cast<std::size_t>(raw_reference);
            if (reference_index >= count || inverse[reference_index] < 0) {
                result.derived_hierarchy_candidate.push_back(
                    static_cast<std::int16_t>(-2));
                result.hierarchy_candidate_is_acyclic = false;
                continue;
            }

            const auto derived = inverse[reference_index];
            result.derived_hierarchy_candidate.push_back(derived);
            if (derived >= static_cast<std::int16_t>(logical_index)) {
                result.hierarchy_candidate_is_acyclic = false;
            }
        }

        if (!result.hierarchy_candidate_is_acyclic) {
            add_warning(result.diagnostics,
                        "mod.transform_domain.hierarchy_candidate_invalid",
                        "MOD derived transform hierarchy candidate is not acyclic",
                        parent_table);
        }
    }

    result.local_transform_records.reserve(count);
    result.transform_records_finite = true;
    for (std::size_t index = 0U; index < count; ++index) {
        LocalTransformRecord transform;
        const auto offset = transform_table +
            index * model_family::TransformCoreAbi::record_size;
        transform.record_offset = static_cast<std::uint64_t>(offset);

        const auto tx = reader.f32_le(
            offset + model_family::TransformCoreAbi::translation_field + 0U);
        const auto ty = reader.f32_le(
            offset + model_family::TransformCoreAbi::translation_field + 4U);
        const auto tz = reader.f32_le(
            offset + model_family::TransformCoreAbi::translation_field + 8U);
        const auto magnitude = reader.f32_le(
            offset + model_family::TransformCoreAbi::translation_magnitude_field);
        const auto rx = reader.f32_le(
            offset + model_family::TransformCoreAbi::rotation_xyz_radians_field + 0U);
        const auto ry = reader.f32_le(
            offset + model_family::TransformCoreAbi::rotation_xyz_radians_field + 4U);
        const auto rz = reader.f32_le(
            offset + model_family::TransformCoreAbi::rotation_xyz_radians_field + 8U);
        const auto reserved = reader.f32_le(
            offset + model_family::TransformCoreAbi::reserved1c_field);
        if (!tx || !ty || !tz || !magnitude || !rx || !ry || !rz || !reserved) {
            add_error(result.diagnostics,
                      "mod.transform_domain.transform_truncated",
                      "MOD local-transform record is truncated",
                      offset);
            return result;
        }

        transform.translation = Vec3f{*tx, *ty, *tz};
        transform.translation_magnitude = *magnitude;
        transform.rotation_xyz_radians = Vec3f{*rx, *ry, *rz};
        transform.reserved1c = *reserved;
        if (!finite(transform)) {
            result.transform_records_finite = false;
        }
        result.local_transform_records.push_back(transform);
    }
    result.transform_records_complete =
        result.local_transform_records.size() == count;

    if (!result.transform_records_finite) {
        add_warning(result.diagnostics,
                    "mod.transform_domain.non_finite_transform",
                    "MOD local-transform records contain non-finite float components",
                    transform_table);
    }

    return result;
}

} // namespace dmc::rengine::formats::mod::transform_domain
