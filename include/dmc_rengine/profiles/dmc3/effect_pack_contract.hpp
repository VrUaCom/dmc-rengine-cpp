#pragma once

#include <array>
#include <span>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// FXBANK contract recovered from corpus plus canonical dmc3.exe.
//
// `*_effect.pac` / the em034 effect bank is a two-slot PNST. Slot 0 is a CRLF
// ASCII manifest and slot 1 is the physical record PNST. The canonical loader
// at 0x1402C04C0 tokenizes the manifest and dispatches each record by
// (kind,u16 id). This does not make external `.index` runtime authority.
struct EffectPackContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::size_t outer_slot_count = 2U;
    static constexpr std::size_t manifest_slot_index = 0U;
    static constexpr std::size_t records_slot_index = 1U;

    static constexpr char comment_prefix = '#';
    static constexpr std::string_view terminator_line = "# End";
    static constexpr char field_separator = ' ';
    static constexpr char model_kind = 'M';

    static constexpr std::uint64_t runtime_loader_va = 0x1402C04C0ULL;
    static constexpr bool manifest_read_site_found = true;
    static constexpr bool model_consumes_following_physical_slot = true;

    struct KindExtent final {
        char kind{};
        std::size_t extent{};
        std::size_t observed{};
    };

    // Observed across the retained corpus: st001_effect.pac and
    // st114_effect.pac, and — since 2026-09-10 — the complete em000
    // extraction, whose pack holds 173 named records. An extent of 0 means
    // variable.
    //
    // em000 changed two of these readings, both in the direction the earlier
    // corpus could not have shown:
    //
    //   * `G` and `M` are kinds the first two packs did not contain at all.
    //     `M` names a MOD payload, which is why 13 of this pack's records read
    //     as models.
    //   * `P` is not fixed. The earlier reading of 704 came from two records;
    //     em000 holds 34 across four extents — 336, 528, 704 and 896 — so the
    //     fixed value was that corpus's, not the format's. This is the same
    //     mistake the MOT reader made with its single track kind and the
    //     texture reader made with its DXT5 coupling, in a third file.
    static constexpr std::array<KindExtent, 7> kinds{
        KindExtent{.kind = 'V', .extent = 368U, .observed = 54U},
        KindExtent{.kind = 'E', .extent = 544U, .observed = 54U},
        KindExtent{.kind = 'P', .extent = 0U, .observed = 36U},
        KindExtent{.kind = 'T', .extent = 0U, .observed = 11U},
        KindExtent{.kind = 'A', .extent = 336U, .observed = 13U},
        KindExtent{.kind = 'G', .extent = 96U, .observed = 12U},
        KindExtent{.kind = 'M', .extent = 0U, .observed = 13U},
    };

    // The extents `P` was observed at, kept as a record of what the corpus
    // holds now that the kind is no longer read as fixed. Not a constraint:
    // a fifth extent would be another payload, not a malformed one.
    static constexpr std::array<std::size_t, 4> observed_p_extents{
        336U, 528U, 704U, 896U};

    // One observed M-companion byte shape, not the companion rule.
    //
    // em000 contains ten populated M companions that are byte-identical: a
    // `0x31` word followed by twelve zeros. Later em034 evidence proves that
    // an M companion may instead be a PTX payload. Companion identity therefore
    // comes from the loader's physical-slot consumption rule, never from these
    // bytes. This predicate is retained only to recognize the observed 0x31
    // payload family.
    static constexpr std::size_t companion_record_size = 16U;
    static constexpr std::uint32_t companion_leading_word = 0x31U;
    static constexpr char companion_observed_after_kind = model_kind;
    static constexpr std::size_t companion_observed_count = 10U;

    /// Whether these record bytes are the unnamed companion record.
    [[nodiscard]] static constexpr bool is_companion_record(
        std::span<const std::byte> record) noexcept {
        if (record.size() != companion_record_size) {
            return false;
        }
        if (std::to_integer<std::uint32_t>(record[0]) != companion_leading_word) {
            return false;
        }
        for (std::size_t index = 1U; index < companion_record_size; ++index) {
            if (record[index] != std::byte{0}) {
                return false;
            }
        }
        return true;
    }

    /// What the grammar makes of one physical line.
    enum class ManifestLineKind : std::uint8_t {
        /// Nothing but whitespace.
        blank,
        /// Opens with `#`. Carried by the file and read by the grammar as
        /// nothing.
        comment,
        /// The `# End` line. A comment as far as reading goes; reported apart
        /// because it is the one comment whose text the format fixes.
        terminator,
        /// `<kind> <decimal identifier>`.
        record,
        /// Text the grammar does not admit.
        invalid,
    };

    struct ManifestLine final {
        ManifestLineKind line_kind{ManifestLineKind::invalid};
        char kind{};
        std::uint32_t identifier{};

        [[nodiscard]] constexpr bool is_record() const noexcept {
            return line_kind == ManifestLineKind::record;
        }
    };

    /// Leading and trailing spaces, tabs and a trailing `\r`.
    [[nodiscard]] static constexpr std::string_view trim_manifest_line(
        std::string_view line) noexcept {
        const auto blank = [](char character) noexcept {
            return character == ' ' || character == '\t';
        };
        while (!line.empty() && (blank(line.back()) || line.back() == '\r')) {
            line.remove_suffix(1U);
        }
        while (!line.empty() && blank(line.front())) {
            line.remove_prefix(1U);
        }
        return line;
    }

    /**
     * One manifest line, by the only grammar this format has.
     *
     * This lives on the contract rather than inside the pack reader because
     * two callers need it: the reader, which walks a manifest it has already
     * been handed, and the classifier, which has to decide whether a nameless
     * text slot *is* one. A second copy of the rule in the second caller is
     * how the two drift, and a slot would then read as a manifest to one and
     * not to the other.
     *
     * The identifier is parsed here rather than with `from_chars` so the whole
     * grammar stays constant-evaluable. Overflow is `invalid`, not a wrap: a
     * line naming an identifier the format cannot hold is not a line.
     */
    [[nodiscard]] static constexpr ManifestLine read_manifest_line(
        std::string_view raw) noexcept {
        const auto line = trim_manifest_line(raw);
        if (line.empty()) {
            return ManifestLine{.line_kind = ManifestLineKind::blank};
        }
        if (line.front() == comment_prefix) {
            return ManifestLine{
                .line_kind = line == terminator_line
                    ? ManifestLineKind::terminator
                    : ManifestLineKind::comment};
        }
        // A kind is one character and the separator is the next, so anything
        // else is not this grammar. The third character begins the decimal.
        if (line.size() < 3U || line[1U] != field_separator) {
            return ManifestLine{};
        }
        std::uint32_t identifier = 0U;
        for (std::size_t index = 2U; index < line.size(); ++index) {
            const auto digit = line[index];
            if (digit < '0' || digit > '9') {
                return ManifestLine{};
            }
            const auto value = static_cast<std::uint32_t>(digit - '0');
            constexpr std::uint32_t limit = 0xFFFFFFFFU;
            if (identifier > (limit - value) / 10U) {
                return ManifestLine{};
            }
            identifier = identifier * 10U + value;
        }
        return ManifestLine{
            .line_kind = ManifestLineKind::record,
            .kind = line.front(),
            .identifier = identifier,
        };
    }

    static constexpr std::size_t texture_dimensions_offset = 0x10U;
    static constexpr std::uint32_t texture_observed_square_small = 128U;
    static constexpr std::uint32_t texture_observed_square_large = 256U;

    [[nodiscard]] static constexpr std::size_t extent_for(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return entry.extent;
            }
        }
        return 0U;
    }

    [[nodiscard]] static constexpr bool is_known_kind(char kind) noexcept {
        for (const auto& entry : kinds) {
            if (entry.kind == kind) {
                return true;
            }
        }
        return false;
    }
};

} // namespace dmc::rengine::profiles::dmc3
