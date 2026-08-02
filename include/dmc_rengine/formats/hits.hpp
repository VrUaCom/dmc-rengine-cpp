#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::hits {

inline constexpr std::uint32_t record_marker = 0x18060001U;
inline constexpr std::size_t record_size = 56U;
inline constexpr std::size_t value_count = 13U;

struct Record final {
    std::uint64_t offset{};
    std::uint32_t marker{};
    std::array<float, value_count> values{};
};

struct ScanResult final {
    bool recognized{false};
    std::vector<Record> records;
    std::vector<ParseDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const noexcept;
};

class RecordScanner final {
public:
    [[nodiscard]] static ScanResult scan(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats::hits
