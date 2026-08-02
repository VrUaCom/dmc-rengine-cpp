#include "dmc_rengine/gdspaces/local_directory_source.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
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
    ResourcePayload payload;
    payload.resource = ResourceRef{
        .id = resource,
        .display_name = std::filesystem::path(resource.logical_path).filename().string(),
        .format = classify(candidate),
        .profile = "unknown",
        .synthetic_name = false,
        .container = is_container_format(classify(candidate)),
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

    const auto format = classify(path);
    return ResourceRef{
        .id = ResourceId{
            .source_id = source_id_,
            .logical_path = relative.generic_string(),
            .container_chain = {},
            .offset = 0,
            .size = size,
        },
        .display_name = path.filename().string(),
        .format = format,
        .profile = "unknown",
        .synthetic_name = false,
        .container = is_container_format(format),
    };
}

bool LocalDirectorySource::contains(const std::filesystem::path& path) const {
    return path_has_prefix(root_, normalized_path(path));
}

std::string LocalDirectorySource::classify(
    const std::filesystem::path& path) {
    auto extension = path.extension().string();
    if (!extension.empty() && extension.front() == '.') {
        extension.erase(extension.begin());
    }

    std::transform(
        extension.begin(), extension.end(), extension.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

    return extension.empty() ? "unknown" : extension;
}

bool LocalDirectorySource::is_container_format(std::string_view format) {
    return format == "nbz" || format == "afs" || format == "pac" ||
           format == "pnst";
}

} // namespace dmc::rengine::gdspaces
