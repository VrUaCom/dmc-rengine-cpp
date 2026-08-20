#pragma once

#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

inline constexpr std::uint32_t zip32_u32_sentinel = 0xFFFFFFFFU;

struct NbzRetailReplacement final {
    std::uint32_t central_index{};
    std::string expected_logical_path;
    std::vector<std::byte> bytes;
};

struct NbzRetailRepackLimits final {
    std::uint64_t io_chunk_bytes{1024ULL * 1024ULL};
    std::uint64_t max_replacement_bytes{512ULL * 1024ULL * 1024ULL};
    std::uint64_t max_metadata_bytes{64ULL * 1024ULL * 1024ULL};
};

enum class NbzRetailRepackStatus : std::uint8_t {
    ok,
    invalid_source,
    invalid_bound_snapshot,
    invalid_output_path,
    output_exists,
    invalid_limits,
    duplicate_replacement,
    replacement_not_found,
    replacement_path_mismatch,
    directory_replacement,
    alias_local_region_unsupported,
    data_descriptor_replacement_unsupported,
    replacement_too_large,
    zip32_overflow,
    source_read_failure,
    output_write_failure,
    source_artifact_mismatch,
    output_validation_failure,
    invalid_receipt,
};

[[nodiscard]] constexpr std::string_view to_string(
    NbzRetailRepackStatus status) noexcept {
    switch (status) {
    case NbzRetailRepackStatus::ok: return "ok";
    case NbzRetailRepackStatus::invalid_source: return "invalid-source";
    case NbzRetailRepackStatus::invalid_bound_snapshot:
        return "invalid-bound-snapshot";
    case NbzRetailRepackStatus::invalid_output_path: return "invalid-output-path";
    case NbzRetailRepackStatus::output_exists: return "output-exists";
    case NbzRetailRepackStatus::invalid_limits: return "invalid-limits";
    case NbzRetailRepackStatus::duplicate_replacement:
        return "duplicate-replacement";
    case NbzRetailRepackStatus::replacement_not_found:
        return "replacement-not-found";
    case NbzRetailRepackStatus::replacement_path_mismatch:
        return "replacement-path-mismatch";
    case NbzRetailRepackStatus::directory_replacement:
        return "directory-replacement";
    case NbzRetailRepackStatus::alias_local_region_unsupported:
        return "alias-local-region-unsupported";
    case NbzRetailRepackStatus::data_descriptor_replacement_unsupported:
        return "data-descriptor-replacement-unsupported";
    case NbzRetailRepackStatus::replacement_too_large:
        return "replacement-too-large";
    case NbzRetailRepackStatus::zip32_overflow: return "zip32-overflow";
    case NbzRetailRepackStatus::source_read_failure:
        return "source-read-failure";
    case NbzRetailRepackStatus::output_write_failure:
        return "output-write-failure";
    case NbzRetailRepackStatus::source_artifact_mismatch:
        return "source-artifact-mismatch";
    case NbzRetailRepackStatus::output_validation_failure:
        return "output-validation-failure";
    case NbzRetailRepackStatus::invalid_receipt: return "invalid-receipt";
    }
    return "invalid-receipt";
}

struct NbzRetailRepackEntryReceipt final {
    std::uint32_t central_index{};
    std::string logical_path;
    bool changed{};
    std::uint16_t original_method{};
    std::uint16_t output_method{};
    std::uint32_t original_local_header_offset{};
    std::uint32_t output_local_header_offset{};
    std::uint32_t output_crc32{};
    std::uint32_t output_compressed_size{};
    std::uint32_t output_uncompressed_size{};

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzRetailRepackReceipt final {
    evidence::ArtifactIdentity source_artifact;
    evidence::ArtifactIdentity output_artifact;
    std::uint32_t output_central_offset{};
    std::uint32_t output_central_size{};
    std::uint32_t changed_entry_count{};
    bool byte_identical{};
    std::vector<NbzRetailRepackEntryReceipt> entries;

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzRetailRepackResult final {
    NbzRetailRepackStatus status{NbzRetailRepackStatus::invalid_source};
    std::optional<NbzRetailRepackReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == NbzRetailRepackStatus::ok && receipt.has_value() &&
            receipt->valid();
    }
};

class NbzRetailRepacker final {
public:
    static constexpr std::uint64_t max_io_chunk_bytes = 64ULL * 1024ULL * 1024ULL;
    static constexpr std::uint64_t max_metadata_bytes = 512ULL * 1024ULL * 1024ULL;
    static constexpr std::uint32_t zip32_u32_sentinel =
        dmc::rengine::profiles::dmc3::zip32_u32_sentinel;

    // Product metadata-preserving retail repack tier. Unchanged local regions
    // are copied byte-for-byte. Changed entries are emitted as method-0 STORE
    // while preserving filename/extra/time/attribute/comment metadata and
    // rebuilding only fields that must change. This is not Capcom compressor
    // or offline-packer equivalence.
    [[nodiscard]] static NbzRetailRepackResult write(
        const gdspaces::NbzZipSource& source,
        const gdspaces::ArtifactBoundNbzZipSerializationSnapshot& bound,
        const std::filesystem::path& output_path,
        std::span<const NbzRetailReplacement> replacements,
        NbzRetailRepackLimits limits = {});
};

} // namespace dmc::rengine::profiles::dmc3
