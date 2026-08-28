#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

// A relative-slot container stores no names, so every name shown for a slot is
// something this tool decided. These check that the decision is always
// attributed — and that a name the container itself carries never gets printed
// as though it were recovered truth.

namespace {

namespace formats = dmc::rengine::formats;
namespace gdspaces = dmc::rengine::gdspaces;
using gdspaces::SlotNameManifest;
using gdspaces::SlotNameOrigin;

[[nodiscard]] std::vector<std::byte> literal(std::string_view text, std::size_t size) {
    std::vector<std::byte> bytes(std::max(size, text.size()), std::byte{0});
    for (std::size_t index = 0U; index < text.size(); ++index) {
        bytes[index] = static_cast<std::byte>(text[index]);
    }
    return bytes;
}

void the_real_stage_manifest_reads_as_a_name_list() {
    // `st001.pac` slot 0, byte for byte, including its NUL padding.
    const auto bytes = literal(
        "st001.ptx\r\nst001.scm\r\nst001.sch\r\n", 48U);
    const auto names = SlotNameManifest::parse(bytes);
    assert(names.size() == 3U);
    assert(names[0] == "st001.ptx");
    assert(names[1] == "st001.scm");
    assert(names[2] == "st001.sch");

    // The manifest sits in slot 0, so its first line names slot 1.
    static_assert(SlotNameManifest::slot_for_line(0U) == 1U);
    static_assert(SlotNameManifest::slot_for_line(2U) == 3U);
    assert(SlotNameManifest::extension_of(names[0]) == "ptx");
    assert(SlotNameManifest::extension_of("NoExtension").empty());
}

void text_that_names_nothing_is_not_a_manifest() {
    // The `# GAME` scene block is text, and a rule that took any text record
    // for a name list would attribute stage geometry to a directive line.
    const auto game = literal(
        "# GAME\r\n\r\n# SET 0 CONFIG\r\n\tcam_init\t2500\r\n", 256U);
    assert(SlotNameManifest::parse(game).empty());

    // One unusable line disqualifies the whole record: attributing some slots
    // and silently skipping others is worse than attributing none.
    const auto partial = literal("good.scm\r\nnot a name\r\n", 48U);
    assert(SlotNameManifest::parse(partial).empty());

    // Binary is not a manifest, whatever it happens to contain.
    std::vector<std::byte> binary(64U, std::byte{0x11});
    assert(SlotNameManifest::parse(binary).empty());
}

[[nodiscard]] std::vector<std::byte> stage_with_manifest() {
    // Slot 0 is the name list; slots 1 and 2 carry payloads whose own tags
    // decide their type. Slot 1's tag agrees with its manifest line and slot
    // 2's does not, which is the difference the attribution has to show.
    const auto manifest = literal("thing.scm\r\nthing.hits\r\n", 48U);
    std::vector<std::byte> scene(64U, std::byte{0});
    scene[0] = static_cast<std::byte>('S');
    scene[1] = static_cast<std::byte>('C');
    scene[2] = static_cast<std::byte>('M');
    std::vector<std::byte> other(64U, std::byte{0});
    other[0] = static_cast<std::byte>('L');
    other[1] = static_cast<std::byte>('I');
    other[2] = static_cast<std::byte>('G');
    other[3] = static_cast<std::byte>('2');

    constexpr std::size_t align = 0x40U;
    const auto slot0 = align;
    const auto slot1 = slot0 + align;
    const auto slot2 = slot1 + align;
    std::vector<std::byte> bytes(slot2 + align, std::byte{0});
    bytes[0] = static_cast<std::byte>('P');
    bytes[1] = static_cast<std::byte>('A');
    bytes[2] = static_cast<std::byte>('C');
    bytes[4] = std::byte{3};
    const auto put = [&bytes](std::size_t at, std::uint32_t value) {
        for (std::size_t index = 0U; index < 4U; ++index) {
            bytes[at + index] = static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
        }
    };
    put(8U, static_cast<std::uint32_t>(slot0));
    put(12U, static_cast<std::uint32_t>(slot1));
    put(16U, static_cast<std::uint32_t>(slot2));
    std::copy(manifest.begin(), manifest.begin() + 48, bytes.begin() + slot0);
    std::copy(scene.begin(), scene.end(), bytes.begin() + slot1);
    std::copy(other.begin(), other.end(), bytes.begin() + slot2);
    return bytes;
}

void a_manifest_name_is_attributed_never_asserted() {
    const auto bytes = stage_with_manifest();
    gdspaces::ResourcePayload parent{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{"test", "thing.pac", {}, 0U, bytes.size()},
            .display_name = "thing.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = bytes,
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
    formats::ContainerParseResult parsed;
    const auto document = formats::PacParser::parse(bytes);
    assert(document.ok());
    parsed.document = *document.document;
    parsed.recognized = true;

    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    assert(expansion.children.size() == 3U);

    // The manifest itself: text, named by its own bytes, attributed to nothing
    // else. A manifest never names the slot it occupies — but it is the slot
    // that explains all the others, so it is named for what it is rather than
    // hidden behind a generic text suffix. This is the file an unpacked folder
    // carries as `.index`.
    const auto& zero = expansion.children[0];
    assert(zero.name_attribution.origin == SlotNameOrigin::byte_derived_suffix);
    assert(zero.payload.resource.display_name == "slot_0000.index");
    // The format is untouched: it is text, and recognizing what it holds does
    // not change what it is.
    assert(zero.payload.resource.format == "txt");

    // Line 0 names slot 1, and the payload's own tag agrees with the
    // extension. That agreement is recorded; it still is not proof.
    const auto& one = expansion.children[1];
    assert(one.name_attribution.origin == SlotNameOrigin::container_manifest);
    assert(one.name_attribution.name == "thing.scm");
    assert(one.name_attribution.corroborated_by_payload);
    // A corroborated line becomes the display name.
    //
    // This assertion used to require the opposite — that the display name stay
    // `slot_0001.scm` and the manifest line live only in the attribution. That
    // was defensible in the abstract and wrong on screen: an operator saw a
    // name this tool invented sitting beside a container that says
    // `thing.scm` and a payload that agrees, and asked why the names do not
    // match. They were right. A name someone wrote and the payload confirms
    // beats a name we made up.
    assert(one.payload.resource.display_name == "thing.scm");
    // It is no longer a name this parser synthesized, and says so.
    assert(!one.payload.resource.synthetic_name);
    // The identity is still untouched — only the display name moved.
    assert(one.payload.resource.id.logical_path.find("slot-0001") !=
        std::string::npos);

    // Line 1 names slot 2, and the payload says it is something else. The name
    // is still shown — it is what the container claims — and the disagreement
    // is visible rather than hidden.
    const auto& two = expansion.children[2];
    assert(two.name_attribution.origin == SlotNameOrigin::container_manifest);
    assert(two.name_attribution.name == "thing.hits");
    assert(!two.name_attribution.corroborated_by_payload);
    assert(two.payload.resource.format == "lig2");
    // And an uncorroborated line does *not* become the display name. An
    // unconfirmed name presented as the name is the failure this whole
    // attribution channel exists to prevent, so the placeholder stands and the
    // claim stays visible beside it.
    assert(two.payload.resource.display_name == "slot_0002.lig2");
    assert(two.payload.resource.synthetic_name);
}

void a_container_without_a_manifest_attributes_nothing() {
    std::vector<std::byte> bytes(0x100U, std::byte{0});
    bytes[0] = static_cast<std::byte>('P');
    bytes[1] = static_cast<std::byte>('A');
    bytes[2] = static_cast<std::byte>('C');
    bytes[4] = std::byte{2};
    bytes[8] = std::byte{0x40};
    bytes[12] = std::byte{0x80};
    bytes[0x40] = static_cast<std::byte>('H');
    bytes[0x41] = static_cast<std::byte>('I');
    bytes[0x42] = static_cast<std::byte>('T');
    bytes[0x43] = static_cast<std::byte>('S');

    gdspaces::ResourcePayload parent{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{"test", "plain.pac", {}, 0U, bytes.size()},
            .display_name = "plain.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = bytes,
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
    formats::ContainerParseResult parsed;
    const auto document = formats::PacParser::parse(bytes);
    assert(document.ok());
    parsed.document = *document.document;
    parsed.recognized = true;

    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    for (const auto& child : expansion.children) {
        assert(child.name_attribution.origin != SlotNameOrigin::container_manifest);
        assert(!child.name_attribution.corroborated_by_payload);
    }
}

void the_sidecar_records_where_every_name_came_from() {
    const auto bytes = stage_with_manifest();
    gdspaces::ResourcePayload parent{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{"test", "thing.pac", {}, 0U, bytes.size()},
            .display_name = "thing.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = bytes,
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
    formats::ContainerParseResult parsed;
    const auto document = formats::PacParser::parse(bytes);
    assert(document.ok());
    parsed.document = *document.document;
    parsed.recognized = true;
    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);

    std::vector<gdspaces::SlotNameAttribution> attributions;
    for (const auto& child : expansion.children) {
        attributions.push_back(child.name_attribution);
    }
    const auto sidecar = SlotNameManifest::render_sidecar(
        parsed.document.format, attributions);

    // The corpus shape: a directive line naming the container, then one line
    // per slot. CRLF, as those files use.
    assert(sidecar.rfind("PAC\r\n", 0U) == 0U);
    assert(sidecar.find("\n0\t") != std::string::npos);
    assert(sidecar.find("1\tthing.scm\tcontainer-manifest\tpayload-agrees\r\n") !=
        std::string::npos);
    // The disagreeing line is written without the corroboration column rather
    // than omitted: a name the container claims is still what it claims.
    assert(sidecar.find("2\tthing.hits\tcontainer-manifest\r\n") !=
        std::string::npos);
    // Every slot gets a line, so the line count is the slot count.
    assert(std::count(sidecar.begin(), sidecar.end(), '\n') ==
        static_cast<std::ptrdiff_t>(expansion.children.size() + 1U));
}

void an_absent_slot_gets_a_line_that_says_so() {
    // A sparse container's absent slot must appear in the sidecar as an absent
    // slot, not go missing. A reader that saw a gap in the indices would read
    // the sparseness as damage, which is the thing this project has spent
    // three commits refusing to do.
    std::vector<std::byte> bytes(0x100U, std::byte{0});
    bytes[0] = static_cast<std::byte>('P');
    bytes[1] = static_cast<std::byte>('A');
    bytes[2] = static_cast<std::byte>('C');
    bytes[4] = std::byte{3};
    bytes[8] = std::byte{0x40};
    // slot 1 absent
    bytes[16] = std::byte{0x80};
    bytes[0x40] = static_cast<std::byte>('H');
    bytes[0x41] = static_cast<std::byte>('I');
    bytes[0x42] = static_cast<std::byte>('T');
    bytes[0x43] = static_cast<std::byte>('S');

    gdspaces::ResourcePayload parent{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{"test", "sparse.pac", {}, 0U, bytes.size()},
            .display_name = "sparse.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = bytes,
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
    formats::ContainerParseResult parsed;
    const auto document = formats::PacParser::parse(bytes);
    assert(document.ok());
    parsed.document = *document.document;
    parsed.recognized = true;
    const auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    assert(expansion.children.size() == 3U);
    assert(expansion.children[1].name_attribution.origin ==
        SlotNameOrigin::absent_slot);

    std::vector<gdspaces::SlotNameAttribution> attributions;
    for (const auto& child : expansion.children) {
        attributions.push_back(child.name_attribution);
    }
    const auto sidecar = SlotNameManifest::render_sidecar("PAC", attributions);
    assert(sidecar.find("1\tslot_0001.empty\tabsent-slot\r\n") !=
        std::string::npos);
    assert(std::count(sidecar.begin(), sidecar.end(), '\n') == 4);
}

} // namespace

int main() {
    the_real_stage_manifest_reads_as_a_name_list();
    text_that_names_nothing_is_not_a_manifest();
    a_manifest_name_is_attributed_never_asserted();
    a_container_without_a_manifest_attributes_nothing();
    the_sidecar_records_where_every_name_came_from();
    an_absent_slot_gets_a_line_that_says_so();
    return 0;
}
