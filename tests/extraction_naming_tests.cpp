// The name a DMC3 HD extraction gives a payload.
//
// Recovered by reproducing a complete, independently produced `em000` unpack —
// 302 payloads across three nesting levels — rather than by reading any tool's
// source. The renderer reproduces all 302 of that archive's paths exactly; the
// cases below are one of every distinct shape it contains, with the real names.

#include "dmc_rengine/profiles/dmc3/extraction_naming.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;
using dmc3::Dmc3ExtractionNaming;
using dmc3::ExtractionNameRequest;

[[nodiscard]] std::string leaf(
    std::vector<std::uint32_t> parents,
    std::uint32_t index,
    std::string extension,
    std::optional<std::string> record = std::nullopt) {
    ExtractionNameRequest request;
    request.stem = "em000";
    request.parent_slots = std::move(parents);
    request.index = index;
    request.extension = std::move(extension);
    request.record_name = std::move(record);
    return Dmc3ExtractionNaming::leaf(request);
}

void a_direct_slot_is_the_stem_and_its_slot() {
    // Three digits, and the number is the physical slot.
    assert(leaf({}, 0U, "ptx") == "em000_000.ptx");
    assert(leaf({}, 2U, "clt") == "em000_002.clt");
    assert(leaf({}, 24U, "tsc") == "em000_024.tsc");
    assert(leaf({}, 38U, "so-graph") == "em000_038.so-graph");
}

void an_empty_slot_leaves_a_gap_in_the_numbering() {
    // em000 declares 42 slots and fills 40. The archive holds em000_024 and
    // em000_026 with nothing between them, and 031 then 033.
    //
    // This is the fact that is easy to confuse with the recovered .index rule
    // and must not be: a manifest *line* N names the N-th populated payload,
    // while an extracted *filename* carries the slot. One is dense, the other
    // sparse; they agree only when a container has no empty slots.
    assert(leaf({}, 24U, "tsc") == "em000_024.tsc");
    assert(leaf({}, 26U, "mod") == "em000_026.mod");
    // Nothing renders for slot 25, because nothing is there — an extraction
    // that emitted a placeholder would invent a resource the container does
    // not hold, which is the same mistake as writing a zero-byte file for it.
}

void one_level_down_carries_the_parent_slot() {
    // Four digits for the parent, three for the child, and the marker says
    // what the parent was, not what the child is.
    assert(leaf({35U}, 0U, "mot") == "em000_pnst0035_000.mot");
    assert(leaf({35U}, 70U, "mot") == "em000_pnst0035_070.mot");
    assert(leaf({41U}, 0U, "effect-manifest") ==
           "em000_pnst0041_000.effect-manifest");
}

void two_levels_down_carry_both_and_the_records_own_name() {
    // An effect pack's manifest reads `V 108`; the extraction writes `V108`.
    // The manifest stays the authority for what the record is called — this is
    // only how that name is spelled in a filename.
    assert(leaf({41U, 1U}, 12U, "effect-v", "V 108") ==
           "em000_pnst0041_pnst0001_012_V108.effect-v");
    assert(leaf({41U, 1U}, 0U, "effect-g", "G 13") ==
           "em000_pnst0041_pnst0001_000_G13.effect-g");
    assert(leaf({41U, 1U}, 141U, "wrapped-dds", "T 25") ==
           "em000_pnst0041_pnst0001_141_T25.wrapped-dds");
    // A kind-M record's primary member keeps its real format; only its name
    // comes from the pack.
    assert(leaf({41U, 1U}, 160U, "mod", "M 18") ==
           "em000_pnst0041_pnst0001_160_M18.mod");
    assert(leaf({41U, 1U}, 165U, "effect-m-companion", "M 17") ==
           "em000_pnst0041_pnst0001_165_M17.effect-m-companion");
}

void a_container_folder_wears_the_name_its_payload_would() {
    // So a folder and the resource it expands read as the same thing.
    assert(
        Dmc3ExtractionNaming::directory("em000", {35U}, {"pac"}) ==
        "em000/em000_035.pac");
    assert(
        Dmc3ExtractionNaming::directory("em000", {41U, 1U}, {"pnst", "pnst"}) ==
        "em000/em000_041.pnst/em000_pnst0041_001.pnst");
    // With no parents there is nothing to descend into.
    assert(Dmc3ExtractionNaming::directory("em000", {}, {}) == "em000");
}

void a_record_name_never_becomes_a_path() {
    // The name comes from a container's manifest, which is data. A separator
    // in it must not decide where the file lands, and whitespace is stripped
    // because a filename is not the place to preserve a manifest's spacing.
    assert(leaf({41U, 1U}, 3U, "effect-e", "E 42") ==
           "em000_pnst0041_pnst0001_003_E42.effect-e");
    // An empty or whitespace-only record name adds nothing rather than a bare
    // trailing underscore.
    assert(leaf({41U, 1U}, 3U, "effect-e", "   ") ==
           "em000_pnst0041_pnst0001_003.effect-e");
    assert(leaf({41U, 1U}, 3U, "effect-e", "") ==
           "em000_pnst0041_pnst0001_003.effect-e");
}

} // namespace

int main() {
    a_direct_slot_is_the_stem_and_its_slot();
    an_empty_slot_leaves_a_gap_in_the_numbering();
    one_level_down_carries_the_parent_slot();
    two_levels_down_carry_both_and_the_records_own_name();
    a_container_folder_wears_the_name_its_payload_would();
    a_record_name_never_becomes_a_path();
    std::cout << "extraction_naming_tests: all assertions held\n";
    return EXIT_SUCCESS;
}
