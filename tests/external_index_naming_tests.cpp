#include "dmc_rengine/gdspaces/external_index_naming.hpp"

#include "dmc_rengine/formats/container_parser.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Applying an extraction tool's external `.index` to a container's slots.
//
// Every unpacked folder carries one, which makes them the largest supply of
// real slot names anywhere, and this project printed `slot_0000` beside them.
// The rule lives in the library rather than in one application because a
// naming rule that lives in a session layer produces a phone showing one name
// and every other consumer showing another for the same slot.
//
// What is asserted here is the ranking. A name a tool wrote is weaker evidence
// than a name the container stores, and the whole value of the feature
// evaporates if applying it can quietly overwrite the stronger claim.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace formats = dmc::rengine::formats;

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> bytes;
    for (const auto value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] gdspaces::ContainerChild child(
    std::uint32_t slot,
    bool populated,
    std::string display_name,
    gdspaces::SlotNameOrigin origin,
    std::vector<std::byte> payload) {
    gdspaces::ContainerChild value;
    value.entry.slot_index = slot;
    value.entry.populated = populated;
    value.entry.logical_name = display_name;
    value.payload.resource.id = gdspaces::ResourceId{
        "index-test", "root.pnst/" + display_name, "PNST[" + std::to_string(slot) + "]",
        0U, static_cast<std::uint64_t>(payload.size())};
    value.payload.resource.display_name = display_name;
    value.payload.resource.synthetic_name = true;
    value.payload.bytes = std::move(payload);
    value.name_attribution = gdspaces::SlotNameAttribution{
        .slot_index = slot,
        .name = display_name,
        .origin = origin,
        .corroborated_by_payload = false,
    };
    return value;
}

[[nodiscard]] gdspaces::ContainerExpansion expansion() {
    gdspaces::ContainerExpansion value;
    value.parser_format = "pnst";
    value.parent.id = gdspaces::ResourceId{"index-test", "root.pnst", {}, 0U, 64U};
    value.parent.display_name = "root.pnst";
    value.parent.format = "pnst";
    value.parent.container = true;

    value.children.push_back(child(
        0U, true, "slot_0000.bin", gdspaces::SlotNameOrigin::parser_placeholder,
        bytes_of("DDS this is not really a dds")));
    value.children.push_back(child(
        1U, true, "slot_0001.bin", gdspaces::SlotNameOrigin::parser_placeholder,
        bytes_of("arbitrary payload bytes")));
    // A slot the container itself already named: the stronger claim.
    value.children.push_back(child(
        2U, true, "stored.scm", gdspaces::SlotNameOrigin::container_manifest,
        bytes_of("stored payload")));
    // An empty slot: reserved numbering, carrying nothing to name.
    value.children.push_back(child(
        3U, false, "slot_0003.empty", gdspaces::SlotNameOrigin::absent_slot, {}));
    return value;
}

void names_are_applied_and_attributed_as_external() {
    auto container = expansion();
    const auto sidecar = bytes_of(
        "PNST\r\n"
        "em035_057_000.dds\r\n"
        "em035_057_001.mod\r\n");

    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{sidecar});
    assert(result.applied());
    assert(result.named_slots == 2U);
    assert(result.directive_matches_parser);

    assert(container.children[0].payload.resource.display_name == "em035_057_000.dds");
    assert(!container.children[0].payload.resource.synthetic_name);
    // Never `container_manifest`: the container stored nothing here, a tool
    // decided this, and a reader must be able to tell the two apart.
    assert(
        container.children[0].name_attribution.origin ==
        gdspaces::SlotNameOrigin::external_index);
    assert(gdspaces::to_string(gdspaces::SlotNameOrigin::external_index) ==
           "external-index");
}

void a_name_the_container_stores_outranks_a_name_a_tool_chose() {
    auto container = expansion();
    const auto sidecar = bytes_of(
        "PNST\r\n"
        "a.dds\r\n"
        "b.mod\r\n"
        "hijacked.txt\r\n");

    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{sidecar});
    assert(result.applied());

    // Slot 2 was named by the container itself. The sidecar line for it is
    // ignored rather than allowed to replace stronger evidence with weaker.
    assert(container.children[2].payload.resource.display_name == "stored.scm");
    assert(
        container.children[2].name_attribution.origin ==
        gdspaces::SlotNameOrigin::container_manifest);
    assert(result.named_slots == 2U);
    assert(result.slots_without_a_line == 1U);
}

void an_empty_slot_is_never_given_a_name() {
    auto container = expansion();
    const auto sidecar = bytes_of(
        "PNST\r\n"
        "a.dds\r\n"
        "b.mod\r\n"
        "c.scm\r\n"
        "d.txt\r\n");

    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{sidecar});

    // Slot 3 carries nothing. Naming it would make an intact sparse container
    // look fully populated, which is how reserved numbering came to read as
    // damage in the first place.
    assert(
        container.children[3].name_attribution.origin ==
        gdspaces::SlotNameOrigin::absent_slot);
    assert(container.children[3].payload.resource.display_name == "slot_0003.empty");
    assert(result.lines_without_a_slot == 1U);
}

void corroboration_comes_from_the_payload_not_from_the_name() {
    auto container = expansion();
    // Slot 0's bytes open with "DDS "; slot 1's are arbitrary. The same
    // sidecar names both with a suffix, so if the name were confirming itself
    // both would count as corroborated.
    const auto sidecar = bytes_of(
        "PNST\r\n"
        "real.dds\r\n"
        "claimed.dds\r\n");

    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{sidecar});
    assert(result.named_slots == 2U);
    assert(result.corroborated_slots == 1U);
    assert(container.children[0].name_attribution.corroborated_by_payload);
    assert(!container.children[1].name_attribution.corroborated_by_payload);
}

void our_own_rendered_sidecar_is_refused() {
    auto container = expansion();
    // This project renders tab-separated columns under the same extension.
    // Reading that back would make our own naming decision look like evidence.
    const auto own = bytes_of(
        "PNST\r\n"
        "0\tslot_0000.bin\tparser-placeholder\r\n"
        "1\tslot_0001.bin\tparser-placeholder\r\n");

    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{own});
    assert(!result.applied());
    assert(result.named_slots == 0U);
    assert(!result.diagnostics.empty());
    assert(result.diagnostics.front().code == "gdspaces.external-index.own-sidecar");
    assert(container.children[0].payload.resource.display_name == "slot_0000.bin");
}

void a_directive_for_another_container_still_reports_the_mismatch() {
    auto container = expansion();
    const auto sidecar = bytes_of(
        "PAC\r\n"
        "a.dds\r\n");

    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{sidecar});
    // Applied, because a sidecar can be right about names while this build
    // parses the container under another format name — but the mismatch is
    // exactly the signal that the wrong folder's index was picked, so it is
    // never silently dropped.
    assert(result.applied());
    assert(!result.directive_matches_parser);
    assert(!result.diagnostics.empty());
    assert(
        result.diagnostics.front().code ==
        "gdspaces.external-index.directive-mismatch");
}

void bytes_that_are_not_an_index_are_refused_with_a_reason() {
    auto container = expansion();
    const auto noise = bytes_of("\x7f\x45\x4c\x46 not text at all");
    const auto result = gdspaces::ExternalIndexNaming::apply_bytes(
        container, std::span<const std::byte>{noise});
    assert(!result.applied());
    assert(!result.diagnostics.empty());
    assert(result.diagnostics.front().code == "gdspaces.external-index.unreadable");
}

} // namespace

int main() {
    names_are_applied_and_attributed_as_external();
    a_name_the_container_stores_outranks_a_name_a_tool_chose();
    an_empty_slot_is_never_given_a_name();
    corroboration_comes_from_the_payload_not_from_the_name();
    our_own_rendered_sidecar_is_refused();
    a_directive_for_another_container_still_reports_the_mismatch();
    bytes_that_are_not_an_index_are_refused_with_a_reason();
    return 0;
}
