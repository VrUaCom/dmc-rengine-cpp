#pragma once

#include "dmc_rengine/formats/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace dmc::rengine::formats::dca {

inline constexpr std::size_t header_size = 0x10U;
inline constexpr std::size_t record_size = 0x410U;

struct Record final {
    std::uint64_t offset{};
    std::uint64_t size{record_size};
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

} // namespace dmc::rengine::formats::dca
