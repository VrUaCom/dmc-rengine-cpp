#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/container_naming_reconciler.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> bytes_from_text(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

[[nodiscard]] unsigned int hex_digit(char character) {
    if (character >= '0' && character <= '9') {
        return static_cast<unsigned int>(character - '0');
    }
    if (character >= 'a' && character <= 'f') {
        return static_cast<unsigned int>(character - 'a') + 10U;
    }
    if (character >= 'A' && character <= 'F') {
        return static_cast<unsigned int>(character - 'A') + 10U;
    }
    assert(false);
    return 0U;
}

[[nodiscard]] std::vector<std::byte> bytes_from_hex(std::string_view hex) {
    assert(hex.size() % 2U == 0U);
    std::vector<std::byte> bytes;
    bytes.reserve(hex.size() / 2U);
    for (std::size_t offset = 0U; offset < hex.size(); offset += 2U) {
        const auto value = (hex_digit(hex[offset]) << 4U) |
            hex_digit(hex[offset + 1U]);
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerChild naming_child(
    std::string_view parent,
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
                    .source_id = "retail-corpus",
                    .logical_path = std::string{parent} + "::PAC/slot-" + slot_text,
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
            .semantic_evidence = {},
        },
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion naming_fixture(
    std::string parent_name,
    std::vector<std::byte> slot_zero_bytes) {
    namespace gdspaces = dmc::rengine::gdspaces;

    gdspaces::ContainerExpansion expansion{
        .parent = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-corpus",
                .logical_path = parent_name,
                .container_chain = {},
                .offset = 0U,
                .size = 0x1000U,
            },
            .display_name = parent_name,
            .format = "pac",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .parser_format = "PAC",
        .children = {},
        .diagnostics = {},
    };

    expansion.children.push_back(naming_child(
        parent_name, 0U, 0x20U, std::move(slot_zero_bytes)));
    expansion.children.push_back(naming_child(
        parent_name, 1U, 0x80U, bytes_from_text("PTX?")));
    expansion.children.push_back(naming_child(
        parent_name, 2U, 0x90U, bytes_from_text("SCM?")));
    expansion.children.push_back(naming_child(
        parent_name, 3U, 0xA0U, bytes_from_text("HITS")));
    return expansion;
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::ContainerNamingReconciler;
    using dmc::rengine::gdspaces::GameProfile;
    using dmc::rengine::gdspaces::ResourceClassifier;

    const auto pac_by_extension = ResourceClassifier::classify(
        "DMC3/room/st001cfg.pac");
    assert(pac_by_extension.format == "pac");
    assert(pac_by_extension.profile == GameProfile::dmc3_hd);
    assert(pac_by_extension.container);
    assert(!pac_by_extension.magic_confirmed);
    assert(!pac_by_extension.structural_confirmed);

    const std::vector<std::byte> pe_bytes{
        std::byte{'M'}, std::byte{'Z'}, std::byte{0x00}, std::byte{0x00}};
    const auto pe_by_magic = ResourceClassifier::classify(
        "unknown/blob.bin", std::span<const std::byte>{pe_bytes});
    assert(pe_by_magic.format == "pe");
    assert(pe_by_magic.magic_confirmed);
    assert(!pe_by_magic.container);

    const std::vector<std::byte> pac_bytes{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{0x00}};
    const auto pac_by_magic = ResourceClassifier::classify(
        "resource.data", std::span<const std::byte>{pac_bytes});
    assert(pac_by_magic.format == "pac");
    assert(pac_by_magic.container);
    assert(pac_by_magic.magic_confirmed);

    // Real extracted DMC3 corpora contain binary PNST payloads under misleading
    // .pac names. A structurally valid relative-slot image must still classify
    // by bytes rather than by extension.
    std::vector<std::byte> binary_pnst(0x20U, std::byte{0});
    binary_pnst[0] = std::byte{'P'};
    binary_pnst[1] = std::byte{'N'};
    binary_pnst[2] = std::byte{'S'};
    binary_pnst[3] = std::byte{'T'};
    put_u32(binary_pnst, 4U, 1U);
    put_u32(binary_pnst, 8U, 0x10U);
    binary_pnst[0x10U] = std::byte{'D'};
    binary_pnst[0x11U] = std::byte{'D'};
    binary_pnst[0x12U] = std::byte{'S'};
    binary_pnst[0x13U] = std::byte{' '};
    const auto pnst_under_pac = ResourceClassifier::classify(
        "stage/st445_005.pac", std::span<const std::byte>{binary_pnst});
    assert(pnst_under_pac.format == "pnst");
    assert(pnst_under_pac.container);
    assert(pnst_under_pac.magic_confirmed);

    // Real .index manifests also start with the literal text line "PNST\r\n".
    // Prefix-only classification would turn those text indices into fake binary
    // containers. Structural PNST validation must fail and path extension must
    // remain the classification authority.
    const std::vector<std::byte> text_pnst_index{
        std::byte{'P'}, std::byte{'N'}, std::byte{'S'}, std::byte{'T'},
        std::byte{'\r'}, std::byte{'\n'},
        std::byte{'e'}, std::byte{'m'}, std::byte{'0'}, std::byte{'3'},
        std::byte{'5'}, std::byte{'_'}, std::byte{'0'}, std::byte{'5'},
        std::byte{'7'}, std::byte{'_'}, std::byte{'0'}, std::byte{'0'},
        std::byte{'0'}, std::byte{'.'}, std::byte{'t'}, std::byte{'x'},
        std::byte{'t'}, std::byte{'\r'}, std::byte{'\n'},
    };
    const auto text_index = ResourceClassifier::classify(
        "em035_057.index", std::span<const std::byte>{text_pnst_index});
    assert(text_index.format == "index");
    assert(!text_index.container);
    assert(!text_index.magic_confirmed);

    const auto invalid_pnst_unknown = ResourceClassifier::classify(
        "unknown/blob.bin", std::span<const std::byte>{text_pnst_index});
    assert(invalid_pnst_unknown.format == "bin");
    assert(!invalid_pnst_unknown.container);
    assert(!invalid_pnst_unknown.magic_confirmed);

    // Canonical HITS magic is exactly the four-byte prefix "HITS".
    // The following header byte is format data, not a '$' magic suffix.
    const std::vector<std::byte> hits_bytes{
        std::byte{'H'}, std::byte{'I'}, std::byte{'T'}, std::byte{'S'}};
    const auto hits = ResourceClassifier::classify(
        "stage/collision.ukn", std::span<const std::byte>{hits_bytes});
    assert(hits.format == "hits");
    assert(hits.magic_confirmed);

    // Exact retained st001 slot-0 corpus image. Raw classification by the new
    // canonical display suffix alone would call this "index". Reconciliation
    // must instead seal structural name-list evidence, and the materialized
    // classifier must use that evidence rather than the display suffix.
    auto st001_name_list = bytes_from_text(
        "st001.ptx\r\n"
        "st001.scm\r\n"
        "st001.sch\r\n");
    st001_name_list.insert(st001_name_list.end(), 15U, std::byte{0});
    assert(st001_name_list.size() == 48U);
    auto st001 = naming_fixture("DMC3/st001.pac", std::move(st001_name_list));
    std::vector<dmc::rengine::gdspaces::ResourceId> st001_ids;
    std::vector<std::vector<std::byte>> st001_bytes;
    for (const auto& child : st001.children) {
        st001_ids.push_back(child.payload.resource.id);
        st001_bytes.push_back(child.payload.bytes);
    }
    const auto st001_reconcile = ContainerNamingReconciler::reconcile(st001);
    assert(st001_reconcile.ok());
    assert(st001_reconcile.embedded_name_list_applied);
    assert(!st001_reconcile.external_index_applied);
    assert(st001.children[0].payload.resource.format == "name-list");
    assert(st001.children[0].payload.resource.display_name == "st001_000.index");
    assert(st001.children[0].payload.semantic_evidence.size() == 1U);
    assert(st001.children[0].payload.semantic_evidence[0].authority_sha256() ==
        "7efcf182f28135a3d694324ba06715ca6f6b075a028d035c89c69809df23faa5");
    for (std::size_t index = 0U; index < st001.children.size(); ++index) {
        assert(st001.children[index].payload.resource.id == st001_ids[index]);
        assert(st001.children[index].payload.bytes == st001_bytes[index]);
    }

    const auto raw_display_probe = ResourceClassifier::classify(
        st001.children[0].payload.resource.display_name,
        std::span<const std::byte>{st001.children[0].payload.bytes});
    assert(raw_display_probe.format == "index");
    assert(!raw_display_probe.structural_confirmed);

    const auto sealed_st001 = ResourceClassifier::classify(
        st001.children[0].payload,
        st001.children[0].payload.resource.display_name);
    assert(sealed_st001.format == "name-list");
    assert(sealed_st001.structural_confirmed);
    assert(!sealed_st001.magic_confirmed);

    // Stale semantic evidence must fail closed. Even though the presentation
    // still ends in .index, changing one authority byte removes structural
    // confirmation and the display suffix is ignored rather than laundered.
    auto stale_st001 = st001.children[0].payload;
    stale_st001.bytes[0] = std::byte{'X'};
    const auto stale_classification = ResourceClassifier::classify(
        stale_st001, stale_st001.resource.display_name);
    assert(stale_classification.format != "index");
    assert(stale_classification.format != "name-list");
    assert(!stale_classification.structural_confirmed);

    // Independent second retained corpus proof: st445 uses the same semantic
    // convention but a different exact byte image and SHA-256.
    auto st445_name_list = bytes_from_text(
        "st445.ptx\r\n"
        "st445.scm\r\n"
        "st445.sch");
    st445_name_list.push_back(std::byte{0});
    assert(st445_name_list.size() == 32U);
    auto st445 = naming_fixture("DMC3/st445.pac", std::move(st445_name_list));
    const auto st445_reconcile = ContainerNamingReconciler::reconcile(st445);
    assert(st445_reconcile.ok());
    assert(st445_reconcile.embedded_name_list_applied);
    assert(st445.children[0].payload.resource.display_name == "st445_000.index");
    assert(st445.children[0].payload.semantic_evidence.size() == 1U);
    assert(st445.children[0].payload.semantic_evidence[0].authority_sha256() ==
        "c93360ac57fd602b5d544dffa5a506f1c6769ed8a194c600e95c1e4873a1e687");
    const auto sealed_st445 = ResourceClassifier::classify(
        st445.children[0].payload,
        st445.children[0].payload.resource.display_name);
    assert(sealed_st445.format == "name-list");
    assert(sealed_st445.structural_confirmed);

    // Exact third retained _000.ukn candidate. This is binary, not a name list.
    // The reconciler must not manufacture semantic evidence from slot number or
    // historical .ukn naming patterns.
    auto m20_binary = bytes_from_hex(
        "0100000000000000000000000000000000000000000000000000000050000000"
        "580200000330e3020000000000000000000065781e0000000000803f0000803f"
        "000065730000000000000000000000000000803f0000803f0200000000000000");
    assert(m20_binary.size() == 96U);
    auto m20 = naming_fixture("DMC3/m20_s00_012.pac", std::move(m20_binary));
    const auto m20_reconcile = ContainerNamingReconciler::reconcile(m20);
    assert(m20_reconcile.ok());
    assert(!m20_reconcile.embedded_name_list_applied);
    assert(m20.children[0].payload.semantic_evidence.empty());
    assert(m20.children[0].payload.resource.format == "unknown");

    assert(ResourceClassifier::profile_from_path("DMC2/data") ==
        GameProfile::dmc2_hd);
    assert(ResourceClassifier::profile_from_path("dmclauncher/config") ==
        GameProfile::dmc_launcher_hd);
    assert(ResourceClassifier::profile_from_path("other/data") ==
        GameProfile::unknown);

    return 0;
}
