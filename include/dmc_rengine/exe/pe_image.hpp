#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::exe {

enum class PeKind {
    pe32,
    pe32_plus,
};

[[nodiscard]] constexpr std::string_view to_string(PeKind kind) noexcept {
    switch (kind) {
    case PeKind::pe32: return "PE32";
    case PeKind::pe32_plus: return "PE32+";
    }
    return "unknown";
}

enum class PeMachine : std::uint16_t {
    unknown = 0,
    i386 = 0x014c,
    amd64 = 0x8664,
    arm64 = 0xaa64,
};

[[nodiscard]] constexpr std::string_view to_string(PeMachine machine) noexcept {
    switch (machine) {
    case PeMachine::i386: return "i386";
    case PeMachine::amd64: return "amd64";
    case PeMachine::arm64: return "arm64";
    case PeMachine::unknown: return "unknown";
    }
    return "unknown";
}

struct PeSection final {
    std::string name;
    std::uint32_t virtual_size{};
    std::uint32_t virtual_address{};
    std::uint32_t raw_size{};
    std::uint32_t raw_offset{};
    std::uint32_t characteristics{};

    [[nodiscard]] bool contains_rva(std::uint32_t rva) const noexcept;
    [[nodiscard]] bool contains_file_offset(std::uint32_t offset) const noexcept;
};

struct PeImage final {
    PeKind kind{PeKind::pe32};
    PeMachine machine{PeMachine::unknown};
    std::uint16_t section_count{};
    std::uint64_t image_base{};
    std::uint32_t entry_point_rva{};
    std::uint32_t size_of_image{};
    std::uint32_t size_of_headers{};
    std::uint16_t subsystem{};
    std::vector<PeSection> sections;

    [[nodiscard]] std::optional<std::uint64_t> rva_to_va(
        std::uint32_t rva) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> rva_to_file_offset(
        std::uint32_t rva) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> file_offset_to_rva(
        std::uint32_t offset) const noexcept;
};

} // namespace dmc::rengine::exe
