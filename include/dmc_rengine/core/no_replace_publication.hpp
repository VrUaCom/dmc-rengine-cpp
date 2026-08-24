#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::core {

enum class NoReplacePublicationStatus {
    success,
    invalid_input,
    staging_conflict,
    staging_write_failed,
    destination_exists,
    publication_failed,
};

[[nodiscard]] std::string_view to_string(NoReplacePublicationStatus status) noexcept;

struct NoReplacePublicationResult final {
    NoReplacePublicationStatus status{NoReplacePublicationStatus::invalid_input};
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == NoReplacePublicationStatus::success;
    }
};

// Atomically publishes an already-validated file without replacement semantics.
// The staged file and destination must live on the same filesystem. The staged
// file is never removed by this function.
[[nodiscard]] NoReplacePublicationResult publish_validated_file_no_replace(
    const std::filesystem::path& validated_staged_file,
    const std::filesystem::path& destination) noexcept;

// Writes bytes into an exclusively-owned same-filesystem staging directory and
// commits them through publish_validated_file_no_replace(). The final
// destination is never truncated or replaced. Owned staging is cleaned on all
// return paths; a pre-existing staging reservation is never removed.
[[nodiscard]] NoReplacePublicationResult publish_bytes_no_replace(
    const std::filesystem::path& destination,
    std::span<const std::byte> bytes,
    std::string_view staging_suffix = ".dmc-rengine-publish.staging") noexcept;

} // namespace dmc::rengine::core
