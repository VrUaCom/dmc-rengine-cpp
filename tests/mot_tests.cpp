#include "dmc_rengine/formats/mot.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

template <typename T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

// One track, two keys, built from MotContract's own recovered offsets: the
// track's declared size must equal 32 + 8 * key_count, and the chain must
// close exactly on a four-byte zero terminator. Key stamps -32768 and -32118
// mirror the real corpus span (650 ticks) the contract documents.
[[nodiscard]] std::vector<std::byte> one_track_fixture() {
    constexpr std::uint32_t data_offset = 0x50U;
    constexpr std::size_t track_offset = data_offset + 4U;
    constexpr std::size_t key_offset = track_offset + 0x20U;
    constexpr std::size_t track_size = 0x20U + 2U * 8U; // 48
    constexpr std::size_t total = track_offset + track_size + 4U; // + terminator

    std::vector<std::byte> bytes(total, std::byte{0});
    put<std::uint32_t>(bytes, 0x00U, data_offset);
    bytes[0x04U] = std::byte{'M'};
    bytes[0x05U] = std::byte{'O'};
    bytes[0x06U] = std::byte{'T'};
    put<float>(bytes, 0x0CU, 650.0F);

    put<std::uint32_t>(bytes, data_offset, 1U); // track_count

    put<std::uint16_t>(bytes, track_offset + 0x00U, static_cast<std::uint16_t>(track_size));
    put<std::uint16_t>(bytes, track_offset + 0x02U, 2U); // key_count
    put<std::uint32_t>(bytes, track_offset + 0x04U, 3U); // observed track kind

    put<std::int16_t>(bytes, key_offset, static_cast<std::int16_t>(-32768));
    put<std::int16_t>(bytes, key_offset + 8U, static_cast<std::int16_t>(-32118));

    // Terminator at track_offset + track_size is already zero.
    return bytes;
}

void test_empty_bytes_are_rejected() {
    using namespace dmc::rengine::formats;
    const auto result = MotParser::parse({});
    assert(!result.ok());
    assert(result.error == MotParseError::truncated_header);
}

void test_wrong_magic_is_rejected() {
    using namespace dmc::rengine::formats;
    auto bytes = one_track_fixture();
    bytes[0x04U] = std::byte{'X'};
    const auto result = MotParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == MotParseError::invalid_magic);
}

void test_one_track_document_is_accepted() {
    using namespace dmc::rengine::formats;
    const auto bytes = one_track_fixture();
    const auto result = MotParser::parse(bytes);
    assert(result.ok());
    assert(result.document->track_count == 1U);
    assert(result.document->tracks.size() == 1U);
    const auto& track = result.document->tracks.front();
    assert(track.key_count == 2U);
    assert(track.kind == 3U);
    assert(track.first_stamp == -32768);
    assert(track.last_stamp == -32118);
    assert(track.span() == 650);
    assert(result.document->duration_matches_stamps);
    assert(result.document->valid());
    assert(MotParser::structurally_valid(bytes));
}

// The chain closing exactly on the terminator is the format's own identity
// check: one extra byte and the reader must refuse rather than guess.
void test_trailing_byte_after_terminator_is_rejected() {
    using namespace dmc::rengine::formats;
    auto bytes = one_track_fixture();
    bytes.push_back(std::byte{0});
    const auto result = MotParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == MotParseError::chain_does_not_close);
}

// A key stamp that does not strictly increase means the run is not a
// timeline, which the parser must not silently accept.
void test_non_increasing_stamp_is_rejected() {
    using namespace dmc::rengine::formats;
    constexpr std::uint32_t data_offset = 0x50U;
    constexpr std::size_t track_offset = data_offset + 4U;
    constexpr std::size_t key_offset = track_offset + 0x20U;

    auto bytes = one_track_fixture();
    put<std::int16_t>(bytes, key_offset + 8U, static_cast<std::int16_t>(-32768));
    const auto result = MotParser::parse(bytes);
    assert(!result.ok());
    assert(result.error == MotParseError::stamp_not_increasing);
}

} // namespace

int main() {
    test_empty_bytes_are_rejected();
    test_wrong_magic_is_rejected();
    test_one_track_document_is_accepted();
    test_trailing_byte_after_terminator_is_rejected();
    test_non_increasing_stamp_is_rejected();
    return 0;
}
