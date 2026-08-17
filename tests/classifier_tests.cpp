#include "dmc_rengine/gdspaces/classifier.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

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

    // Physical bytes outrank a misleading filename extension. This protects
    // sparse PNST identity from being collapsed into the PAC namespace.
    const std::vector<std::byte> pnst_bytes{
        std::byte{'P'}, std::byte{'N'}, std::byte{'S'}, std::byte{'T'}};
    const auto pnst_by_magic = ResourceClassifier::classify(
        "DMC3/room/misleading.pac", std::span<const std::byte>{pnst_bytes});
    assert(pnst_by_magic.format == "pnst");
    assert(pnst_by_magic.container);
    assert(pnst_by_magic.magic_confirmed);

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
