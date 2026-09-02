#include "dmc_rengine/formats/effect_pack.hpp"

#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The effect container is the first format in this project whose slot names
// are stored rather than invented. That claim is only worth anything if the
// reader refuses the cases where a line is not a name for a slot, so most of
// what follows checks the refusals.

namespace {

namespace formats = dmc::rengine::formats;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Contract = dmc3::EffectPackContract;
using Walk = dmc3::RelativeSlotWalkContract;

void put_u32(std::vector<std::byte>& bytes, std::size_t at, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[at + index] = static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
    }
}

void put_text(
    std::vector<std::byte>& bytes, std::size_t at, std::string_view text) {
    for (std::size_t index = 0U; index < text.size(); ++index) {
        bytes[at + index] = static_cast<std::byte>(text[index]);
    }
}

// Builds a PNST whose slots hold the supplied payloads, laid out the way the
// runtime's own walk reads one: a magic, a slot count, a table of
// container-relative offsets.
[[nodiscard]] std::vector<std::byte> make_pnst(
    const std::vector<std::vector<std::byte>>& payloads) {
    const auto table_end =
        Walk::offset_table_offset + payloads.size() * Walk::offset_entry_bytes;
    auto cursor = (table_end + 0x0FU) & ~static_cast<std::size_t>(0x0FU);
    std::vector<std::size_t> offsets;
    std::size_t total = cursor;
    for (const auto& payload : payloads) {
        offsets.push_back(total);
        total += payload.size();
    }

    std::vector<std::byte> bytes(total, std::byte{0});
    put_text(bytes, 0U, Walk::pnst_magic);
    put_u32(bytes, Walk::slot_count_offset,
            static_cast<std::uint32_t>(payloads.size()));
    for (std::size_t index = 0U; index < payloads.size(); ++index) {
        put_u32(
            bytes,
            Walk::offset_table_offset + index * Walk::offset_entry_bytes,
            static_cast<std::uint32_t>(offsets[index]));
        std::copy(
            payloads[index].begin(), payloads[index].end(),
            bytes.begin() + static_cast<std::ptrdiff_t>(offsets[index]));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_manifest(
    std::string_view text, std::size_t slot_bytes) {
    std::vector<std::byte> bytes(slot_bytes, std::byte{0});
    assert(text.size() <= slot_bytes);
    put_text(bytes, 0U, text);
    return bytes;
}

[[nodiscard]] std::vector<std::byte> record_of(std::size_t extent) {
    std::vector<std::byte> bytes(extent, std::byte{0});
    if (!bytes.empty()) {
        bytes[0] = std::byte{0x09};
    }
    return bytes;
}

// The corpus shape: three kinds, one line each, extents the contract names.
[[nodiscard]] std::vector<std::byte> make_effect_pack(
    std::string_view manifest,
    const std::vector<std::size_t>& extents) {
    std::vector<std::vector<std::byte>> records;
    records.reserve(extents.size());
    for (const auto extent : extents) {
        records.push_back(record_of(extent));
    }
    return make_pnst({make_manifest(manifest, 0x50U), make_pnst(records)});
}

void a_manifest_names_every_record() {
    const auto bytes = make_effect_pack(
        "V 922\r\nE 1454\r\nA 220\r\n# End\r\n",
        {Contract::extent_for('V'), Contract::extent_for('E'),
         Contract::extent_for('A')});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(parsed.ok());
    const auto& document = *parsed.document;
    assert(document.manifest_line_count == 3U);
    assert(document.populated_record_count == 3U);
    assert(document.manifest_names_every_populated_record);
    assert(document.extents_match_known_kinds);

    // The names are the manifest's own text, not a slot index dressed up.
    assert(document.records[0].name == "V 922");
    assert(document.records[0].kind == 'V');
    assert(document.records[0].identifier == 922U);
    assert(document.records[2].identifier == 220U);
    assert(document.records[1].extent == Contract::extent_for('E'));
}

// The equality of counts is the whole basis for pairing a line with a slot.
// Without it a manifest line is a line that happens to sit nearby.
void a_short_manifest_is_refused() {
    const auto bytes = make_effect_pack(
        "V 922\r\nE 1454\r\n# End\r\n",
        {Contract::extent_for('V'), Contract::extent_for('E'),
         Contract::extent_for('A')});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(!parsed.ok());
    assert(parsed.error == formats::EffectPackParseError::line_count_mismatch);
}

void a_long_manifest_is_refused() {
    const auto bytes = make_effect_pack(
        "V 922\r\nE 1454\r\nA 220\r\nP 469\r\n# End\r\n",
        {Contract::extent_for('V'), Contract::extent_for('E'),
         Contract::extent_for('A')});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(!parsed.ok());
    assert(parsed.error == formats::EffectPackParseError::line_count_mismatch);
}

void a_line_without_an_identifier_is_refused() {
    const auto bytes = make_effect_pack(
        "V 922\r\nE\r\n# End\r\n",
        {Contract::extent_for('V'), Contract::extent_for('E')});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(!parsed.ok());
    assert(parsed.error ==
           formats::EffectPackParseError::malformed_manifest_line);
}

void a_binary_first_slot_is_not_a_manifest() {
    std::vector<std::byte> binary(0x50U, std::byte{0xEE});
    const auto bytes = make_pnst(
        {binary, make_pnst({record_of(Contract::extent_for('V'))})});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(!parsed.ok());
    assert(parsed.error == formats::EffectPackParseError::manifest_not_text);
}

void an_effect_pack_has_exactly_two_slots() {
    const auto bytes = make_pnst(
        {make_manifest("V 922\r\n# End\r\n", 0x50U),
         make_pnst({record_of(Contract::extent_for('V'))}),
         record_of(0x20U)});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(!parsed.ok());
    assert(parsed.error ==
           formats::EffectPackParseError::wrong_outer_slot_count);
}

// A kind whose extent varies — `T` — must not be reported as mismatched, and a
// fixed kind at the wrong extent must be.
void a_variable_kind_does_not_fail_the_extent_check() {
    const auto bytes = make_effect_pack(
        "T 125\r\n# End\r\n", {22112U});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(parsed.ok());
    assert(parsed.document->extents_match_known_kinds);
    assert(!parsed.document->records[0].extent_matches_kind);
    assert(parsed.document->records[0].kind_known);
}

void a_fixed_kind_at_the_wrong_extent_is_reported() {
    const auto bytes = make_effect_pack("V 922\r\n# End\r\n", {512U});
    const auto parsed = formats::EffectPackParser::parse(
        std::span<const std::byte>{bytes});
    assert(parsed.ok());
    assert(!parsed.document->extents_match_known_kinds);
    assert(!parsed.document->records[0].extent_matches_kind);
}

// The contract's own numbers, held where a later edit would have to notice.
static_assert(Contract::outer_slot_count == 2U);
static_assert(Contract::manifest_slot_index == 0U);
static_assert(Contract::records_slot_index == 1U);
static_assert(Contract::extent_for('V') == 368U);
static_assert(Contract::extent_for('E') == 544U);
static_assert(Contract::extent_for('P') == 704U);
static_assert(Contract::extent_for('A') == 336U);
// `T` is the one kind whose extent varies, and zero is how that is said.
static_assert(Contract::extent_for('T') == 0U);
static_assert(Contract::is_known_kind('T'));
static_assert(!Contract::is_known_kind('Z'));
// The corpus this rests on: 20 records across two files.
static_assert(
    Contract::kinds[0].observed + Contract::kinds[1].observed +
        Contract::kinds[2].observed + Contract::kinds[3].observed +
        Contract::kinds[4].observed == 20U);
// Nothing in the image has been shown to read the manifest, and the contract
// has to keep saying so.
static_assert(!Contract::manifest_read_site_found);
// The texture dimension offset is the one the texture framing already uses.
static_assert(Contract::texture_dimensions_offset == 0x10U);

} // namespace

int main() {
    a_manifest_names_every_record();
    a_short_manifest_is_refused();
    a_long_manifest_is_refused();
    a_line_without_an_identifier_is_refused();
    a_binary_first_slot_is_not_a_manifest();
    an_effect_pack_has_exactly_two_slots();
    a_variable_kind_does_not_fail_the_extent_check();
    a_fixed_kind_at_the_wrong_extent_is_reported();
    return 0;
}
