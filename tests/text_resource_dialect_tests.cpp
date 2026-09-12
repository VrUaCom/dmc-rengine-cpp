// Text is an encoding, not an identity.
//
// A cloth definition and a scroll table are both readable ASCII, and calling
// either one `txt` throws away what the payload states about itself in its own
// opening line. Recovered 2026-09-08 from a complete em000 extraction, where
// eight CLT payloads and one TSC were all being reported as `txt`.

#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"
#include "dmc_rengine/profiles/dmc3/text_resource_dialects.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;
namespace gdspaces = dmc::rengine::gdspaces;

[[nodiscard]] std::vector<std::byte> text(std::string_view value) {
    std::vector<std::byte> bytes;
    bytes.reserve(value.size());
    for (const auto character : value) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

void a_cloth_definition_names_itself() {
    // Mirrored from em000 slot 9, whose opening line is `;em002_01.clt`.
    const auto bytes = text(
        ";em002_01.clt\r\n\r\nClothNum\t1\r\n\r\nClothNo     0\r\n"
        "Gravity     0.000000  -0.050000  0.000000\r\n");
    const auto identity = dmc3::TextResourceDialects::identify(bytes);
    assert(identity.recognized());
    assert(identity.dialect == dmc3::TextResourceDialect::clt);

    // The name is the payload's own, and it is not the slot's. em000 slot 9
    // holds em002's cloth — seven of the eight in that corpus belong to a
    // different actor than the container they sit in, so a name taken from the
    // enclosing container would have been wrong almost every time.
    assert(identity.embedded_original_name.has_value());
    assert(*identity.embedded_original_name == "em002_01.clt");
}

void a_scroll_table_is_not_a_cloth() {
    const auto bytes = text(
        "\r\n.TSC\t\r\n\r\n\t# RELATIVE\t\t\r\n\r\n\t\t<Start\r\n"
        "\t\t\tScrlNo\t\t0\r\n");
    const auto identity = dmc3::TextResourceDialects::identify(bytes);
    assert(identity.dialect == dmc3::TextResourceDialect::tsc);
    // A scroll table says nothing about its own filename, and inventing one
    // would be this project naming a resource and then reading the name back.
    assert(!identity.embedded_original_name.has_value());
}

void an_arbitrary_comment_is_not_a_cloth() {
    // A `;` line alone is not evidence. What makes the marker a marker is that
    // the commented name is a `.clt`.
    const auto bytes = text(";just a note\r\nsome other authoring text\r\n");
    assert(!dmc3::TextResourceDialects::identify(bytes).recognized());

    // And an empty payload cannot announce anything.
    assert(!dmc3::TextResourceDialects::identify({}).recognized());
}

void the_classifier_reports_the_dialect_rather_than_txt() {
    const auto clt = text(";em000_01.clt\r\n\r\nClothNum\t1\r\n");
    // A container slot carries a synthesized `slot_NNNN.bin` path and no
    // stored name, which is exactly the situation the browser is in. The
    // payload's own opening line is the only identity available.
    const auto seen = gdspaces::ResourceClassifier::classify(
        "slot_0002.bin", std::span<const std::byte>{clt});
    assert(seen.format == "clt");
    assert(seen.structural_confirmed);
    // Read out of the bytes, not matched against a signature table.
    assert(!seen.magic_confirmed);

    const auto tsc = text("\r\n.TSC\t\r\n\t\t<Start\r\n");
    assert(
        gdspaces::ResourceClassifier::classify(
            "slot_0024.bin", std::span<const std::byte>{tsc}).format == "tsc");

    // Text this project cannot place is not a guess at a dialect: it falls
    // back to the path, the same as any payload the classifier cannot read.
    //
    // This case used to use `G 13\r\nG 75\r\nG 152\r\n` as its example of
    // unplaceable text. That is an effect pack's manifest — the reading below
    // — so what the assertion actually pinned was the gap that made the
    // manifest unrecognizable, written down as if it were the correct answer.
    // The example is now text that really is unplaceable: authoring lines with
    // no dialect marker and no grammar this project holds.
    const auto plain = text("ClothNum\t1\r\nGravity 0.000000\r\n");
    const auto unplaced = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{plain});
    assert(unplaced.format == "bin");
    assert(!unplaced.structural_confirmed);

    // A payload does not arrive at its own length. A container slot is padded
    // to the container's alignment, so a short text record reaches the
    // classifier with trailing zeros on it — and refusing those refused every
    // real slot while every fixture sized to its content passed.
    auto padded = text(";em000_01.clt\r\n\r\nClothNum\t1\r\n");
    padded.resize(64U, std::byte{0});
    const auto padded_identity = dmc3::TextResourceDialects::identify(padded);
    assert(padded_identity.dialect == dmc3::TextResourceDialect::clt);
    assert(padded_identity.embedded_original_name.has_value());
    assert(*padded_identity.embedded_original_name == "em000_01.clt");

    auto padded_scroll = text("\r\n.TSC\t\r\n\t\t<Start\r\n");
    padded_scroll.resize(64U, std::byte{0});
    assert(
        dmc3::TextResourceDialects::identify(padded_scroll).dialect ==
        dmc3::TextResourceDialect::tsc);

    // A `.TSC` run inside a binary payload is not a scroll table. The marker
    // is searched rather than anchored, so the encoding is what keeps the
    // search from typing arbitrary bytes.
    std::vector<std::byte> binary(64U, std::byte{0xC3});
    binary[10U] = std::byte{'.'};
    binary[11U] = std::byte{'T'};
    binary[12U] = std::byte{'S'};
    binary[13U] = std::byte{'C'};
    assert(!dmc3::TextResourceDialects::identify(binary).recognized());
    assert(
        gdspaces::ResourceClassifier::classify(
            "slot_0031.bin", std::span<const std::byte>{binary}).format !=
        "tsc");
}

// The dialect that was declared and could never be returned.
//
// `TextResourceDialect::effect_manifest` existed in the enum and in
// `to_string` from the day the dialects were recovered, and `identify()` had
// no branch that produced it. So slot 0 of every effect pack in the corpus
// classified as `txt` — the resource whose whole content is the names of the
// records beside it, typed as untyped text. That is this project's other
// recurring defect: something implemented and unreachable.
void an_effect_manifest_is_not_untyped_text() {
    // The shape em000's pack has: `<kind> <id>` lines and nothing else.
    const auto manifest = text(
        "V 26\r\nE 26\r\nP 26\r\nM 17\r\nG 608\r\n# End\r\n");
    const auto identity = dmc3::TextResourceDialects::identify(manifest);
    assert(identity.dialect == dmc3::TextResourceDialect::effect_manifest);
    // It names the records; it does not name itself. Nothing is synthesized.
    assert(!identity.embedded_original_name.has_value());

    assert(
        gdspaces::ResourceClassifier::classify(
            "slot_0000.bin", std::span<const std::byte>{manifest}).format ==
        "effect-manifest");

    // Padded to a container's alignment, the way it actually arrives.
    auto padded = manifest;
    padded.resize(96U, std::byte{0});
    assert(
        dmc3::TextResourceDialects::identify(padded).dialect ==
        dmc3::TextResourceDialect::effect_manifest);

    // A manifest of one record still reads, because the file closes itself.
    assert(
        dmc3::TextResourceDialects::identify(text("A 3\r\n# End\r\n")).dialect ==
        dmc3::TextResourceDialect::effect_manifest);
}

// What keeps the shape test from typing things that merely resemble it.
void the_manifest_grammar_admits_only_the_manifest() {
    const auto refused = [](std::string_view value) {
        return !dmc3::TextResourceDialects::identify(text(value)).recognized();
    };

    // One line is a coincidence three bytes of anything can produce.
    assert(refused("A 1\r\n"));
    // A kind the corpus does not hold. The pack reader admits any single
    // character once it knows it holds a manifest; deciding that it does is a
    // stricter question, and this is the difference.
    assert(refused("Z 1\r\nZ 2\r\nZ 3\r\n# End\r\n"));
    // One line that is not the grammar refuses the payload, however many are.
    assert(refused("V 26\r\nE 26\r\nClothNum\t1\r\n"));
    // A kind and no identifier, an identifier that is not a decimal, and a
    // separator that is not the one character the format uses.
    assert(refused("V\r\nE\r\nP\r\n"));
    assert(refused("V 26\r\nE 2x\r\n"));
    assert(refused("V\t26\r\nE\t26\r\n"));
    // Comments alone are a file of comments.
    assert(refused("# a note\r\n# another\r\n# End\r\n"));

    // And the two dialects that announce themselves first still do. A cloth
    // definition is read as cloth, not as a manifest with a bad first line.
    assert(
        dmc3::TextResourceDialects::identify(
            text(";em000_01.clt\r\nV 26\r\nE 26\r\n")).dialect ==
        dmc3::TextResourceDialect::clt);
}

// The grammar has one home. Both callers ask it, so a line the pack reader
// walks and a line the classifier judges can never be read two ways.
void the_line_grammar_is_the_contract_s() {
    using Contract = dmc::rengine::profiles::dmc3::EffectPackContract;

    static_assert(Contract::read_manifest_line("V 26").is_record());
    static_assert(Contract::read_manifest_line("V 26").kind == 'V');
    static_assert(Contract::read_manifest_line("V 26").identifier == 26U);
    // Trailing CR and surrounding whitespace are the file's, not the line's.
    static_assert(Contract::read_manifest_line("  G 608 \r").identifier == 608U);
    static_assert(
        Contract::read_manifest_line("# End").line_kind ==
        Contract::ManifestLineKind::terminator);
    static_assert(
        Contract::read_manifest_line("# something").line_kind ==
        Contract::ManifestLineKind::comment);
    static_assert(
        Contract::read_manifest_line("\t ").line_kind ==
        Contract::ManifestLineKind::blank);
    static_assert(
        Contract::read_manifest_line("V 26x").line_kind ==
        Contract::ManifestLineKind::invalid);
    // An identifier wider than the field is not a line. Wrapping it would
    // invent a record number the file does not contain.
    static_assert(
        Contract::read_manifest_line("V 4294967295").identifier == 4294967295U);
    static_assert(
        Contract::read_manifest_line("V 4294967296").line_kind ==
        Contract::ManifestLineKind::invalid);
}

} // namespace

int main() {
    a_cloth_definition_names_itself();
    a_scroll_table_is_not_a_cloth();
    an_arbitrary_comment_is_not_a_cloth();
    the_classifier_reports_the_dialect_rather_than_txt();
    an_effect_manifest_is_not_untyped_text();
    the_manifest_grammar_admits_only_the_manifest();
    the_line_grammar_is_the_contract_s();
    std::cout << "text_resource_dialect_tests: all assertions held\n";
    return EXIT_SUCCESS;
}
