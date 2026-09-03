#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::so::graph {

inline constexpr std::size_t type6_header_size = 0x0EU;
inline constexpr std::size_t type8_header_size = 0x08U;

struct IndexedBlock final {
    std::uint16_t type{};
    std::uint64_t base_offset{};
    std::uint64_t extent_size{};
    std::vector<std::uint16_t> header_words;
    std::vector<std::uint16_t> entry_offsets;
};

struct ParseResult final {
    bool recognized{false};
    std::vector<IndexedBlock> blocks;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

[[nodiscard]] ParseResult parse(std::span<const std::byte> bytes);

} // namespace dmc::rengine::formats::so::graph
