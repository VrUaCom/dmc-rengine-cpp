#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::so::volume_table {

inline constexpr std::size_t record_size = 0x50U;

// Recovered 2026-09-09 from the complete em000 extraction (source archive
// SHA-256 306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b),
// whose slot 40 holds 23 records.
//
// The reader used to accept any byte run whose length divided by 0x50, which
// across that corpus meant 28 of 306 payloads — motions, models and a scroll
// table among them. What the records actually agree on is recorded here.

/// A centre in `vector0.xyz` and a radius in `vector1.x`. 22 of 23 records.
inline constexpr std::uint32_t sphere_record_type = 2U;
/// Two points in `vector0`/`vector1` and a radius in `vector2.x`. 1 of 23.
inline constexpr std::uint32_t segment_record_type = 4U;

/// The twelve bytes between the type and the first vector. Zero in all 23.
inline constexpr std::size_t reserved_offset = 0x04U;
inline constexpr std::size_t reserved_bytes = 0x0CU;

/// `vector0` is a position in homogeneous coordinates, so its w is 1 exactly.
inline constexpr std::size_t first_vector_offset = 0x10U;
inline constexpr float position_w = 1.0F;

[[nodiscard]] constexpr bool record_type_is_known(std::uint32_t type) noexcept {
    return type == sphere_record_type || type == segment_record_type;
}

struct Vec4 final {
    float x{};
    float y{};
    float z{};
    float w{};
};

struct Record final {
    std::uint32_t type{};
    std::array<std::byte, 12> prefix_unknown{};
    Vec4 vector0{};
    Vec4 vector1{};
    Vec4 vector2{};
    Vec4 vector3{};
};

struct ParseResult final {
    bool recognized{false};
    std::vector<Record> records;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

/**
 * Whether these bytes carry this payload, without building its records.
 *
 * Classification runs on every slot of every container a browser opens, so
 * the identity question is separated from the reading: `parse` answers both
 * and allocates, this answers only the first and does not.
 */
[[nodiscard]] bool recognizes(std::span<const std::byte> bytes) noexcept;

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::so::volume_table
