#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace dmc::rengine::core {

struct Sha256Digest final {
    std::array<std::uint8_t, 32> bytes{};

    [[nodiscard]] std::string hex() const;
    friend bool operator==(const Sha256Digest&, const Sha256Digest&) = default;
};

// Incremental SHA-256 state for large artifacts. The accumulator owns only a
// single partial 64-byte block and can therefore hash files without whole-file
// materialization. `finalize()` is non-destructive and may be repeated.
class Sha256Accumulator final {
public:
    Sha256Accumulator() noexcept;

    [[nodiscard]] bool update(std::span<const std::byte> input) noexcept;
    [[nodiscard]] std::optional<Sha256Digest> finalize() const noexcept;
    [[nodiscard]] std::uint64_t byte_count() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

private:
    std::array<std::uint32_t, 8> state_{};
    std::array<std::byte, 64> tail_{};
    std::size_t tail_size_{};
    std::uint64_t total_size_{};
    bool valid_{true};
};

class Sha256 final {
public:
    [[nodiscard]] static Sha256Digest compute(
        std::span<const std::byte> input);
};

} // namespace dmc::rengine::core
