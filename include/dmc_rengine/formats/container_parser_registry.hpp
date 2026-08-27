#pragma once

#include "dmc_rengine/formats/container_parser.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace dmc::rengine::formats {

class ContainerParserRegistry final {
public:
    [[nodiscard]] bool register_parser(
        std::unique_ptr<IContainerParser> parser);

    [[nodiscard]] const IContainerParser* find_by_id(
        std::string_view parser_id) const noexcept;

    [[nodiscard]] const IContainerParser* select(
        std::span<const std::byte> bytes,
        std::string_view logical_path) const noexcept;

    // The parser whose format matches the resource's own extension, whether or
    // not it would accept these bytes.
    //
    // This exists so a refusal can say why. A parser that does not recognize a
    // byte image scores zero and is never selected, so the registry's answer
    // for `at.ptx` was "no parser recognized this" — true, useless, and
    // indistinguishable from "this file is of no known kind". The parser the
    // name points at knows the actual reason; asking it costs one parse and
    // turns a shrug into a diagnosis.
    [[nodiscard]] const IContainerParser* named_by_path(
        std::string_view logical_path) const noexcept;

    [[nodiscard]] ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view logical_path) const;

    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<std::unique_ptr<IContainerParser>> parsers_;
};

} // namespace dmc::rengine::formats
