#include "dmc_rengine/formats/clt.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::formats::clt {
namespace {

[[nodiscard]] ParseResult fail(ParseError error, std::string message) {
    return ParseResult{
        .document = std::nullopt,
        .error = error,
        .message = std::move(message),
    };
}

[[nodiscard]] std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r");
    return value.substr(first, last - first + 1U);
}

[[nodiscard]] SourceLine decode_line(std::string raw, std::size_t source_line) {
    SourceLine line{
        .source_line = source_line,
        .raw = raw,
    };
    const auto cleaned = trim(std::move(raw));
    if (cleaned.empty()) {
        return line;
    }
    if (cleaned.front() == ';') {
        line.comment = true;
        line.value = cleaned.substr(1U);
        return line;
    }
    if (cleaned == "$" || cleaned == "End") {
        line.key = cleaned;
        return line;
    }
    const auto separator = cleaned.find_first_of(" \t");
    if (separator == std::string::npos) {
        line.key = cleaned;
        return line;
    }
    line.key = cleaned.substr(0U, separator);
    line.value = trim(cleaned.substr(separator + 1U));
    return line;
}

} // namespace

bool Document::valid() const noexcept {
    if (physical_size == 0U || text_size == 0U || embedded_name.empty()) {
        return false;
    }
    return std::any_of(
        lines.begin(), lines.end(),
        [](const SourceLine& line) { return line.key == "$"; });
}

ParseResult Reader::parse(std::span<const std::byte> bytes) {
    if (bytes.empty()) {
        return fail(ParseError::empty, "CLT resource is empty");
    }

    std::size_t text_size = bytes.size();
    for (std::size_t i = 0U; i < bytes.size(); ++i) {
        if (std::to_integer<unsigned int>(bytes[i]) == 0U) {
            text_size = i;
            break;
        }
        const auto value = std::to_integer<unsigned int>(bytes[i]);
        if (value != '\r' && value != '\n' && value != '\t' &&
            (value < 0x20U || value > 0x7EU)) {
            return fail(
                ParseError::non_text_byte,
                "CLT textual extent contains a non-ASCII byte");
        }
    }

    if (text_size == 0U) {
        return fail(ParseError::empty, "CLT textual extent is empty");
    }
    for (std::size_t i = text_size; i < bytes.size(); ++i) {
        if (std::to_integer<unsigned int>(bytes[i]) != 0U) {
            return fail(
                ParseError::nonzero_tail,
                "CLT bytes after the first NUL are not zero padding");
        }
    }

    std::string text;
    text.reserve(text_size);
    for (std::size_t i = 0U; i < text_size; ++i) {
        text.push_back(static_cast<char>(std::to_integer<unsigned int>(bytes[i])));
    }

    Document document{
        .physical_size = bytes.size(),
        .text_size = text_size,
        .zero_padding_size = bytes.size() - text_size,
    };

    std::size_t at = 0U;
    std::size_t source_line = 0U;
    while (at <= text.size()) {
        ++source_line;
        auto end = text.find('\n', at);
        if (end == std::string::npos) {
            end = text.size();
        }
        auto raw = text.substr(at, end - at);
        auto line = decode_line(raw, source_line);
        if (line.comment) {
            const auto candidate = trim(line.value);
            if (candidate.size() >= 4U &&
                candidate.rfind(".clt") == candidate.size() - 4U) {
                document.embedded_name = candidate;
            }
        }
        document.lines.push_back(std::move(line));
        if (end == text.size()) {
            break;
        }
        at = end + 1U;
    }

    if (document.embedded_name.empty()) {
        return fail(
            ParseError::missing_clt_identity,
            "CLT corpus identity line ';*.clt' was not found");
    }
    const bool terminator = std::any_of(
        document.lines.begin(), document.lines.end(),
        [](const SourceLine& line) { return line.key == "$"; });
    if (!terminator) {
        return fail(ParseError::missing_terminator, "CLT '$' terminator is absent");
    }

    return ParseResult{
        .document = std::move(document),
        .error = ParseError::none,
        .message = {},
    };
}

bool Reader::structurally_valid(std::span<const std::byte> bytes) noexcept {
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats::clt
