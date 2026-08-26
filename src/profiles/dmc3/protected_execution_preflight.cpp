#include "dmc_rengine/profiles/dmc3/protected_execution_preflight.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::profiles::dmc3 {
namespace {

namespace gdspaces = dmc::rengine::gdspaces;

void add_error(
    ProtectedExecutionPreflightResult& result,
    std::string code,
    std::string message) {
    result.diagnostics.push_back(ProtectedExecutionPreflightDiagnostic{
        .code = std::move(code),
        .message = std::move(message),
    });
}

[[nodiscard]] const gdspaces::ResourceRef* find_bootstrap_volume(
    const gdspaces::LocalDirectorySource& source,
    gdspaces::ResourceRef& storage) {
    for (const std::string_view candidate : {
             std::string_view{"dmc3-0.nbz"},
             std::string_view{"DMC3-0.nbz"}}) {
        const auto lookup = source.lookup_direct_path(candidate);
        if (!lookup.valid()) {
            continue;
        }
        if (lookup.resolved()) {
            storage = *lookup.resource;
            return &storage;
        }
    }
    return nullptr;
}

} // namespace

ProtectedExecutionPreflightResult ProtectedExecutionPreflightBinder::observe(
    const std::filesystem::path& executable_directory) {
    ProtectedExecutionPreflightResult result;

    std::error_code error;
    const auto absolute_directory =
        std::filesystem::absolute(executable_directory, error);
    if (error ||
        !std::filesystem::is_directory(absolute_directory, error) || error) {
        add_error(
            result,
            "dmc3.protected-execution.root",
            "The supplied executable directory is not a readable directory.");
        return result;
    }

    constexpr std::string_view executable_source_id =
        "dmc3-protected-execution-preflight-exe";
    gdspaces::SourceRegistry executable_registry;
    auto executable_source = std::make_unique<gdspaces::LocalDirectorySource>(
        std::string{executable_source_id}, absolute_directory, false);
    auto* executable_source_view = executable_source.get();
    if (!executable_registry.mount(std::move(executable_source))) {
        add_error(
            result,
            "dmc3.protected-execution.exe-mount",
            "GDSpaces could not mount the executable directory.");
        return result;
    }

    const auto executable_lookup =
        executable_source_view->lookup_direct_path("dmc3.exe");
    if (!executable_lookup.valid() || !executable_lookup.resolved()) {
        add_error(
            result,
            "dmc3.protected-execution.exe-not-found",
            "GDSpaces could not resolve executable-relative dmc3.exe.");
        return result;
    }

    const auto executable_payload =
        executable_registry.read(executable_lookup.resource->id);
    if (!executable_payload.has_value() || !executable_payload->readable() ||
        executable_payload->bytes.size() != executable_lookup.resource->id.size) {
        add_error(
            result,
            "dmc3.protected-execution.exe-read",
            "GDSpaces could not materialize the exact dmc3.exe resource bytes.");
        return result;
    }

    const auto executable_size = static_cast<std::uint64_t>(
        executable_payload->bytes.size());
    const auto digest = core::Sha256::compute(
        std::span<const std::byte>{executable_payload->bytes});
    const auto digest_hex = digest.hex();
    const auto match = classify_executable_authority(
        digest_hex, executable_size);
    if (!match.recognized() || match.authority == nullptr) {
        add_error(
            result,
            "dmc3.protected-execution.unknown-authority",
            "The observed dmc3.exe is not an exact registered executable authority.");
        return result;
    }

    const auto& authority = *match.authority;
    if (!authority.valid() ||
        authority.role != ExecutableAuthorityRole::protected_distribution ||
        authority.instruction_reverse_authority ||
        !authority.distribution_provenance_authority ||
        !authority.original_execution_candidate) {
        add_error(
            result,
            "dmc3.protected-execution.wrong-authority-role",
            "The observed executable is recognized but is not the protected-distribution original-execution authority.");
        return result;
    }

    const auto data_directory = absolute_directory / "data" / "dmc3";
    error.clear();
    if (!std::filesystem::is_directory(data_directory, error) || error) {
        add_error(
            result,
            "dmc3.protected-execution.data-root",
            "The protected executable is recognized, but executable-relative data/dmc3 is missing.");
        return result;
    }

    constexpr std::string_view data_source_id =
        "dmc3-protected-execution-preflight-data";
    gdspaces::SourceRegistry data_registry;
    auto data_source = std::make_unique<gdspaces::LocalDirectorySource>(
        std::string{data_source_id}, data_directory, false);
    auto* data_source_view = data_source.get();
    if (!data_registry.mount(std::move(data_source))) {
        add_error(
            result,
            "dmc3.protected-execution.data-mount",
            "GDSpaces could not mount executable-relative data/dmc3.");
        return result;
    }

    gdspaces::ResourceRef bootstrap_storage;
    const auto* bootstrap = find_bootstrap_volume(
        *data_source_view, bootstrap_storage);
    if (bootstrap == nullptr) {
        add_error(
            result,
            "dmc3.protected-execution.bootstrap-missing",
            "Contiguous DMC3 archive bootstrap cannot start because DMC3-0.nbz is missing.");
        return result;
    }

    const auto executable_path = absolute_directory /
        std::filesystem::path{executable_lookup.resource->id.logical_path};
    const auto bootstrap_path = data_directory /
        std::filesystem::path{bootstrap->id.logical_path};

    result.snapshot.emplace(
        absolute_directory,
        executable_path,
        digest_hex,
        executable_size,
        authority.id,
        data_directory,
        bootstrap_path,
        bootstrap->id.logical_path);
    return result;
}

} // namespace dmc::rengine::profiles::dmc3
