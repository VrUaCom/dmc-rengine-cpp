#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::core {

enum class RawDeflateStatus {
    ok,
    invalid_expected_size,
    truncated_input,
    trailing_input,
    reserved_block_type,
    stored_length_mismatch,
    invalid_huffman_tree,
    invalid_huffman_symbol,
    invalid_code_length_repeat,
    missing_end_of_block,
    invalid_length_symbol,
    invalid_distance_symbol,
    invalid_distance,
    output_limit_exceeded,
    output_size_mismatch,
};

[[nodiscard]] constexpr const char* to_string(
    RawDeflateStatus status) noexcept {
    switch (status) {
    case RawDeflateStatus::ok: return "ok";
    case RawDeflateStatus::invalid_expected_size: return "invalid-expected-size";
    case RawDeflateStatus::truncated_input: return "truncated-input";
    case RawDeflateStatus::trailing_input: return "trailing-input";
    case RawDeflateStatus::reserved_block_type: return "reserved-block-type";
    case RawDeflateStatus::stored_length_mismatch: return "stored-length-mismatch";
    case RawDeflateStatus::invalid_huffman_tree: return "invalid-huffman-tree";
    case RawDeflateStatus::invalid_huffman_symbol: return "invalid-huffman-symbol";
    case RawDeflateStatus::invalid_code_length_repeat: return "invalid-code-length-repeat";
    case RawDeflateStatus::missing_end_of_block: return "missing-end-of-block";
    case RawDeflateStatus::invalid_length_symbol: return "invalid-length-symbol";
    case RawDeflateStatus::invalid_distance_symbol: return "invalid-distance-symbol";
    case RawDeflateStatus::invalid_distance: return "invalid-distance";
    case RawDeflateStatus::output_limit_exceeded: return "output-limit-exceeded";
    case RawDeflateStatus::output_size_mismatch: return "output-size-mismatch";
    }
    return "invalid-huffman-symbol";
}

struct RawDeflateResult final {
    RawDeflateStatus status{RawDeflateStatus::truncated_input};
    std::vector<std::byte> bytes;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RawDeflateStatus::ok;
    }
};

class RawDeflate final {
public:
    [[nodiscard]] static RawDeflateResult inflate(
        std::span<const std::byte> compressed,
        std::uint64_t expected_size);
};

} // namespace dmc::rengine::core
