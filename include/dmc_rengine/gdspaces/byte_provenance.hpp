#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace dmc::rengine::gdspaces {

enum class ByteOriginKind {
    direct_source_span,
    transformed_source_span,
    materialized_parent_span,
};

[[nodiscard]] constexpr std::string_view to_string(
    ByteOriginKind kind) noexcept {
    switch (kind) {
    case ByteOriginKind::direct_source_span: return "direct-source-span";
    case ByteOriginKind::transformed_source_span: return "transformed-source-span";
    case ByteOriginKind::materialized_parent_span: return "materialized-parent-span";
    }
    return "direct-source-span";
}

enum class ByteTransform {
    none,
    zip_stored,
    zip_deflate,
    unknown,
};

[[nodiscard]] constexpr std::string_view to_string(
    ByteTransform transform) noexcept {
    switch (transform) {
    case ByteTransform::none: return "none";
    case ByteTransform::zip_stored: return "zip-stored";
    case ByteTransform::zip_deflate: return "zip-deflate";
    case ByteTransform::unknown: return "unknown";
    }
    return "unknown";
}

struct ByteProvenance final {
    ByteOriginKind kind{ByteOriginKind::direct_source_span};
    std::string authority_id;
    std::uint64_t offset{};
    std::uint64_t stored_size{};
    std::uint64_t materialized_size{};
    ByteTransform transform{ByteTransform::none};
    std::optional<std::uint32_t> crc32;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] bool direct_byte_mapping() const noexcept;
};

} // namespace dmc::rengine::gdspaces
