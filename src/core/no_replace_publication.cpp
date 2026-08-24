#include "dmc_rengine/core/no_replace_publication.hpp"

#include <fstream>
#include <limits>
#include <system_error>
#include <utility>

namespace dmc::rengine::core {
namespace {

struct OwnedStagingDirectory final {
    std::filesystem::path path;

    ~OwnedStagingDirectory() {
        if (!path.empty()) {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }
    }
};

[[nodiscard]] NoReplacePublicationResult failure(
    NoReplacePublicationStatus status,
    std::string detail) {
    return NoReplacePublicationResult{
        .status = status,
        .detail = std::move(detail),
    };
}

} // namespace

std::string_view to_string(NoReplacePublicationStatus status) noexcept {
    switch (status) {
    case NoReplacePublicationStatus::success:
        return "success";
    case NoReplacePublicationStatus::invalid_input:
        return "invalid-input";
    case NoReplacePublicationStatus::staging_conflict:
        return "staging-conflict";
    case NoReplacePublicationStatus::staging_write_failed:
        return "staging-write-failed";
    case NoReplacePublicationStatus::destination_exists:
        return "destination-exists";
    case NoReplacePublicationStatus::publication_failed:
        return "publication-failed";
    }
    return "unknown";
}

NoReplacePublicationResult publish_validated_file_no_replace(
    const std::filesystem::path& validated_staged_file,
    const std::filesystem::path& destination) noexcept {
    if (validated_staged_file.empty() || destination.empty() ||
        validated_staged_file == destination) {
        return failure(
            NoReplacePublicationStatus::invalid_input,
            "Validated staging and destination paths must be distinct and non-empty.");
    }

    std::error_code error;
    if (!std::filesystem::is_regular_file(validated_staged_file, error) || error) {
        return failure(
            NoReplacePublicationStatus::invalid_input,
            "Validated staging path is not a readable regular file.");
    }

    error.clear();
    std::filesystem::create_hard_link(validated_staged_file, destination, error);
    if (!error) {
        return NoReplacePublicationResult{
            .status = NoReplacePublicationStatus::success,
            .detail = {},
        };
    }

    std::error_code exists_error;
    const bool destination_exists =
        std::filesystem::exists(destination, exists_error) && !exists_error;
    if (destination_exists) {
        return failure(
            NoReplacePublicationStatus::destination_exists,
            "Destination already exists; no replacement was performed.");
    }

    return failure(
        NoReplacePublicationStatus::publication_failed,
        "Final no-replace publication failed: " + error.message());
}

NoReplacePublicationResult publish_bytes_no_replace(
    const std::filesystem::path& destination,
    std::span<const std::byte> bytes,
    std::string_view staging_suffix) noexcept {
    if (destination.empty() || staging_suffix.empty() ||
        bytes.size() > static_cast<std::size_t>(
            std::numeric_limits<std::streamsize>::max())) {
        return failure(
            NoReplacePublicationStatus::invalid_input,
            "Destination/staging input is invalid or payload exceeds stream limits.");
    }

    const auto parent = destination.parent_path();
    std::error_code error;
    if (parent.empty() || !std::filesystem::is_directory(parent, error) || error) {
        return failure(
            NoReplacePublicationStatus::invalid_input,
            "Destination parent directory must already exist.");
    }

    const auto staging_directory = std::filesystem::path{
        destination.string() + std::string{staging_suffix}};
    error.clear();
    if (!std::filesystem::create_directory(staging_directory, error) || error) {
        return failure(
            NoReplacePublicationStatus::staging_conflict,
            "Unable to acquire exclusive staging ownership.");
    }
    OwnedStagingDirectory owned{staging_directory};

    const auto staged_file = staging_directory / "payload.bin";
    std::ofstream stream(
        staged_file,
        std::ios::binary | std::ios::out | std::ios::trunc);
    if (!stream) {
        return failure(
            NoReplacePublicationStatus::staging_write_failed,
            "Unable to create staging payload.");
    }

    if (!bytes.empty()) {
        stream.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    stream.flush();
    const bool write_ok = static_cast<bool>(stream);
    stream.close();
    if (!write_ok) {
        return failure(
            NoReplacePublicationStatus::staging_write_failed,
            "Unable to write the complete staging payload.");
    }

    return publish_validated_file_no_replace(staged_file, destination);
}

} // namespace dmc::rengine::core
