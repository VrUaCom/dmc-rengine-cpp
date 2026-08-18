#pragma once

#include "dmc_rengine/gdspaces/source.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct SourceLookupReport final {
    std::string source_id;
    std::string provider_key;
    std::uint32_t normalization_flags{};
    bool provider_key_valid{false};
    bool source_available{false};
    std::vector<ResourceRef> matches;

    [[nodiscard]] bool found() const noexcept {
        return provider_key_valid && source_available && !matches.empty();
    }

    [[nodiscard]] bool unique() const noexcept {
        return provider_key_valid && source_available && matches.size() == 1U;
    }

    [[nodiscard]] bool ambiguous() const noexcept {
        return provider_key_valid && source_available && matches.size() > 1U;
    }
};

class SourceRegistry final {
public:
    [[nodiscard]] bool mount(std::unique_ptr<ISource> source);
    [[nodiscard]] const ISource* find(std::string_view source_id) const noexcept;
    [[nodiscard]] std::vector<ResourceRef> enumerate_all() const;

    // Query exactly one mounted source with an already-normalized provider key.
    // Missing source, invalid key, no hit, unique hit and ambiguous hit remain
    // distinguishable in the returned report.
    [[nodiscard]] SourceLookupReport lookup(
        std::string_view source_id,
        std::string_view provider_key,
        std::uint32_t normalization_flags) const;

    [[nodiscard]] std::optional<ResourcePayload> read(
        const ResourceId& resource) const;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::vector<std::unique_ptr<ISource>> sources_;
};

} // namespace dmc::rengine::gdspaces
