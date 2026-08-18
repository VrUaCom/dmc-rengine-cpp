#pragma once

#include "dmc_rengine/gdspaces/source.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

class SourceRegistry final {
public:
    [[nodiscard]] bool mount(std::unique_ptr<ISource> source);
    [[nodiscard]] const ISource* find(std::string_view source_id) const noexcept;
    [[nodiscard]] std::vector<ResourceRef> enumerate_all() const;
    [[nodiscard]] std::optional<ResourcePayload> read(
        const ResourceId& resource) const;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::vector<std::unique_ptr<ISource>> sources_;
};

} // namespace dmc::rengine::gdspaces
