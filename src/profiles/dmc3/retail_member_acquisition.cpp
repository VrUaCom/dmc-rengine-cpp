#include "dmc_rengine/profiles/dmc3/retail_member_acquisition.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_member.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <limits>
#include <memory>
#include <span>
#include <sstream>
#include <system_error>

namespace dmc::rengine::profiles::dmc3 {
namespace {

namespace core = dmc::rengine::core;
namespace evidence = dmc::rengine::evidence;
namespace gdspaces = dmc::rengine::gdspaces;

struct DiscoveredVolume final {
    std::uint32_t index{};
    std::filesystem::path path;
};

struct MountedArchive final {
    std::uint32_t index{};
    std::string source_id;
    std::filesystem::path path;
    const gdspaces::NbzZipSource* source{};
};

[[nodiscard]] std::string lower_ascii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

[[nodiscard]] std::optional<std::uint32_t> numbered_volume_index(
    const std::filesystem::path& path) {
    const auto name = lower_ascii(path.filename().string());
    constexpr std::string_view prefix = "dmc3-";
    constexpr std::string_view suffix = ".nbz";
    if (name.size() <= prefix.size() + suffix.size() ||
        !name.starts_with(prefix) || !name.ends_with(suffix)) {
        return std::nullopt;
    }

    const auto digits = std::string_view{name}.substr(
        prefix.size(), name.size() - prefix.size() - suffix.size());
    std::uint64_t parsed{};
    const auto* first = digits.data();
    const auto* last = digits.data() + digits.size();
    const auto result = std::from_chars(first, last, parsed, 10);
    if (digits.empty() || result.ec != std::errc{} || result.ptr != last ||
        parsed > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(parsed);
}

[[nodiscard]] std::optional<std::vector<DiscoveredVolume>> discover_volumes(
    const std::filesystem::path& data_directory) {
    std::error_code error;
    if (!std::filesystem::is_directory(data_directory, error) || error) {
        return std::nullopt;
    }

    std::vector<DiscoveredVolume> volumes;
    for (std::filesystem::directory_iterator it{data_directory, error}, end;
         !error && it != end; it.increment(error)) {
        error.clear();
        if (!it->is_regular_file(error) || error) {
            error.clear();
            continue;
        }
        if (const auto index = numbered_volume_index(it->path()); index.has_value()) {
            volumes.push_back(DiscoveredVolume{*index, it->path()});
        }
    }
    if (error) {
        return std::nullopt;
    }

    std::sort(
        volumes.begin(), volumes.end(),
        [](const DiscoveredVolume& left, const DiscoveredVolume& right) {
            if (left.index != right.index) {
                return left.index < right.index;
            }
            return left.path.generic_string() < right.path.generic_string();
        });
    // Two files claiming one runtime index is ambiguity, not a preference to
    // resolve here.
    for (std::size_t index = 1U; index < volumes.size(); ++index) {
        if (volumes[index - 1U].index == volumes[index].index) {
            return std::nullopt;
        }
    }
    return volumes;
}

[[nodiscard]] const DiscoveredVolume* find_volume(
    const std::vector<DiscoveredVolume>& volumes,
    std::uint32_t index) noexcept {
    const auto found = std::find_if(
        volumes.begin(), volumes.end(),
        [index](const DiscoveredVolume& volume) { return volume.index == index; });
    return found == volumes.end() ? nullptr : &*found;
}

[[nodiscard]] std::optional<core::Sha256Digest> sha256_file_exact(
    const std::filesystem::path& path,
    std::uint64_t expected_size) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return std::nullopt;
    }

    core::Sha256Accumulator accumulator;
    std::vector<char> buffer(1024U * 1024U);
    std::uint64_t consumed = 0U;
    while (consumed < expected_size) {
        const auto amount = static_cast<std::size_t>(
            std::min<std::uint64_t>(expected_size - consumed, buffer.size()));
        stream.read(buffer.data(), static_cast<std::streamsize>(amount));
        if (stream.gcount() != static_cast<std::streamsize>(amount)) {
            return std::nullopt;
        }
        if (!accumulator.update(std::as_bytes(std::span<const char>{
                buffer.data(), amount}))) {
            return std::nullopt;
        }
        consumed += static_cast<std::uint64_t>(amount);
    }

    char extra{};
    stream.read(&extra, 1);
    if (stream.gcount() != 0) {
        return std::nullopt;
    }
    return accumulator.finalize();
}

[[nodiscard]] const gdspaces::NbzZipEntry* find_entry(
    const gdspaces::NbzZipSource& source,
    std::string_view logical_path) noexcept {
    const auto& entries = source.entries();
    const auto found = std::find_if(
        entries.begin(), entries.end(),
        [logical_path](const gdspaces::NbzZipEntry& entry) {
            return entry.logical_path == logical_path;
        });
    return found == entries.end() ? nullptr : &*found;
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size() + 8U);
    for (const unsigned char character : value) {
        switch (character) {
        case '"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
            if (character < 0x20U) {
                static constexpr char digits[] = "0123456789abcdef";
                escaped += "\\u00";
                escaped += digits[(character >> 4U) & 0x0FU];
                escaped += digits[character & 0x0FU];
            } else {
                escaped += static_cast<char>(character);
            }
        }
    }
    return escaped;
}

[[nodiscard]] RetailAcquisition failure(
    RetailAcquisitionStatus status,
    std::string detail) {
    return RetailAcquisition{
        .status = status,
        .detail = std::move(detail),
        .receipt = std::nullopt,
        .bytes = {},
    };
}

} // namespace

RetailAcquisition RetailMemberAcquisition::acquire(
    const std::filesystem::path& executable_directory,
    std::string_view game_request) {
    const auto data_directory = executable_directory /
        std::filesystem::path{VolumeBootstrapPolicy::data_subdirectory()};

    const auto discovered = discover_volumes(data_directory);
    if (!discovered.has_value()) {
        return failure(
            RetailAcquisitionStatus::volume_scan_ambiguous,
            "Cannot uniquely scan executable-relative DMC3 volumes.");
    }

    std::vector<std::uint32_t> present_indices;
    present_indices.reserve(discovered->size());
    for (const auto& volume : *discovered) {
        present_indices.push_back(volume.index);
    }

    const auto bootstrap = VolumeBootstrapPolicy::plan(present_indices);
    if (!bootstrap.valid() || bootstrap.registered_archives.empty()) {
        return failure(
            RetailAcquisitionStatus::no_contiguous_bootstrap,
            "No valid contiguous runtime DMC3 archive bootstrap.");
    }

    gdspaces::SourceRegistry registry;
    constexpr std::string_view physical_source_id = "dmc3-retail-physical";
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{physical_source_id}, data_directory, false))) {
        return failure(
            RetailAcquisitionStatus::physical_mount_failed,
            "Cannot mount the physical data directory.");
    }

    std::vector<MountedArchive> mounted_archives;
    mounted_archives.reserve(bootstrap.registered_archives.size());
    for (const auto& archive : bootstrap.registered_archives) {
        const auto* physical = find_volume(*discovered, archive.index);
        if (physical == nullptr) {
            return failure(
                RetailAcquisitionStatus::archive_mount_failed,
                "Runtime bootstrap references an undiscovered archive.");
        }

        auto source_id =
            std::string{"dmc3-retail-volume-"} + std::to_string(archive.index);
        auto source =
            std::make_unique<gdspaces::NbzZipSource>(source_id, physical->path);
        if (!source->valid()) {
            return failure(
                RetailAcquisitionStatus::archive_mount_failed,
                "Invalid runtime-mounted NBZ/ZIP volume: " +
                    physical->path.string());
        }
        const auto* source_pointer = source.get();
        if (!registry.mount(std::move(source))) {
            return failure(
                RetailAcquisitionStatus::archive_mount_failed,
                "Cannot mount the NBZ source.");
        }
        mounted_archives.push_back(MountedArchive{
            .index = archive.index,
            .source_id = std::move(source_id),
            .path = physical->path,
            .source = source_pointer,
        });
    }

    RuntimeSourceBindings bindings;
    bindings.physical_source_id = std::string{physical_source_id};
    for (const auto volume_index : bootstrap.archive_resolution_order) {
        const auto iterator = std::find_if(
            mounted_archives.begin(), mounted_archives.end(),
            [volume_index](const MountedArchive& archive) {
                return archive.index == volume_index;
            });
        if (iterator == mounted_archives.end()) {
            return failure(
                RetailAcquisitionStatus::invalid_source_bindings,
                "Incomplete archive source binding.");
        }
        bindings.archives.push_back(ArchiveSourceBinding{
            .volume_index = volume_index,
            .source_id = iterator->source_id,
        });
    }
    if (!bindings.valid_for(bootstrap)) {
        return failure(
            RetailAcquisitionStatus::invalid_source_bindings,
            "Invalid runtime source bindings.");
    }

    const auto resolved = RuntimeResourceResolver::resolve(
        game_request, bootstrap, bindings, registry);
    if (!resolved.ok()) {
        std::string detail{"The resolver did not produce one resource: "};
        detail += to_string(resolved.status);
        if (!resolved.detail.empty()) {
            detail += ": ";
            detail += resolved.detail;
        }
        return failure(RetailAcquisitionStatus::unresolved_request, std::move(detail));
    }

    const auto archive_iterator = std::find_if(
        mounted_archives.begin(), mounted_archives.end(),
        [&](const MountedArchive& archive) {
            return archive.source_id == resolved.resolved->id.source_id;
        });
    if (archive_iterator == mounted_archives.end()) {
        return failure(
            RetailAcquisitionStatus::resolved_outside_archive,
            "The request resolved to the physical provider; archive provenance is required.");
    }

    const auto* entry =
        find_entry(*archive_iterator->source, resolved.resolved->id.logical_path);
    if (entry == nullptr || !archive_iterator->source->index_receipt().has_value()) {
        return failure(
            RetailAcquisitionStatus::member_metadata_unavailable,
            "Selected archive member metadata is unavailable.");
    }

    const auto archive_size = archive_iterator->source->index_receipt()->archive_size;
    const auto first_digest = sha256_file_exact(archive_iterator->path, archive_size);
    if (!first_digest.has_value()) {
        return failure(
            RetailAcquisitionStatus::archive_identity_unstable,
            "Cannot establish the selected archive identity.");
    }

    const evidence::ArtifactIdentity artifact{
        .id = archive_iterator->source_id,
        .role = "dmc3-retail-nbz",
        .sha256 = first_digest->hex(),
        .size = archive_size,
    };
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        *archive_iterator->source, artifact);
    if (!bound.ok()) {
        return failure(
            RetailAcquisitionStatus::archive_identity_unstable,
            "The archive changed or could not be bound to its observed identity.");
    }

    const auto observed = gdspaces::NbzZipArtifactMemberObserver::observe(
        *archive_iterator->source, *bound.snapshot, entry->central_index);
    if (!observed.ok()) {
        return failure(
            RetailAcquisitionStatus::member_observation_failed,
            "The selected member could not be materialized from the artifact-bound archive.");
    }

    const auto bytes = observed.observation->materialized_bytes();
    const auto resource_digest = core::Sha256::compute(bytes);

    RetailAcquisition result;
    result.status = RetailAcquisitionStatus::acquired;
    result.bytes.assign(bytes.begin(), bytes.end());
    result.receipt = RetailAcquisitionReceipt{
        .game_request = std::string{game_request},
        .selected_volume_index = archive_iterator->index,
        .resolver_probe_count = resolved.probes.size(),
        .first_missing_index = bootstrap.first_missing_index,
        .ignored_after_first_gap = bootstrap.present_after_first_gap.size(),
        .ignored_outside_runtime_domain =
            bootstrap.present_outside_runtime_index_domain.size(),
        .archive_path = archive_iterator->path,
        .archive_size = artifact.size,
        .archive_sha256 = artifact.sha256,
        .entry = observed.observation->entry(),
        .materialized_size = bytes.size(),
        .materialized_sha256 = resource_digest.hex(),
        .provenance = observed.observation->byte_provenance(),
    };
    return result;
}

std::string RetailMemberAcquisition::receipt_json(
    const RetailAcquisitionReceipt& receipt,
    const std::filesystem::path& output_file) {
    std::ostringstream document;
    document
        << "{\n"
        << "  \"schema_version\": 2,\n"
        << "  \"evidence_class\": \"artifact-bound-retail-member-acquisition\",\n"
        << "  \"game_request\": \"" << escape_json(receipt.game_request) << "\",\n"
        << "  \"resolver_status\": \"resolved\",\n"
        << "  \"resolver_probe_count\": " << receipt.resolver_probe_count << ",\n"
        << "  \"selected_volume_index\": " << receipt.selected_volume_index << ",\n"
        << "  \"bootstrap\": {\n"
        << "    \"first_missing_index\": " << receipt.first_missing_index << ",\n"
        << "    \"ignored_after_first_gap_count\": "
        << receipt.ignored_after_first_gap << ",\n"
        << "    \"ignored_outside_runtime_domain_count\": "
        << receipt.ignored_outside_runtime_domain << "\n"
        << "  },\n"
        << "  \"archive\": {\n"
        << "    \"path\": \"" << escape_json(receipt.archive_path.generic_string())
        << "\",\n"
        << "    \"size\": " << receipt.archive_size << ",\n"
        << "    \"sha256\": \"" << receipt.archive_sha256 << "\"\n"
        << "  },\n"
        << "  \"member\": {\n"
        << "    \"logical_path\": \"" << escape_json(receipt.entry.logical_path)
        << "\",\n"
        << "    \"central_index\": " << receipt.entry.central_index << ",\n"
        << "    \"compression_method\": " << receipt.entry.compression_method << ",\n"
        << "    \"crc32\": " << receipt.entry.crc32 << ",\n"
        << "    \"compressed_size\": " << receipt.entry.compressed_size << ",\n"
        << "    \"uncompressed_size\": " << receipt.entry.uncompressed_size << ",\n"
        << "    \"local_header_offset\": " << receipt.entry.local_header_offset << ",\n"
        << "    \"data_offset\": " << receipt.entry.data_offset << "\n"
        << "  },\n"
        << "  \"materialized_resource\": {\n"
        << "    \"size\": " << receipt.materialized_size << ",\n"
        << "    \"sha256\": \"" << receipt.materialized_sha256 << "\",\n"
        << "    \"origin_kind\": \"" << gdspaces::to_string(receipt.provenance.kind)
        << "\",\n"
        << "    \"transform\": \"" << gdspaces::to_string(receipt.provenance.transform)
        << "\",\n"
        << "    \"source_offset\": " << receipt.provenance.offset << ",\n"
        << "    \"stored_size\": " << receipt.provenance.stored_size << ",\n"
        << "    \"materialized_size\": " << receipt.provenance.materialized_size;
    if (receipt.provenance.crc32.has_value()) {
        document << ",\n    \"provenance_crc32\": " << *receipt.provenance.crc32
                 << '\n';
    } else {
        document << '\n';
    }
    document
        << "  },\n"
        << "  \"output_file\": \"" << escape_json(output_file.generic_string())
        << "\",\n"
        << "  \"publication\": \"atomic-no-replace-per-artifact\"\n"
        << "}\n";
    return document.str();
}

} // namespace dmc::rengine::profiles::dmc3
