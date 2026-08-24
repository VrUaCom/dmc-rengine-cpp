#include "dmc3_retail_acquisition_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_member.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace {

namespace core = dmc::rengine::core;
namespace evidence = dmc::rengine::evidence;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

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
    const auto iterator = std::lower_bound(
        volumes.begin(), volumes.end(), index,
        [](const DiscoveredVolume& volume, std::uint32_t candidate) {
            return volume.index < candidate;
        });
    return iterator == volumes.end() || iterator->index != index
        ? nullptr
        : &*iterator;
}

[[nodiscard]] std::filesystem::path normalized_absolute(
    const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    if (error) {
        return path.lexically_normal();
    }
    auto normalized = std::filesystem::weakly_canonical(absolute, error);
    if (error) {
        return absolute.lexically_normal();
    }
    return normalized;
}

[[nodiscard]] bool path_component_equal(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#ifdef _WIN32
    return lower_ascii(left.generic_string()) == lower_ascii(right.generic_string());
#else
    return left == right;
#endif
}

[[nodiscard]] bool is_within(
    const std::filesystem::path& child,
    const std::filesystem::path& parent) {
    const auto child_normalized = normalized_absolute(child);
    const auto parent_normalized = normalized_absolute(parent);
    auto child_it = child_normalized.begin();
    auto parent_it = parent_normalized.begin();
    for (; parent_it != parent_normalized.end(); ++parent_it, ++child_it) {
        if (child_it == child_normalized.end() ||
            !path_component_equal(*child_it, *parent_it)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::optional<core::Sha256Digest> sha256_file(
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
        const auto remaining = expected_size - consumed;
        const auto amount = static_cast<std::size_t>(
            std::min<std::uint64_t>(remaining, buffer.size()));
        stream.read(buffer.data(), static_cast<std::streamsize>(amount));
        if (stream.gcount() != static_cast<std::streamsize>(amount)) {
            return std::nullopt;
        }
        const auto bytes = std::as_bytes(std::span<const char>{
            buffer.data(), amount});
        if (!accumulator.update(bytes)) {
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

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] std::filesystem::path receipt_path_for(
    const std::filesystem::path& output_file) {
    return std::filesystem::path{output_file.string() + ".receipt.json"};
}

[[nodiscard]] const gdspaces::NbzZipEntry* find_entry(
    const gdspaces::NbzZipSource& source,
    std::string_view logical_path) noexcept {
    const auto& entries = source.entries();
    const auto iterator = std::find_if(
        entries.begin(), entries.end(),
        [logical_path](const gdspaces::NbzZipEntry& entry) {
            return !entry.directory && entry.logical_path == logical_path;
        });
    return iterator == entries.end() ? nullptr : &*iterator;
}

} // namespace

int run_extract_dmc3_retail_member(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    const std::filesystem::path& output_file) {
    const auto data_directory = executable_directory /
        std::filesystem::path{dmc3::VolumeBootstrapPolicy::data_subdirectory()};
    const auto receipt_path = receipt_path_for(output_file);

    if (output_file.empty() || output_file.filename().empty() ||
        is_within(output_file, data_directory) ||
        is_within(receipt_path, data_directory)) {
        std::cerr
            << "extract-dmc3-retail-member: output and receipt must be outside the retail game data tree\n";
        return 2;
    }

    const auto discovered = discover_volumes(data_directory);
    if (!discovered.has_value()) {
        std::cerr
            << "extract-dmc3-retail-member: cannot uniquely scan executable-relative DMC3 volumes\n";
        return 3;
    }

    std::vector<std::uint32_t> present_indices;
    present_indices.reserve(discovered->size());
    for (const auto& volume : *discovered) {
        present_indices.push_back(volume.index);
    }

    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(present_indices);
    if (!bootstrap.valid() || bootstrap.registered_archives.empty() ||
        !bootstrap.present_after_first_gap.empty() ||
        !bootstrap.present_outside_runtime_index_domain.empty()) {
        std::cerr
            << "extract-dmc3-retail-member: invalid or ambiguous contiguous DMC3 archive bootstrap\n";
        return 4;
    }

    gdspaces::SourceRegistry registry;
    constexpr std::string_view physical_source_id = "dmc3-retail-physical";
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{physical_source_id}, data_directory, false))) {
        std::cerr
            << "extract-dmc3-retail-member: cannot mount physical data directory\n";
        return 5;
    }

    std::vector<MountedArchive> mounted_archives;
    mounted_archives.reserve(bootstrap.registered_archives.size());
    for (const auto& archive : bootstrap.registered_archives) {
        const auto* physical = find_volume(*discovered, archive.index);
        if (physical == nullptr) {
            std::cerr
                << "extract-dmc3-retail-member: bootstrap references an undiscovered archive\n";
            return 5;
        }

        auto source_id = std::string{"dmc3-retail-volume-"} +
            std::to_string(archive.index);
        auto source = std::make_unique<gdspaces::NbzZipSource>(
            source_id, physical->path);
        if (!source->valid()) {
            std::cerr
                << "extract-dmc3-retail-member: invalid NBZ/ZIP volume: "
                << physical->path.string() << '\n';
            return 6;
        }
        const auto* source_pointer = source.get();
        if (!registry.mount(std::move(source))) {
            std::cerr
                << "extract-dmc3-retail-member: cannot mount NBZ source\n";
            return 6;
        }
        mounted_archives.push_back(MountedArchive{
            .index = archive.index,
            .source_id = std::move(source_id),
            .path = physical->path,
            .source = source_pointer,
        });
    }

    dmc3::RuntimeSourceBindings bindings;
    bindings.physical_source_id = std::string{physical_source_id};
    for (const auto volume_index : bootstrap.archive_resolution_order) {
        const auto iterator = std::find_if(
            mounted_archives.begin(), mounted_archives.end(),
            [volume_index](const MountedArchive& archive) {
                return archive.index == volume_index;
            });
        if (iterator == mounted_archives.end()) {
            std::cerr
                << "extract-dmc3-retail-member: incomplete archive source binding\n";
            return 7;
        }
        bindings.archives.push_back(dmc3::ArchiveSourceBinding{
            .volume_index = volume_index,
            .source_id = iterator->source_id,
        });
    }
    if (!bindings.valid_for(bootstrap)) {
        std::cerr
            << "extract-dmc3-retail-member: invalid runtime source bindings\n";
        return 7;
    }

    const auto resolved = dmc3::RuntimeResourceResolver::resolve(
        game_request, bootstrap, bindings, registry);
    if (!resolved.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: resolver did not produce one resource: "
            << dmc3::to_string(resolved.status);
        if (!resolved.detail.empty()) {
            std::cerr << ": " << resolved.detail;
        }
        std::cerr << '\n';
        return 8;
    }

    const auto archive_iterator = std::find_if(
        mounted_archives.begin(), mounted_archives.end(),
        [&](const MountedArchive& archive) {
            return archive.source_id == resolved.resolved->id.source_id;
        });
    if (archive_iterator == mounted_archives.end()) {
        std::cerr
            << "extract-dmc3-retail-member: request resolved to the physical provider; archive provenance required\n";
        return 9;
    }

    const auto* entry = find_entry(
        *archive_iterator->source, resolved.resolved->id.logical_path);
    if (entry == nullptr || !archive_iterator->source->index_receipt().has_value()) {
        std::cerr
            << "extract-dmc3-retail-member: selected archive member metadata is unavailable\n";
        return 9;
    }

    const auto archive_size =
        archive_iterator->source->index_receipt()->archive_size;
    const auto first_digest = sha256_file(archive_iterator->path, archive_size);
    if (!first_digest.has_value()) {
        std::cerr
            << "extract-dmc3-retail-member: cannot establish selected archive identity\n";
        return 10;
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
        std::cerr
            << "extract-dmc3-retail-member: archive changed or could not be bound to its observed identity\n";
        return 10;
    }

    const auto observed = gdspaces::NbzZipArtifactMemberObserver::observe(
        *archive_iterator->source,
        *bound.snapshot,
        entry->central_index);
    if (!observed.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: selected member could not be materialized from the artifact-bound archive\n";
        return 11;
    }

    const auto bytes = observed.observation->materialized_bytes();
    const auto resource_digest = core::Sha256::compute(bytes);
    const auto& provenance = observed.observation->byte_provenance();
    const auto& observed_entry = observed.observation->entry();

    std::error_code error;
    auto output_parent = output_file.parent_path();
    if (output_parent.empty()) {
        output_parent = std::filesystem::current_path(error);
        if (error) {
            std::cerr
                << "extract-dmc3-retail-member: cannot resolve output parent directory\n";
            return 12;
        }
    }
    std::filesystem::create_directories(output_parent, error);
    if (error || !std::filesystem::is_directory(output_parent, error) || error) {
        std::cerr
            << "extract-dmc3-retail-member: cannot create output parent directory\n";
        return 12;
    }

    std::ostringstream receipt;
    receipt
        << "{\n"
        << "  \"schema_version\": 2,\n"
        << "  \"evidence_class\": \"artifact-bound-retail-member-acquisition\",\n"
        << "  \"game_request\": \"" << escape_json(game_request) << "\",\n"
        << "  \"resolver_status\": \"resolved\",\n"
        << "  \"resolver_probe_count\": " << resolved.probes.size() << ",\n"
        << "  \"selected_volume_index\": " << archive_iterator->index << ",\n"
        << "  \"archive\": {\n"
        << "    \"path\": \"" << escape_json(archive_iterator->path.generic_string()) << "\",\n"
        << "    \"size\": " << artifact.size << ",\n"
        << "    \"sha256\": \"" << artifact.sha256 << "\"\n"
        << "  },\n"
        << "  \"member\": {\n"
        << "    \"logical_path\": \"" << escape_json(observed_entry.logical_path) << "\",\n"
        << "    \"central_index\": " << observed_entry.central_index << ",\n"
        << "    \"compression_method\": " << observed_entry.compression_method << ",\n"
        << "    \"crc32\": " << observed_entry.crc32 << ",\n"
        << "    \"compressed_size\": " << observed_entry.compressed_size << ",\n"
        << "    \"uncompressed_size\": " << observed_entry.uncompressed_size << ",\n"
        << "    \"local_header_offset\": " << observed_entry.local_header_offset << ",\n"
        << "    \"data_offset\": " << observed_entry.data_offset << "\n"
        << "  },\n"
        << "  \"materialized_resource\": {\n"
        << "    \"size\": " << bytes.size() << ",\n"
        << "    \"sha256\": \"" << resource_digest.hex() << "\",\n"
        << "    \"origin_kind\": \"" << gdspaces::to_string(provenance.kind) << "\",\n"
        << "    \"transform\": \"" << gdspaces::to_string(provenance.transform) << "\",\n"
        << "    \"source_offset\": " << provenance.offset << ",\n"
        << "    \"stored_size\": " << provenance.stored_size << ",\n"
        << "    \"materialized_size\": " << provenance.materialized_size;
    if (provenance.crc32.has_value()) {
        receipt << ",\n    \"provenance_crc32\": " << *provenance.crc32 << '\n';
    } else {
        receipt << '\n';
    }
    receipt
        << "  },\n"
        << "  \"output_file\": \"" << escape_json(output_file.generic_string()) << "\",\n"
        << "  \"publication\": \"atomic-no-replace-per-artifact\"\n"
        << "}\n";

    const auto output_publication = core::publish_bytes_no_replace(
        output_file, bytes, {}, ".dmc-rengine-retail-member.staging");
    if (!output_publication.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: extracted output publication failed ("
            << core::to_string(output_publication.status) << "): "
            << output_publication.detail << '\n';
        return 13;
    }

    const auto receipt_text = receipt.str();
    const auto receipt_bytes = std::as_bytes(std::span<const char>{
        receipt_text.data(), receipt_text.size()});
    const auto receipt_publication = core::publish_bytes_no_replace(
        receipt_path,
        receipt_bytes,
        {},
        ".dmc-rengine-retail-receipt.staging");
    if (!receipt_publication.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: receipt publication failed after output publication; output was left intact to avoid destructive ownership races ("
            << core::to_string(receipt_publication.status) << "): "
            << receipt_publication.detail << '\n';
        return 14;
    }

    std::cout
        << "DMC3 retail member acquisition: VERIFIED\n"
        << "Game request: " << game_request << '\n'
        << "Selected volume: " << archive_iterator->index << '\n'
        << "Archive: " << archive_iterator->path.string() << '\n'
        << "Archive SHA-256: " << artifact.sha256 << '\n'
        << "Member: " << observed_entry.logical_path << '\n'
        << "Member SHA-256: " << resource_digest.hex() << '\n'
        << "Materialized bytes: " << bytes.size() << '\n'
        << "Transform: " << gdspaces::to_string(provenance.transform) << '\n'
        << "Output: " << output_file.string() << '\n'
        << "Receipt: " << receipt_path.string() << '\n';
    return 0;
}

void print_dmc3_retail_acquisition_help() {
    std::cout
        << "  extract-dmc3-retail-member <exe-dir> <game-request> <output-file>\n"
        << "                            Resolve and artifact-bind one exact member from contiguous retail DMC3 NBZ volumes with SHA/provenance receipt\n";
}

int try_run_dmc3_retail_acquisition_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "extract-dmc3-retail-member") {
        return -1;
    }
    if (argc != 5) {
        std::cerr
            << "extract-dmc3-retail-member: expected <exe-dir> <game-request> <output-file>\n";
        return 1;
    }
    return run_extract_dmc3_retail_member(
        std::filesystem::path{argv[2]},
        std::string_view{argv[3]},
        std::filesystem::path{argv[4]});
}

} // namespace dmc::rengine::cli
