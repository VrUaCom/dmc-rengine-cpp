#include "structure_commands.hpp"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "dmc_rengine/formats/pac.hpp"
#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/resource_structure.hpp"

namespace dmc::rengine::cli {
namespace {

[[nodiscard]] std::optional<std::vector<std::byte>> read_file(const std::filesystem::path& path) {
    std::ifstream in{path, std::ios::binary};
    if (!in) return std::nullopt;
    const std::vector<char> raw{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    std::vector<std::byte> out(raw.size());
    for (std::size_t i = 0U; i < raw.size(); ++i) out[i] = static_cast<std::byte>(raw[i]);
    return out;
}

// Slot `slot` of a PAC or PNST image; empty when the container or slot is not there.
[[nodiscard]] std::optional<std::span<const std::byte>> container_slot(std::span<const std::byte> bytes,
                                                                     std::uint32_t slot) {
    auto parsed = formats::PacParser::parse(bytes);
    if (!parsed.ok()) parsed = formats::PnstParser::parse(bytes);
    if (!parsed.ok()) return std::nullopt;
    for (const auto& entry : parsed.document->entries) {
        if (entry.slot_index != slot || !entry.populated || !entry.valid(bytes.size())) continue;
        return bytes.subspan(static_cast<std::size_t>(entry.offset), static_cast<std::size_t>(entry.size));
    }
    return std::nullopt;
}

} // namespace

void print_structure_help() {
    std::cout
        << "  structure <file> [--slot N] [--format F]\n"
        << "                             Read the structure of a resource the way the game does:\n"
        << "                             cloth (clt), UV scroll (tsc), event (evt), stage collision\n"
        << "                             (hits), effect bank (pnst), collision-shapes, motion-script,\n"
        << "                             or the slot roles of a character pac; --slot reads one slot\n"
        << "                             of a PAC / PNST, --format overrides the classifier\n";
}

int try_run_structure_command(int argc, char** argv) {
    if (argc <= 1 || std::string_view{argv[1]} != "structure") return -1;
    if (argc < 3) {
        std::cerr << "structure: usage: structure <file> [--slot N] [--format F]\n";
        return 1;
    }
    const std::filesystem::path path{argv[2]};
    std::optional<std::uint32_t> slot;
    std::string format;
    for (int i = 3; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--slot" && i + 1 < argc) {
            slot = static_cast<std::uint32_t>(std::strtoul(argv[++i], nullptr, 10));
        } else if (arg == "--format" && i + 1 < argc) {
            format = argv[++i];
        } else {
            std::cerr << "structure: unknown argument " << arg << "\n";
            return 1;
        }
    }
    const auto file = read_file(path);
    if (!file) {
        std::cerr << "structure: cannot read " << path.string() << "\n";
        return 1;
    }
    std::span<const std::byte> bytes{*file};
    // A slot has no name of its own: the classifier reads its bytes only.
    std::string name = path.filename().string();
    if (slot) {
        const auto inner = container_slot(bytes, *slot);
        if (!inner) {
            std::cerr << "structure: " << path.filename().string() << " has no populated slot " << *slot << "\n";
            return 1;
        }
        bytes = *inner;
        name += "/" + std::to_string(*slot);
    }
    if (format.empty()) {
        format = gdspaces::ResourceClassifier::classify(slot ? std::string_view{} : std::string_view{name}, bytes)
                     .format;
    }
    if (!integration::has_structure(format)) {
        std::cerr << "structure: no structure reader for format '" << format << "' (readers:";
        for (const auto f : integration::structure_formats()) std::cerr << " " << f;
        std::cerr << ")\n";
        return 2;
    }
    std::string detail;
    const auto view = integration::read_structure(format, bytes, slot ? std::string_view{name} : path.string(), detail);
    if (!view) {
        std::cerr << "structure: " << format << ": " << detail << "\n";
        return 2;
    }
    std::cout << name << "  [" << view->format << "]\n"
              << "Reader: " << view->reader << "\n"
              << view->summary << "\n";
    for (const auto& section : view->sections) {
        std::cout << "\n" << section.title << "\n";
        for (const auto& row : section.rows) std::cout << "  " << row.label << ": " << row.value << "\n";
    }
    if (view->truncated) std::cout << "\n(truncated at " << integration::kMaxStructureSections << " sections)\n";
    return 0;
}

} // namespace dmc::rengine::cli
