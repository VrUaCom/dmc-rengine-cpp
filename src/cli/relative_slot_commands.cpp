#include "relative_slot_commands.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::cli {
namespace {

namespace core = dmc::rengine::core;
namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] std::optional<unsigned int> parse_slot_index(std::string_view text) noexcept {
    if (text.empty()) return std::nullopt;
    unsigned long long value{};
    const auto* first = text.data();
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    if (result.ec != std::errc{} || result.ptr != last || value > std::numeric_limits<unsigned int>::max()) return std::nullopt;
    return static_cast<unsigned int>(value);
}

[[nodiscard]] std::filesystem::path normalized_absolute(const std::filesystem::path& path) {
    std::error_code error;
    auto absolute = std::filesystem::absolute(path, error);
    if (error) return path.lexically_normal();
    auto canonical = std::filesystem::weakly_canonical(absolute, error);
    return error ? absolute.lexically_normal() : canonical;
}

[[nodiscard]] std::optional<gdspaces::ResourcePayload> read_local_resource(const std::filesystem::path& path, std::string_view source_id) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) return std::nullopt;
    const auto size = std::filesystem::file_size(absolute, error);
    if (error) return std::nullopt;
    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(std::string{source_id}, absolute.parent_path(), false))) return std::nullopt;
    const gdspaces::ResourceId id{.source_id=std::string{source_id}, .logical_path=absolute.filename().generic_string(), .container_chain={}, .offset=0U, .size=size};
    return registry.read(id);
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) { return core::Sha256::compute(bytes).hex(); }

[[nodiscard]] bool verify_rebuilt_children(
    const gdspaces::ResourcePayload& source_parent,
    const gdspaces::ContainerExpansion& source_expansion,
    unsigned int target_slot,
    std::span<const std::byte> replacement_bytes,
    const std::filesystem::path& staged_path,
    std::string_view expected_format,
    std::uint32_t expected_slots) {
    auto staged = read_local_resource(staged_path, "relative-slot-staged-validation");
    if (!staged.has_value() || !staged->readable()) return false;
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(std::span<const std::byte>{staged->bytes.data(), staged->bytes.size()}, staged_path.filename().generic_string());
    if (!parsed.ok() || parsed.document.format != expected_format || parsed.document.declared_slot_count != expected_slots) return false;
    const auto expansion = gdspaces::ContainerExpander::expand(*staged, parsed);
    if (!expansion.usable() || expansion.children.size() != source_expansion.children.size()) return false;

    for (std::size_t index = 0; index < source_expansion.children.size(); ++index) {
        const auto& before = source_expansion.children[index];
        const auto& after = expansion.children[index];
        if (before.entry.populated != after.entry.populated) return false;
        if (!before.entry.populated) continue;
        if (!before.payload.readable() || !after.payload.readable()) return false;
        const auto after_bytes = std::span<const std::byte>{after.payload.bytes.data(), after.payload.bytes.size()};
        if (index == target_slot) {
            if (!std::equal(replacement_bytes.begin(), replacement_bytes.end(), after_bytes.begin(), after_bytes.end())) return false;
        } else {
            const auto before_bytes = std::span<const std::byte>{before.payload.bytes.data(), before.payload.bytes.size()};
            if (!std::equal(before_bytes.begin(), before_bytes.end(), after_bytes.begin(), after_bytes.end())) return false;
        }
    }
    (void)source_parent;
    return true;
}

} // namespace

int run_rebuild_relative_slot(const std::filesystem::path& parent_file, unsigned int slot_index, const std::filesystem::path& replacement_file, const std::filesystem::path& output_file) {
    if (parent_file.empty() || replacement_file.empty() || output_file.empty() || normalized_absolute(parent_file) == normalized_absolute(output_file)) {
        std::cerr << "rebuild-relative-slot: source container and output must be distinct paths\n"; return 2;
    }
    auto parent = read_local_resource(parent_file, "relative-slot-parent");
    if (!parent.has_value() || !parent->readable()) { std::cerr << "rebuild-relative-slot: parent container is not readable\n"; return 3; }
    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(std::span<const std::byte>{parent->bytes.data(), parent->bytes.size()}, parent->resource.id.logical_path);
    if (!parsed.ok() || (parsed.document.format != "PAC" && parsed.document.format != "PNST")) { std::cerr << "rebuild-relative-slot: parent is not a canonical PAC/PNST relative-slot container\n"; return 4; }
    const auto expansion = gdspaces::ContainerExpander::expand(*parent, parsed);
    if (!expansion.usable() || slot_index >= expansion.children.size()) { std::cerr << "rebuild-relative-slot: slot index is outside the parsed topology\n"; return 5; }
    const auto& child = expansion.children[slot_index];
    if (!child.entry.populated || !child.payload.readable()) { std::cerr << "rebuild-relative-slot: target slot is empty or unreadable\n"; return 5; }
    auto replacement = read_local_resource(replacement_file, "relative-slot-replacement");
    if (!replacement.has_value() || !replacement->readable()) { std::cerr << "rebuild-relative-slot: replacement file is not readable\n"; return 6; }
    const auto replacement_bytes = replacement->bytes;
    dmc3::AuthoredChildImage authored{.resource=child.payload.resource.id, .source_sha256=sha256_of(std::span<const std::byte>{child.payload.bytes.data(), child.payload.bytes.size()}), .output_sha256=sha256_of(std::span<const std::byte>{replacement->bytes.data(), replacement->bytes.size()}), .revision=1U, .writer_mode="cli-complete-child-image", .bytes=std::move(replacement->bytes)};
    const std::vector<dmc3::AuthoredChildImage> authored_children{std::move(authored)};
    const auto rebuilt = dmc3::RelativeSlotPackedReflowWriter::rebuild(*parent, expansion, authored_children);
    if (!rebuilt.ok()) { std::cerr << "rebuild-relative-slot: canonical packed reflow failed (" << dmc3::to_string(rebuilt.status) << ")"; if (!rebuilt.detail.empty()) std::cerr << ": " << rebuilt.detail; std::cerr << '\n'; return 7; }
    std::error_code error;
    const auto output_parent = output_file.parent_path();
    if (!output_parent.empty()) { std::filesystem::create_directories(output_parent, error); if (error) { std::cerr << "rebuild-relative-slot: cannot create output directory\n"; return 8; } }
    const auto expected_format = rebuilt.receipt->output_topology.format;
    const auto expected_slots = rebuilt.receipt->output_topology.declared_slot_count;
    const auto validator = [&parent, &expansion, slot_index, &replacement_bytes, expected_format, expected_slots](const std::filesystem::path& staged_path) {
        return verify_rebuilt_children(*parent, expansion, slot_index, std::span<const std::byte>{replacement_bytes.data(), replacement_bytes.size()}, staged_path, expected_format, expected_slots);
    };
    const auto publication = core::publish_bytes_no_replace(output_file, std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()}, validator, ".dmc-rengine-relative-slot.staging");
    if (!publication.ok()) { std::cerr << "rebuild-relative-slot: output publication failed (" << core::to_string(publication.status) << ")"; if (!publication.detail.empty()) std::cerr << ": " << publication.detail; std::cerr << '\n'; return 9; }
    std::cout << "Relative-slot rebuild: VERIFIED\n" << "Format: " << rebuilt.receipt->output_topology.format << '\n' << "Target slot: " << slot_index << '\n' << "Source SHA-256: " << rebuilt.receipt->source_sha256 << '\n' << "Output SHA-256: " << rebuilt.receipt->output_sha256 << '\n' << "Source bytes: " << rebuilt.receipt->source_topology.container_size << '\n' << "Output bytes: " << rebuilt.receipt->output_topology.container_size << '\n' << "Output: " << output_file.string() << '\n';
    return 0;
}

void print_relative_slot_help() { std::cout << "  rebuild-relative-slot <container-file> <slot-index> <replacement-file> <output-file>\n" << "                            Replace one populated PAC/PNST slot and safely reflow the packed container\n"; }

int try_run_relative_slot_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "rebuild-relative-slot") return -1;
    if (argc != 6) { std::cerr << "rebuild-relative-slot: expected <container-file> <slot-index> <replacement-file> <output-file>\n"; return 1; }
    const auto slot_index = parse_slot_index(argv[3]);
    if (!slot_index.has_value()) { std::cerr << "rebuild-relative-slot: invalid slot index\n"; return 1; }
    return run_rebuild_relative_slot(std::filesystem::path{argv[2]}, *slot_index, std::filesystem::path{argv[4]}, std::filesystem::path{argv[5]});
}

} // namespace dmc::rengine::cli
