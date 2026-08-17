#pragma once

#include "dmc_rengine/formats/container.hpp"

#include <cstddef>
#include <span>
#include <string_view>

namespace dmc::rengine::formats {

class IContainerParser {
public:
    virtual ~IContainerParser() = default;

    [[nodiscard]] virtual std::string_view id() const noexcept = 0;
    [[nodiscard]] virtual std::string_view format() const noexcept = 0;

    // Explicit opt-in for parse-result reuse by immutable byte identity.
    // Returning true means parse() is structurally pure with respect to the
    // supplied bytes: identical bytes produce the same ContainerParseResult
    // regardless of logical_path/context. Default false prevents a future
    // context-sensitive parser from being cached accidentally.
    [[nodiscard]] virtual bool supports_byte_identity_reuse() const noexcept {
        return false;
    }

    // Probe scores are intentionally coarse: 0 = no match, 100 = exact magic.
    [[nodiscard]] virtual int probe(
        std::span<const std::byte> bytes,
        std::string_view logical_path) const noexcept = 0;

    [[nodiscard]] virtual ContainerParseResult parse(
        std::span<const std::byte> bytes,
        std::string_view logical_path) const = 0;
};

} // namespace dmc::rengine::formats
