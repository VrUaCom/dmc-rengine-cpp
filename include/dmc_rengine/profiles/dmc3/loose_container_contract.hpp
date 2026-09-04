#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// Instruction-backed contract for the recovered `.lst` loose-container family.
//
// `.lst` is the original runtime's *fallback* representation for a
// container-backed resource: the exact packed container wins whenever it exists
// with positive size, and only otherwise is the extension rewritten and a
// container synthesized from the list. Calling it an override inverts the
// recovered precedence and would make a synthesized container shadow a real one.
struct LooseContainerContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    static constexpr std::uint64_t image_base = 0x140000000ULL;

    static constexpr std::uint64_t representation_selector_va = 0x1401B79E0ULL;
    static constexpr std::array<std::uint64_t, 5> parser_helper_vas{
        0x1401B7B90ULL,
        0x1401B7C70ULL,
        0x1401B7D10ULL,
        0x1401B7E60ULL,
        0x1401B7FD0ULL,
    };
    static constexpr std::uint64_t loose_materializer_va = 0x1401B85C0ULL;
    static constexpr std::uint64_t extension_rewrite_va = 0x1401B9390ULL;
    static constexpr std::uint64_t generic_materializer_va = 0x1402EF4D0ULL;

    // The selector only applies to container-backed resources.
    static constexpr std::uint16_t container_backed_kind16 = 0U;

    // The rewrite replaces an existing extension. A path with no extension
    // boundary cannot enter it, so such a path has no `.lst` fallback at all.
    static constexpr std::string_view list_extension = ".lst";
    static constexpr bool requires_existing_extension = true;
    static constexpr bool packed_representation_wins = true;

    // Recovered scanner bounds.
    static constexpr std::size_t scan_limit = 0x1FC0U;
    static constexpr std::size_t token_limit = 0x100U;
    static constexpr std::size_t synthesis_alignment = 0x40U;

    // The grammar is CRLF-oriented: normal child text ends at CR or NUL, and
    // skip/control states return to normal at LF. An LF-only normal line is not
    // proven equivalent, so a product reader must fail it closed rather than
    // advertise a normalized file as original-compatible.
    static constexpr std::uint8_t child_terminator_cr = 0x0DU;
    static constexpr std::uint8_t child_terminator_nul = 0x00U;
    static constexpr std::uint8_t state_return_lf = 0x0AU;
    static constexpr bool lf_only_normal_line_is_equivalent = false;

    // Line classes in the normal state. `/` is the recovered comment marker and
    // `#` is a magic directive — an older shorthand had these the other way
    // round, and that shorthand must not be copied into clean code.
    static constexpr char comment_marker = '/';
    static constexpr char directive_marker = '#';

    // Only a declared child slot increments the slot count. Directive, comment
    // and blank lines do not.
    static constexpr bool directive_increments_slot_count = false;
    static constexpr bool comment_increments_slot_count = false;
    static constexpr bool dummy_increments_slot_count = true;

    // Up to four immediate bytes after `#` become the synthesized magic, with
    // no whitespace skipped before capture. `#PNST` is therefore an ordinary
    // four-byte magic, not evidence of a hardcoded PNST directive parser.
    static constexpr std::size_t directive_magic_bytes = 4U;
    static constexpr bool directive_skips_whitespace = false;
    static constexpr std::array<char, 4> default_magic{'P', 'A', 'C', '\0'};

    [[nodiscard]] static consteval std::uint32_t rva_of(
        std::uint64_t virtual_address) noexcept {
        return static_cast<std::uint32_t>(virtual_address - image_base);
    }

    [[nodiscard]] static constexpr bool terminates_child_text(
        std::uint8_t value) noexcept {
        return value == child_terminator_cr || value == child_terminator_nul;
    }
};

} // namespace dmc::rengine::profiles::dmc3
