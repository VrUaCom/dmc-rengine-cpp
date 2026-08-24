#include "dmc3_retail_acquisition_commands.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <algorithm>
#include <array>
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
            // Two differently-cased or otherwise duplicate physical filenames
            // for one runtime volume index are ambiguous on case-sensitive
            // hosts. Fail closed instead of choosing one product-side.
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

[[nodiscard]] std::optional<core::Sha256Digest> sha256_file(
    const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return std::nullopt;
    }

    core::Sha256Accumulator accumulator;
    std::vector<char> buffer(1024U * 1024U);
    while (stream) {
        stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto count = stream.gcount();
        if (count < 0) {
            return std::nullopt;
        }
        if (count != 0) {
            const auto bytes = std::as_bytes(std::span<const char>{
                buffer.data(), static_cast<std::size_t>(count)});
            if (!accumulator.update(bytes)) {
                return std::nullopt;
            }
        }
    }
    if (!stream.eof()) {
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

[[nodiscard]] bool write_new_binary(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::error_code error;
    if (std::filesystem::exists(path, error) || error) {
        return false;
    }
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }

    std::ofstream stream(path, std::ios::binary | std::ios::out);
    if (!stream) {
        return false;
    }
    if (!bytes.empty()) {
        stream.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    return stream.good();
}

[[nodiscard]] bool write_new_text(
    const std::filesystem::path& path,
    std::string_view text) {
    std::error_code error;
    if (std::filesystem::exists(path, error) || error) {
        return false;
    }
    std::ofstream stream(path, std::ios::binary | std::ios::out);
    if (!stream) {
        return false;
    }
    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    return stream.good();
}

[[nodiscard]] std::filesystem::path receipt_path_for(
    const std::filesystem::path& output_file) {
    return std::filesystem::path{output_file.string() + ".receipt.json"};
}

} // namespace

int run_extract_dmc3_retail_member(
    const std::filesystem::path& executable_directory,
    std::string_view game_request,
    const std::filesystem::path& output_file) {
    const auto data_directory = executable_directory /
        std::filesystem::path{dmc3::VolumeBootstrapPolicy::data_subdirectory()};
    const auto discovered = discover_volumes(data_directory);
    if (!discovered.has_value()) {
        std::cerr
            << "extract-dmc3-retail-member: cannot uniquely scan executable-relative DMC3 volumes\n";
        return 2;
    }

    std::vector<std::uint32_t> present_indices;
    present_indices.reserve(discovered->size());
    for (const auto& volume : *discovered) {
        present_indices.push_back(volume.index);
    }

    const auto bootstrap = dmc3::VolumeBootstrapPolicy::plan(present_indices);
    if (!bootstrap.valid() || bootstrap.registered_archives.empty()) {
        std::cerr
            << "extract-dmc3-retail-member: no valid contiguous DMC3 archive bootstrap\n";
        return 3;
    }

    gdspaces::SourceRegistry registry;
    constexpr std::string_view physical_source_id = "dmc3-retail-physical";
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{physical_source_id}, data_directory, false))) {
        std::cerr
            << "extract-dmc3-retail-member: cannot mount physical data directory\n";
        return 4;
    }

    std::vector<MountedArchive> mounted_archives;
    mounted_archives.reserve(bootstrap.registered_archives.size());
    for (const auto& archive : bootstrap.registered_archives) {
        const auto* physical = find_volume(*discovered, archive.index);
        if (physical == nullptr) {
            std::cerr
                << "extract-dmc3-retail-member: bootstrap references an undiscovered archive\n";
            return 4;
        }

        auto source_id = std::string{"dmc3-retail-volume-"} +
            std::to_string(archive.index);
        auto source = std::make_unique<gdspaces::NbzZipSource>(
            source_id, physical->path);
        if (!source->valid()) {
            std::cerr
                << "extract-dmc3-retail-member: invalid NBZ/ZIP volume: "
                << physical->path.string() << '\n';
            return 5;
        }
        const auto* source_pointer = source.get();
        if (!registry.mount(std::move(source))) {
            std::cerr
                << "extract-dmc3-retail-member: cannot mount NBZ source\n";
            return 5;
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
            return 6;
        }
        bindings.archives.push_back(dmc3::ArchiveSourceBinding{
            .volume_index = volume_index,
            .source_id = iterator->source_id,
        });
    }
    if (!bindings.valid_for(bootstrap)) {
        std::cerr
            << "extract-dmc3-retail-member: invalid runtime source bindings\n";
        return 6;
    }

    const auto resolved = dmc3::RuntimeResourceResolver::resolve(
        game_request, bootstrap, bindings, registry);
    if (!resolved.ok()) {
        std::cerr
            << "extract-dmc3-retail-member: resolver did not produce one archive resource: "
            << dmc3::to_string(resolved.status);
        if (!resolved.detail.empty()) {
            std::cerr << ": " << resolved.detail;
        }
        std::cerr << '\n';
        return 7;
    }

    const auto archive_iterator = std::find_if(
        mounted_archives.begin(), mounted_archives.end(),
        [&](const MountedArchive& archive) {
            return archive.source_id == resolved.resolved->id.source_id;
        });
    if (archive_iterator == mounted_archives.end()) {
        std::cerr
            << "extract-dmc3-retail-member: request resolved to the physical provider; archive provenance required\n";
        return 8;
    }

    const auto entry_iterator = std::find_if(
        archive_iterator->source->entries().begin(),
        archive_iterator->source->entries().end(),
        [&](const gdspaces::NbzZipEntry& entry) {
            return !entry.directory &&
                entry.logical_path == resolved.resolved->id.logical_path;
        });
    if (entry_iterator == archive_iterator->source->entries().end()) {
        std::cerr
            << "extract-dmc3-retail-member: selected member metadata is unavailable\n";
        return 8;
    }

    const auto payload = registry.read(resolved.resolved->id);
    if (!payload.has_value() || !payload->readable() ||
        !payload->byte_provenance.has_value() ||
        !payload->byte_provenance->valid()) {
        std::cerr
            << "extract-dmc3-retail-member: selected member failed materialization/provenance validation\n";
        return 9;
    }

    const auto archive_digest = sha256_file(archive_iterator->path);
    if (!archive_digest.has_value()) {
        std::cerr
            << "extract-dmc3-retail-member: cannot hash selected source archive\n";
        return 10;
    }
    const auto resource_digest = core::Sha256::compute(
        std::span<const std::byte>{payload->bytes});

    std::error_code error;
    const auto archive_size = std::filesystem::file_size(archive_iterator->path, error);
    if (error) {
        std::cerr
            << "extract-dmc3-retail-member: cannot read selected archive size\n";
        return 10;
    }

    const auto receipt_path = receipt_path_for(output_file);
    if (std::filesystem::exists(output_file, error) || error) {
        std::cerr
            << "extract-dmc3-retail-member: output already exists or cannot be checked\n";
        return 11;
    }
    error.clear();
    if (std::filesystem::exists(receipt_path, error) || error) {
        std::cerr
            << "extract-dmc3-retail-member: receipt already exists or cannot be checked\n";
        return 11;
    }

    const auto& provenance = *payload->byte_provenance;
    std::ostringstream receipt;
    receipt
        << "{\n"
        << "  \"schema_version\": 1,\n"
        << "  \"evidence_class\": \"direct-retail-member-acquisition\",\n"
        << "  \"game_request\": \"" << escape_json(game_request) << "\",\n"
        << "  \"resolver_status\": \"resolved\",\n"
        << "  \"resolver_probe_count\": " << resolved.probes.size() << ",\n"
        << "  \"selected_volume_index\": " << archive_iterator->index << ",\n"
        << "  \"archive\": {\n"
        << "    \"path\": \"" << escape_json(archive_iterator->path.generic_string()) << "\",\n"
        << "    \"size\": " << archive_size << ",\n"
        << "    \"sha256\": \"" << archive_digest->hex() << "\"\n"
        << "  },\n"
        << "  \"member\": {\n"
        << "    \"logical_path\": \"" << escape_json(entry_iterator->logical_path) << "\",\n"
        << "    \"central_index\": " << entry_iterator->central_index << ",\n"
        << "    \"compression_method\": " << entry_iterator->compression_method << ",\n"
        << "    \"crc32\": " << entry_iterator->crc32 << ",\n"
        << "    \"compressed_size\": " << entry_iterator->compressed_size << ",\n"
        << "    \"uncompressed_size\": " << entry_iterator->uncompressed_size << ",\n"
        << "    \"local_header_offset\": " << entry_iterator->local_header_offset << ",\n"
        << "    \"data_offset\": " << entry_iterator->data_offset << "\n"
        << "  },\n"
        << "  \"materialized_resource\": {\n"
        << "    \"size\": " << payload->bytes.size() << ",\n"
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
        << "  \"output_file\": \"" << escape_json(output_file.generic_string()) << "\"\n"
        << "}\n";

    if (!write_new_binary(output_file, std::span<const std::byte>{payload->bytes})) {
        std::cerr
            << "extract-dmc3-retail-member: cannot publish extracted member without overwrite\n";
        return 12;
    }
    if (!write_new_text(receipt_path, receipt.str())) {
        std::filesystem::remove(output_file, error);
        std::cerr
            << "extract-dmc3-retail-member: receipt publication failed; extracted output removed\n";
        return 12;
    }

    std::cout
        << "DMC3 retail member acquisition: VERIFIED\n"
        << "Game request: " << game_request << '\n'
        << "Selected volume: " << archive_iterator->index << '\n'
        << "Archive: " << archive_iterator->path.string() << '\n'
        << "Archive SHA-256: " << archive_digest->hex() << '\n'
        << "Member: " << entry_iterator->logical_path << '\n'
        << "Member SHA-256: " << resource_digest.hex() << '\n'
        << "Materialized bytes: " << payload->bytes.size() << '\n'
        << "Transform: " << gdspaces::to_string(provenance.transform) << '\n'
        << "Output: " << output_file.string() << '\n'
        << "Receipt: " << receipt_path.string() << '\n';
    return 0;
}

void print_dmc3_retail_acquisition_help() {
    std::cout
        << "  extract-dmc3-retail-member <exe-dir> <game-request> <output-file>\n"
        << "                            Resolve and extract one exact member from contiguous retail DMC3 NBZ volumes with SHA/provenance receipt\n";
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