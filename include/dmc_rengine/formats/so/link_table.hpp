#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::so::link_table {

inline constexpr std::size_t record_size = 0x04U;

struct Record final {
    std::uint8_t field0{};
    std::uint8_t field1{};
    std::uint8_t field2{};
    std::uint8_t field3{};
};

struct ParseResult final {
    bool recognized{false};
    std::vector<Record> records;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::so::link_table
