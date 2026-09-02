#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::formats {

// The document and group shell that DMC3's `SCM` and `MOD` payloads share.
//
// Both are walked by their own relocation routine, and those two routines
// agree byte for byte about this outer layer: a count at `+0x10` read as a
// *byte*, a relocated pointer at `+0x20`, and a table of `0x40`-byte groups at
// `+0x40` each holding a byte count and a relocated pointer. They then diverge
// completely about what a batch is, which is why only this much is shared —
// the same reason PAC and PNST share a relative-slot envelope and nothing else.
enum class ModelShellError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    group_limit,
    truncated_group_table,
    pointer_out_of_bounds,
};

struct ModelShellGroup final {
    std::uint32_t group_index{};
    std::uint64_t group_offset{};
    std::uint32_t batch_count{};
    std::uint64_t first_batch_offset{};
};

struct ModelShell final {
    std::uint64_t document_size{};
    std::uint32_t group_count{};
    // The byte at `+0x11`. `MOD`'s routine branches on it; `SCM`'s never reads
    // it. Carried here because a reader that dropped it would lose the only
    // recorded difference between two documents that otherwise look identical.
    std::uint8_t document_mode{};
    std::uint64_t document_pointer{};
    std::vector<ModelShellGroup> groups;
};

struct ModelShellResult final {
    std::optional<ModelShell> shell;
    ModelShellError error{ModelShellError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return shell.has_value() && error == ModelShellError::none;
    }
};

class RelocatedModelShell final {
public:
    static constexpr std::size_t group_count_offset = 0x10U;
    static constexpr std::size_t document_mode_offset = 0x11U;
    static constexpr std::size_t document_pointer_offset = 0x20U;
    static constexpr std::size_t group_table_offset = 0x40U;
    static constexpr std::size_t group_stride = 0x40U;
    static constexpr std::size_t group_batch_count_offset = 0x00U;
    static constexpr std::size_t group_batch_pointer_offset = 0x08U;

    // Both counts are bytes in the recovered routines, so neither can exceed
    // 255 in a real document. The limits are stated anyway, because a limit
    // that happens to be unreachable is still the thing a malformed file has
    // to run into.
    static constexpr std::uint32_t max_group_count = 256U;
    static constexpr std::uint32_t max_batch_count = 256U;

    // `magic` is compared for exactly `magic.size()` bytes — three, in both
    // recovered probes.
    [[nodiscard]] static ModelShellResult parse(
        std::span<const std::byte> bytes,
        std::string_view magic,
        std::size_t batch_stride);
};

} // namespace dmc::rengine::formats
