#pragma once

#include "dmc_rengine/gdspaces/byte_provenance.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

enum class RetailAcquisitionStatus {
    acquired,
    volume_scan_ambiguous,
    no_contiguous_bootstrap,
    physical_mount_failed,
    archive_mount_failed,
    invalid_source_bindings,
    unresolved_request,
    resolved_outside_archive,
    member_metadata_unavailable,
    archive_identity_unstable,
    member_observation_failed,
};

[[nodiscard]] constexpr std::string_view to_string(
    RetailAcquisitionStatus status) noexcept {
    switch (status) {
    case RetailAcquisitionStatus::acquired: return "acquired";
    case RetailAcquisitionStatus::volume_scan_ambiguous:
        return "volume-scan-ambiguous";
    case RetailAcquisitionStatus::no_contiguous_bootstrap:
        return "no-contiguous-bootstrap";
    case RetailAcquisitionStatus::physical_mount_failed:
        return "physical-mount-failed";
    case RetailAcquisitionStatus::archive_mount_failed:
        return "archive-mount-failed";
    case RetailAcquisitionStatus::invalid_source_bindings:
        return "invalid-source-bindings";
    case RetailAcquisitionStatus::unresolved_request:
        return "unresolved-request";
    case RetailAcquisitionStatus::resolved_outside_archive:
        return "resolved-outside-archive";
    case RetailAcquisitionStatus::member_metadata_unavailable:
        return "member-metadata-unavailable";
    case RetailAcquisitionStatus::archive_identity_unstable:
        return "archive-identity-unstable";
    case RetailAcquisitionStatus::member_observation_failed:
        return "member-observation-failed";
    }
    return "unresolved-request";
}

/// What a direct-retail acquisition observed, in the terms the evidence gate
/// asks for: which volume won, what the archive was, which central entry was
/// selected, and what the materialized bytes are.
struct RetailAcquisitionReceipt final {
    std::string game_request;
    std::uint32_t selected_volume_index{};
    std::size_t resolver_probe_count{};

    std::uint32_t first_missing_index{};
    std::size_t ignored_after_first_gap{};
    std::size_t ignored_outside_runtime_domain{};

    std::filesystem::path archive_path;
    std::uint64_t archive_size{};
    std::string archive_sha256;

    gdspaces::NbzZipEntry entry;

    std::uint64_t materialized_size{};
    std::string materialized_sha256;
    gdspaces::ByteProvenance provenance;
};

struct RetailAcquisition final {
    RetailAcquisitionStatus status{RetailAcquisitionStatus::unresolved_request};
    std::string detail;
    std::optional<RetailAcquisitionReceipt> receipt;
    /// The materialized member. Empty unless the acquisition succeeded.
    std::vector<std::byte> bytes;

    [[nodiscard]] bool ok() const noexcept {
        return status == RetailAcquisitionStatus::acquired && receipt.has_value();
    }
};

/// Direct-retail member acquisition: scan the executable-relative volumes,
/// bootstrap them, resolve a game request, bind the winning archive to its
/// exact identity, and observe the selected member inside that binding.
///
/// This lives in the library rather than in a command because the receipt it
/// produces is a Layer-1 evidence gate, and an evidence gate reachable only
/// from a desktop command line cannot be produced by whoever actually holds a
/// retail installation. Publication is deliberately not part of it: the caller
/// owns where bytes go, and every frontend already has its own publisher.
class RetailMemberAcquisition final {
public:
    [[nodiscard]] static RetailAcquisition acquire(
        const std::filesystem::path& executable_directory,
        std::string_view game_request);

    /// The canonical receipt document. One emitter, so a receipt written by a
    /// phone and a receipt written by the command line are the same bytes.
    [[nodiscard]] static std::string receipt_json(
        const RetailAcquisitionReceipt& receipt,
        const std::filesystem::path& output_file);
};

} // namespace dmc::rengine::profiles::dmc3
