#include "dmc_rengine/gdspaces/container_index_probe.hpp"

#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

// Finding the index in every kind of archive, rather than in the one kind we
// happened to look for.
//
// A census of every text slot in every container of the corpus finds 26
// candidates and four real indexes, in two dialects. The rest are tagged
// binary records whose first bytes are printable — `CAM`, `EVE`, `POS`,
// `HITS` — and text that names nothing: `# END`, `# GAME`, `# DOOR 0`.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Probe = gdspaces::ContainerIndexProbe;
using Dialect = gdspaces::ContainerIndexDialect;
using Walk = dmc3::RelativeSlotWalkContract;
using Effect = dmc3::EffectPackContract;

void put_u32(std::vector<std::byte>& b, std::size_t at, std::uint32_t v) {
    for (std::size_t i = 0U; i < 4U; ++i) {
        b[at + i] = static_cast<std::byte>((v >> (8U * i)) & 0xFFU);
    }
}
void put_text(std::vector<std::byte>& b, std::size_t at, std::string_view t) {
    for (std::size_t i = 0U; i < t.size(); ++i) {
        b[at + i] = static_cast<std::byte>(t[i]);
    }
}

[[nodiscard]] std::vector<std::byte> make_container(
    std::string_view magic, const std::vector<std::vector<std::byte>>& payloads) {
    const auto table_end = 8U + payloads.size() * 4U;
    const auto first = (table_end + 0x0FU) & ~static_cast<std::size_t>(0x0FU);
    std::size_t total = first;
    std::vector<std::size_t> offsets;
    for (const auto& p : payloads) {
        offsets.push_back(total);
        total += p.size();
    }
    std::vector<std::byte> bytes(total, std::byte{0});
    put_text(bytes, 0U, magic);
    put_u32(bytes, 4U, static_cast<std::uint32_t>(payloads.size()));
    for (std::size_t i = 0U; i < payloads.size(); ++i) {
        put_u32(bytes, 8U + i * 4U, static_cast<std::uint32_t>(offsets[i]));
        std::copy(payloads[i].begin(), payloads[i].end(),
                  bytes.begin() + static_cast<std::ptrdiff_t>(offsets[i]));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> text_slot(
    std::string_view text, std::size_t bytes) {
    std::vector<std::byte> slot(bytes, std::byte{0});
    put_text(slot, 0U, text);
    return slot;
}

[[nodiscard]] std::vector<std::byte> tagged(std::string_view tag, std::size_t n) {
    std::vector<std::byte> slot(n, std::byte{0});
    put_text(slot, 0U, tag);
    slot[n - 1U] = std::byte{0x7F};
    return slot;
}

// A stage container: slot 0 lists filenames, which name its own siblings.
void a_stage_container_index_names_its_own_slots() {
    auto scm = tagged("SCM ", 0x40U);
    const auto container = make_container(
        Walk::pac_magic,
        {text_slot("st001.ptx\r\nst001.scm\r\nst001.sch\r\n", 0x30U),
         tagged("PTX", 0x40U), scm, tagged("HITS", 0x40U)});

    const auto result = Probe::probe(std::span<const std::byte>{container});
    assert(result.found());
    assert(result.dialect == Dialect::filename_list);
    assert(result.index_slot_index == gdspaces::SlotNameManifest::k_manifest_slot);
    assert(result.named_slot_count == 3U);
    // Its own siblings, not a sibling container's children. Getting this
    // backwards would apply every name one level off.
    assert(!result.names_a_sibling_container);
}

// An effect container: slot 0 lists kinds and identifiers, which name the
// children of the container in slot 1.
void an_effect_container_index_names_a_sibling() {
    std::vector<std::vector<std::byte>> records;
    for (const auto kind : {'V', 'E', 'A'}) {
        records.push_back(std::vector<std::byte>(Effect::extent_for(kind), std::byte{0}));
    }
    const auto container = make_container(
        Walk::pnst_magic,
        {text_slot("V 922\r\nE 1454\r\nA 220\r\n# End\r\n", 0x50U),
         make_container(Walk::pnst_magic, records)});

    const auto result = Probe::probe(std::span<const std::byte>{container});
    assert(result.found());
    assert(result.dialect == Dialect::kind_and_identifier);
    assert(result.index_slot_index ==
           static_cast<std::uint32_t>(Effect::manifest_slot_index));
    assert(result.named_slot_count == 3U);
    assert(result.names_a_sibling_container);
    assert(result.named_sibling_slot_index ==
           static_cast<std::uint32_t>(Effect::records_slot_index));
}

// Text that names nothing is not an index. All three of these are real slot-0
// payloads from the corpus.
void text_that_names_nothing_is_not_an_index() {
    for (const auto* block : {"# END\r\n", "# GAME\r\n", "# DOOR 0\r\n"}) {
        const auto container = make_container(
            Walk::pac_magic,
            {text_slot(block, 0x20U), tagged("LIG2", 0x40U), tagged("CAM", 0x40U)});
        const auto result = Probe::probe(std::span<const std::byte>{container});
        assert(!result.found());
        assert(result.dialect == Dialect::none);
        assert(result.named_slot_count == 0U);
    }
}

// The census's false positives: a tagged binary record whose first bytes are
// printable reads as one line of text until you count the lines.
void a_tagged_record_is_not_an_index() {
    for (const auto* tag : {"CAM", "EVE", "POS", "ITM", "STE", "DCA", "HITS"}) {
        const auto container = make_container(
            Walk::pac_magic, {tagged(tag, 0x80U), tagged("LIG2", 0x40U)});
        assert(!Probe::probe(std::span<const std::byte>{container}).found());
    }
}

void a_container_without_a_first_slot_has_no_index() {
    // An absent slot 0.
    auto bytes = make_container(
        Walk::pac_magic, {tagged("LIG2", 0x40U), tagged("CAM", 0x40U)});
    put_u32(bytes, 8U, 0U);
    assert(!Probe::probe(std::span<const std::byte>{bytes}).found());

    // Not a container at all.
    const std::vector<std::byte> rubbish(0x40U, std::byte{0xEE});
    assert(!Probe::probe(std::span<const std::byte>{rubbish}).found());
    assert(!Probe::probe({}).found());
    const std::vector<std::byte> tiny(4U, std::byte{0});
    assert(!Probe::probe(std::span<const std::byte>{tiny}).found());
}

// The two dialects must stay distinguishable: an effect manifest's lines carry
// no extension, so the filename reader refuses them, and a filename list is
// not a two-slot pack.
void the_dialects_do_not_collide() {
    const auto effect_lines = text_slot("V 922\r\nE 1454\r\n# End\r\n", 0x40U);
    assert(gdspaces::SlotNameManifest::parse(
               std::span<const std::byte>{effect_lines}).empty());

    // A filename list in a two-slot container is still the filename dialect:
    // it is not a valid effect pack, because its lines are not kind/id.
    const auto container = make_container(
        Walk::pnst_magic,
        {text_slot("thing.scm\r\n", 0x20U), tagged("SCM ", 0x40U)});
    const auto result = Probe::probe(std::span<const std::byte>{container});
    assert(result.dialect == Dialect::filename_list);
    assert(!result.names_a_sibling_container);
}

static_assert(gdspaces::to_string(Dialect::none) == "none");
static_assert(gdspaces::to_string(Dialect::filename_list) == "filename-list");
static_assert(
    gdspaces::to_string(Dialect::kind_and_identifier) == "kind-and-identifier");
// Both dialects put the index in slot 0, recorded rather than assumed.
static_assert(Effect::manifest_slot_index == 0U);
static_assert(gdspaces::SlotNameManifest::k_manifest_slot == 0U);

} // namespace

int main() {
    a_stage_container_index_names_its_own_slots();
    an_effect_container_index_names_a_sibling();
    text_that_names_nothing_is_not_an_index();
    a_tagged_record_is_not_an_index();
    a_container_without_a_first_slot_has_no_index();
    the_dialects_do_not_collide();
    return 0;
}
