#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::profiles::dmc3 {

struct ProtectedExecutionPreflightDiagnostic final {
    std::string code;
    std::string message;
};

class ProtectedExecutionPreflightSnapshot final {
public:
    [[nodiscard]] const std::filesystem::path& executable_directory() const noexcept {
        return executable_directory_;
    }

    [[nodiscard]] const std::filesystem::path& executable_path() const noexcept {
        return executable_path_;
    }

    [[nodiscard]] const std::string& executable_sha256() const noexcept {
        return executable_sha256_;
    }

    [[nodiscard]] std::uint64_t executable_size() const noexcept {
        return executable_size_;
    }

    [[nodiscard]] const std::string& execution_authority_id() const noexcept {
        return execution_authority_id_;
    }

    [[nodiscard]] const std::filesystem::path& data_directory() const noexcept {
        return data_directory_;
    }

    [[nodiscard]] const std::filesystem::path& bootstrap_volume_path() const noexcept {
        return bootstrap_volume_path_;
    }

    [[nodiscard]] const std::string& bootstrap_volume_logical_path() const noexcept {
        return bootstrap_volume_logical_path_;
    }

    // Presence/co-location is deliberately weaker than an EXE<->NBZ pairing
    // receipt. A preflight snapshot can never claim archive pairing.
    [[nodiscard]] bool archive_pairing_verified() const noexcept {
        return false;
    }

private:
    friend class ProtectedExecutionPreflightBinder;

    ProtectedExecutionPreflightSnapshot(
        std::filesystem::path executable_directory,
        std::filesystem::path executable_path,
        std::string executable_sha256,
        std::uint64_t executable_size,
        std::string execution_authority_id,
        std::filesystem::path data_directory,
        std::filesystem::path bootstrap_volume_path,
        std::string bootstrap_volume_logical_path)
        : executable_directory_(std::move(executable_directory)),
          executable_path_(std::move(executable_path)),
          executable_sha256_(std::move(executable_sha256)),
          executable_size_(executable_size),
          execution_authority_id_(std::move(execution_authority_id)),
          data_directory_(std::move(data_directory)),
          bootstrap_volume_path_(std::move(bootstrap_volume_path)),
          bootstrap_volume_logical_path_(std::move(bootstrap_volume_logical_path)) {}

    std::filesystem::path executable_directory_;
    std::filesystem::path executable_path_;
    std::string executable_sha256_;
    std::uint64_t executable_size_{};
    std::string execution_authority_id_;
    std::filesystem::path data_directory_;
    std::filesystem::path bootstrap_volume_path_;
    std::string bootstrap_volume_logical_path_;
};

struct ProtectedExecutionPreflightResult final {
    std::optional<ProtectedExecutionPreflightSnapshot> snapshot;
    std::vector<ProtectedExecutionPreflightDiagnostic> diagnostics;

    [[nodiscard]] bool ready() const noexcept {
        return snapshot.has_value() && diagnostics.empty();
    }
};

class ProtectedExecutionPreflightBinder final {
public:
    // Observes the current local protected-distribution candidate through
    // GDSpaces and binds an immutable preflight snapshot only when the exact
    // registered execution authority and bootstrap-volume presence both pass.
    //
    // This is a pre-launch observation, not proof that a later process executed
    // the same bytes. A future launcher must revalidate at the launch boundary.
    [[nodiscard]] static ProtectedExecutionPreflightResult observe(
        const std::filesystem::path& executable_directory);
};

} // namespace dmc::rengine::profiles::dmc3
