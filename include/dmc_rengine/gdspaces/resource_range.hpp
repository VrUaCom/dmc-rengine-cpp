#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ResourceRangeRequest final {
    ResourceId resource;
    std::uint64_t offset{};
    std::uint64_t size{};

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && size != 0U &&
               offset <= resource.size && size <= resource.size - offset;
    }
};

struct ResourceRangePayload final {
    ResourceRef resource;
    std::uint64_t offset{};
    std::uint64_t requested_size{};
    std::vector<std::byte> bytes;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool valid() const noexcept {
        return resource.valid() && requested_size != 0U &&
               offset <= resource.id.size &&
               requested_size <= resource.id.size - offset &&
               bytes.size() <= requested_size;
    }

    [[nodiscard]] bool exact() const noexcept {
        return valid() && bytes.size() == requested_size;
    }

    [[nodiscard]] bool readable() const noexcept;
};

} // namespace dmc::rengine::gdspaces
