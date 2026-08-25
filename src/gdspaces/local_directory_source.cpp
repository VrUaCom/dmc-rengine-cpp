#include "dmc_rengine/gdspaces/local_directory_source.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <span>
#include <system_error>
#include <utility>

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] std::filesystem::path normalized_path(
    const std::filesystem::path& path) {
    std::error_code error;
    auto normalized = std::filesystem::weakly_canonical(path, error);
    if (!error) {
        return normalized;
    }

    error.clear();
    normalized = std::filesystem::absolute(path, error);
    return error ? path.lexically_normal() : normalized.lexically_normal();
}

[[nodiscard]] std::filesystem::path native_relative_path(
    std::string_view logical_path) {
    std::string native{logical_path};
#ifndef _WIN32
    std::replace(native.begin(), native.end(), '\\', '/');
#endif
    return std::filesystem::path{native};
}

[[nodiscard]] bool path_has_prefix(
    const std::filesystem::path& prefix,
    const std::filesystem::path& value) {
    auto prefix_iterator = prefix.begin();
    auto value_iterator = value.begin();

    for (; prefix_iterator != prefix.end(); ++prefix_iterator, ++value_iterator) {
        if (value_iterator == value.end() || *prefix_iterator != *value_iterator) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool ordinary_missing_error(const std::error_code& error) noexcept {
    return error == std::errc::no_such_file_or_directory ||
        error == std::errc::not_a_directory;
}

} // namespace

LocalDirectorySource::LocalDirectorySource(
    std::string source_id,
    std::filesystem::path root,
    bool recursive)
    : source_id_(std::move(source_id)),
      root_(normalized_path(root)),
      recursive_(recursive) {}

std::string_view LocalDirectorySource::id() const noexcept {
    return source_id_;
}

std::string_view LocalDirectorySource::kind() const noexcept {
    return "local-directory";
}

std::vector<ResourceRef> LocalDirectorySource::enumerate() const {
    std::vector<ResourceRef> resources;
    std::error_code error;

    if (!std::filesystem::is_directory(root_, error) || error) {
        return resources;
    }

    const auto append_file = [this, &resources](
                                 const std::filesystem::directory_entry& entry) {
        std::error_code entry_error;
        if (!entry.is_regular_file(entry_error) || entry_error) {
            return;
        }

        const auto raw_size = entry.file_size(entry_error);
        if (entry_error || raw_size > std::numeric_limits<std::uint64_t>::max()) {
            return;
        }

        resources.push_back(describe(
            entry.path(), static_cast<std::uint64_t>(raw_size)));
    };

    if (recursive_) {
        std::filesystem::recursive_directory_iterator iterator(
            root_, std::filesystem::directory_options::skip_permission_denied, error);
        const std::filesystem::recursive_directory_iterator end;
        while (!error && iterator != end) {
            append_file(*iterator);
            iterator.increment(error);
        }
    } else {
        std::filesystem::directory_iterator iterator(
            root_, std::filesystem::directory_options::skip_permission_denied, error);
        const std::filesystem::directory_iterator end;
        while (!error && iterator != end) {
            append_file(*iterator);
            iterator.increment(error);
        }
    }

    std::sort(
        resources.begin(), resources.end(),
        [](const ResourceRef& left, const ResourceRef& right) {
            return left.id.logical_path < right.id.logical_path;
        });

    return resources;
}

std::optional<ResourcePayload> LocalDirectorySource::read(
    const ResourceId& resource) const {
    if (resource.source_id != source_id_ || !resource.valid()) {
        return std::nullopt;
    }

    const auto candidate = normalized_path(root_ / resource.logical_path);
    const auto initial_classification = ResourceClassifier::classify(
        resource.logical_path);

    ResourcePayload payload;
    payload.resource = ResourceRef{
        .id = resource,
        .display_name = std::filesystem::path(resource.logical_path).filename().string(),
        .format = initial_classification.format,
        .profile = std::string(to_string(initial_classification.profile)),
        .synthetic_name = false,
        .container = initial_classification.container,
    };

    if (!contains(candidate)) {
        payload.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::error,
            .code = "gdspaces.path_outside_root",
            .message = "The requested resource resolves outside the mounted root.",
            .resource = resource,
        });
        return payload;
    }

    std::error_code error;
    const auto raw_size = std::filesystem::file_size(candidate, error);
    if (error || raw_size > std::numeric_limits<std::size_t>::max()) {
        payload.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::error,
            .code = "gdspaces.file_size_failed",
            .message = "Unable to determine a safe resource size.",
            .resource = resource,
        });
        return payload;
    }

    std::ifstream stream(candidate, std::ios::binary);
    if (!stream) {
        payload.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::error,
            .code = "gdspaces.open_failed",
            .message = "Unable to open the resource for reading.",
            .resource = resource,
        });
        return payload;
    }

    payload.bytes.resize(static_cast<std::size_t>(raw_size));
    if (!payload.bytes.empty()) {
        if (payload.bytes.size() >
            static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
            payload.diagnostics.push_back(Diagnostic{
                .severity = DiagnosticSeverity::error,
                .code = "gdspaces.resource_too_large",
                .message = "The resource is too large for the current stream API.",
                .resource = resource,
            });
            payload.bytes.clear();
            return payload;
        }

        stream.read(
            reinterpret_cast<char*>(payload.bytes.data()),
            static_cast<std::streamsize>(payload.bytes.size()));
        if (!stream) {
            payload.diagnostics.push_back(Diagnostic{
                .severity = DiagnosticSeverity::error,
                .code = "gdspaces.read_failed",
                .message = "The resource could not be read completely.",
                .resource = resource,
            });
            payload.bytes.clear();
            return payload;
        }
    }

    const auto classification = ResourceClassifier::classify(
        resource.logical_path,
        std::span<const std::byte>{payload.bytes});
    payload.resource.format = classification.format;
    payload.resource.profile = std::string(to_string(classification.profile));
    payload.resource.container = classification.container;
    payload.byte_provenance = ByteProvenance{
        .kind = ByteOriginKind::direct_source_span,
        .authority_id = resource.canonical(),
        .offset = 0U,
        .stored_size = static_cast<std::uint64_t>(raw_size),
        .materialized_size = static_cast<std::uint64_t>(raw_size),
        .transform = ByteTransform::none,
        .crc32 = std::nullopt,
    };

    if (resource.size != 0U && resource.size != raw_size) {
        payload.diagnostics.push_back(Diagnostic{
            .severity = DiagnosticSeverity::warning,
            .code = "gdspaces.size_changed",
            .message = "The resource size changed after enumeration.",
            .resource = resource,
        });
    }

    return payload;
}

DirectPathLookupResult LocalDirectorySource::lookup_direct_path(
    std::string_view logical_path) const {
    if (logical_path.empty() || logical_path.find('\0') != std::string_view::npos) {
        return DirectPathLookupResult{
            .status = DirectPathLookupStatus::rejected,
            .resource = std::nullopt,
            .detail = "Direct path is empty or not C-string-compatible.",
        };
    }

    const auto relative = native_relative_path(logical_path);
    if (relative.empty() || relative.is_absolute()) {
        return DirectPathLookupResult{
            .status = DirectPathLookupStatus::rejected,
            .resource = std::nullopt,
            .detail = "Direct path must remain relative to the mounted source root.",
        };
    }

    const auto candidate = normalized_path(root_ / relative);
    if (!contains(candidate)) {
        return DirectPathLookupResult{
            .status = DirectPathLookupStatus::rejected,
            .resource = std::nullopt,
            .detail = "Direct path resolves outside the mounted source root.",
        };
    }

    std::error_code error;
    const auto status = std::filesystem::status(candidate, error);
    if (error) {
        if (ordinary_missing_error(error)) {
            return DirectPathLookupResult{
                .status = DirectPathLookupStatus::not_found,
                .resource = std::nullopt,
                .detail = {},
            };
        }
        return DirectPathLookupResult{
            .status = DirectPathLookupStatus::io_error,
            .resource = std::nullopt,
            .detail = "Native path status lookup failed: " + error.message(),
        };
    }
    if (!std::filesystem::exists(status) || !std::filesystem::is_regular_file(status)) {
        return DirectPathLookupResult{
            .status = DirectPathLookupStatus::not_found,
            .resource = std::nullopt,
            .detail = {},
        };
    }

    const auto resources = enumerate();
    for (const auto& resource : resources) {
        const auto enumerated_path = normalized_path(
            root_ / native_relative_path(resource.id.logical_path));
        std::error_code equivalent_error;
        const auto equivalent = std::filesystem::equivalent(
            candidate, enumerated_path, equivalent_error);
        if (!equivalent_error && equivalent) {
            return DirectPathLookupResult{
                .status = DirectPathLookupStatus::resolved,
                .resource = resource,
                .detail = {},
            };
        }
    }

    return DirectPathLookupResult{
        .status = DirectPathLookupStatus::io_error,
        .resource = std::nullopt,
        .detail = "Native path exists but no canonical ResourceRef identity was recovered from this source enumeration.",
    };
}

const std::filesystem::path& LocalDirectorySource::root() const noexcept {
    return root_;
}

bool LocalDirectorySource::recursive() const noexcept {
    return recursive_;
}

ResourceRef LocalDirectorySource::describe(
    const std::filesystem::path& path,
    std::uint64_t size) const {
    std::error_code error;
    auto relative = std::filesystem::relative(path, root_, error);
    if (error) {
        relative = path.filename();
    }

    const auto logical_path = relative.generic_string();
    const auto classification = ResourceClassifier::classify(logical_path);
    return ResourceRef{
        .id = ResourceId{
            .source_id = source_id_,
            .logical_path = logical_path,
            .container_chain = {},
            .offset = 0,
            .size = size,
        },
        .display_name = path.filename().string(),
        .format = classification.format,
        .profile = std::string(to_string(classification.profile)),
        .synthetic_name = false,
        .container = initial_classification.container,
    };
}

bool LocalDirectorySource::contains(const std::filesystem::path& path) const {
    return path_has_prefix(root_, normalized_path(path));
}

} // namespace dmc::rengine::gdspaces
