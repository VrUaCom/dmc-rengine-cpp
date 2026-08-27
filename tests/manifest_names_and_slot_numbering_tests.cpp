#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"
#include "dmc_rengine/profiles/dmc3/authoring_extension_contract.hpp"

#include <cassert>
#include <string>

// Two facts read off the two stage containers in the corpus, and one
// correction they force.
//
// A stage manifest names slots 1, 2 and 3 as `.ptx`, `.scm` and `.sch`. The
// product checked those names against the payload by comparing the extension
// to the classified format as plain text, so two of the three agreed and the
// third showed on screen in red as unconfirmed. It was never wrong: `sch` and
// `hits` are one record under two names, in both files.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Extensions = dmc3::AuthoringExtensionContract;
using Numbering = dmc3::ModelGroupNumberingContract;

void the_manifest_lines_of_a_stage_all_corroborate() {
    // The three lines both stage containers carry, against the format each
    // slot's payload independently declares for itself.
    assert(Extensions::names_the_same_resource(
        gdspaces::SlotNameManifest::extension_of("st001.ptx"), "ptx"));
    assert(Extensions::names_the_same_resource(
        gdspaces::SlotNameManifest::extension_of("st001.scm"), "scm"));
    // The one that used to read as a disagreement.
    assert(Extensions::names_the_same_resource(
        gdspaces::SlotNameManifest::extension_of("st001.sch"), "hits"));
    assert(Extensions::names_the_same_resource(
        gdspaces::SlotNameManifest::extension_of("st114.sch"), "hits"));
}

// The equivalence must stay narrow. Listing a pairing means these two names
// were seen naming the same payload — nothing in the image maps an extension
// to a type at all, so an unlisted pairing is not corroborated and has to say
// so rather than being waved through.
void an_unevidenced_pairing_is_not_corroborated() {
    assert(!Extensions::names_the_same_resource("sch", "scm"));
    assert(!Extensions::names_the_same_resource("ptx", "dds"));
    assert(!Extensions::names_the_same_resource("scm", "mod"));
    assert(!Extensions::names_the_same_resource("", "hits"));
    assert(!Extensions::names_the_same_resource("sch", ""));
    // And it must not run backwards: a payload classified `sch` is not a
    // thing, so the pairing is directional.
    assert(!Extensions::names_the_same_resource("hits", "sch"));
}

void a_plain_match_still_corroborates() {
    assert(Extensions::names_the_same_resource("mot", "mot"));
    assert(Extensions::names_the_same_resource("dds", "dds"));
}

// Nine consecutive empty rows look like damage. They are not: the populated
// indices are multiples of ten and the gaps are reserved identity space.
void a_model_group_numbers_its_slots_by_tens() {
    // st001.pac slot 5: 11 declared, populated 0 and 10.
    assert(Numbering::declared_slots_for(Numbering::st001_populated) ==
           Numbering::st001_declared_slots);
    // st114.pac slot 5: 21 declared, populated 0, 10 and 20.
    assert(Numbering::declared_slots_for(Numbering::st114_populated) ==
           Numbering::st114_declared_slots);

    assert(Numbering::index_is_on_stride(0U));
    assert(Numbering::index_is_on_stride(10U));
    assert(Numbering::index_is_on_stride(20U));
    assert(!Numbering::index_is_on_stride(1U));
    assert(!Numbering::index_is_on_stride(9U));
}

// The contract's own numbers, held where an edit would have to notice.
static_assert(Numbering::observed_stride == 10U);
static_assert(Numbering::observed_containers == 2U);
static_assert(Numbering::declared_slots_for(0U) == 0U);
// One group needs one slot, not eleven.
static_assert(Numbering::declared_slots_for(1U) == 1U);
static_assert(Extensions::pairings.size() == 3U);
// Every pairing rests on both stage containers, not on one.
static_assert(Extensions::pairings[0].observed_files == 2U);
static_assert(Extensions::pairings[1].observed_files == 2U);
static_assert(Extensions::pairings[2].observed_files == 2U);
// The manifest's line order is the slot order, so the recorded slots must be
// consecutive from the first slot after the manifest itself.
static_assert(Extensions::pairings[0].manifest_slot == 1U);
static_assert(Extensions::pairings[1].manifest_slot == 2U);
static_assert(Extensions::pairings[2].manifest_slot == 3U);
static_assert(
    Extensions::pairings[0].manifest_slot ==
    gdspaces::SlotNameManifest::slot_for_line(0U));
static_assert(
    Extensions::pairings[2].manifest_slot ==
    gdspaces::SlotNameManifest::slot_for_line(2U));
// Exactly one pairing is not a plain string match; that is the whole reason
// this table exists, and a table where all three matched would be dead code.
static_assert(
    Extensions::pairings[2].extension != Extensions::pairings[2].format);
static_assert(
    Extensions::pairings[0].extension == Extensions::pairings[0].format);

} // namespace

int main() {
    the_manifest_lines_of_a_stage_all_corroborate();
    an_unevidenced_pairing_is_not_corroborated();
    a_plain_match_still_corroborates();
    a_model_group_numbers_its_slots_by_tens();
    return 0;
}
