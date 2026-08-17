#include "dmc_rengine/core/raw_deflate.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::core {
namespace {

constexpr unsigned max_code_bits = 15U;
constexpr std::array<std::uint16_t, 29> length_base{
    3, 4, 5, 6, 7, 8, 9, 10,
    11, 13, 15, 17,
    19, 23, 27, 31,
    35, 43, 51, 59,
    67, 83, 99, 115,
    131, 163, 195, 227,
    258,
};
constexpr std::array<std::uint8_t, 29> length_extra{
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1,
    2, 2, 2, 2,
    3, 3, 3, 3,
    4, 4, 4, 4,
    5, 5, 5, 5,
    0,
};
constexpr std::array<std::uint16_t, 30> distance_base{
    1, 2, 3, 4,
    5, 7, 9, 13,
    17, 25, 33, 49,
    65, 97, 129, 193,
    257, 385, 513, 769,
    1025, 1537, 2049, 3073,
    4097, 6145, 8193, 12289,
    16385, 24577,
};
constexpr std::array<std::uint8_t, 30> distance_extra{
    0, 0, 0, 0,
    1, 1, 2, 2,
    3, 3, 4, 4,
    5, 5, 6, 6,
    7, 7, 8, 8,
    9, 9, 10, 10,
    11, 11, 12, 12,
    13, 13,
};
constexpr std::array<std::uint8_t, 19> code_length_order{
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
    11, 4, 12, 3, 13, 2, 14, 1, 15,
};

class BitReader final {
public:
    explicit BitReader(std::span<const std::byte> input) noexcept
        : input_(input) {}

    [[nodiscard]] bool read_bits(unsigned count, std::uint32_t& value) noexcept {
        if (count > 24U) {
            return false;
        }
        value = 0U;
        for (unsigned index = 0U; index < count; ++index) {
            if (byte_index_ >= input_.size()) {
                return false;
            }
            const auto current = std::to_integer<std::uint8_t>(input_[byte_index_]);
            const auto bit = static_cast<std::uint32_t>(
                (current >> bit_index_) & 1U);
            value |= bit << index;
            ++bit_index_;
            if (bit_index_ == 8U) {
                bit_index_ = 0U;
                ++byte_index_;
            }
        }
        return true;
    }

    void align_to_byte() noexcept {
        if (bit_index_ != 0U) {
            bit_index_ = 0U;
            ++byte_index_;
        }
    }

    [[nodiscard]] bool read_aligned_u16(std::uint16_t& value) noexcept {
        if (bit_index_ != 0U || byte_index_ > input_.size() ||
            input_.size() - byte_index_ < 2U) {
            return false;
        }
        value = static_cast<std::uint16_t>(
            std::to_integer<std::uint8_t>(input_[byte_index_]) |
            (static_cast<std::uint16_t>(
                 std::to_integer<std::uint8_t>(input_[byte_index_ + 1U])) << 8U));
        byte_index_ += 2U;
        return true;
    }

    [[nodiscard]] bool read_aligned_bytes(
        std::size_t count,
        std::vector<std::byte>& output) noexcept {
        if (bit_index_ != 0U || byte_index_ > input_.size() ||
            count > input_.size() - byte_index_) {
            return false;
        }
        output.insert(
            output.end(),
            input_.begin() + static_cast<std::ptrdiff_t>(byte_index_),
            input_.begin() + static_cast<std::ptrdiff_t>(byte_index_ + count));
        byte_index_ += count;
        return true;
    }

    [[nodiscard]] bool fully_consumed() const noexcept {
        if (bit_index_ == 0U) {
            return byte_index_ == input_.size();
        }
        return byte_index_ < input_.size() && byte_index_ + 1U == input_.size();
    }

private:
    std::span<const std::byte> input_;
    std::size_t byte_index_{};
    unsigned bit_index_{};
};

class HuffmanTree final {
public:
    struct DecodeResult final {
        RawDeflateStatus status{RawDeflateStatus::invalid_huffman_symbol};
        int symbol{-1};
    };

    [[nodiscard]] bool build(
        std::span<const std::uint8_t> lengths,
        bool allow_empty = false) {
        nodes_.clear();
        nodes_.push_back(Node{});

        std::array<std::uint16_t, max_code_bits + 1U> counts{};
        std::size_t symbol_count = 0U;
        for (const auto length : lengths) {
            if (length > max_code_bits) {
                return false;
            }
            if (length != 0U) {
                ++counts[length];
                ++symbol_count;
            }
        }
        if (symbol_count == 0U) {
            return allow_empty;
        }

        int remaining = 1;
        for (unsigned bits = 1U; bits <= max_code_bits; ++bits) {
            remaining <<= 1;
            remaining -= counts[bits];
            if (remaining < 0) {
                return false;
            }
        }

        std::array<std::uint16_t, max_code_bits + 1U> next_code{};
        std::uint32_t code = 0U;
        for (unsigned bits = 1U; bits <= max_code_bits; ++bits) {
            code = (code + counts[bits - 1U]) << 1U;
            next_code[bits] = static_cast<std::uint16_t>(code);
        }

        for (std::size_t symbol = 0U; symbol < lengths.size(); ++symbol) {
            const auto length = lengths[symbol];
            if (length == 0U) {
                continue;
            }

            const auto symbol_code = next_code[length]++;
            int node_index = 0;
            for (unsigned depth = 0U; depth < length; ++depth) {
                const auto shift = static_cast<unsigned>(length - depth - 1U);
                const auto bit = static_cast<unsigned>((symbol_code >> shift) & 1U);
                if (nodes_[static_cast<std::size_t>(node_index)].symbol >= 0) {
                    return false;
                }

                auto child = nodes_[static_cast<std::size_t>(node_index)].child[bit];
                if (child < 0) {
                    child = static_cast<int>(nodes_.size());
                    nodes_[static_cast<std::size_t>(node_index)].child[bit] = child;
                    nodes_.push_back(Node{});
                }
                node_index = child;
            }

            auto& leaf = nodes_[static_cast<std::size_t>(node_index)];
            if (leaf.symbol >= 0 || leaf.child[0] >= 0 || leaf.child[1] >= 0) {
                return false;
            }
            leaf.symbol = static_cast<int>(symbol);
        }
        return true;
    }

    [[nodiscard]] DecodeResult decode(BitReader& reader) const noexcept {
        int node_index = 0;
        for (unsigned depth = 0U; depth < max_code_bits; ++depth) {
            std::uint32_t bit = 0U;
            if (!reader.read_bits(1U, bit)) {
                return DecodeResult{
                    .status = RawDeflateStatus::truncated_input,
                    .symbol = -1,
                };
            }
            const auto& node = nodes_[static_cast<std::size_t>(node_index)];
            const auto next = node.child[bit];
            if (next < 0) {
                return DecodeResult{
                    .status = RawDeflateStatus::invalid_huffman_symbol,
                    .symbol = -1,
                };
            }
            node_index = next;
            const auto& candidate = nodes_[static_cast<std::size_t>(node_index)];
            if (candidate.symbol >= 0) {
                return DecodeResult{
                    .status = RawDeflateStatus::ok,
                    .symbol = candidate.symbol,
                };
            }
        }
        return DecodeResult{
            .status = RawDeflateStatus::invalid_huffman_symbol,
            .symbol = -1,
        };
    }

private:
    struct Node final {
        std::array<int, 2> child{-1, -1};
        int symbol{-1};
    };

    std::vector<Node> nodes_;
};

[[nodiscard]] RawDeflateResult failure(
    RawDeflateStatus status,
    std::string detail) {
    return RawDeflateResult{
        .status = status,
        .bytes = {},
        .detail = std::move(detail),
    };
}

[[nodiscard]] bool append_room(
    std::size_t current,
    std::uint64_t amount,
    std::size_t expected) noexcept {
    if (amount > std::numeric_limits<std::size_t>::max()) {
        return false;
    }
    const auto count = static_cast<std::size_t>(amount);
    return current <= expected && count <= expected - current;
}

[[nodiscard]] bool fixed_trees(
    HuffmanTree& literal_length,
    HuffmanTree& distance) {
    std::array<std::uint8_t, 288> literal_lengths{};
    for (std::size_t symbol = 0U; symbol <= 143U; ++symbol) {
        literal_lengths[symbol] = 8U;
    }
    for (std::size_t symbol = 144U; symbol <= 255U; ++symbol) {
        literal_lengths[symbol] = 9U;
    }
    for (std::size_t symbol = 256U; symbol <= 279U; ++symbol) {
        literal_lengths[symbol] = 7U;
    }
    for (std::size_t symbol = 280U; symbol <= 287U; ++symbol) {
        literal_lengths[symbol] = 8U;
    }
    std::array<std::uint8_t, 32> distance_lengths{};
    distance_lengths.fill(5U);
    return literal_length.build(literal_lengths) && distance.build(distance_lengths);
}

[[nodiscard]] RawDeflateStatus dynamic_trees(
    BitReader& reader,
    HuffmanTree& literal_length,
    HuffmanTree& distance) {
    std::uint32_t raw_hlit = 0U;
    std::uint32_t raw_hdist = 0U;
    std::uint32_t raw_hclen = 0U;
    if (!reader.read_bits(5U, raw_hlit) ||
        !reader.read_bits(5U, raw_hdist) ||
        !reader.read_bits(4U, raw_hclen)) {
        return RawDeflateStatus::truncated_input;
    }

    const auto hlit = raw_hlit + 257U;
    const auto hdist = raw_hdist + 1U;
    const auto hclen = raw_hclen + 4U;
    if (hlit > 286U || hdist > 32U || hclen > 19U) {
        return RawDeflateStatus::invalid_huffman_tree;
    }

    std::array<std::uint8_t, 19> code_lengths{};
    for (std::uint32_t index = 0U; index < hclen; ++index) {
        std::uint32_t value = 0U;
        if (!reader.read_bits(3U, value)) {
            return RawDeflateStatus::truncated_input;
        }
        code_lengths[code_length_order[index]] = static_cast<std::uint8_t>(value);
    }

    HuffmanTree code_length_tree;
    if (!code_length_tree.build(code_lengths)) {
        return RawDeflateStatus::invalid_huffman_tree;
    }

    const auto total = static_cast<std::size_t>(hlit + hdist);
    std::vector<std::uint8_t> lengths;
    lengths.reserve(total);
    while (lengths.size() < total) {
        const auto decoded = code_length_tree.decode(reader);
        if (decoded.status != RawDeflateStatus::ok) {
            return decoded.status;
        }

        if (decoded.symbol >= 0 && decoded.symbol <= 15) {
            lengths.push_back(static_cast<std::uint8_t>(decoded.symbol));
            continue;
        }

        std::uint32_t extra = 0U;
        std::size_t repeat = 0U;
        std::uint8_t value = 0U;
        switch (decoded.symbol) {
        case 16:
            if (lengths.empty() || !reader.read_bits(2U, extra)) {
                return lengths.empty()
                    ? RawDeflateStatus::invalid_code_length_repeat
                    : RawDeflateStatus::truncated_input;
            }
            repeat = static_cast<std::size_t>(extra + 3U);
            value = lengths.back();
            break;
        case 17:
            if (!reader.read_bits(3U, extra)) {
                return RawDeflateStatus::truncated_input;
            }
            repeat = static_cast<std::size_t>(extra + 3U);
            value = 0U;
            break;
        case 18:
            if (!reader.read_bits(7U, extra)) {
                return RawDeflateStatus::truncated_input;
            }
            repeat = static_cast<std::size_t>(extra + 11U);
            value = 0U;
            break;
        default:
            return RawDeflateStatus::invalid_huffman_symbol;
        }

        if (repeat > total - lengths.size()) {
            return RawDeflateStatus::invalid_code_length_repeat;
        }
        lengths.insert(lengths.end(), repeat, value);
    }

    const auto literal_lengths = std::span<const std::uint8_t>{
        lengths.data(), static_cast<std::size_t>(hlit)};
    const auto distance_lengths = std::span<const std::uint8_t>{
        lengths.data() + static_cast<std::ptrdiff_t>(hlit),
        static_cast<std::size_t>(hdist)};
    if (literal_lengths.size() <= 256U || literal_lengths[256U] == 0U) {
        return RawDeflateStatus::missing_end_of_block;
    }
    if (!literal_length.build(literal_lengths) ||
        !distance.build(distance_lengths, true)) {
        return RawDeflateStatus::invalid_huffman_tree;
    }
    return RawDeflateStatus::ok;
}

[[nodiscard]] RawDeflateStatus decode_compressed_block(
    BitReader& reader,
    const HuffmanTree& literal_length,
    const HuffmanTree& distance,
    std::vector<std::byte>& output,
    std::size_t expected_size) {
    for (;;) {
        const auto decoded = literal_length.decode(reader);
        if (decoded.status != RawDeflateStatus::ok) {
            return decoded.status;
        }
        const auto symbol = decoded.symbol;
        if (symbol >= 0 && symbol <= 255) {
            if (output.size() >= expected_size) {
                return RawDeflateStatus::output_limit_exceeded;
            }
            output.push_back(static_cast<std::byte>(symbol));
            continue;
        }
        if (symbol == 256) {
            return RawDeflateStatus::ok;
        }
        if (symbol < 257 || symbol > 285) {
            return RawDeflateStatus::invalid_length_symbol;
        }

        const auto length_index = static_cast<std::size_t>(symbol - 257);
        std::uint32_t length_extra_value = 0U;
        const auto length_extra_bits = length_extra[length_index];
        if (length_extra_bits != 0U &&
            !reader.read_bits(length_extra_bits, length_extra_value)) {
            return RawDeflateStatus::truncated_input;
        }
        const auto length = static_cast<std::uint64_t>(length_base[length_index]) +
            length_extra_value;

        const auto decoded_distance = distance.decode(reader);
        if (decoded_distance.status != RawDeflateStatus::ok) {
            return decoded_distance.status;
        }
        if (decoded_distance.symbol < 0 || decoded_distance.symbol >= 30) {
            return RawDeflateStatus::invalid_distance_symbol;
        }
        const auto distance_index = static_cast<std::size_t>(decoded_distance.symbol);
        std::uint32_t distance_extra_value = 0U;
        const auto distance_extra_bits = distance_extra[distance_index];
        if (distance_extra_bits != 0U &&
            !reader.read_bits(distance_extra_bits, distance_extra_value)) {
            return RawDeflateStatus::truncated_input;
        }
        const auto match_distance =
            static_cast<std::uint64_t>(distance_base[distance_index]) +
            distance_extra_value;
        if (match_distance == 0U || match_distance > output.size()) {
            return RawDeflateStatus::invalid_distance;
        }
        if (!append_room(output.size(), length, expected_size)) {
            return RawDeflateStatus::output_limit_exceeded;
        }

        const auto distance_size = static_cast<std::size_t>(match_distance);
        const auto length_size = static_cast<std::size_t>(length);
        for (std::size_t index = 0U; index < length_size; ++index) {
            output.push_back(output[output.size() - distance_size]);
        }
    }
}

} // namespace

RawDeflateResult RawDeflate::inflate(
    std::span<const std::byte> compressed,
    std::uint64_t expected_size) {
    if (expected_size > std::numeric_limits<std::size_t>::max()) {
        return failure(
            RawDeflateStatus::invalid_expected_size,
            "The expected materialized size exceeds the host addressable range.");
    }
    const auto expected = static_cast<std::size_t>(expected_size);

    RawDeflateResult result{
        .status = RawDeflateStatus::ok,
        .bytes = {},
        .detail = {},
    };
    result.bytes.reserve(std::min<std::size_t>(expected, 16U * 1024U * 1024U));

    BitReader reader(compressed);
    bool final_block = false;
    while (!final_block) {
        std::uint32_t final_value = 0U;
        std::uint32_t block_type = 0U;
        if (!reader.read_bits(1U, final_value) ||
            !reader.read_bits(2U, block_type)) {
            return failure(
                RawDeflateStatus::truncated_input,
                "The DEFLATE block header is truncated.");
        }
        final_block = final_value != 0U;

        if (block_type == 0U) {
            reader.align_to_byte();
            std::uint16_t length = 0U;
            std::uint16_t inverse_length = 0U;
            if (!reader.read_aligned_u16(length) ||
                !reader.read_aligned_u16(inverse_length)) {
                return failure(
                    RawDeflateStatus::truncated_input,
                    "A stored DEFLATE block is missing LEN/NLEN.");
            }
            if (static_cast<std::uint16_t>(~length) != inverse_length) {
                return failure(
                    RawDeflateStatus::stored_length_mismatch,
                    "A stored DEFLATE block has inconsistent LEN/NLEN values.");
            }
            if (!append_room(result.bytes.size(), length, expected)) {
                return failure(
                    RawDeflateStatus::output_limit_exceeded,
                    "A stored DEFLATE block exceeds the expected materialized size.");
            }
            if (!reader.read_aligned_bytes(length, result.bytes)) {
                return failure(
                    RawDeflateStatus::truncated_input,
                    "A stored DEFLATE block payload is truncated.");
            }
            continue;
        }

        if (block_type == 3U) {
            return failure(
                RawDeflateStatus::reserved_block_type,
                "The DEFLATE stream uses reserved BTYPE=3.");
        }

        HuffmanTree literal_length;
        HuffmanTree distance;
        if (block_type == 1U) {
            if (!fixed_trees(literal_length, distance)) {
                return failure(
                    RawDeflateStatus::invalid_huffman_tree,
                    "The fixed DEFLATE Huffman tables could not be constructed.");
            }
        } else {
            const auto status = dynamic_trees(reader, literal_length, distance);
            if (status != RawDeflateStatus::ok) {
                return failure(
                    status,
                    "The dynamic DEFLATE Huffman tables are invalid or truncated.");
            }
        }

        const auto status = decode_compressed_block(
            reader,
            literal_length,
            distance,
            result.bytes,
            expected);
        if (status != RawDeflateStatus::ok) {
            return failure(
                status,
                "The compressed DEFLATE block could not be decoded safely.");
        }
    }

    if (result.bytes.size() != expected) {
        return failure(
            RawDeflateStatus::output_size_mismatch,
            "The DEFLATE stream ended at a size different from the expected materialized size.");
    }
    if (!reader.fully_consumed()) {
        return failure(
            RawDeflateStatus::trailing_input,
            "The raw DEFLATE stream has trailing bytes after the final block.");
    }

    result.status = RawDeflateStatus::ok;
    return result;
}

} // namespace dmc::rengine::core
