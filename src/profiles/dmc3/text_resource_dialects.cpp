#include "dmc_rengine/profiles/dmc3/text_resource_dialects.hpp"

#include "dmc_rengine/profiles/dmc3/effect_pack_contract.hpp"

#include <algorithm>
#include <cctype>

namespace dmc::rengine::profiles::dmc3 {
namespace {

[[nodiscard]] std::string_view head(
    std::span<const std::byte> bytes, std::size_t limit) noexcept {
    const auto count = std::min(bytes.size(), limit);
    return std::string_view{
        reinterpret_cast<const char*>(bytes.data()), count};
}

/** The first line, without its terminator. */
[[nodiscard]] std::string_view first_line(std::string_view text) noexcept {
    const auto end = text.find_first_of("\r\n");
    return end == std::string_view::npos ? text : text.substr(0, end);
}

/**
 * The leading run of text in the probe window, or empty if there is none.
 *
 * The `.TSC` marker is searched rather than anchored, and a four-byte literal
 * turns up in binary often enough that searching for it alone would type
 * arbitrary payloads as scroll tables. A dialect marker is only evidence when
 * the bytes around it are text, so the encoding is established first and the
 * identity is read out of what that leaves.
 *
 * The run ends at the first NUL rather than requiring the whole window to be
 * text, because a payload does not arrive at its own length: a container slot
 * is padded to the container's alignment, so a short text record reaches this
 * function with trailing zeros. Demanding text of every byte in the window
 * refused exactly the real slots this is for, while a synthetic buffer sized
 * to its content passed — which is the kind of guard that looks correct until
 * it meets a file.
 */
[[nodiscard]] std::string_view text_run(std::string_view window) noexcept {
    const auto end = window.find('\0');
    const auto run = end == std::string_view::npos ? window : window.substr(0, end);
    const auto printable = [](char character) noexcept {
        const auto value = static_cast<unsigned char>(character);
        return value == '\t' || value == '\r' || value == '\n' ||
            (value >= 0x20U && value < 0x7FU);
    };
    return std::all_of(run.begin(), run.end(), printable) ? run
                                                          : std::string_view{};
}

[[nodiscard]] bool ends_with_ci(
    std::string_view text, std::string_view suffix) noexcept {
    if (text.size() < suffix.size()) return false;
    const auto tail = text.substr(text.size() - suffix.size());
    return std::equal(
        tail.begin(), tail.end(), suffix.begin(), [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b));
        });
}

/**
 * Whether the probe window reads as an effect pack's record manifest.
 *
 * The grammar is not restated here: `EffectPackContract::read_manifest_line`
 * is the one home for it, and the pack reader walks a manifest with the same
 * call. What differs is the question. The reader has already been handed a
 * manifest and only has to walk it, so it admits any single-character kind;
 * this has to decide whether an unnamed slot *is* one, so it admits only the
 * kinds the corpus holds. A stricter test for identifying a payload than for
 * reading one already identified is the right asymmetry: the cost of a wrong
 * yes here is a resource typed as something it is not.
 *
 * Only whole lines count. The window is sixty-four bytes and a manifest is
 * longer, so the last line in it is usually cut in half — judging a fragment
 * would make the verdict depend on where the window happens to land.
 */
[[nodiscard]] bool reads_as_effect_manifest(std::string_view text) noexcept {
    using Contract = EffectPackContract;

    std::size_t records = 0U;
    bool terminated = false;
    std::size_t at = 0U;
    while (true) {
        const auto end = text.find('\n', at);
        if (end == std::string_view::npos) {
            break;  // a partial trailing line; the window cut it
        }
        const auto line = Contract::read_manifest_line(text.substr(at, end - at));
        at = end + 1U;

        switch (line.line_kind) {
        case Contract::ManifestLineKind::invalid:
            return false;
        case Contract::ManifestLineKind::record:
            if (!Contract::is_known_kind(line.kind)) {
                return false;
            }
            ++records;
            break;
        case Contract::ManifestLineKind::terminator:
            terminated = true;
            break;
        case Contract::ManifestLineKind::blank:
        case Contract::ManifestLineKind::comment:
            break;
        }
    }

    // One record line is a coincidence a short binary payload can produce —
    // `A 1` followed by a NUL is three bytes of anything. Two in a row, or one
    // the file then closes with its terminator, is the format.
    return records >= 2U || (records == 1U && terminated);
}

} // namespace

TextResourceIdentity TextResourceDialects::identify(
    std::span<const std::byte> bytes) noexcept {
    TextResourceIdentity identity;
    const auto text = text_run(head(bytes, k_probe_bytes));
    if (text.empty()) {
        return identity;
    }

    // A cloth definition opens with its own filename as a comment. The `.clt`
    // suffix on that line is what makes this a dialect marker rather than any
    // comment: an arbitrary `;` line is not evidence, a `;` line naming a
    // `.clt` file is.
    const auto opening = first_line(text);
    if (opening.size() > 1U && opening.front() == ';' &&
        ends_with_ci(opening, ".clt")) {
        identity.dialect = TextResourceDialect::clt;
        identity.embedded_original_name = std::string{opening.substr(1U)};
        return identity;
    }

    // A scroll table opens with a `.TSC` tag, after a leading blank line in
    // every observed payload. Searched rather than anchored, because the blank
    // line is a property of these payloads and not obviously of the format.
    if (text.find(".TSC") != std::string_view::npos) {
        identity.dialect = TextResourceDialect::tsc;
        return identity;
    }

    // An effect pack's manifest is the one dialect here that announces itself
    // by its whole shape rather than by an opening marker: every line is
    // `<kind> <id>`, and there is nothing else in the file. It says no
    // filename, so none is invented.
    if (reads_as_effect_manifest(text)) {
        identity.dialect = TextResourceDialect::effect_manifest;
        return identity;
    }

    return identity;
}

} // namespace dmc::rengine::profiles::dmc3
