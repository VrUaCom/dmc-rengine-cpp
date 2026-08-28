#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/loose_container_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/slot_zero_manifest_contract.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

// The question this project carried for a long time: does the game read a
// stage container's slot 0 name list?
//
// It does not. It reaches slot 0 — the walk starts there — hands it to the
// dispatcher, and the dispatcher finds nothing it recognizes and returns. The
// assertions here hold each step of that, and the reasons the names could not
// have been used even if they had been read.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Verdict = dmc3::SlotZeroManifestContract;
using Walk = dmc3::RelativeSlotWalkContract;
using Resource = dmc3::ResourceTypeContract;

// The real slot 0 of st001.pac, byte for byte.
constexpr std::string_view k_real_manifest =
    "st001.ptx\r\nst001.scm\r\nst001.sch\r\n";

void the_walk_reaches_slot_zero() {
    // The index register starts at 2 and the load is [base + index*4], so the
    // first read is byte 8 — the first offset-table entry.
    static_assert(Verdict::pac_walk_first_dword_index == 2U);
    static_assert(
        Verdict::pac_walk_first_dword_index * 4U == Verdict::offset_table_offset);
    static_assert(Verdict::offset_table_offset == Walk::offset_table_offset);
    static_assert(Verdict::slot_zero_is_dispatched);
    // Slot 0 is not skipped by the absent-slot rule either: a real stage
    // container gives it a non-zero offset.
    static_assert(Walk::absent_slot_offset == 0U);
}

// And every branch of the dispatcher turns on a first byte the manifest does
// not have.
void the_dispatcher_recognizes_nothing_in_it() {
    assert(!k_real_manifest.empty());
    const auto first = k_real_manifest.front();
    assert(first == Verdict::manifest_first_byte);
    for (const auto alternative : Verdict::dispatcher_first_byte_alternatives) {
        assert(first != alternative);
    }
    // The four tags the dispatcher handles all begin with one of those bytes,
    // so a payload failing all four cannot reach a handler.
    for (const auto tag : Walk::dispatched_payload_tags) {
        assert(!tag.empty());
        assert(std::find(
                   Verdict::dispatcher_first_byte_alternatives.begin(),
                   Verdict::dispatcher_first_byte_alternatives.end(),
                   tag.front()) !=
               Verdict::dispatcher_first_byte_alternatives.end());
        assert(tag.front() != first);
    }
    static_assert(!Verdict::dispatcher_handles_the_manifest);
    static_assert(Verdict::dispatcher_va == Walk::pnst_walk_va);
}

// Nothing else in the image walks a container, so there is no second path that
// could read slot 0 as text.
void nothing_else_walks_a_container() {
    static_assert(Verdict::magic_checked_walks == 2U);
    static_assert(
        Verdict::container_relative_walk_sites > Verdict::magic_checked_walks);
    static_assert(Verdict::pac_magic_compare_va != Verdict::pnst_magic_compare_va);
    // The two magic-checked walks are the two this project already models.
    static_assert(Verdict::pnst_magic_compare_va > Walk::pnst_walk_va);
    static_assert(Verdict::pac_magic_compare_va > Walk::pac_walk_va);
    // And neither the resolver nor the semantic materialization path has one.
    static_assert(!Verdict::walk_in_resolver_range);
    static_assert(!Verdict::walk_in_materialization_range);
    static_assert(Verdict::resolver_range_first < Verdict::resolver_range_last);
    static_assert(
        Verdict::materialization_range_first < Verdict::materialization_range_last);
}

// Even read, the names could not have typed anything.
void the_names_could_not_have_been_used() {
    static_assert(Verdict::sch_occurrences_in_image == 0U);
    static_assert(Verdict::scm_string_occurrences_in_image == 0U);
    static_assert(!Verdict::names_are_constructible_by_format_string);

    // `SCM` is recognized by magic and has no extension entry, so a `.scm`
    // line says nothing the payload's own tag does not.
    static_assert(!Verdict::scm_has_an_extension_entry);
    assert(Resource::type_for_name("st001.scm") == Resource::TypeCode::unknown);
    // `.sch` matches nothing at all.
    static_assert(!Verdict::sch_has_an_extension_entry);
    assert(Resource::type_for_name("st001.sch") == Resource::TypeCode::unknown);
    // `.ptx` is the one that does resolve — which is why it was the line that
    // made the whole manifest look like a runtime authority.
    assert(
        Resource::type_for_name("st001.ptx") == Resource::TypeCode::texture_pack);
}

void the_verdict_is_recorded_both_ways() {
    static_assert(!Verdict::runtime_consults_the_manifest);
    static_assert(Verdict::manifest_is_build_metadata);
    // Establishing that the game ignores it changes its authority, not its
    // accuracy — the product still reads and shows it.
    static_assert(Verdict::manifest_remains_worth_showing);

    const std::vector<std::byte> slot_zero = [] {
        std::vector<std::byte> bytes;
        for (const auto value : k_real_manifest) {
            bytes.push_back(static_cast<std::byte>(value));
        }
        bytes.resize(48U, std::byte{0});
        return bytes;
    }();
    const auto names = gdspaces::SlotNameManifest::parse(
        std::span<const std::byte>{slot_zero});
    assert(names.size() == 3U);
    assert(names[0] == "st001.ptx");
    assert(names[2] == "st001.sch");
    // And the mapping this project attributes stays what it was.
    assert(gdspaces::SlotNameManifest::slot_for_line(0U) == 1U);
    assert(gdspaces::SlotNameManifest::slot_for_line(2U) == 3U);
}

// The runtime's own naming authority for a loose container is `.lst`, which is
// a separate recovery. Pointing at it is the useful half of the negative
// result: the question was not unanswerable, it was aimed at the wrong file.
void the_loose_representation_is_the_real_authority() {
    static_assert(
        Verdict::loose_representation_selector_va ==
        dmc3::LooseContainerContract::representation_selector_va);
    static_assert(
        Verdict::canonical_target_sha256 ==
        dmc3::LooseContainerContract::canonical_target_sha256);
    static_assert(
        Verdict::canonical_target_sha256 == Walk::canonical_target_sha256);
}

} // namespace

int main() {
    the_walk_reaches_slot_zero();
    the_dispatcher_recognizes_nothing_in_it();
    nothing_else_walks_a_container();
    the_names_could_not_have_been_used();
    the_verdict_is_recorded_both_ways();
    the_loose_representation_is_the_real_authority();
    return 0;
}
