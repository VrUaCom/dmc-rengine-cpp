#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::mod::transform_domain {

struct Vec3f final {
    float x{};
    float y{};
    float z{};
};

// Evidence-backed serialized local transform. Records are indexed by MOD node
// index. This is still local-space only: no world-matrix/world-position
// authority is implied by parsing the record.
struct LocalTransformRecord final {
    std::uint64_t record_offset{};
    Vec3f translation{};

    // +0x0C is data-confirmed as length(translation.xyz) on all 285 records in
    // the current em000+pl000+id100 corpus. Canonical MOD/EFM initializer
    // 0x1402FA080 passes the enclosing float4 to 0x140031200, whose lane mask
    // preserves the matrix W component instead of applying this fourth source
    // scalar. CMotion binding 0x14030F850 likewise copies +0x00/+04/+08 and
    // skips +0x0C. It is therefore an auxiliary/cached magnitude, not a
    // homogeneous translation-W input.
    float translation_magnitude{};

    Vec3f rotation_xyz_radians{};

    // +0x1C remains byte-preserved. It is zero on all 285 current MOD transform
    // records. More importantly, canonical rotation helper 0x140330450 reads
    // only +0x10/+0x14/+0x18, and CMotion binding 0x14030F850 also skips this
    // fourth scalar while advancing the source record by 0x20. The SCM sibling
    // initializer explicitly clears its local fourth rotation lane before
    // calling the same helper. This strongly supports a reserved/alignment
    // role, but does not authorize a global writer-zero rule until every
    // relevant consumer/family is closed.
    float reserved1c{};
};

struct ParseResult final {
    bool recognized{false};
    std::uint8_t raw_domain_count{};
    std::uint64_t document_offset{};

    std::uint32_t parent_relative_offset{};
    std::uint32_t order_relative_offset{};
    std::uint32_t adapter_relative_offset{};
    std::uint32_t transform_relative_offset{};

    // Raw byte-preserving compatibility views. Canonical semantics are exposed
    // separately below so callers no longer have to reinterpret these arrays.
    std::vector<std::uint8_t> reference_table;
    std::vector<std::uint8_t> permutation_table;
    std::vector<std::uint8_t> adapter_table;

    // 2026-09-05 provenance correction: 0x1402FA080 is MOD/EFM, not the
    // SCM-specific initializer. Its evaluation loop confirms that +0x00 is
    // parentByOrderPosition and +0x04 is nodeAtOrderPosition. Parent values are
    // already node indices; they must not be run through inverse permutation.
    std::vector<std::int16_t> parent_by_order_position;
    std::vector<std::uint8_t> node_at_order_position;

    // Retained as a compatibility mirror for the earlier research API. It now
    // mirrors parent_by_order_position exactly instead of applying the rejected
    // inverse-permutation heuristic.
    std::vector<std::int16_t> derived_hierarchy_candidate;

    // Serialized transform records are node-indexed; evaluation order is kept
    // in node_at_order_position rather than by reordering this vector.
    std::vector<LocalTransformRecord> local_transform_records_by_node_index;

    bool permutation_is_complete{false};
    bool hierarchy_is_topological{false};
    bool hierarchy_candidate_is_acyclic{false};
    bool serialized_layout_matches_core{false};
    bool transform_records_complete{false};
    bool transform_records_finite{false};
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::mod::transform_domain
