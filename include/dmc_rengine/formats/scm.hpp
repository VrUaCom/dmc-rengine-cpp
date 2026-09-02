#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace dmc::rengine::formats {

enum class ScmParseError : std::uint8_t {
    none,
    truncated_header,
    invalid_magic,
    group_limit,
    truncated_group_table,
    group_out_of_bounds,
    batch_out_of_bounds,
    array_out_of_bounds,
    array_packing_mismatch,
    missing_strip_marker,
    invalid_document,
};

// One primitive batch: four parallel arrays of the same length, plus the
// scratch buffer the original runtime rebuilds its strip index list into.
//
// Offsets are stored as read, relative to the document base, because that is
// what the file contains. The runtime overwrites them with pointers; a reader
// that did the same would destroy the only copy of the file's own addressing.
struct ScmPrimitiveBatch final {
    std::uint32_t group_index{};
    std::uint32_t batch_index{};
    std::uint64_t batch_offset{};
    std::uint32_t index_count{};

    std::uint64_t position_offset{};
    std::uint64_t normal_offset{};
    std::uint64_t attribute_offset{};
    std::uint64_t index_offset{};

    // Relative to the batch, not to the document. Resolved here so a caller
    // never has to remember which base this one uses.
    std::uint64_t strip_offset{};
    std::int32_t stored_strip_length{};

    // How many indices carry the skip flag the strip rebuild breaks a run on.
    std::uint32_t skipped_index_count{};

    [[nodiscard]] bool valid(std::uint64_t document_size) const noexcept;
};

struct ScmGroup final {
    std::uint32_t group_index{};
    std::uint64_t group_offset{};
    std::uint32_t batch_count{};
    std::uint64_t first_batch_offset{};
};

struct ScmDocument final {
    std::uint64_t document_size{};
    std::uint32_t group_count{};
    std::uint64_t document_pointer{};
    std::vector<ScmGroup> groups;
    std::vector<ScmPrimitiveBatch> batches;

    [[nodiscard]] std::uint64_t total_index_count() const noexcept;
    [[nodiscard]] bool valid() const noexcept;
};

struct ScmParseResult final {
    std::optional<ScmDocument> document;
    ScmParseError error{ScmParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == ScmParseError::none;
    }
};

// Structural reader for the DMC3 scene model, built from the recovered
// relocation routine (`ScmContract::relocate_va`) rather than from inspection.
//
// It validates what the routine relies on and nothing beyond it: that every
// relocatable offset lands inside the document, that the four arrays of a
// group are packed the way the file lays them out, and that each strip buffer
// carries the marker the routine tests for. What the attribute and index words
// *mean* is not claimed here.
class ScmParser final {
public:
    // Product-side denial-of-service bound. The runtime reads both counts as
    // bytes, so neither can exceed 255 in a real document.
    static constexpr std::uint32_t k_max_group_count = 256U;
    static constexpr std::uint32_t k_max_batch_count = 256U;

    [[nodiscard]] static ScmParseResult parse(std::span<const std::byte> bytes);
};

} // namespace dmc::rengine::formats
