#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {
namespace {

constexpr std::uint64_t max_hash_chunk_bytes = 64ULL * 1024ULL * 1024ULL;

void add_error(
    NbzZipArtifactBindingResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(Diagnostic{
        .severity = DiagnosticSeverity::error,
        .code = std::move(code),
        .message = std::move(message),
        .resource = std::nullopt,
    });
}

[[nodiscard]] unsigned char ascii_lower(unsigned char value) noexcept {
    if (value >= static_cast<unsigned char>('A') &&
        value <= static_cast<unsigned char>('F')) {
        return static_cast<unsigned char>(
            value - static_cast<unsigned char>('A') +
            static_cast<unsigned char>('a'));
    }
    return value;
}

[[nodiscard]] bool same_hex_digest(
    std::string_view left,
    std::string_view right) noexcept {
    if (left.size() != 64U || right.size() != 64U) {
        return false;
    }
    for (std::size_t index = 0U; index < left.size(); ++index) {
        const auto lhs = static_cast<unsigned char>(left[index]);
        const auto rhs = static_cast<unsigned char>(right[index]);
        if (std::isxdigit(lhs) == 0 || std::isxdigit(rhs) == 0 ||
            ascii_lower(lhs) != ascii_lower(rhs)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::optional<std::string> hash_exact_file(
    const std::filesystem::path& path,
    std::uint64_t expected_size,
    std::uint64_t chunk_bytes,
    std::string_view phase,
    NbzZipArtifactBindingResult& result) {
    if (chunk_bytes == 0U || chunk_bytes > max_hash_chunk_bytes ||
        chunk_bytes > static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.hash-chunk",
            "The streaming hash chunk size is outside the supported product bounds.");
        return std::nullopt;
    }

    std::error_code error;
    const auto size_before = std::filesystem::file_size(path, error);
    if (error || size_before != expected_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.size-mismatch",
            std::string{phase} +
                " hash pass observed an archive size different from the expected artifact.");
        return std::nullopt;
    }

    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.open-failed",
            std::string{phase} + " hash pass could not open the NBZ archive.");
        return std::nullopt;
    }

    const auto buffer_size = static_cast<std::size_t>(chunk_bytes);
    std::vector<std::byte> buffer(buffer_size);
    core::Sha256Accumulator accumulator;

    std::uint64_t consumed = 0U;
    while (consumed < expected_size) {
        const auto remaining = expected_size - consumed;
        const auto amount = static_cast<std::size_t>(
            std::min<std::uint64_t>(chunk_bytes, remaining));
        stream.read(
            reinterpret_cast<char*>(buffer.data()),
            static_cast<std::streamsize>(amount));
        if (stream.gcount() != static_cast<std::streamsize>(amount)) {
            add_error(
                result,
                "gdspaces.nbz.artifact-binding.short-read",
                std::string{phase} +
                    " hash pass could not read the exact expected archive bytes.");
            return std::nullopt;
        }
        if (!accumulator.update(std::span<const std::byte>{
                buffer.data(), amount})) {
            add_error(
                result,
                "gdspaces.nbz.artifact-binding.hash-overflow",
                "The archive exceeds the supported SHA-256 byte-count domain.");
            return std::nullopt;
        }
        consumed += static_cast<std::uint64_t>(amount);
    }

    const auto size_after = std::filesystem::file_size(path, error);
    if (error || size_after != expected_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.size-changed",
            std::string{phase} +
                " hash pass observed the archive size changing during verification.");
        return std::nullopt;
    }

    const auto digest = accumulator.finalize();
    if (!digest.has_value() || accumulator.byte_count() != expected_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.hash-finalize",
            "The streaming SHA-256 accumulator could not finalize the exact archive.");
        return std::nullopt;
    }
    return digest->hex();
}

} // namespace

bool ArtifactBoundNbzZipSerializationSnapshot::valid() const noexcept {
    return artifact.valid() && serialization.valid() &&
        artifact.size == serialization.archive_size &&
        same_hex_digest(artifact.sha256, pre_scan_sha256) &&
        same_hex_digest(artifact.sha256, post_scan_sha256) &&
        same_hex_digest(pre_scan_sha256, post_scan_sha256);
}

bool NbzZipArtifactBindingResult::ok() const noexcept {
    if (!snapshot.has_value() || !snapshot->valid()) {
        return false;
    }
    return std::none_of(
        diagnostics.begin(), diagnostics.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

NbzZipArtifactBindingResult NbzZipArtifactSerializationBinder::bind(
    const NbzZipSource& source,
    const evidence::ArtifactIdentity& expected_artifact,
    NbzZipSerializationLimits serialization_limits,
    NbzZipArtifactBindingLimits binding_limits) {
    NbzZipArtifactBindingResult result;

    if (!expected_artifact.valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.invalid-artifact",
            "Artifact binding requires a valid expected ArtifactIdentity.");
        return result;
    }
    if (!source.valid() || !source.index_receipt().has_value() ||
        !source.index_receipt()->valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.invalid-source",
            "Artifact binding requires a valid indexed NBZ source.");
        return result;
    }
    if (expected_artifact.size != source.index_receipt()->archive_size) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.identity-size",
            "Expected ArtifactIdentity size does not match the indexed NBZ archive size.");
        return result;
    }

    auto pre_scan = hash_exact_file(
        source.archive_path(),
        expected_artifact.size,
        binding_limits.hash_chunk_bytes,
        "Pre-scan",
        result);
    if (!pre_scan.has_value()) {
        return result;
    }
    if (!same_hex_digest(*pre_scan, expected_artifact.sha256)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.hash-mismatch",
            "Pre-scan SHA-256 does not match the expected ArtifactIdentity.");
        return result;
    }

    auto scan = NbzZipSerializationScanner::scan(source, serialization_limits);
    result.diagnostics.insert(
        result.diagnostics.end(),
        scan.diagnostics.begin(),
        scan.diagnostics.end());
    if (!scan.ok()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.serialization-scan",
            "The NBZ serialization snapshot could not be produced during artifact binding.");
        return result;
    }

    auto post_scan = hash_exact_file(
        source.archive_path(),
        expected_artifact.size,
        binding_limits.hash_chunk_bytes,
        "Post-scan",
        result);
    if (!post_scan.has_value()) {
        return result;
    }
    if (!same_hex_digest(*post_scan, expected_artifact.sha256) ||
        !same_hex_digest(*pre_scan, *post_scan)) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.scan-window-changed",
            "The archive identity changed across the serialization scan window.");
        return result;
    }

    ArtifactBoundNbzZipSerializationSnapshot bound{
        .artifact = expected_artifact,
        .serialization = std::move(*scan.snapshot),
        .pre_scan_sha256 = std::move(*pre_scan),
        .post_scan_sha256 = std::move(*post_scan),
    };
    if (!bound.valid()) {
        add_error(
            result,
            "gdspaces.nbz.artifact-binding.invalid-receipt",
            "Artifact-bound NBZ serialization snapshot failed its final receipt invariants.");
        return result;
    }

    result.snapshot = std::move(bound);
    return result;
}

} // namespace dmc::rengine::gdspaces
