#pragma once

#include "dmc_rengine/formats/relocated_model_shell.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::formats {

enum class ModParseError : std::uint8_t {
    none,
    shell_rejected,
    batch_out_of_bounds,
    array_out_of_bounds,
    array_packing_mismatch,
    invalid_document,
};

// One batch: a vertex stream, not an indexed mesh.
//
// The recovered strip rebuild writes loop counters into the strip buffer, so a
// triangle refers to positions *within this batch* directly. The array at the
// index offset is a per-vertex control word whose high bit breaks the strip
// run — it is not a vertex index, and reading it as one would be the natural
// mistake.
struct ModPrimitiveBatch final {
    std::uint32_t group_index{};
    std::uint32_t batch_index{};
    std::uint64_t batch_offset{};
    std::uint32_t vertex_count{};

    std::uint64_t position_offset{};
    std::uint64_t normal_offset{};
    std::uint64_t attribute_offset{};
    std::uint64_t secondary_offset{};
    std::uint64_t control_offset{};

    // Relative to the batch, not to the document. Resolved here so a caller
    // never has to remember which base this one uses.
    std::uint64_t strip_offset{};
    std::int32_t stored_strip_length{};
    bool strip_marker_present{};

    // Vertices whose control word carries the strip-break bit, counted as
    // stored. A document the runtime has already loaded has none, because the
    // rebuild clears them.
    std::uint32_t break_count{};

    [[nodiscard]] bool valid(std::uint64_t document_size) const noexcept;
};

struct ModDocument final {
    std::uint64_t document_size{};
    std::uint32_t group_count{};
    std::uint8_t document_mode{};
    std::uint64_t document_pointer{};
    std::vector<ModelShellGroup> groups;
    std::vector<ModPrimitiveBatch> batches;

    [[nodiscard]] std::uint64_t total_vertex_count() const noexcept;
    [[nodiscard]] bool valid() const noexcept;
};

struct ModParseResult final {
    std::optional<ModDocument> document;
    ModParseError error{ModParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ModParseError::none;
    }
};

// Structural reader for the DMC3 model payload, built from the recovered
// relocation routine (`ModContract::relocate_va`).
//
// It validates what that routine relies on and nothing beyond it. What the
// attribute, secondary and control words hold is not claimed, and neither is
// anything about writing.
class ModParser final {
public:
    [[nodiscard]] static ModParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
