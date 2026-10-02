#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// What a recovered reader takes out of a payload, as sections of named rows.
//
// One shape for every format the GDSpaces browsers show a structure for —
// the desktop CLI (`dmc-rengine structure`) and Pocket GDSpace both render
// it. Each row comes from a canonical reader of this library (cloth chains,
// scroll records, event commands, HITS, the effect bank's E / P / G / T / A /
// V / M records, character collision tables, motion scripts, the slot roles
// of character archives); nothing here re-derives a field, it only formats.
namespace dmc::rengine::integration {

struct StructureRow final {
    std::string label;
    std::string value;
};

struct StructureSection final {
    std::string title;
    std::vector<StructureRow> rows;
};

struct StructureView final {
    std::string format;
    /// The canonical reader that produced the rows, with its image address.
    std::string reader;
    std::string summary;
    std::vector<StructureSection> sections;
    /// More records exist than are listed (kMaxStructureSections).
    bool truncated{false};
};

inline constexpr std::size_t kMaxStructureSections = 256U;

/// Formats read_structure answers for (classifier format keys).
[[nodiscard]] std::span<const std::string_view> structure_formats() noexcept;
[[nodiscard]] bool has_structure(std::string_view format) noexcept;

/**
 * Reads `bytes` as `format`. `name` is the resource's name or path; the slot
 * roles of a character archive (`pac`) depend on it ("pl000.pac", "em028.pac"
 * ...). Empty with `detail` set when the format has no structure reader or
 * the reader declines the bytes.
 */
[[nodiscard]] std::optional<StructureView> read_structure(
    std::string_view format, std::span<const std::byte> bytes, std::string_view name,
    std::string& detail);

} // namespace dmc::rengine::integration
