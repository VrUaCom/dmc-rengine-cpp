#include "dmc_rengine/gdspaces/resource_key_index.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace dmc::rengine::gdspaces {

bool ResourceKeyIndex::add(std::string key, ResourceRef resource) {
    if (key.empty() || !resource.valid()) {
        return false;
    }

    const auto identity = resource.id.canonical();
    const auto existing_key = resource_keys_.find(identity);
    if (existing_key != resource_keys_.end()) {
        // One immutable ResourceId belongs to exactly one key domain inside a
        // single index. Re-adding it, whether under the same or another key,
        // is a caller/build error rather than a second resource identity.
        return false;
    }

    auto& bucket = entries_[key];
    const auto position = std::lower_bound(
        bucket.begin(), bucket.end(), identity,
        [](const ResourceRef& candidate, const std::string& canonical) {
            return candidate.id.canonical() < canonical;
        });
    bucket.insert(position, std::move(resource));
    resource_keys_.emplace(identity, std::move(key));
    ++indexed_resources_;
    return true;
}

ResourceKeyLookup ResourceKeyIndex::lookup(std::string_view key) const {
    ResourceKeyLookup result{
        .key = std::string{key},
        .matches = {},
    };
    if (key.empty()) {
        return result;
    }

    const auto iterator = entries_.find(key);
    if (iterator != entries_.end()) {
        result.matches = iterator->second;
    }
    return result;
}

ResourceKeyIndexStats ResourceKeyIndex::stats() const noexcept {
    ResourceKeyIndexStats result{
        .indexed_resources = indexed_resources_,
        .key_count = entries_.size(),
        .ambiguous_keys = 0U,
    };
    for (const auto& [key, resources] : entries_) {
        static_cast<void>(key);
        if (resources.size() > 1U) {
            ++result.ambiguous_keys;
        }
    }
    return result;
}

bool ResourceKeyIndex::empty() const noexcept {
    return indexed_resources_ == 0U;
}

} // namespace dmc::rengine::gdspaces
