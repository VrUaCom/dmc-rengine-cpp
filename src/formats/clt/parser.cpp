#include "dmc_rengine/formats/clt/parser.hpp"

#include "dmc_rengine/formats/clt/abi.hpp"

#include <algorithm>
#include <charconv>
#include <string_view>
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
    if (cleaned.front() == TextAbi::comment_prefix) {
        line.comment = true;
        line.value = cleaned.substr(1U);
        return line;
    }
    if (cleaned == TextAbi::terminator || cleaned == TextAbi::record_end) {
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

[[nodiscard]] std::optional<std::uint32_t> parse_u32_token(
    std::string_view text) noexcept {
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string_view::npos) {
        return std::nullopt;
    }
    text.remove_prefix(first);
    const auto end = text.find_first_of(" \t");
    const auto token = text.substr(0U, end);
    std::uint32_t value = 0U;
    const auto result = std::from_chars(token.data(), token.data() + token.size(), value);
    if (result.ec != std::errc{} || result.ptr != token.data() + token.size()) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] std::optional<BoneDirective> parse_bone(
    const SourceLine& line) {
    auto text = std::string_view{line.value};
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string_view::npos) {
        return std::nullopt;
    }
    text.remove_prefix(first);
    const auto separator = text.find_first_of(" \t");
    if (separator == std::string_view::npos) {
        return std::nullopt;
    }
    const auto index_token = text.substr(0U, separator);
    std::uint32_t node_index = 0U;
    const auto parsed = std::from_chars(
        index_token.data(), index_token.data() + index_token.size(), node_index);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != index_token.data() + index_token.size()) {
        return std::nullopt;
    }
    text.remove_prefix(separator);
    const auto axis_first = text.find_first_not_of(" \t");
    if (axis_first == std::string_view::npos) {
        return std::nullopt;
    }
    text.remove_prefix(axis_first);
    const auto axis_end = text.find_first_of(" \t");
    const auto axis = text.substr(0U, axis_end);
    if (axis.empty()) {
        return std::nullopt;
    }
    return BoneDirective{
        .source_line = line.source_line,
        .node_index = node_index,
        .axis_token = std::string{axis},
    };
}

void project_integer_if_present(
    const SourceLine& line,
    std::string_view key,
    std::optional<std::uint32_t>& destination,
    bool& malformed) {
    if (line.key != key) {
        return;
    }
    const auto value = parse_u32_token(line.value);
    if (!value.has_value()) {
        malformed = true;
        return;
    }
    // Current em000 samples contain one record per CLT. If a wider corpus uses
    // repeated singleton-looking directives, raw SourceLine remains authority;
    // the first typed projection is intentionally not overwritten.
    if (!destination.has_value()) {
        destination = *value;
    }
}

} // namespace

bool Document::valid() const noexcept {
    if (source_bytes.size() != physical_size || physical_size == 0U ||
        text_size == 0U || embedded_name.empty()) {
        return false;
    }
    return std::any_of(
        lines.begin(), lines.end(),
        [](const SourceLine& line) { return line.key == TextAbi::terminator; });
}

ParseResult Parser::parse(std::span<const std::byte> bytes) {
    if (bytes.empty()) {
        return fail(ParseError::empty, "CLT resource is empty");
    }

    std::size_t text_size = bytes.size();
    for (std::size_t i = 0U; i < bytes.size(); ++i) {
        const auto value = std::to_integer<unsigned int>(bytes[i]);
        if (value == 0U) {
            text_size = i;
            break;
        }
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
        .source_bytes = std::vector<std::byte>(bytes.begin(), bytes.end()),
        .physical_size = bytes.size(),
        .text_size = text_size,
        .zero_padding_size = bytes.size() - text_size,
    };

    std::size_t at = 0U;
    std::size_t source_line = 0U;
    bool malformed_integer = false;
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
            if (candidate.size() >= TextAbi::embedded_extension.size() &&
                candidate.rfind(TextAbi::embedded_extension) ==
                    candidate.size() - TextAbi::embedded_extension.size()) {
                document.embedded_name = candidate;
            }
        }

        project_integer_if_present(
            line, TextAbi::key_cloth_num, document.cloth_num, malformed_integer);
        project_integer_if_present(
            line, TextAbi::key_cloth_no, document.cloth_no, malformed_integer);
        project_integer_if_present(
            line, TextAbi::key_cloth_id, document.cloth_id, malformed_integer);
        project_integer_if_present(
            line, TextAbi::key_wind_local, document.wind_local_raw, malformed_integer);
        project_integer_if_present(
            line, TextAbi::key_wind_parent, document.wind_parent_raw, malformed_integer);
        project_integer_if_present(
            line, TextAbi::key_wind_type, document.wind_type_raw, malformed_integer);
        project_integer_if_present(
            line, TextAbi::key_limit_length, document.limit_length_raw, malformed_integer);

        if (line.key == TextAbi::key_bone) {
            const auto bone = parse_bone(line);
            if (!bone.has_value()) {
                return fail(
                    ParseError::invalid_bone_directive,
                    "CLT Bone directive does not contain an integer index and axis token");
            }
            document.bones.push_back(*bone);
        }

        document.lines.push_back(std::move(line));
        if (end == text.size()) {
            break;
        }
        at = end + 1U;
    }

    if (malformed_integer) {
        return fail(
            ParseError::invalid_integer_directive,
            "CLT known integer directive contains a non-integer first token");
    }
    if (document.embedded_name.empty()) {
        return fail(
            ParseError::missing_clt_identity,
            "CLT corpus identity line ';*.clt' was not found");
    }
    const bool terminator = std::any_of(
        document.lines.begin(), document.lines.end(),
        [](const SourceLine& line) { return line.key == TextAbi::terminator; });
    if (!terminator) {
        return fail(ParseError::missing_terminator, "CLT '$' terminator is absent");
    }

    return ParseResult{
        .document = std::move(document),
        .error = ParseError::none,
        .message = {},
    };
}

bool Parser::structurally_valid(std::span<const std::byte> bytes) noexcept {
    return parse(bytes).ok();
}

} // namespace dmc::rengine::formats::clt
