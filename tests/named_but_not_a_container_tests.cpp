#include "dmc_rengine/formats/container_parser_registry.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

// A name is not evidence about bytes.
//
// A real volume contains `at.ptx`, whose bytes no structural parser accepts.
// The product classified it `ptx` from its extension — reasonable — and then
// claimed it was a container on the strength of that, which is not. An
// operator was offered "Open", took the offer, and got
// "No registered container parser recognized the resource": true, useless,
// and indistinguishable from "this file is of no known kind".
//
// These hold the two halves of the fix: the claim is not made, and when the
// name still leads somewhere the refusal says what was actually wrong.

namespace {

namespace formats = dmc::rengine::formats;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> bytes;
    for (const auto value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

// Bytes that are not a texture pack by any reading: no count, no DDS, and far
// too short for the 0x800-byte bundle header.
[[nodiscard]] std::vector<std::byte> not_a_texture_pack() {
    std::vector<std::byte> bytes(0x40U, std::byte{0});
    bytes[0] = std::byte{0x7F};
    bytes[1] = std::byte{0x22};
    return bytes;
}

void a_name_alone_does_not_make_a_container() {
    const auto bytes = not_a_texture_pack();
    const auto classified = gdspaces::ResourceClassifier::classify(
        "GData.afs/at.ptx", std::span<const std::byte>{bytes});

    // The name still decides the format — it says more than "bytes" does.
    assert(classified.format == "ptx");
    assert(!classified.byte_derived);
    // But it does not decide that this is something that can be opened.
    assert(!classified.container);
}

void structure_still_makes_a_container() {
    // The same rule must not cost a real container its expansion. A PAC is
    // recognized by its own three bytes, so the claim is byte-derived.
    auto bytes = bytes_of("PAC");
    bytes.resize(0x40U, std::byte{0});
    const auto classified = gdspaces::ResourceClassifier::classify(
        "GData.afs/st001.pac", std::span<const std::byte>{bytes});
    assert(classified.format == "pac");
    assert(classified.byte_derived);
    assert(classified.container);
}

void a_path_only_index_stays_optimistic() {
    // An index built before materialization has nothing better to go on than
    // the name, and a tree that offered nothing until every member was read
    // would be worse than one that occasionally takes an offer back.
    const auto classified =
        gdspaces::ResourceClassifier::classify("GData.afs/at.ptx");
    assert(classified.format == "ptx");
    assert(classified.container);
}

// A volume is mounted by name through a different path, so the rule must not
// reach it: an `.nbz` that fails to open has to fail as a volume.
void a_volume_is_still_named_by_its_name() {
    const auto bytes = not_a_texture_pack();
    const auto classified = gdspaces::ResourceClassifier::classify(
        "downloads/dmc3-0.nbz", std::span<const std::byte>{bytes});
    assert(classified.format == "nbz");
    assert(!classified.byte_derived);
    assert(classified.container);
}

void a_refusal_says_what_was_wrong() {
    const auto registry = dmc3::make_container_parser_registry();
    const auto bytes = not_a_texture_pack();
    const auto parsed = registry.parse(
        std::span<const std::byte>{bytes}, "GData.afs/at.ptx");

    assert(!parsed.ok());
    assert(!parsed.diagnostics.empty());

    // The parser the name points at answered, so the reason is the texture
    // parser's own and not the registry's shrug.
    bool named_the_format = false;
    bool said_the_name_is_not_evidence = false;
    for (const auto& diagnostic : parsed.diagnostics) {
        if (diagnostic.code.rfind("dmc3.ptx.", 0U) == 0U) {
            named_the_format = true;
        }
        if (diagnostic.code == "container.named_but_not_recognized") {
            said_the_name_is_not_evidence = true;
        }
        assert(diagnostic.code != "container.no_parser");
    }
    assert(named_the_format);
    assert(said_the_name_is_not_evidence);
}

void an_unnamed_resource_still_gets_the_plain_answer() {
    const auto registry = dmc3::make_container_parser_registry();
    const auto bytes = not_a_texture_pack();
    const auto parsed = registry.parse(
        std::span<const std::byte>{bytes}, "GData.afs/slot_0003.bin");

    assert(!parsed.ok());
    assert(parsed.diagnostics.size() == 1U);
    assert(parsed.diagnostics.front().code == "container.no_parser");
}

// A name that points at a parser which *would* accept the bytes must still go
// through selection, not through the explanation path.
void a_real_container_is_not_explained_away() {
    const auto registry = dmc3::make_container_parser_registry();
    auto bytes = bytes_of("PNST");
    bytes.resize(0x40U, std::byte{0});
    bytes[4] = std::byte{1};
    bytes[8] = std::byte{0x20};
    const auto parsed = registry.parse(
        std::span<const std::byte>{bytes}, "GData.afs/st001_effect.pac");
    for (const auto& diagnostic : parsed.diagnostics) {
        assert(diagnostic.code != "container.named_but_not_recognized");
    }
}

} // namespace

int main() {
    a_name_alone_does_not_make_a_container();
    structure_still_makes_a_container();
    a_path_only_index_stays_optimistic();
    a_volume_is_still_named_by_its_name();
    a_refusal_says_what_was_wrong();
    an_unnamed_resource_still_gets_the_plain_answer();
    a_real_container_is_not_explained_away();
    return 0;
}
