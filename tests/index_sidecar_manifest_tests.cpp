#include "dmc_rengine/gdspaces/index_sidecar_manifest.hpp"

#include "dmc_rengine/gdspaces/slot_name_manifest.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

// The external `.index` an unpacked folder carries.
//
// These are the largest source of real slot names available anywhere — one per
// folder — and this project has been showing `slot_0000` beside them. The
// reader is built out of refusals, because the one thing worse than no names
// is our own guesses read back as evidence.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
using Index = gdspaces::IndexSidecarManifest;

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> bytes;
    for (const auto value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

// The shape this repository recorded from files that were actually read: a
// directive line, then one plain name per slot.
void a_real_index_names_every_slot() {
    const auto raw = bytes_of(
        "PNST\r\n"
        "em035_057_000.txt\r\n"
        "em035_057_001.mod\r\n"
        "em035_057_002.dds\r\n");
    const auto parsed = Index::parse(std::span<const std::byte>{raw});
    assert(parsed.has_value());
    assert(parsed->container_directive == "PNST");
    assert(parsed->entries.size() == 3U);

    // Line 0 is the directive, so line 1 names slot 0.
    assert(parsed->entries[0].slot_index == 0U);
    assert(parsed->entries[0].name == "em035_057_000.txt");
    assert(parsed->entries[0].source_line == 1U);
    assert(parsed->entries[2].slot_index == 2U);
    assert(parsed->entries[2].name == "em035_057_002.dds");

    // A PAC directive works the same way.
    const auto pac = bytes_of("PAC\r\nst001_000.txt\r\n");
    const auto pac_parsed = Index::parse(std::span<const std::byte>{pac});
    assert(pac_parsed.has_value());
    assert(pac_parsed->container_directive == "PAC");
    assert(pac_parsed->entries[0].slot_index == 0U);
}

// The rule that matters most: this project writes a file under the same
// extension, and reading it back as authority would be reading back our own
// decision as evidence.
void our_own_sidecar_is_refused() {
    const std::vector<gdspaces::SlotNameAttribution> attributions{
        gdspaces::SlotNameAttribution{
            .slot_index = 0U,
            .name = "st001.ptx",
            .origin = gdspaces::SlotNameOrigin::container_manifest,
            .corroborated_by_payload = true,
        },
    };
    const auto ours = gdspaces::SlotNameManifest::render_sidecar("PAC", attributions);
    const auto raw = bytes_of(ours);

    // It opens with a valid directive line, so only the tab tells them apart.
    assert(ours.rfind("PAC\r\n", 0U) == 0U);
    assert(Index::is_own_rendered_sidecar(std::span<const std::byte>{raw}));
    assert(!Index::parse(std::span<const std::byte>{raw}).has_value());
}

void anything_that_is_not_this_shape_is_refused() {
    // No directive line.
    assert(!Index::parse(std::span<const std::byte>{
        bytes_of("em035_057_000.txt\r\nem035_057_001.mod\r\n")}).has_value());
    // A directive and nothing else.
    assert(!Index::parse(std::span<const std::byte>{bytes_of("PNST\r\n")}).has_value());
    // A directive this project does not know.
    assert(!Index::parse(std::span<const std::byte>{
        bytes_of("ZZZZ\r\nname.txt\r\n")}).has_value());
    // Binary.
    std::vector<std::byte> binary(0x20U, std::byte{0xEE});
    assert(!Index::parse(std::span<const std::byte>{binary}).has_value());
    // Empty.
    assert(!Index::parse({}).has_value());
    // A binary PNST container, which opens with the same four characters and
    // must not be read as its own index.
    std::vector<std::byte> container(0x40U, std::byte{0});
    container[0] = std::byte{'P'};
    container[1] = std::byte{'N'};
    container[2] = std::byte{'S'};
    container[3] = std::byte{'T'};
    container[4] = std::byte{2};
    assert(!Index::parse(std::span<const std::byte>{container}).has_value());
}

// A blank line between names would shift every slot after it, so the read ends
// rather than silently renumbering.
void a_blank_line_ends_the_read() {
    const auto raw = bytes_of("PNST\r\nfirst.txt\r\n\r\nthird.txt\r\n");
    const auto parsed = Index::parse(std::span<const std::byte>{raw});
    assert(parsed.has_value());
    assert(parsed->entries.size() == 1U);
    assert(parsed->entries[0].name == "first.txt");
}

// The mapping differs from the embedded slot-0 manifest's, and conflating them
// would shift every name by one.
void the_mapping_is_not_the_embedded_one() {
    // External: line 0 is a directive, so line 1 names slot 0.
    static_assert(Index::slot_for_line(1U) == 0U);
    static_assert(Index::slot_for_line(3U) == 2U);
    // Embedded: the manifest occupies slot 0, so line 0 names slot 1.
    static_assert(gdspaces::SlotNameManifest::slot_for_line(0U) == 1U);
    static_assert(
        Index::slot_for_line(1U) != gdspaces::SlotNameManifest::slot_for_line(1U));
}

} // namespace

int main() {
    a_real_index_names_every_slot();
    our_own_sidecar_is_refused();
    anything_that_is_not_this_shape_is_refused();
    a_blank_line_ends_the_read();
    the_mapping_is_not_the_embedded_one();
    return 0;
}
