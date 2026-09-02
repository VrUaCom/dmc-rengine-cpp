#include "dmc_rengine/core/no_replace_publication.hpp"

#include <cerrno>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

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

/**
 * Creates `destination` only if nothing is there, and copies `source` into it.
 *
 * This is the second way to publish without replacing, and it exists because
 * the first one is not universally available. A hard link is the better
 * primitive — one inode, no second copy of the bytes — but plenty of real
 * filesystems refuse to make one: FUSE-backed emulated storage on Android, the
 * FAT and exFAT on removable cards, and various vendor overlays. A phone
 * reported `link()` failing with EACCES in a directory it had just created a
 * directory and written a file in, which is that refusal exactly.
 *
 * The guarantee is unchanged, not weakened. `O_EXCL` (and `CREATE_NEW` on
 * Windows) is atomic and fails when the destination exists, which is the whole
 * of what the link gave us. What it costs is a second write of the payload.
 */
[[nodiscard]] bool copy_into_exclusively_created_file(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    std::error_code& error) {
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        error = std::make_error_code(std::errc::io_error);
        return false;
    }

#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        destination.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        error.assign(
            ::GetLastError() == ERROR_FILE_EXISTS
                ? static_cast<int>(std::errc::file_exists)
                : static_cast<int>(std::errc::permission_denied),
            std::generic_category());
        return false;
    }
#else
    const int handle = ::open(
        destination.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (handle < 0) {
        error.assign(errno, std::generic_category());
        return false;
    }
#endif

    std::vector<char> buffer(1U << 16U);
    bool wrote_everything = true;
    while (input && wrote_everything) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto filled = static_cast<std::size_t>(input.gcount());
        if (filled == 0U) {
            break;
        }
        std::size_t written = 0U;
        while (written < filled) {
#if defined(_WIN32)
            DWORD produced = 0;
            if (::WriteFile(
                    handle, buffer.data() + written,
                    static_cast<DWORD>(filled - written), &produced, nullptr) == 0 ||
                produced == 0) {
                wrote_everything = false;
                break;
            }
#else
            const auto produced = ::write(
                handle, buffer.data() + written, filled - written);
            if (produced <= 0) {
                if (errno == EINTR) continue;
                wrote_everything = false;
                break;
            }
#endif
            written += static_cast<std::size_t>(produced);
        }
    }

#if defined(_WIN32)
    const bool flushed = ::FlushFileBuffers(handle) != 0;
    ::CloseHandle(handle);
#else
    // The bytes have to be on the device before this reports success: a
    // publication that survives only in page cache is not a published file.
    const bool flushed = ::fsync(handle) == 0 || errno == EINVAL;
    ::close(handle);
#endif

    if (!wrote_everything || !flushed) {
        // A half-written destination is worse than none: it looks like a
        // finished export. Take it back out.
        std::error_code cleanup;
        std::filesystem::remove(destination, cleanup);
        error = std::make_error_code(std::errc::io_error);
        return false;
    }
    return true;
}

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
    case NoReplacePublicationStatus::staging_validation_failed:
        return "staging-validation-failed";
    case NoReplacePublicationStatus::destination_exists:
        return "destination-exists";
    case NoReplacePublicationStatus::publication_failed:
        return "publication-failed";
    }
    return "unknown";
}

NoReplacePublicationResult publish_validated_file_no_replace(
    const std::filesystem::path& validated_staged_file,
    const std::filesystem::path& destination) {
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
    const auto link_error = error;

    std::error_code exists_error;
    const bool destination_exists =
        std::filesystem::exists(destination, exists_error) && !exists_error;
    if (destination_exists) {
        return failure(
            NoReplacePublicationStatus::destination_exists,
            "Destination already exists; no replacement was performed.");
    }

    // The link failed and nothing is in the way, so the filesystem is the
    // thing refusing. Publishing by exclusive create keeps the same no-replace
    // guarantee and costs one extra write of the payload. Reporting the link
    // error instead — which is what used to happen — turns "this card cannot
    // do hard links" into "Permission denied" and an export that never
    // happened.
    std::error_code copy_error;
    if (copy_into_exclusively_created_file(
            validated_staged_file, destination, copy_error)) {
        return NoReplacePublicationResult{
            .status = NoReplacePublicationStatus::success,
            .detail = {},
        };
    }
    if (copy_error == std::errc::file_exists) {
        return failure(
            NoReplacePublicationStatus::destination_exists,
            "Destination already exists; no replacement was performed.");
    }

    // Both ways failed, so both reasons are reported: the link error alone
    // sends anyone reading it after a permission that was never the problem.
    return failure(
        NoReplacePublicationStatus::publication_failed,
        "Final no-replace publication failed. Hard link: " +
            link_error.message() + ". Exclusive create: " +
            copy_error.message() + ".");
}

NoReplacePublicationResult publish_bytes_no_replace(
    const std::filesystem::path& destination,
    std::span<const std::byte> bytes,
    const StagedFileValidator& validator,
    std::string_view staging_suffix) {
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
    if (!stream) {
        stream.close();
        return failure(
            NoReplacePublicationStatus::staging_write_failed,
            "Unable to flush the complete staging payload.");
    }
    stream.close();
    if (!stream) {
        return failure(
            NoReplacePublicationStatus::staging_write_failed,
            "Unable to close the complete staging payload.");
    }

    if (validator && !validator(staged_file)) {
        return failure(
            NoReplacePublicationStatus::staging_validation_failed,
            "The complete staged artifact failed pre-publication validation.");
    }

    return publish_validated_file_no_replace(staged_file, destination);
}

} // namespace dmc::rengine::core
