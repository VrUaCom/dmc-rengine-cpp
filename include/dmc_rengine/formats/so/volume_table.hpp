#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::so::volume_table {

inline constexpr std::size_t record_size = 0x50U;

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

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::so::volume_table
