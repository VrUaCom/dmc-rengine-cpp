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

// Evidence-backed serialized local transform. This record is intentionally
// local-space only: no MOD world-matrix/world-position authority is implied.
struct LocalTransformRecord final {
    std::uint64_t record_offset{};
    Vec3f translation{};
    float translation_magnitude{};
    Vec3f rotation_xyz_radians{};
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

    // Legacy API names retained for compatibility. The shared Model Family ABI
    // identifies these serialized arrays as the parent-domain and evaluation-
    // order domains. MOD world-propagation semantics remain a separate gate.
    std::vector<std::uint8_t> reference_table;
    std::vector<std::uint8_t> permutation_table;
    std::vector<std::uint8_t> adapter_table;

    std::vector<std::int16_t> derived_hierarchy_candidate;
    std::vector<LocalTransformRecord> local_transform_records;

    bool permutation_is_complete{false};
    bool hierarchy_candidate_is_acyclic{false};
    bool serialized_layout_matches_core{false};
    bool transform_records_complete{false};
    bool transform_records_finite{false};
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::mod::transform_domain
