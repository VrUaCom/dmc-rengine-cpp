// Text is an encoding, not an identity.
//
// A cloth definition and a scroll table are both readable ASCII, and calling
// either one `txt` throws away what the payload states about itself in its own
// opening line. Recovered 2026-09-08 from a complete em000 extraction, where
// eight CLT payloads and one TSC were all being reported as `txt`.

#include "dmc_rengine/gdspaces/classifier.hpp"
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
    const auto plain = text("G 13\r\nG 75\r\nG 152\r\n");
    const auto unplaced = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{plain});
    assert(unplaced.format == "bin");
    assert(!unplaced.structural_confirmed);

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

} // namespace

int main() {
    a_cloth_definition_names_itself();
    a_scroll_table_is_not_a_cloth();
    an_arbitrary_comment_is_not_a_cloth();
    the_classifier_reports_the_dialect_rather_than_txt();
    std::cout << "text_resource_dialect_tests: all assertions held\n";
    return EXIT_SUCCESS;
}
