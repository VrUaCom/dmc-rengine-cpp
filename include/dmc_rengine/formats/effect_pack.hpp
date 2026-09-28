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

    // M only: the immediately following physical slot consumed by the EXE
    // registrar ABI. The slot is recorded even when it is empty.
    std::optional<std::uint32_t> companion_slot_index;
    std::uint64_t companion_offset{};
    std::uint64_t companion_extent{};
    bool companion_populated{false};
};

struct EffectPackDocument final {
    std::uint64_t document_size{};
    std::uint32_t manifest_line_count{};
    std::uint32_t populated_record_count{};
    /**
     * Populated physical companion slots consumed by manifest kind `M`.
     *
     * The canonical loader 0x1402C04C0 always advances one additional physical
     * record slot after an M record and passes both pointers to 0x1402E35D0.
     * The companion may be empty or may contain arbitrary payload bytes (the
     * corpus includes both 0x31/zero blocks and PTX payloads), so byte identity
     * is not companion authority.
     */
    std::uint32_t companion_record_count{};
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

// Structural/runtime reader for the FXBANK convention. The canonical loader
// consumes one physical inner-PNST slot per manifest record and one additional
// physical slot after every M record. It refuses malformed/ambiguous structures
// instead of inventing names or treating companion bytes as naming authority.
class EffectPackParser final {
public:
    static constexpr std::uint32_t k_max_records = 4096U;

    [[nodiscard]] static EffectPackParseResult parse(
        std::span<const std::byte> bytes);
    [[nodiscard]] static bool structurally_valid(
        std::span<const std::byte> bytes) noexcept;
};

} // namespace dmc::rengine::formats
