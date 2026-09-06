#pragma once

#include <cstddef>

namespace dmc::rengine::formats::model_family {

[[nodiscard]] constexpr std::size_t align4(std::size_t value) noexcept {
    return (value + 3U) & ~std::size_t{3U};
}

[[nodiscard]] constexpr std::size_t align16(std::size_t value) noexcept {
    return (value + 15U) & ~std::size_t{15U};
}

// Shared serialized node-domain shell recovered in both SCM and MOD. The
// +0x08 array deliberately remains adapter-specific: SCM binds geometry
// objects there, while the current MOD corpus carries a different node-side
// classification/type domain.
struct NodeDomainCoreAbi final {
    static constexpr std::size_t block_header_size = 0x20U;
    static constexpr std::size_t parent_rel_field = 0x00U;
    static constexpr std::size_t order_rel_field = 0x04U;
    static constexpr std::size_t adapter_array_rel_field = 0x08U;
    static constexpr std::size_t transform_rel_field = 0x0CU;

    [[nodiscard]] static constexpr std::size_t expected_parent_rel(
        std::size_t) noexcept {
        return 0x20U;
    }

    [[nodiscard]] static constexpr std::size_t expected_order_rel(
        std::size_t node_count) noexcept {
        return 0x20U + align4(node_count);
    }

    [[nodiscard]] static constexpr std::size_t expected_adapter_array_rel(
        std::size_t node_count) noexcept {
        return 0x20U + 2U * align4(node_count);
    }

    [[nodiscard]] static constexpr std::size_t expected_transform_rel(
        std::size_t node_count) noexcept {
        return align16(0x20U + 3U * align4(node_count));
    }
};

// The serialized transform record is the same 0x20-byte shell in SCM and the
// three bound MOD payloads. The SCM-specific builder 0x1402FA360 and the
// MOD/EFM builder 0x1402FA080 both feed rotation XYZ to 0x140330450 and
// translation XYZ to 0x140031200, establishing a shared local-transform core.
struct TransformCoreAbi final {
    static constexpr std::size_t record_size = 0x20U;
    static constexpr std::size_t translation_field = 0x00U;
    static constexpr std::size_t translation_magnitude_field = 0x0CU;
    static constexpr std::size_t rotation_xyz_radians_field = 0x10U;
    static constexpr std::size_t reserved1c_field = 0x1CU;
};

static_assert(NodeDomainCoreAbi::block_header_size == 0x20U);
static_assert(NodeDomainCoreAbi::expected_parent_rel(24U) == 0x20U);
static_assert(NodeDomainCoreAbi::expected_order_rel(24U) == 0x38U);
static_assert(NodeDomainCoreAbi::expected_adapter_array_rel(24U) == 0x50U);
static_assert(NodeDomainCoreAbi::expected_transform_rel(24U) == 0x70U);
static_assert(NodeDomainCoreAbi::expected_order_rel(33U) == 0x44U);
static_assert(NodeDomainCoreAbi::expected_adapter_array_rel(33U) == 0x68U);
static_assert(NodeDomainCoreAbi::expected_transform_rel(33U) == 0x90U);
static_assert(TransformCoreAbi::record_size == 0x20U);

} // namespace dmc::rengine::formats::model_family
