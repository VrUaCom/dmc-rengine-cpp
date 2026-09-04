#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats {

enum class ShwParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    entry_limit,
    truncated_entry_table,
    pointer_out_of_bounds,
    invalid_document,
};

// One shadow entry: four bases the runtime relocates and never indexes
// through. Their element widths are not recoverable from the routine, so this
// records where each array starts and says nothing about how long it is.
struct ShwEntry final {
    std::uint32_t entry_index{};
    std::uint64_t entry_offset{};
    std::array<std::uint64_t, 4U> array_offsets{};
};

struct ShwDocument final {
    std::uint64_t document_size{};
    std::uint32_t entry_count{};
    std::vector<ShwEntry> entries;

    [[nodiscard]] bool valid() const noexcept;
};

struct ShwParseResult final {
    std::optional<ShwDocument> document;
    ShwParseError error{ShwParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ShwParseError::none;
    }
};

// Structural reader for the DMC3 shadow payload, from
// `ShwContract::relocate_va`.
//
// This one does *not* use the model shell. Three of the four type handlers
// share that shell and this is the fourth; its table starts at `+0x30` rather
// than `+0x40`, it has no groups, no batches and no strip rebuild. Reading it
// with the shell would take its first entry for a group header.
//
// No `SHW` payload exists in the supplied corpus, so this is recovered and not
// corroborated.
class ShwParser final {
public:
    [[nodiscard]] static ShwParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
