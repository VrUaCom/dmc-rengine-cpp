#pragma once

#include <cstddef>
#include <cstdint>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract recovered from the canonical DMC3 HD executable.
//
// This type describes the original type-0 physical-provider boundary. It does
// not make the current GDSpaces source-derived physical lookup index an exact
// emulation of that boundary; RuntimeLookupEvidenceClass intentionally keeps
// that product path classified separately until controlled parity receipts are
// available.
struct PhysicalProviderContract final {
    static constexpr std::uint64_t physical_mount_add_va = 0x140326D20ULL;
    static constexpr std::uint64_t resource_mount_resolve_va = 0x140327430ULL;
    static constexpr std::uint64_t resource_path_exists_va = 0x140327720ULL;
    static constexpr std::uint64_t resource_file_open_va = 0x140327800ULL;
    static constexpr std::uint64_t root_join_va = 0x1403272C0ULL;

    static constexpr std::uint32_t normalization_flags = 0x0CU;
    static constexpr std::size_t path_capacity = 0x400U;

    // CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
    //             OPEN_EXISTING, 0, nullptr)
    static constexpr std::uint32_t desired_access = 0x80000000U; // GENERIC_READ
    static constexpr std::uint32_t share_mode = 0x00000001U;     // FILE_SHARE_READ
    static constexpr std::uint32_t creation_disposition = 3U;   // OPEN_EXISTING
    static constexpr std::uint32_t flags_and_attributes = 0U;

    static constexpr std::uint32_t error_file_not_found = 2U;
    static constexpr std::uint32_t error_path_not_found = 3U;
    static constexpr std::uint32_t error_no_more_files = 18U;

    [[nodiscard]] static constexpr bool is_open_miss_error(
        std::uint32_t error) noexcept {
        return error == error_file_not_found || error == error_path_not_found;
    }

    [[nodiscard]] static constexpr bool is_exists_miss_error(
        std::uint32_t error) noexcept {
        return is_open_miss_error(error) || error == error_no_more_files;
    }

    [[nodiscard]] static constexpr bool root_has_join_separator(
        char value) noexcept {
        return value == '/' || value == ':' || value == '\\';
    }
};

} // namespace dmc::rengine::profiles::dmc3
