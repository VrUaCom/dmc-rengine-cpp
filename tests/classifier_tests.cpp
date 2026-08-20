#include "dmc_rengine/gdspaces/classifier.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
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

} // namespace

int main() {
    using dmc::rengine::gdspaces::GameProfile;
    using dmc::rengine::gdspaces::ResourceClassifier;

    const auto pac_by_extension = ResourceClassifier::classify(
        "DMC3/room/st001cfg.pac");
    assert(pac_by_extension.format == "pac");
    assert(pac_by_extension.profile == GameProfile::dmc3_hd);
    assert(pac_by_extension.container);
    assert(!pac_by_extension.magic_confirmed);

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

    assert(ResourceClassifier::profile_from_path("DMC2/data") ==
        GameProfile::dmc2_hd);
    assert(ResourceClassifier::profile_from_path("dmclauncher/config") ==
        GameProfile::dmc_launcher_hd);
    assert(ResourceClassifier::profile_from_path("other/data") ==
        GameProfile::unknown);

    return 0;
}
