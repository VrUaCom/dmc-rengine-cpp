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

    [[nodiscard]] bool found() const noexcept {
        return !matches.empty();
    }

    [[nodiscard]] bool unique() const noexcept {
        return matches.size() == 1U;
    }

    [[nodiscard]] bool ambiguous() const noexcept {
        return matches.size() > 1U;
    }
};

struct ResourceKeyIndexStats final {
    std::size_t indexed_resources{};
    std::size_t unique_keys{};
    std::size_t ambiguous_keys{};
};

// Derived lookup representation over immutable ResourceRef identities.
//
// The index intentionally does not know how keys are normalized and never
// chooses a winner among equal keys. Profile/provider code supplies the key;
// callers receive every distinct ResourceRef mapped to that key.
class ResourceKeyIndex final {
public:
    // Returns false for invalid input, duplicate ResourceId insertion, or an
    // attempt to assign one ResourceId to two different keys in this index.
    [[nodiscard]] bool add(std::string key, ResourceRef resource);

    [[nodiscard]] ResourceKeyLookup lookup(std::string_view key) const;
    [[nodiscard]] ResourceKeyIndexStats stats() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::map<std::string, std::vector<ResourceRef>, std::less<>> entries_;
    std::map<std::string, std::string, std::less<>> resource_keys_;
    std::size_t indexed_resources_{};
};

} // namespace dmc::rengine::gdspaces
