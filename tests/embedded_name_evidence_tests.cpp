#include "dmc_rengine/gdspaces/embedded_name_evidence.hpp"
#include "dmc_rengine/gdspaces/index_manifest.hpp"
#include "dmc_rengine/gdspaces/index_name_overlay.hpp"
#include "dmc_rengine/gdspaces/index_slot_name_authority.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> real_st001_name_list() {
    auto bytes = bytes_from_text(
        "st001.ptx\r\n"
        "st001.scm\r\n"
        "st001.sch\r\n");
    bytes.insert(bytes.end(), 15U, std::byte{0});
    assert(bytes.size() == 48U);
    return bytes;
}

[[nodiscard]] std::string real_st001_index_text() {
    return
        "st001_000.ukn\r\n"
        "st001_001 folder\r\n"
        "st001_002.scm\r\n"
        "st001_003.ukn\r\n"
        "st001_004.txt\r\n"
        "st001_005.pac\r\n"
        "st001_006.ukn\r\n"
        "st001_007.pac\r\n";
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload index_payload(
    std::string text) {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = bytes_from_text(text);
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-index-source",
                .logical_path = "GData.afs/st001.index",
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = "st001.index",
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerChild child(
    std::uint32_t slot,
    std::uint64_t offset,
    std::vector<std::byte> bytes) {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto size = static_cast<std::uint64_t>(bytes.size());
    const auto slot_text = std::to_string(slot);
    return gdspaces::ContainerChild{
        .entry = formats::ContainerEntry{
            .slot_index = slot,
            .offset = offset,
            .size = size,
            .logical_name = "slot_" + slot_text + ".bin",
            .populated = true,
            .synthetic_name = true,
        },
        .payload = gdspaces::ResourcePayload{
            .resource = gdspaces::ResourceRef{
                .id = gdspaces::ResourceId{
                    .source_id = "retail-pac-source",
                    .logical_path = "GData.afs/st001.pac::PAC/slot-" + slot_text,
                    .container_chain = "PAC[" + slot_text + "]",
                    .offset = offset,
                    .size = size,
                },
                .display_name = "slot_" + slot_text + ".bin",
                .format = "unknown",
                .profile = "dmc3-hd",
                .synthetic_name = true,
                .container = false,
            },
            .bytes = std::move(bytes),
            .diagnostics = {},
            .byte_provenance = std::nullopt,
            .name_evidence = {},
        },
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion expansion_fixture() {
    namespace gdspaces = dmc::rengine::gdspaces;

    gdspaces::ContainerExpansion expansion{
        .parent = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-pac-source",
                .logical_path = "GData.afs/st001.pac",
                .container_chain = {},
                .offset = 0U,
                .size = 0x1000U,
            },
            .display_name = "st001.pac",
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
    };

    expansion.children.push_back(child(0U, 0x20U, real_st001_name_list()));
    expansion.children.push_back(child(1U, 0x50U, bytes_from_text("PTX?")));
    expansion.children.push_back(child(2U, 0x60U, bytes_from_text("SCM?")));
    expansion.children.push_back(child(3U, 0x70U, bytes_from_text("HITS")));
    expansion.children.push_back(child(4U, 0x80U, bytes_from_text("TEXT")));
    expansion.children.push_back(child(5U, 0x90U, bytes_from_text("PN?")));
    expansion.children.push_back(child(6U, 0xA0U, bytes_from_text("UKN?")));
    expansion.children.push_back(child(7U, 0xB0U, bytes_from_text("PAC?")));
    return expansion;
}

void apply_index(
    dmc::rengine::gdspaces::ContainerExpansion& expansion,
    const dmc::rengine::gdspaces::ResourcePayload& index) {
    namespace gdspaces = dmc::rengine::gdspaces;
    const auto manifest = gdspaces::IndexManifestParser::parse(index);
    assert(manifest.ok());
    const auto binding = gdspaces::IndexSlotNameBinder::bind(
        expansion, *manifest.manifest);
    assert(binding.ok());
    const auto overlay = gdspaces::IndexNameOverlayBuilder::build(
        expansion, *binding.binding);
    assert(overlay.ok());
    const auto applied = gdspaces::IndexNameOverlayBuilder::apply(
        expansion, *overlay.overlay);
    assert(applied.ok());
}

[[nodiscard]] std::size_t evidence_count(
    const dmc::rengine::gdspaces::ResourcePayload& payload,
    dmc::rengine::gdspaces::ResourceNameEvidenceKind kind) {
    return static_cast<std::size_t>(std::count_if(
        payload.name_evidence.begin(), payload.name_evidence.end(),
        [&](const dmc::rengine::gdspaces::ResourceNameEvidence& evidence) {
            return evidence.kind() == kind && evidence.valid();
        }));
}

void assert_real_observation(
    const dmc::rengine::gdspaces::EmbeddedNameListObservation& observation) {
    assert(observation.authority_sha256() ==
           "7efcf182f28135a3d694324ba06715ca6f6b075a028d035c89c69809df23faa5");
    assert(observation.aliases().size() == 3U);
    assert(observation.aliases()[0].name() == "st001.ptx");
    assert(observation.aliases()[0].source_offset() == 0U);
    assert(observation.aliases()[1].name() == "st001.scm");
    assert(observation.aliases()[1].source_offset() == 11U);
    assert(observation.aliases()[2].name() == "st001.sch");
    assert(observation.aliases()[2].source_offset() == 22U);
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto real_index = index_payload(real_st001_index_text());

    // Path A: recover the retained embedded aliases first, then apply the
    // stronger external .index. This uses the exact 48-byte st001_000.ukn
    // corpus payload recovered from the canonical v6 archive.
    auto embedded_then_index = expansion_fixture();
    assert(embedded_then_index.usable());
    std::vector<gdspaces::ResourceId> ids;
    std::vector<std::vector<std::byte>> bytes;
    for (const auto& entry : embedded_then_index.children) {
        ids.push_back(entry.payload.resource.id);
        bytes.push_back(entry.payload.bytes);
    }

    const auto observed = gdspaces::EmbeddedNameEvidenceBuilder::observe(
        embedded_then_index);
    assert(observed.ok());
    assert_real_observation(*observed.observation);
    assert(observed.observation->authority_resource() ==
           embedded_then_index.children[0].payload.resource.id);

    const auto embedded_applied = gdspaces::EmbeddedNameEvidenceBuilder::apply(
        embedded_then_index, *observed.observation);
    assert(embedded_applied.ok());
    assert(embedded_then_index.children[0].payload.resource.format == "name-list");
    assert(embedded_then_index.children[0].payload.resource.display_name ==
           "st001_000.name-list.txt");
    assert(embedded_then_index.children[1].payload.resource.display_name ==
           "st001.ptx");
    assert(embedded_then_index.children[2].payload.resource.display_name ==
           "st001.scm");
    // The historical embedded alias says .sch, but HITS bytes are stronger
    // semantic evidence and therefore canonicalize the display suffix.
    assert(embedded_then_index.children[3].payload.resource.display_name ==
           "st001.hits");
    for (std::size_t slot = 1U; slot <= 3U; ++slot) {
        assert(evidence_count(
                   embedded_then_index.children[slot].payload,
                   gdspaces::ResourceNameEvidenceKind::embedded_alias) == 1U);
    }

    apply_index(embedded_then_index, real_index);
    assert(embedded_then_index.children[0].payload.resource.display_name ==
           "st001_000.txt");
    assert(embedded_then_index.children[3].payload.resource.display_name ==
           "st001_003.hits");
    for (std::size_t slot = 0U; slot < embedded_then_index.children.size(); ++slot) {
        assert(embedded_then_index.children[slot].payload.resource.id == ids[slot]);
        assert(embedded_then_index.children[slot].payload.bytes == bytes[slot]);
        assert(evidence_count(
                   embedded_then_index.children[slot].payload,
                   gdspaces::ResourceNameEvidenceKind::external_index) == 1U);
    }
    for (std::size_t slot = 1U; slot <= 3U; ++slot) {
        assert(evidence_count(
                   embedded_then_index.children[slot].payload,
                   gdspaces::ResourceNameEvidenceKind::embedded_alias) == 1U);
    }

    // Path B: external .index first, embedded aliases second. The result must
    // be identical: evidence order cannot change physical identity or display
    // priority.
    auto index_then_embedded = expansion_fixture();
    apply_index(index_then_embedded, real_index);
    const auto observed_second = gdspaces::EmbeddedNameEvidenceBuilder::observe(
        index_then_embedded);
    assert(observed_second.ok());
    const auto embedded_second = gdspaces::EmbeddedNameEvidenceBuilder::apply(
        index_then_embedded, *observed_second.observation);
    assert(embedded_second.ok());

    for (std::size_t slot = 0U; slot < embedded_then_index.children.size(); ++slot) {
        const auto& left = embedded_then_index.children[slot].payload;
        const auto& right = index_then_embedded.children[slot].payload;
        assert(left.resource.id == right.resource.id);
        assert(left.bytes == right.bytes);
        assert(left.resource.display_name == right.resource.display_name);
    }
    assert(index_then_embedded.children[0].payload.resource.format == "name-list");
    assert(index_then_embedded.children[0].payload.resource.display_name ==
           "st001_000.txt");
    assert(index_then_embedded.children[3].payload.resource.display_name ==
           "st001_003.hits");

    // A sealed observation cannot be replayed after the slot-0 authority bytes
    // change, even when physical ResourceId remains the same.
    auto mutated = expansion_fixture();
    const auto before_mutation = gdspaces::EmbeddedNameEvidenceBuilder::observe(mutated);
    assert(before_mutation.ok());
    mutated.children[0].payload.bytes[0] = std::byte{'X'};
    const auto rejected = gdspaces::EmbeddedNameEvidenceBuilder::apply(
        mutated, *before_mutation.observation);
    assert(!rejected.ok());

    // Recovered v6 fail-closed boundaries: oversized data, low-printable data,
    // or printable text without a retained filename extension is not promoted
    // to embedded naming authority.
    auto oversized = expansion_fixture();
    oversized.children[0].payload.bytes.assign(4097U, std::byte{'A'});
    oversized.children[0].payload.resource.id.size = 4097U;
    oversized.children[0].entry.size = 4097U;
    assert(!gdspaces::EmbeddedNameEvidenceBuilder::observe(oversized).ok());

    auto binary_like = expansion_fixture();
    binary_like.children[0].payload.bytes.assign(48U, std::byte{0x01});
    assert(!gdspaces::EmbeddedNameEvidenceBuilder::observe(binary_like).ok());

    auto no_extension = expansion_fixture();
    no_extension.children[0].payload.bytes = bytes_from_text("plain_name\r\n");
    no_extension.children[0].payload.resource.id.size =
        static_cast<std::uint64_t>(no_extension.children[0].payload.bytes.size());
    no_extension.children[0].entry.size =
        static_cast<std::uint64_t>(no_extension.children[0].payload.bytes.size());
    assert(!gdspaces::EmbeddedNameEvidenceBuilder::observe(no_extension).ok());

    return 0;
}
