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

// One named effect record: a manifest line paired with the slot it names.
//
// `name` is the manifest's own text. It is the first name in this project that
// a container actually stored, so it is not attributed as invented and must
// not be presented as though it were.
struct EffectRecord final {
    std::uint32_t slot_index{};
    char kind{};
    std::uint32_t identifier{};
    std::string name;
    std::uint64_t offset{};
    std::uint64_t extent{};
    // True where the kind has a fixed extent in the contract and this record
    // matches it. False for `T`, whose extent varies by design.
    bool extent_matches_kind{false};
    bool kind_known{false};
};

struct EffectPackDocument final {
    std::uint64_t document_size{};
    std::uint32_t manifest_line_count{};
    std::uint32_t record_slot_count{};
    // The whole point of the format: the manifest names every slot, one line
    // each, and this says the two agreed.
    bool manifest_names_every_slot{false};
    // True where every record whose kind has a fixed extent matched it.
    bool extents_match_kinds{false};
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

// Structural reader for `*_effect.pac`.
//
// It refuses anything whose manifest line count does not equal its record slot
// count, because that equality is the only thing that makes a line a name for
// a particular slot rather than a line that happens to sit nearby.
class EffectPackParser final {
public:
    static constexpr std::uint32_t k_max_records = 4096U;

    [[nodiscard]] static EffectPackParseResult parse(
        std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats
