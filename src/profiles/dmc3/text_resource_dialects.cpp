#include "dmc_rengine/profiles/dmc3/text_resource_dialects.hpp"

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
 * Whether the probe window reads as text at all.
 *
 * The `.TSC` marker is searched rather than anchored, and a four-byte literal
 * turns up in binary often enough that searching for it alone would type
 * arbitrary payloads as scroll tables. A dialect marker is only evidence when
 * the bytes around it are text, so the encoding is checked before the
 * identity is read out of it.
 */
[[nodiscard]] bool reads_as_text(std::string_view text) noexcept {
    return std::all_of(text.begin(), text.end(), [](char character) {
        const auto value = static_cast<unsigned char>(character);
        return value == '\t' || value == '\r' || value == '\n' ||
            (value >= 0x20U && value < 0x7FU);
    });
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

} // namespace

TextResourceIdentity TextResourceDialects::identify(
    std::span<const std::byte> bytes) noexcept {
    TextResourceIdentity identity;
    const auto text = head(bytes, k_probe_bytes);
    if (text.empty() || !reads_as_text(text)) {
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

    return identity;
}

} // namespace dmc::rengine::profiles::dmc3
