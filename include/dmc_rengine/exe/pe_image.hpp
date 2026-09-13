#pragma once

#include <cstddef>
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

/// One entry of the optional header's data-directory array.
struct PeDataDirectory final {
    std::uint32_t rva{};
    std::uint32_t size{};

    [[nodiscard]] bool present() const noexcept {
        return rva != 0U && size != 0U;
    }

    friend bool operator==(const PeDataDirectory&, const PeDataDirectory&) = default;
};

/// Index into the data-directory array, in specification order.
enum class PeDirectory : std::size_t {
    export_table = 0,
    import_table = 1,
    resource_table = 2,
    exception_table = 3,
    certificate_table = 4,
    base_relocation_table = 5,
    debug = 6,
    architecture = 7,
    global_pointer = 8,
    tls_table = 9,
    load_config_table = 10,
    bound_import = 11,
    iat = 12,
    delay_import_descriptor = 13,
    clr_runtime_header = 14,
    reserved = 15,
};

[[nodiscard]] constexpr std::string_view to_string(PeDirectory directory) noexcept {
    switch (directory) {
    case PeDirectory::export_table: return "export";
    case PeDirectory::import_table: return "import";
    case PeDirectory::resource_table: return "resource";
    case PeDirectory::exception_table: return "exception";
    case PeDirectory::certificate_table: return "certificate";
    case PeDirectory::base_relocation_table: return "base-relocation";
    case PeDirectory::debug: return "debug";
    case PeDirectory::architecture: return "architecture";
    case PeDirectory::global_pointer: return "global-pointer";
    case PeDirectory::tls_table: return "tls";
    case PeDirectory::load_config_table: return "load-config";
    case PeDirectory::bound_import: return "bound-import";
    case PeDirectory::iat: return "iat";
    case PeDirectory::delay_import_descriptor: return "delay-import";
    case PeDirectory::clr_runtime_header: return "clr-runtime-header";
    case PeDirectory::reserved: return "reserved";
    }
    return "reserved";
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
    std::uint16_t dll_characteristics{};
    std::uint16_t characteristics{};
    std::uint32_t timestamp{};
    std::uint32_t section_alignment{};
    std::uint32_t file_alignment{};
    std::uint32_t checksum{};
    std::uint32_t size_of_code{};
    std::uint32_t size_of_initialized_data{};
    std::uint32_t size_of_uninitialized_data{};
    std::vector<PeSection> sections;
    std::vector<PeDataDirectory> data_directories;

    /// Directory entry, or an empty entry when the image declares fewer
    /// directories than the specification's sixteen.
    [[nodiscard]] PeDataDirectory directory(PeDirectory entry) const noexcept;

    [[nodiscard]] std::optional<std::uint64_t> rva_to_va(
        std::uint32_t rva) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> rva_to_file_offset(
        std::uint32_t rva) const noexcept;
    [[nodiscard]] std::optional<std::uint32_t> file_offset_to_rva(
        std::uint32_t offset) const noexcept;
};

} // namespace dmc::rengine::exe
