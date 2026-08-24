#include "nbz_copy_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/evidence/artifact.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_artifact_binding.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_repacker.hpp"
#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <algorithm>
#include <charconv>
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

[[nodiscard]] std::optional<std::uint32_t> parse_central_index(
    std::string_view text) noexcept {
    if (text.empty()) {
        return std::nullopt;
    }
    std::uint64_t value{};
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    if (result.ec != std::errc{} || result.ptr != last ||
        value > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint32_t>(value);
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

[[nodiscard]] std::optional<std::vector<std::byte>> read_authored_resource(
    const std::filesystem::path& input_path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(input_path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        return std::nullopt;
    }
    const auto size = std::filesystem::file_size(absolute, error);
    if (error) {
        return std::nullopt;
    }

    constexpr std::string_view source_id = "nbz-copy-authored-input";
    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, absolute.parent_path(), false))) {
        return std::nullopt;
    }
    const gdspaces::ResourceId id{
        .source_id = std::string{source_id},
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = size,
    };
    auto payload = registry.read(id);
    if (!payload.has_value() || !payload->readable()) {
        return std::nullopt;
    }
    return std::move(payload->bytes);
}

[[nodiscard]] std::filesystem::path normalized_absolute(
    const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    return error ? path.lexically_normal() : absolute.lexically_normal();
}

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << '?';
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] int run_build_nbz_copy(
    const std::filesystem::path& source_path,
    std::string_view central_index_text,
    const std::filesystem::path& replacement_path,
    const std::filesystem::path& output_path) {
    const auto central_index = parse_central_index(central_index_text);
    if (!central_index.has_value()) {
        std::cerr << "build-nbz-copy: invalid central index\n";
        return 2;
    }
    if (source_path.empty() || output_path.empty() ||
        normalized_absolute(source_path) == normalized_absolute(output_path)) {
        std::cerr
            << "build-nbz-copy: source and output must be distinct paths\n";
        return 2;
    }

    gdspaces::NbzZipSource source("nbz-copy-source", source_path);
    if (!source.valid() || !source.index_receipt().has_value()) {
        std::cerr << "build-nbz-copy: source is not a valid supported NBZ/ZIP artifact\n";
        return 3;
    }

    const auto& entries = source.entries();
    const auto selected = std::find_if(
        entries.begin(), entries.end(),
        [&](const gdspaces::NbzZipEntry& entry) {
            return !entry.directory && entry.central_index == *central_index;
        });
    if (selected == entries.end()) {
        std::cerr << "build-nbz-copy: central index does not identify a file member\n";
        return 4;
    }
    const auto selected_position = static_cast<std::size_t>(
        std::distance(entries.begin(), selected));

    const auto source_size = source.index_receipt()->archive_size;
    const auto source_digest = sha256_file_exact(source_path, source_size);
    if (!source_digest.has_value()) {
        std::cerr << "build-nbz-copy: cannot establish exact source artifact identity\n";
        return 5;
    }

    const evidence::ArtifactIdentity artifact{
        .id = "nbz-copy-source-artifact",
        .role = "nbz-source-artifact",
        .sha256 = source_digest->hex(),
        .size = source_size,
    };
    const auto bound = gdspaces::NbzZipArtifactSerializationBinder::bind(
        source, artifact);
    if (!bound.ok()) {
        std::cerr << "build-nbz-copy: source changed or could not be artifact-bound\n";
        return 5;
    }

    auto replacement_bytes = read_authored_resource(replacement_path);
    if (!replacement_bytes.has_value()) {
        std::cerr << "build-nbz-copy: replacement resource is not readable\n";
        return 6;
    }

    const auto& serialization_entries = bound.snapshot->serialization().entries;
    if (selected_position >= serialization_entries.size()) {
        std::cerr << "build-nbz-copy: selected entry is absent from bound serialization\n";
        return 7;
    }
    const auto physical_offset =
        serialization_entries[selected_position].local_record_offset;

    std::vector<gdspaces::NbzZipMemberReplacement> replacements;
    for (std::size_t index = 0U; index < serialization_entries.size(); ++index) {
        if (serialization_entries[index].local_record_offset == physical_offset) {
            replacements.push_back(gdspaces::NbzZipMemberReplacement{
                .central_index = entries[index].central_index,
                .materialized_bytes = *replacement_bytes,
            });
        }
    }
    if (replacements.empty()) {
        std::cerr << "build-nbz-copy: no physical replacement identity was derived\n";
        return 7;
    }

    std::error_code error;
    const auto output_parent = output_path.parent_path();
    if (!output_parent.empty()) {
        std::filesystem::create_directories(output_parent, error);
        if (error) {
            std::cerr << "build-nbz-copy: cannot create output parent directory\n";
            return 8;
        }
    }

    const auto repacked = gdspaces::NbzZipRetailRepacker::write(
        source, *bound.snapshot, replacements, output_path);
    if (!repacked.ok()) {
        std::cerr
            << "build-nbz-copy: canonical NBZ copy authoring failed ("
            << gdspaces::to_string(repacked.status) << ")";
        if (!repacked.detail.empty()) {
            std::cerr << ": " << repacked.detail;
        }
        std::cerr << '\n';
        return 9;
    }

    const auto receipt_path = std::filesystem::path{
        output_path.string() + ".receipt.json"};
    std::ostringstream receipt;
    receipt
        << "{\n"
        << "  \"schema_version\": 1,\n"
        << "  \"evidence_class\": \"artifact-bound-nbz-copy-authoring\",\n"
        << "  \"source\": {\n"
        << "    \"path\": \"" << escape_json(source_path.generic_string()) << "\",\n"
        << "    \"size\": " << repacked.receipt->source_size << ",\n"
        << "    \"sha256\": \"" << repacked.receipt->source_sha256 << "\"\n"
        << "  },\n"
        << "  \"target\": {\n"
        << "    \"central_index\": " << *central_index << ",\n"
        << "    \"logical_path\": \"" << escape_json(selected->logical_path) << "\",\n"
        << "    \"physical_alias_count\": " << replacements.size() << "\n"
        << "  },\n"
        << "  \"replacement_materialized_size\": " << replacement_bytes->size() << ",\n"
        << "  \"output\": {\n"
        << "    \"path\": \"" << escape_json(output_path.generic_string()) << "\",\n"
        << "    \"size\": " << repacked.receipt->output_size << ",\n"
        << "    \"sha256\": \"" << repacked.receipt->output_sha256 << "\"\n"
        << "  },\n"
        << "  \"publication\": \"validated-no-replace-output-copy\"\n"
        << "}\n";

    const auto receipt_text = receipt.str();
    const auto receipt_bytes = std::as_bytes(std::span<const char>{
        receipt_text.data(), receipt_text.size()});
    const auto receipt_publication = core::publish_bytes_no_replace(
        receipt_path,
        receipt_bytes,
        {},
        ".dmc-rengine-nbz-copy-receipt.staging");
    if (!receipt_publication.ok()) {
        std::cerr
            << "build-nbz-copy: output copy is valid, but receipt publication failed ("
            << core::to_string(receipt_publication.status)
            << "); output was left intact\n";
        return 10;
    }

    std::cout
        << "NBZ artifact copy authoring: VERIFIED\n"
        << "Source SHA-256: " << repacked.receipt->source_sha256 << '\n'
        << "Target central index: " << *central_index << '\n'
        << "Target member: " << selected->logical_path << '\n'
        << "Physical aliases changed: " << replacements.size() << '\n'
        << "Output SHA-256: " << repacked.receipt->output_sha256 << '\n'
        << "Output bytes: " << repacked.receipt->output_size << '\n'
        << "Output: " << output_path.string() << '\n'
        << "Receipt: " << receipt_path.string() << '\n';
    return 0;
}

} // namespace

void print_nbz_copy_help() {
    std::cout
        << "  build-nbz-copy <source-nbz> <central-index> <replacement-file> <output-nbz>\n"
        << "                            Build a verified immutable-source NBZ copy with one physical member replacement\n";
}

int try_run_nbz_copy_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "build-nbz-copy") {
        return -1;
    }
    if (argc != 6) {
        std::cerr
            << "build-nbz-copy: expected <source-nbz> <central-index> <replacement-file> <output-nbz>\n";
        return 1;
    }
    return run_build_nbz_copy(
        std::filesystem::path{argv[2]},
        std::string_view{argv[3]},
        std::filesystem::path{argv[4]},
        std::filesystem::path{argv[5]});
}

} // namespace dmc::rengine::cli
