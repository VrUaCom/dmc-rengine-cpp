#pragma once

#include <cstdint>
#include <string_view>

namespace dmc::rengine::formats::model_family {

// High-level semantic family shared by the DMC3-HD SCM and MOD adapters.
// This deliberately does not assert binary-layout identity between the formats.
enum class SourceFormat : std::uint8_t {
    scm,
    mod,
};

enum class Capability : std::uint32_t {
    geometry = 1U << 0U,
    normals = 1U << 1U,
    uv = 1U << 2U,
    topology = 1U << 3U,
    node_hierarchy = 1U << 4U,
    transforms = 1U << 5U,
    texture_binding = 1U << 6U,
    alpha_control = 1U << 7U,
    legacy_gs_sampler = 1U << 8U,
    skeletal_skinning = 1U << 9U,
    experimental_authoring = 1U << 10U,
};

using CapabilityMask = std::uint32_t;

[[nodiscard]] constexpr CapabilityMask bit(Capability capability) noexcept {
    return static_cast<CapabilityMask>(capability);
}

[[nodiscard]] constexpr bool has(
    CapabilityMask mask,
    Capability capability) noexcept {
    return (mask & bit(capability)) != 0U;
}

struct Profile final {
    SourceFormat source_format{};
    CapabilityMask capabilities{};
    std::uint8_t max_serialized_skin_influences{};
    bool production_writer_authorized{};
};

[[nodiscard]] constexpr Profile scm_profile() noexcept {
    return Profile{
        SourceFormat::scm,
        bit(Capability::geometry) |
            bit(Capability::normals) |
            bit(Capability::uv) |
            bit(Capability::topology) |
            bit(Capability::node_hierarchy) |
            bit(Capability::transforms) |
            bit(Capability::texture_binding) |
            bit(Capability::alpha_control) |
            bit(Capability::legacy_gs_sampler) |
            bit(Capability::experimental_authoring),
        0U,
        false,
    };
}

[[nodiscard]] constexpr Profile mod_profile() noexcept {
    return Profile{
        SourceFormat::mod,
        bit(Capability::geometry) |
            bit(Capability::normals) |
            bit(Capability::uv) |
            bit(Capability::topology) |
            bit(Capability::node_hierarchy) |
            bit(Capability::transforms) |
            bit(Capability::skeletal_skinning),
        3U,
        false,
    };
}

[[nodiscard]] constexpr std::string_view to_string(
    SourceFormat source_format) noexcept {
    switch (source_format) {
    case SourceFormat::scm: return "scm";
    case SourceFormat::mod: return "mod";
    }
    return "unknown";
}

} // namespace dmc::rengine::formats::model_family
