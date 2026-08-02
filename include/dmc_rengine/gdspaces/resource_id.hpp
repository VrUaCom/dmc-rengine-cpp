#pragma once

#include <cstdint>
#include <string>

namespace dmc::rengine::gdspaces {

struct ResourceId final {
    std::string source_id;
    std::string logical_path;
    std::string container_chain;
    std::uint64_t offset{};
    std::uint64_t size{};

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::string canonical() const;

    friend bool operator==(const ResourceId&, const ResourceId&) = default;
};

} // namespace dmc::rengine::gdspaces
