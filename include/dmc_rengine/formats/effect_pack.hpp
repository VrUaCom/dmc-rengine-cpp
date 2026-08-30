#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::formats {

enum class EffectPackParseError : std::uint8_t {
    none,
    not_a_container,
    wrong_outer_slot_count,
    manifest_missing,
    manifest_not_text,
    malformed_manifest_line,
    line_count_mismatch,
    invalid_document,
};

struct EffectRecord final {
    std::uint32_t slot_index{};
    char kind{};
    std::uint32_t identifier{};
    std::string name;
    std::size_t source_line{};
    std::uint64_t offset{};
    std::uint64_t extent{};
    bool extent_matches_kind{false};
    bool kind_known{false};
};

struct EffectPackDocument final {
    std::uint64_t document_size{};
    std::uint32_t manifest_line_count{};
    std::uint32_t populated_record_count{};
    bool manifest_names_every_populated_record{false};
    bool extents_match_known_kinds{false};
    std::string manifest_text;
    std::vector<EffectRecord> records;

    [[nodiscard]] bool valid() const noexcept;
};

struct EffectPackParseResult final {
    std::optional<EffectPackDocument> document;
    EffectPackParseError error{EffectPackParseError::none};
    std::string message;

    [[nodiscard]] bool ok() const noexcept {
        return document.has_value() && error == EffectPackParseError::none;
    }
};

// Structural reader for the effect-container convention recovered in #254.
// It requires exactly one manifest line for each populated record payload and
// refuses malformed/ambiguous structures instead of inventing names.
class EffectPackParser final {
public:
    static constexpr std::uint32_t k_max_records = 4096U;

    [[nodiscard]] static EffectPackParseResult parse(
        std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats
