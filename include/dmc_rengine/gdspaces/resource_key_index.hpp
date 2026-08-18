#pragma once

#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct ResourceKeyLookup final {
    std::string key;
    std::vector<ResourceRef> matches;

    [[nodiscard]] bool found() const noexcept { return !matches.empty(); }
    [[nodiscard]] bool unique() const noexcept { return matches.size() == 1U; }
    [[nodiscard]] bool ambiguous() const noexcept { return matches.size() > 1U; }
};

struct ResourceKeyIndexStats final {
    std::size_t indexed_resources{};
    std::size_t key_count{};
    std::size_t ambiguous_keys{};
};

// Derived lookup representation over immutable ResourceRef identities from one
// exact source/mount authority. Normalization is supplied by profile code; the
// index never chooses a winner among equal keys and never flattens sources.
class ResourceKeyIndex final {
public:
    explicit ResourceKeyIndex(std::string source_id);

    [[nodiscard]] std::string_view source_id() const noexcept;
    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] bool add(std::string key, ResourceRef resource);
    [[nodiscard]] ResourceKeyLookup lookup(std::string_view key) const;
    [[nodiscard]] ResourceKeyIndexStats stats() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::string source_id_;
    std::map<std::string, std::vector<ResourceRef>, std::less<>> entries_;
    std::map<std::string, std::string, std::less<>> resource_keys_;
    std::size_t indexed_resources_{};
};

} // namespace dmc::rengine::gdspaces
