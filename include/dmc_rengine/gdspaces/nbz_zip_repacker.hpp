#pragma once

#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

enum class NbzZipRetailRepackStatus : std::uint8_t {
    ok,
    invalid_source,
    invalid_binding,
    invalid_destination,
    invalid_replacement,
    duplicate_replacement,
    alias_replacement_incomplete,
    alias_replacement_conflict,
    unsupported_compression_method,
    unresolved_local_metadata,
    unresolved_data_descriptor,
    zip32_overflow,
    source_open_failed,
    source_read_failed,
    source_artifact_mismatch,
    destination_open_failed,
    destination_write_failed,
    destination_commit_failed,
    canonical_reopen_failed,
    canonical_validation_failed,
};

[[nodiscard]] constexpr std::string_view to_string(
    NbzZipRetailRepackStatus status) noexcept {
    switch (status) {
    case NbzZipRetailRepackStatus::ok: return "ok";
    case NbzZipRetailRepackStatus::invalid_source: return "invalid-source";
    case NbzZipRetailRepackStatus::invalid_binding: return "invalid-binding";
    case NbzZipRetailRepackStatus::invalid_destination: return "invalid-destination";
    case NbzZipRetailRepackStatus::invalid_replacement: return "invalid-replacement";
    case NbzZipRetailRepackStatus::duplicate_replacement: return "duplicate-replacement";
    case NbzZipRetailRepackStatus::alias_replacement_incomplete: return "alias-replacement-incomplete";
    case NbzZipRetailRepackStatus::alias_replacement_conflict: return "alias-replacement-conflict";
    case NbzZipRetailRepackStatus::unsupported_compression_method: return "unsupported-compression-method";
    case NbzZipRetailRepackStatus::unresolved_local_metadata: return "unresolved-local-metadata";
    case NbzZipRetailRepackStatus::unresolved_data_descriptor: return "unresolved-data-descriptor";
    case NbzZipRetailRepackStatus::zip32_overflow: return "zip32-overflow";
    case NbzZipRetailRepackStatus::source_open_failed: return "source-open-failed";
    case NbzZipRetailRepackStatus::source_read_failed: return "source-read-failed";
    case NbzZipRetailRepackStatus::source_artifact_mismatch: return "source-artifact-mismatch";
    case NbzZipRetailRepackStatus::destination_open_failed: return "destination-open-failed";
    case NbzZipRetailRepackStatus::destination_write_failed: return "destination-write-failed";
    case NbzZipRetailRepackStatus::destination_commit_failed: return "destination-commit-failed";
    case NbzZipRetailRepackStatus::canonical_reopen_failed: return "canonical-reopen-failed";
    case NbzZipRetailRepackStatus::canonical_validation_failed: return "canonical-validation-failed";
    }
    return "canonical-validation-failed";
}

struct NbzZipMemberReplacement final {
    std::uint32_t central_index{};
    std::vector<std::byte> materialized_bytes;
};

struct NbzZipRetailRepackLimits final {
    std::uint64_t io_chunk_bytes{1024ULL * 1024ULL};
};

struct NbzZipRetailRepackEntryReceipt final {
    std::uint32_t central_index{};
    bool changed{};
    std::uint16_t compression_method{};
    std::uint32_t crc32{};
    std::uint32_t compressed_size{};
    std::uint32_t uncompressed_size{};
    std::uint32_t local_header_offset{};
};

struct NbzZipRetailRepackReceipt final {
    std::string source_sha256;
    std::string output_sha256;
    std::uint64_t source_size{};
    std::uint64_t output_size{};
    std::uint32_t central_offset{};
    std::uint32_t central_size{};
    std::vector<NbzZipRetailRepackEntryReceipt> entries;

    [[nodiscard]] bool valid() const noexcept;
};

struct NbzZipRetailRepackResult final {
    NbzZipRetailRepackStatus status{NbzZipRetailRepackStatus::invalid_source};
    std::optional<NbzZipRetailRepackReceipt> receipt;
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == NbzZipRetailRepackStatus::ok &&
            receipt.has_value() && receipt->valid();
    }
};

class NbzZipRetailRepacker final {
public:
    // Writes a new classic-ZIP/NBZ artifact without materializing the complete
    // source archive. Unchanged physical local regions are copied byte-for-byte.
    // Changed aliases must be acknowledged together and carry byte-identical
    // replacement payloads. The source artifact is revalidated from the same
    // streaming observation that supplies copied source spans.
    [[nodiscard]] static NbzZipRetailRepackResult write(
        const NbzZipSource& source,
        const ArtifactBoundNbzZipSerializationSnapshot& bound,
        std::span<const NbzZipMemberReplacement> replacements,
        const std::filesystem::path& destination,
        NbzZipRetailRepackLimits limits = {});
};

} // namespace dmc::rengine::gdspaces
