#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_packed_reflow_writer.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) {
        return {};
    }
    const auto end = stream.tellg();
    if (end <= 0) {
        return {};
    }
    const auto size = static_cast<std::size_t>(end);
    std::vector<std::byte> bytes(size);
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
    if (!stream) {
        return {};
    }
    return bytes;
}

[[nodiscard]] bool write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        return false;
    }
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    return stream.good();
}

[[nodiscard]] bool parse_u32(std::string_view text, std::uint32_t& output) noexcept {
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto result = std::from_chars(begin, end, output, 10);
    return result.ec == std::errc{} && result.ptr == end;
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_parent_payload(
    std::string source_id,
    std::string logical_path,
    std::vector<std::byte> bytes) {
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return dmc::rengine::gdspaces::ResourcePayload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = std::move(source_id),
                .logical_path = logical_path,
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = std::move(logical_path),
            .format = "unknown",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
    };
}

} // namespace

int main(int argc, char** argv) {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    if (argc != 5) {
        std::cerr
            << "usage: dmc-rengine-relative-slot-corpus-reflow "
            << "<source-container> <slot-index> <replacement-child> <output-container>\n";
        return 2;
    }

    std::uint32_t slot_index = 0U;
    if (!parse_u32(argv[2], slot_index)) {
        std::cerr << "invalid slot index\n";
        return 2;
    }

    auto source_bytes = read_file(argv[1]);
    const auto replacement_bytes = read_file(argv[3]);
    if (source_bytes.empty() || replacement_bytes.empty()) {
        std::cerr << "source/replacement read failed\n";
        return 3;
    }

    const auto source_sha = sha256_of(
        std::span<const std::byte>{source_bytes.data(), source_bytes.size()});
    auto parent = make_parent_payload(
        "pass85-real-corpus-source",
        std::filesystem::path{argv[1]}.filename().string(),
        std::move(source_bytes));

    const auto registry = dmc3::make_container_parser_registry();
    const auto parsed = registry.parse(
        std::span<const std::byte>{parent.bytes.data(), parent.bytes.size()},
        parent.resource.id.logical_path);
    if (!parsed.ok()) {
        std::cerr << "source container parse failed\n";
        return 4;
    }
    auto expansion = gdspaces::ContainerExpander::expand(parent, parsed);
    if (!expansion.usable() || slot_index >= expansion.children.size()) {
        std::cerr << "source expansion/slot failed\n";
        return 5;
    }

    const auto& source_child = expansion.children[slot_index];
    if (!source_child.entry.populated || source_child.payload.bytes.empty()) {
        std::cerr << "requested source slot is empty\n";
        return 5;
    }

    const auto source_child_sha = sha256_of(std::span<const std::byte>{
        source_child.payload.bytes.data(), source_child.payload.bytes.size()});
    const auto replacement_sha = sha256_of(std::span<const std::byte>{
        replacement_bytes.data(), replacement_bytes.size()});
    if (source_child_sha == replacement_sha) {
        std::cerr << "replacement is byte-identical to source child\n";
        return 6;
    }

    const std::vector<dmc3::AuthoredChildImage> authored{
        dmc3::AuthoredChildImage{
            .resource = source_child.payload.resource.id,
            .source_sha256 = source_child_sha,
            .output_sha256 = replacement_sha,
            .revision = 1U,
            .writer_mode = "compiled-real-texture-slot-reflow",
            .bytes = replacement_bytes,
        },
    };

    const auto rebuilt = dmc3::RelativeSlotPackedReflowWriter::rebuild(
        parent, expansion, authored);
    if (!rebuilt.ok()) {
        std::cerr << "relative-slot reflow failed: "
                  << to_string(rebuilt.status)
                  << " detail=" << rebuilt.detail << '\n';
        return 7;
    }

    if (!write_file(
            argv[4],
            std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()})) {
        std::cerr << "output write failed\n";
        return 8;
    }

    auto output_parent = make_parent_payload(
        "pass85-real-corpus-output",
        std::filesystem::path{argv[4]}.filename().string(),
        rebuilt.bytes);
    const auto output_parsed = registry.parse(
        std::span<const std::byte>{
            output_parent.bytes.data(), output_parent.bytes.size()},
        output_parent.resource.id.logical_path);
    if (!output_parsed.ok()) {
        std::cerr << "output container reparse failed\n";
        return 9;
    }
    const auto output_expansion = gdspaces::ContainerExpander::expand(
        output_parent, output_parsed);
    if (!output_expansion.usable() ||
        output_expansion.children.size() != expansion.children.size()) {
        std::cerr << "output expansion topology failed\n";
        return 10;
    }

    for (std::size_t index = 0U; index < expansion.children.size(); ++index) {
        const auto& before = expansion.children[index];
        const auto& after = output_expansion.children[index];
        if (before.entry.populated != after.entry.populated) {
            std::cerr << "output occupancy changed\n";
            return 11;
        }
        if (!before.entry.populated) {
            continue;
        }
        if (index == slot_index) {
            if (after.payload.bytes != replacement_bytes) {
                std::cerr << "replacement child mismatch after reparse\n";
                return 12;
            }
        } else if (before.payload.bytes != after.payload.bytes) {
            std::cerr << "untouched sibling bytes changed\n";
            return 13;
        }
    }

    const auto output_sha = sha256_of(std::span<const std::byte>{
        rebuilt.bytes.data(), rebuilt.bytes.size()});
    const auto& output_child = output_expansion.children[slot_index];
    std::cout
        << "{\"status\":\"ok\","
        << "\"format\":\"" << expansion.parser_format << "\","
        << "\"slot_index\":" << slot_index << ','
        << "\"declared_slots\":" << expansion.children.size() << ','
        << "\"source_container_size\":" << parent.bytes.size() << ','
        << "\"output_container_size\":" << rebuilt.bytes.size() << ','
        << "\"source_child_size\":" << source_child.payload.bytes.size() << ','
        << "\"output_child_size\":" << output_child.payload.bytes.size() << ','
        << "\"source_container_sha256\":\"" << source_sha << "\","
        << "\"output_container_sha256\":\"" << output_sha << "\","
        << "\"source_child_sha256\":\"" << source_child_sha << "\","
        << "\"output_child_sha256\":\"" << replacement_sha << "\"}\n";
    return 0;
}
