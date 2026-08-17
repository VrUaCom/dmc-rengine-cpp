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

    // A byte-pure parser produces the same ContainerParseResult for identical
    // supplied bytes regardless of logical path/context. Generic callers may
    // use this explicit capability to reuse a parse result for alias resource
    // identities that share one immutable byte provenance span. The default is
    // deliberately false so future path/context-sensitive parsers cannot be
    // cached accidentally.
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
