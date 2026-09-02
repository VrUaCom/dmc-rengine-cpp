#include "dmc_rengine/gdspaces/effect_pack_names.hpp"

#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

// The bridge that carries an effect container's stored names from the outer
// container, where they are written, to the inner one, whose slots they name.
//
// Everything here is about what it refuses. A name list attached to the wrong
// container is worse than no names at all, because it looks stored.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Contract = dmc3::EffectPackContract;
using Walk = dmc3::RelativeSlotWalkContract;

void put_u32(std::vector<std::byte>& bytes, std::size_t at, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[at + index] = static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
    }
}

void put_text(std::vector<std::byte>& bytes, std::size_t at, std::string_view text) {
    for (std::size_t index = 0U; index < text.size(); ++index) {
        bytes[at + index] = static_cast<std::byte>(text[index]);
    }
}

[[nodiscard]] std::vector<std::byte> make_pnst(
    const std::vector<std::vector<std::byte>>& payloads) {
    const auto table_end =
        Walk::offset_table_offset + payloads.size() * Walk::offset_entry_bytes;
    const auto first = (table_end + 0x0FU) & ~static_cast<std::size_t>(0x0FU);
    std::size_t total = first;
    std::vector<std::size_t> offsets;
    for (const auto& payload : payloads) {
        offsets.push_back(total);
        total += payload.size();
    }
    std::vector<std::byte> bytes(total, std::byte{0});
    put_text(bytes, 0U, Walk::pnst_magic);
    put_u32(bytes, Walk::slot_count_offset,
            static_cast<std::uint32_t>(payloads.size()));
    for (std::size_t index = 0U; index < payloads.size(); ++index) {
        put_u32(bytes, Walk::offset_table_offset + index * Walk::offset_entry_bytes,
                static_cast<std::uint32_t>(offsets[index]));
        std::copy(payloads[index].begin(), payloads[index].end(),
                  bytes.begin() + static_cast<std::ptrdiff_t>(offsets[index]));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> record_of(std::size_t extent) {
    return std::vector<std::byte>(extent, std::byte{0});
}

[[nodiscard]] std::vector<std::byte> make_effect_pack(
    std::string_view manifest, const std::vector<std::size_t>& extents) {
    std::vector<std::byte> manifest_slot(0x50U, std::byte{0});
    put_text(manifest_slot, 0U, manifest);
    std::vector<std::vector<std::byte>> records;
    for (const auto extent : extents) {
        records.push_back(record_of(extent));
    }
    return make_pnst({manifest_slot, make_pnst(records)});
}

void the_manifest_names_the_record_slot() {
    const auto pack = make_effect_pack(
        "V 922\r\nE 1454\r\nA 220\r\n# End\r\n",
        {Contract::extent_for('V'), Contract::extent_for('E'),
         Contract::extent_for('A')});
    assert(gdspaces::is_effect_pack(std::span<const std::byte>{pack}));

    const auto names = gdspaces::effect_pack_slot_names(
        std::span<const std::byte>{pack},
        static_cast<std::uint32_t>(Contract::records_slot_index));
    assert(names.size() == 3U);
    assert(names[0].name == "V 922");
    assert(names[1].name == "E 1454");
    assert(names[2].name == "A 220");
    for (const auto& attribution : names) {
        // Stored, not invented — that is the whole point of this format.
        assert(attribution.origin == gdspaces::SlotNameOrigin::container_manifest);
        // Every kind here has a fixed extent and every record matched it.
        assert(attribution.corroborated_by_payload);
    }
    // The slot each name belongs to travels with it, so a sparse record
    // container cannot shift the list.
    assert(names[0].slot_index == 0U);
    assert(names[2].slot_index == 2U);
}

// Asked about any other slot, the answer is nothing — never the list shifted.
void no_other_slot_gets_the_list() {
    const auto pack = make_effect_pack(
        "V 922\r\n# End\r\n", {Contract::extent_for('V')});
    assert(gdspaces::effect_pack_slot_names(
               std::span<const std::byte>{pack},
               static_cast<std::uint32_t>(Contract::manifest_slot_index))
               .empty());
    assert(gdspaces::effect_pack_slot_names(
               std::span<const std::byte>{pack}, 2U).empty());
    assert(gdspaces::effect_pack_slot_names(
               std::span<const std::byte>{pack}, 99U).empty());
}

// A container that is not an effect pack contributes nothing, however much it
// resembles one. A near miss must not produce a plausible-looking name list.
void a_near_miss_gets_nothing() {
    // Line count and slot count disagree.
    const auto mismatched = make_effect_pack(
        "V 922\r\nE 1454\r\n# End\r\n", {Contract::extent_for('V')});
    assert(!gdspaces::is_effect_pack(std::span<const std::byte>{mismatched}));
    assert(gdspaces::effect_pack_slot_names(
               std::span<const std::byte>{mismatched},
               static_cast<std::uint32_t>(Contract::records_slot_index))
               .empty());

    // A plain PNST with no manifest at all.
    const auto plain = make_pnst({record_of(0x40U), record_of(0x40U)});
    assert(!gdspaces::is_effect_pack(std::span<const std::byte>{plain}));
    assert(gdspaces::effect_pack_slot_names(
               std::span<const std::byte>{plain},
               static_cast<std::uint32_t>(Contract::records_slot_index))
               .empty());

    // Not a container.
    const std::vector<std::byte> rubbish(0x40U, std::byte{0xEE});
    assert(!gdspaces::is_effect_pack(std::span<const std::byte>{rubbish}));
    assert(gdspaces::effect_pack_slot_names(
               std::span<const std::byte>{rubbish}, 1U).empty());
    assert(gdspaces::effect_pack_slot_names({}, 1U).empty());
}

// A kind whose extent varies cannot corroborate, and must not borrow
// confidence from the records around it.
void a_variable_kind_does_not_claim_corroboration() {
    const auto pack = make_effect_pack(
        "V 922\r\nT 125\r\n# End\r\n", {Contract::extent_for('V'), 22112U});
    const auto names = gdspaces::effect_pack_slot_names(
        std::span<const std::byte>{pack},
        static_cast<std::uint32_t>(Contract::records_slot_index));
    assert(names.size() == 2U);
    assert(names[0].corroborated_by_payload);
    // `T` is the kind with no fixed extent.
    assert(!names[1].corroborated_by_payload);
    // Both are still stored names.
    assert(names[1].origin == gdspaces::SlotNameOrigin::container_manifest);
}

} // namespace

int main() {
    the_manifest_names_the_record_slot();
    no_other_slot_gets_the_list();
    a_near_miss_gets_nothing();
    a_variable_kind_does_not_claim_corroboration();
    return 0;
}
