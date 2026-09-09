#include "dmc_rengine/formats/mot.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/mot_contract.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// The motion payload has no usable magic — the `MOT` tag it carries is
// compared nowhere in the executable — so what identifies it is that its own
// arithmetic closes. These check that the arithmetic is enforced rather than
// assumed, because it is the only thing standing between reading a motion and
// reading whatever else happens to be there.

namespace {

namespace formats = dmc::rengine::formats;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Contract = dmc3::MotContract;

void put_u16(std::vector<std::byte>& bytes, std::size_t at, std::uint16_t value) {
    bytes[at] = static_cast<std::byte>(value & 0xFFU);
    bytes[at + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, std::size_t at, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[at + index] = static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
    }
}

// Builds a motion the way the real one is laid out: a fixed header, a track
// count, then a chain of tracks whose sizes land exactly on the terminator.
// Stamps run from the observed first stamp across the declared duration, which
// is what makes the header and the keys agree.
[[nodiscard]] std::vector<std::byte> make_motion(
    const std::vector<std::uint16_t>& key_counts,
    std::int16_t span = 650,
    std::uint32_t kind = Contract::observed_track_kind) {
    std::size_t total = Contract::observed_data_offset + Contract::track_count_bytes;
    for (const auto keys : key_counts) {
        total += Contract::track_bytes(keys, kind);
    }
    // Real payloads are padded to a 16-byte boundary and every byte of that
    // padding is zero; the four bytes this fixture used to append were one
    // instance of that, read as a terminator when only one payload was known.
    // The fixture pads like the corpus does, so the tests exercise the shape
    // the parser actually meets.
    if (total % Contract::document_alignment != 0U) {
        total += Contract::document_alignment -
            (total % Contract::document_alignment);
    }

    std::vector<std::byte> bytes(total, std::byte{0});
    put_u32(bytes, Contract::data_offset_field,
        static_cast<std::uint32_t>(Contract::observed_data_offset));
    bytes[Contract::magic_offset + 0U] = static_cast<std::byte>('M');
    bytes[Contract::magic_offset + 1U] = static_cast<std::byte>('O');
    bytes[Contract::magic_offset + 2U] = static_cast<std::byte>('T');
    const auto duration = std::bit_cast<std::uint32_t>(static_cast<float>(span));
    put_u32(bytes, Contract::duration_field, duration);
    put_u32(bytes, Contract::duration_mirror_field, duration);
    put_u32(bytes, Contract::observed_data_offset,
        static_cast<std::uint32_t>(key_counts.size()));

    // 24 channel domains imply the historical 0x50 header; one selected
    // channel per synthetic track supplies an independent count contract.
    put_u16(bytes, 0x1CU, 24U);
    for (std::size_t i = 0; i < key_counts.size(); ++i) {
        put_u16(bytes, 0x1EU + 2U * i, 1U);
    }

    auto cursor = Contract::observed_data_offset + Contract::track_count_bytes;
    for (const auto keys : key_counts) {
        put_u16(bytes, cursor + Contract::track_size_offset,
            static_cast<std::uint16_t>(Contract::track_bytes(keys, kind)));
        put_u16(bytes, cursor + Contract::track_key_count_offset, keys);
        put_u32(bytes, cursor + Contract::track_kind_offset, kind);
        for (std::uint16_t key = 0U; key < keys; ++key) {
            // First and last land on the declared span; the rest are spread
            // between them, strictly increasing as the format requires.
            // Written with the flag bit set, which is how every real key
            // carries a stamp: the low 15 bits are the position.
            const auto stamp = static_cast<std::uint16_t>(
                Contract::observed_first_stamp +
                (keys == 1U ? 0 : span * key / (keys - 1U)));
            put_u16(
                bytes,
                cursor + Contract::header_bytes_for_kind(kind) +
                    key * Contract::key_bytes_for_kind(kind),
                static_cast<std::uint16_t>(stamp | Contract::key_flag_mask));
        }
        cursor += Contract::track_bytes(keys, kind);
    }
    return bytes;
}

void a_motion_reads_as_a_chain_of_tracks() {
    const auto bytes = make_motion({97U, 119U, 100U});
    const auto parsed = formats::MotParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->track_count == 3U);
    assert(parsed.document->total_key_count == 97U + 119U + 100U);
    assert(parsed.document->duration == 650.0F);

    // The header's duration and every track's stamp span are two independent
    // places in the file, and they agree. That agreement is what raises this
    // layout above a plausible reading of one sample.
    assert(parsed.document->duration_matches_stamps);
    for (const auto& track : parsed.document->tracks) {
        assert(track.kind == Contract::observed_track_kind);
        assert(track.first_stamp == Contract::observed_first_stamp);
        assert(track.span() == 650);
        assert(track.key_offset ==
            track.track_offset + Contract::track_header_bytes);
    }
    // Tracks follow one another exactly, with no gap to hide anything in.
    assert(parsed.document->tracks[1].track_offset ==
        parsed.document->tracks[0].track_offset + Contract::track_bytes(97U));
}

void a_size_that_does_not_match_its_keys_is_refused() {
    // The identity `size == 32 + 8 * keys` is the whole basis for reading this
    // format. A track that breaks it is not a track this reader knows, and
    // reading it anyway would be reading something else.
    auto bytes = make_motion({16U});
    const auto track = Contract::observed_data_offset + Contract::track_count_bytes;
    put_u16(bytes, track + Contract::track_size_offset,
        static_cast<std::uint16_t>(Contract::track_bytes(16U) + 8U));
    const auto refused = formats::MotParser::parse(bytes);
    assert(!refused.ok());
    assert(refused.error == formats::MotParseError::track_size_mismatch);
}

void stamps_that_do_not_advance_are_refused() {
    auto bytes = make_motion({8U});
    const auto keys = Contract::observed_data_offset +
        Contract::track_count_bytes + Contract::track_header_bytes;
    // Make key 2 land on key 1's stamp.
    const auto first = static_cast<std::uint16_t>(
        static_cast<std::uint8_t>(bytes[keys + Contract::key_bytes]) |
        (static_cast<std::uint16_t>(
            static_cast<std::uint8_t>(bytes[keys + Contract::key_bytes + 1U])) << 8U));
    put_u16(bytes, keys + 2U * Contract::key_bytes, first);
    const auto refused = formats::MotParser::parse(bytes);
    assert(!refused.ok());
    assert(refused.error == formats::MotParseError::stamp_not_increasing);
}

void the_track_arithmetic_matches_the_measured_corpus() {
    // Literal (size, key_count) pairs counted off the em000 extraction, so
    // this pins the geometry against reality rather than against the fixture
    // that shares the contract's own arithmetic.
    //
    // Kind 2, the six distinct shapes observed across its 156 tracks:
    static_assert(Contract::track_bytes(1U, Contract::compact_track_kind) == 20U);
    static_assert(Contract::track_bytes(2U, Contract::compact_track_kind) == 24U);
    static_assert(Contract::track_bytes(3U, Contract::compact_track_kind) == 28U);
    static_assert(Contract::track_bytes(4U, Contract::compact_track_kind) == 32U);
    static_assert(Contract::track_bytes(5U, Contract::compact_track_kind) == 36U);
    static_assert(Contract::track_bytes(10U, Contract::compact_track_kind) == 56U);

    // Kind 3, from its 4,962:
    static_assert(Contract::track_bytes(1U, Contract::observed_track_kind) == 40U);
    static_assert(Contract::track_bytes(2U, Contract::observed_track_kind) == 48U);
    static_assert(Contract::track_bytes(3U, Contract::observed_track_kind) == 56U);
    static_assert(Contract::track_bytes(5U, Contract::observed_track_kind) == 72U);
    static_assert(Contract::track_bytes(9U, Contract::observed_track_kind) == 104U);

    // And the two kinds genuinely differ, which is the whole point.
    static_assert(
        Contract::track_bytes(4U, Contract::compact_track_kind) !=
        Contract::track_bytes(4U, Contract::observed_track_kind));
}

void a_compact_track_is_read_by_its_own_geometry() {
    // The second track kind, recovered from a complete em000 extraction.
    //
    // Kind 2 carries a 16-byte header and 4-byte keys where kind 3 carries 32
    // and 8. This reader applied kind 3's arithmetic to every track, so any
    // motion containing one compact track was refused outright: 72 of the 82
    // real motions in that corpus, and the ten it accepted were simply the ten
    // with no compact track in them.
    const auto bytes = make_motion({4U, 2U}, 650, Contract::compact_track_kind);
    const auto parsed = formats::MotParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->track_count == 2U);
    assert(parsed.document->total_key_count == 6U);

    const auto& track = parsed.document->tracks.front();
    assert(track.kind == Contract::compact_track_kind);
    // Keys start straight after a 16-byte header, not a 32-byte one.
    assert(
        track.key_offset ==
        Contract::observed_data_offset + Contract::track_count_bytes +
            Contract::compact_track_header_bytes);

    // Kind 3 in the same reader still reads by its own geometry.
    const auto wide = formats::MotParser::parse(make_motion({4U, 2U}));
    assert(wide.ok());
    assert(wide.document->tracks.front().kind == Contract::observed_track_kind);
    assert(
        wide.document->tracks.front().key_offset ==
        Contract::observed_data_offset + Contract::track_count_bytes +
            Contract::track_header_bytes);
}

void a_stamp_is_fifteen_bits_under_a_flag() {
    // Every key's leading value carries the flag in its top bit. Read as a
    // signed 16-bit stamp, a set flag reads as -32768 and a timeline that
    // rises past 0x8000 looks like it falls — which is what made three real
    // motions look corrupt.
    const auto bytes = make_motion({5U});
    const auto parsed = formats::MotParser::parse(bytes);
    assert(parsed.ok());

    const auto& track = parsed.document->tracks.front();
    assert(track.first_stamp == 0U);
    assert(track.last_stamp == 650U);
    assert(track.span() == 650);
    // The fixture sets the flag on every key, as the corpus does on all but
    // 465 of its 94,875.
    assert(track.flagged_key_count == 5U);
}

void a_chain_that_misses_the_terminator_is_refused() {
    // Landing early or late means the declared sizes describe a different file
    // than the one supplied, which is exactly the case a magic-free format has
    // to catch for itself.
    auto bytes = make_motion({16U, 16U});
    bytes.push_back(std::byte{0});
    assert(!formats::MotParser::structurally_valid(bytes));

    auto short_chain = make_motion({16U, 16U});
    put_u32(short_chain, Contract::observed_data_offset, 1U);
    const auto refused = formats::MotParser::parse(short_chain);
    assert(!refused.ok());
    assert(refused.error == formats::MotParseError::chain_does_not_close);
}

void an_unrelated_payload_is_not_a_motion() {
    // The tag alone must not be enough: a payload carrying `MOT` at the right
    // offset and nothing else is refused, because the runtime never reads that
    // tag. Product validation also checks the marker and channel grammar.
    std::vector<std::byte> impostor(0x200U, std::byte{0});
    impostor[Contract::magic_offset + 0U] = static_cast<std::byte>('M');
    impostor[Contract::magic_offset + 1U] = static_cast<std::byte>('O');
    impostor[Contract::magic_offset + 2U] = static_cast<std::byte>('T');
    put_u32(impostor, Contract::data_offset_field,
        static_cast<std::uint32_t>(Contract::observed_data_offset));
    assert(!formats::MotParser::structurally_valid(impostor));
}

void the_classifier_names_it_from_the_structure() {
    const auto bytes = make_motion({32U, 40U});
    // A slot has no name of its own, so the structure is all there is.
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{bytes});
    assert(classified.format == "mot");
    // The bytes outrank the path: this one says `.bin` and is a motion.
    assert(classified.structural_confirmed);
    // There is no magic comparison behind this, and the classification must
    // not claim one.
    assert(!classified.magic_confirmed);
    assert(!classified.container);

    // The registry that types a motion does so by extension, and this is the
    // extension it uses.
    using Animation = dmc3::AnimationTypeContract;
    assert(Animation::type_for_extension(".mot") == Animation::TypeCode::motion);
    static_assert(!Animation::payload_tag_is_compared);
    static_assert(Contract::track_bytes(0U) == Contract::track_header_bytes);
    static_assert(Contract::track_bytes(97U) == 808U);
    static_assert(
        Contract::canonical_target_sha256 == Animation::canonical_target_sha256);
}

} // namespace

int main() {
    a_motion_reads_as_a_chain_of_tracks();
    a_size_that_does_not_match_its_keys_is_refused();
    stamps_that_do_not_advance_are_refused();
    the_track_arithmetic_matches_the_measured_corpus();
    a_compact_track_is_read_by_its_own_geometry();
    a_stamp_is_fifteen_bits_under_a_flag();
    a_chain_that_misses_the_terminator_is_refused();
    an_unrelated_payload_is_not_a_motion();
    the_classifier_names_it_from_the_structure();
    return 0;
}
