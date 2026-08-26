#include "dmc_rengine/profiles/dmc3/retail_member_acquisition.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Direct-retail acquisition is a Layer-1 evidence gate. It is exercised here
// through the library rather than through the command that used to own it,
// because the point of moving it was that any frontend can produce the receipt.

namespace {

namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::vector<std::byte> ascii(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

void write_file(const std::filesystem::path& path, std::span<const std::byte> bytes) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    assert(stream.good());
}

[[nodiscard]] std::vector<std::byte> build_volume(
    const std::vector<std::uint32_t>& present,
    std::string_view logical_path,
    std::span<const std::byte> payload) {
    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(present);
    assert(bootstrap.valid());
    const std::vector<dmc3::NbzOverlayMember> members{
        dmc3::NbzOverlayMember{
            .logical_path = std::string{logical_path},
            .bytes = std::vector<std::byte>{payload.begin(), payload.end()},
        },
    };
    const auto overlay = dmc3::NbzStoreOverlayWriter::build(bootstrap, members);
    assert(overlay.ok());
    return overlay.bytes;
}

} // namespace

int main() {
    const auto root = std::filesystem::temp_directory_path() /
        "dmc-rengine-retail-acquisition-tests";
    std::filesystem::remove_all(root);
    const auto data = root / "data" / "dmc3";

    const auto payload = ascii("retail-acquisition-member-payload");
    write_file(
        data / "DMC3-0.nbz",
        build_volume({0U}, "GData.afs/em000.pac", payload));

    // The request is a basename, not a pre-guessed archive path: the recovered
    // request policy is what turns it into the member that actually wins.
    const auto acquired =
        dmc3::RetailMemberAcquisition::acquire(root, "obj/em000.pac");
    assert(acquired.ok());
    const auto& receipt = *acquired.receipt;
    assert(receipt.entry.logical_path == "GData.afs/em000.pac");
    assert(receipt.selected_volume_index == 0U);
    assert(receipt.materialized_size == payload.size());
    assert(acquired.bytes == payload);
    assert(receipt.archive_sha256.size() == 64U);
    assert(receipt.materialized_sha256.size() == 64U);
    assert(receipt.ignored_after_first_gap == 0U);

    // The receipt document carries the identities the gate asks for, and the
    // emitter is shared so every frontend writes the same bytes.
    const auto document = dmc3::RetailMemberAcquisition::receipt_json(
        receipt, root / "out" / "em000.pac");
    assert(document.find("\"evidence_class\": \"artifact-bound-retail-member-acquisition\"")
        != std::string::npos);
    assert(document.find(receipt.archive_sha256) != std::string::npos);
    assert(document.find(receipt.materialized_sha256) != std::string::npos);
    assert(document.find("\"selected_volume_index\": 0") != std::string::npos);

    // A higher contiguous volume wins, because every mount is prepended.
    write_file(
        data / "DMC3-1.nbz",
        build_volume({0U, 1U}, "GData.afs/em000.pac", ascii("newer-volume-payload")));
    const auto newer =
        dmc3::RetailMemberAcquisition::acquire(root, "obj/em000.pac");
    assert(newer.ok());
    assert(newer.receipt->selected_volume_index == 1U);
    assert(newer.bytes == ascii("newer-volume-payload"));

    // A volume past the first gap is discovery evidence, never a mount: the
    // original runtime could not reach it either.
    write_file(
        data / "DMC3-3.nbz",
        build_volume({0U}, "GData.afs/em000.pac", ascii("unreachable-payload")));
    const auto gapped =
        dmc3::RetailMemberAcquisition::acquire(root, "obj/em000.pac");
    assert(gapped.ok());
    assert(gapped.receipt->selected_volume_index == 1U);
    assert(gapped.receipt->first_missing_index == 2U);
    assert(gapped.receipt->ignored_after_first_gap == 1U);

    // A request nothing carries is a resolver answer, not a crash.
    const auto missing =
        dmc3::RetailMemberAcquisition::acquire(root, "obj/nothing-here.pac");
    assert(!missing.ok());
    assert(missing.status == dmc3::RetailAcquisitionStatus::unresolved_request);

    // No volumes at all is its own answer, distinct from an unresolved request.
    const auto empty = dmc3::RetailMemberAcquisition::acquire(
        root / "empty", "obj/em000.pac");
    assert(!empty.ok());
    assert(empty.status == dmc3::RetailAcquisitionStatus::volume_scan_ambiguous);

    std::filesystem::remove_all(root);
    return 0;
}
