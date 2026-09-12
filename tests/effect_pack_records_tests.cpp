// A pack may hold records its manifest does not name.
//
// `EffectPackParser` had no test at all, which is how the rule below survived:
// it required the manifest's line count to equal the container's populated
// record count. That was true of the two packs it was recovered from
// (st001_effect.pac, st114_effect.pac) and is not a property of the format.
//
// The complete em000 extraction supplied 2026-09-08 holds a pack of **183
// populated records against 173 manifest lines**. Requiring equality refused
// the whole pack, so all 173 named records lost their names over ten the
// manifest never claimed — which is why every effect record in that archive
// reached the browser as a placeholder.
//
// The ten are byte-identical: a `0x31` word followed by twelve zeros, each in
// the slot immediately after a record the manifest calls `M`. Setting exactly
// those aside, line k names record k for all 173 — kind and identifier, in
// order, with no exception. That mapping is what this file pins.
//
// The same corpus also moved two kind readings. `G` and `M` were absent from
// the earlier two packs entirely, and `P` is not the fixed 704 those two
// showed: em000 holds 34 P records across four extents. A fixed extent
// recovered from two samples is the same mistake the MOT reader made with its
// single track kind.

#include "dmc_rengine/formats/effect_pack.hpp"

#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace formats = dmc::rengine::formats;
using Contract = dmc::rengine::profiles::dmc3::EffectPackContract;

void put_u32(
    std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] =
            static_cast<std::byte>((value >> (index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> text_bytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_pnst(
    const std::vector<std::optional<std::vector<std::byte>>>& slots) {
    const auto slot_count = static_cast<std::uint32_t>(slots.size());
    const auto header_size = 8U + slots.size() * 4U;
    std::vector<std::byte> bytes(header_size, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, slot_count);

    std::size_t next = header_size;
    for (std::size_t slot = 0U; slot < slots.size(); ++slot) {
        if (!slots[slot].has_value() || slots[slot]->empty()) {
            continue;
        }
        put_u32(bytes, 8U + slot * 4U, static_cast<std::uint32_t>(next));
        bytes.insert(bytes.end(), slots[slot]->begin(), slots[slot]->end());
        next += slots[slot]->size();
    }
    return bytes;
}

/// The sixteen bytes every companion in the corpus carries.
[[nodiscard]] std::vector<std::byte> companion_record() {
    std::vector<std::byte> bytes(Contract::companion_record_size, std::byte{0});
    put_u32(bytes, 0U, Contract::companion_leading_word);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> record_of(std::size_t size, std::byte fill) {
    return std::vector<std::byte>(size, fill);
}

/// A two-slot pack: the manifest, then the container the manifest names.
[[nodiscard]] std::vector<std::byte> make_pack(
    std::string_view manifest,
    const std::vector<std::optional<std::vector<std::byte>>>& records) {
    return make_pnst({text_bytes(manifest), make_pnst(records)});
}

// The shape em000 has, in miniature: an `M` record with a companion behind it,
// and named records either side.
void a_companion_is_not_a_named_record() {
    const auto pack = make_pack(
        "V 26\r\nM 17\r\nG 608\r\n# End\r\n",
        {
            record_of(368U, std::byte{0xA1}),  // V
            record_of(2000U, std::byte{0xA2}), // M
            companion_record(),                // named by nothing
            record_of(96U, std::byte{0xA3}),   // G
        });

    const auto parsed = formats::EffectPackParser::parse(pack);
    assert(parsed.ok());
    const auto& document = *parsed.document;

    assert(document.manifest_line_count == 3U);
    assert(document.populated_record_count == 4U);
    assert(document.companion_record_count == 1U);
    assert(document.manifest_names_every_populated_record);

    // Line k names record k once the companion is set aside — and the records
    // it names are the ones on either side of it, not shifted by it.
    assert(document.records.size() == 3U);
    assert(document.records[0].kind == 'V');
    assert(document.records[0].identifier == 26U);
    assert(document.records[1].kind == 'M');
    assert(document.records[1].identifier == 17U);
    assert(document.records[2].kind == 'G');
    assert(document.records[2].identifier == 608U);

    // The companion's slot is not one of the named ones.
    for (const auto& record : document.records) {
        assert(record.slot_index != 2U);
    }
}

// The rule the corpus contradicted. A pack with no companions still has to
// hold, because that is what the earlier two packs are.
void a_pack_without_companions_is_unchanged() {
    const auto pack = make_pack(
        "V 26\r\nA 3\r\n# End\r\n",
        {
            record_of(368U, std::byte{0xB1}),
            record_of(336U, std::byte{0xB2}),
        });

    const auto parsed = formats::EffectPackParser::parse(pack);
    assert(parsed.ok());
    assert(parsed.document->manifest_line_count == 2U);
    assert(parsed.document->populated_record_count == 2U);
    assert(parsed.document->companion_record_count == 0U);
    assert(parsed.document->records.size() == 2U);
}

// A record left over after the companions are set aside is still a refusal:
// the reader may not invent a name for a payload the manifest does not claim.
void an_unexplained_extra_record_is_still_refused() {
    const auto pack = make_pack(
        "V 26\r\n# End\r\n",
        {
            record_of(368U, std::byte{0xC1}),
            record_of(368U, std::byte{0xC2}),  // not a companion, not named
        });

    const auto parsed = formats::EffectPackParser::parse(pack);
    assert(!parsed.ok());
    assert(parsed.error == formats::EffectPackParseError::line_count_mismatch);
}

// The companion is recognized by its bytes, not by what precedes it: all ten
// in the corpus are the same sixteen bytes and none carries an identifier, so
// an identifier match would have been a rule the payloads cannot support.
void the_companion_is_a_constant() {
    const auto companion = companion_record();
    assert(Contract::is_companion_record(companion));

    auto wrong_word = companion;
    put_u32(wrong_word, 0U, Contract::companion_leading_word + 1U);
    assert(!Contract::is_companion_record(wrong_word));

    auto dirty_tail = companion;
    dirty_tail[8U] = std::byte{1};
    assert(!Contract::is_companion_record(dirty_tail));

    // Sixteen bytes is part of the rule, not an accident of the check.
    assert(!Contract::is_companion_record(record_of(15U, std::byte{0})));
    assert(!Contract::is_companion_record(record_of(17U, std::byte{0})));
    assert(!Contract::is_companion_record({}));
}

// What the corpus says about kinds, held as literals so a future edit to the
// table has to disagree with the measurement out loud.
void the_kind_table_carries_what_the_corpus_holds() {
    static_assert(Contract::kinds.size() == 7U);
    static_assert(Contract::is_known_kind('G'));
    static_assert(Contract::is_known_kind('M'));

    // Fixed extents the corpus confirms.
    static_assert(Contract::extent_for('V') == 368U);
    static_assert(Contract::extent_for('E') == 544U);
    static_assert(Contract::extent_for('A') == 336U);
    static_assert(Contract::extent_for('G') == 96U);

    // Variable. `P` earned its 0 by contradiction: 34 records across four
    // extents, against an earlier reading of a fixed 704 taken from two.
    static_assert(Contract::extent_for('T') == 0U);
    static_assert(Contract::extent_for('M') == 0U);
    static_assert(Contract::extent_for('P') == 0U);
    static_assert(Contract::observed_p_extents.size() == 4U);
    static_assert(Contract::observed_p_extents[0] == 336U);
    static_assert(Contract::observed_p_extents[3] == 896U);

    static_assert(!Contract::is_known_kind('Z'));
    static_assert(Contract::extent_for('Z') == 0U);
}

// The counts em000 actually holds, so the arithmetic that made this change
// necessary is written down where it can be checked rather than recalled.
void the_corpus_arithmetic_is_recorded() {
    constexpr std::size_t manifest_lines = 173U;
    constexpr std::size_t populated_records = 183U;
    static_assert(
        manifest_lines + Contract::companion_observed_count == populated_records);
    static_assert(Contract::companion_observed_after_kind == 'M');
}

} // namespace

int main() {
    a_companion_is_not_a_named_record();
    a_pack_without_companions_is_unchanged();
    an_unexplained_extra_record_is_still_refused();
    the_companion_is_a_constant();
    the_kind_table_carries_what_the_corpus_holds();
    the_corpus_arithmetic_is_recorded();
    std::cout << "effect_pack_records_tests: all assertions held\n";
    return EXIT_SUCCESS;
}
