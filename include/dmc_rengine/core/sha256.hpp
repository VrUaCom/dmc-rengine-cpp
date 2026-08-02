#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace dmc::rengine::core {

struct Sha256Digest final {
    std::array<std::uint8_t, 32> bytes{};

    [[nodiscard]] std::string hex() const;
    friend bool operator==(const Sha256Digest&, const Sha256Digest&) = default;
};

class Sha256 final {
public:
    [[nodiscard]] static Sha256Digest compute(
        std::span<const std::byte> input);
};

} // namespace dmc::rengine::core
