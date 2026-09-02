#include "dmc_rengine/formats/container_parser_registry.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace dmc::rengine::formats {

bool ContainerParserRegistry::register_parser(
    std::unique_ptr<IContainerParser> parser) {
    if (!parser || parser->id().empty() || parser->format().empty() ||
        find_by_id(parser->id()) != nullptr) {
        return false;
    }

    parsers_.push_back(std::move(parser));
    return true;
}

const IContainerParser* ContainerParserRegistry::find_by_id(
    std::string_view parser_id) const noexcept {
    const auto iterator = std::find_if(
        parsers_.begin(), parsers_.end(),
        [parser_id](const std::unique_ptr<IContainerParser>& parser) {
            return parser->id() == parser_id;
        });

    return iterator == parsers_.end() ? nullptr : iterator->get();
}

const IContainerParser* ContainerParserRegistry::select(
    std::span<const std::byte> bytes,
    std::string_view logical_path) const noexcept {
    const IContainerParser* selected = nullptr;
    int selected_score = 0;

    for (const auto& parser : parsers_) {
        const auto score = parser->probe(bytes, logical_path);
        if (score > selected_score) {
            selected = parser.get();
            selected_score = score;
        }
    }

    return selected;
}

const IContainerParser* ContainerParserRegistry::named_by_path(
    std::string_view logical_path) const noexcept {
    const auto dot = logical_path.rfind('.');
    if (dot == std::string_view::npos || dot + 1U >= logical_path.size()) {
        return nullptr;
    }
    const auto extension = logical_path.substr(dot + 1U);
    if (extension.size() > 8U) {
        return nullptr;
    }

    std::array<char, 8U> lowered{};
    for (std::size_t index = 0U; index < extension.size(); ++index) {
        const auto value = static_cast<unsigned char>(extension[index]);
        lowered[index] = static_cast<char>(
            (value >= 'A' && value <= 'Z') ? value - 'A' + 'a' : value);
    }
    const std::string_view key{lowered.data(), extension.size()};

    for (const auto& parser : parsers_) {
        if (parser->format() == key) {
            return parser.get();
        }
    }
    return nullptr;
}

ContainerParseResult ContainerParserRegistry::parse(
    std::span<const std::byte> bytes,
    std::string_view logical_path) const {
    const auto* parser = select(bytes, logical_path);
    if (parser != nullptr) {
        return parser->parse(bytes, logical_path);
    }

    // Nothing recognized these bytes. Before reporting that, ask the parser the
    // resource's own name points at — it is the one that can say what it
    // wanted and did not find. Its answer replaces a shrug with a reason, and
    // the extra diagnostic below records that the claim came from the name.
    if (const auto* named = named_by_path(logical_path); named != nullptr) {
        auto result = named->parse(bytes, logical_path);
        result.recognized = false;
        result.document = ContainerDocument{};
        result.diagnostics.push_back(ParseDiagnostic{
            .severity = ParseSeverity::warning,
            .code = "container.named_but_not_recognized",
            .message =
                "The resource's name claims a container format that its bytes "
                "do not match. The name is not evidence about the bytes.",
            .offset = 0,
        });
        return result;
    }

    ContainerParseResult result;
    result.diagnostics.push_back(ParseDiagnostic{
        .severity = ParseSeverity::warning,
        .code = "container.no_parser",
        .message = "No registered container parser recognized the resource.",
        .offset = 0,
    });
    return result;
}

std::size_t ContainerParserRegistry::size() const noexcept {
    return parsers_.size();
}

} // namespace dmc::rengine::formats
