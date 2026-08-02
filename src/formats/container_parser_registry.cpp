#include "dmc_rengine/formats/container_parser_registry.hpp"

#include <algorithm>
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

ContainerParseResult ContainerParserRegistry::parse(
    std::span<const std::byte> bytes,
    std::string_view logical_path) const {
    const auto* parser = select(bytes, logical_path);
    if (parser == nullptr) {
        ContainerParseResult result;
        result.diagnostics.push_back(ParseDiagnostic{
            .severity = ParseSeverity::warning,
            .code = "container.no_parser",
            .message = "No registered container parser recognized the resource.",
            .offset = 0,
        });
        return result;
    }

    return parser->parse(bytes, logical_path);
}

std::size_t ContainerParserRegistry::size() const noexcept {
    return parsers_.size();
}

} // namespace dmc::rengine::formats
