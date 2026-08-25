#include "dmc_rengine/gdspaces/resource_key_index.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::gdspaces {

bool ResourceKeyIndexBuildReceipt::valid() const noexcept {
    if (source_id.empty()) {
        return false;
    }

    return indexed_count + rejected_invalid_resource_count +
            rejected_source_mismatch_count + rejected_c_string_count +
            rejected_empty_key_count + duplicate_identity_count == input_count;
}

ResourceKeyIndex ResourceKeyIndex::build(
    std::string source_id,
    std::uint32_t normalization_flags,
    std::span<const ResourceRef> resources) {
    ResourceKeyIndex result;
    result.source_id_ = std::move(source_id);
    result.normalization_flags_ = normalization_flags;
    result.receipt_ = ResourceKeyIndexBuildReceipt{
        .source_id = result.source_id_,
        .normalization_flags = normalization_flags,
        .input_count = resources.size(),
    };

    if (result.source_id_.empty()) {
        return result;
    }

    result.entries_.reserve(resources.size());
    for (const auto& resource : resources) {
        if (!resource.valid()) {
            ++result.receipt_.rejected_invalid_resource_count;
            continue;
        }
        if (resource.id.source_id != result.source_id_) {
            ++result.receipt_.rejected_source_mismatch_count;
            continue;
        }
        if (!ResourcePathNormalizer::c_string_compatible(
                resource.id.logical_path)) {
            ++result.receipt_.rejected_c_string_count;
            continue;
        }

        auto key = ResourcePathNormalizer::normalize(
            resource.id.logical_path, normalization_flags);
        if (key.empty()) {
            ++result.receipt_.rejected_empty_key_count;
            continue;
        }

        result.entries_.push_back(Entry{
            .key = std::move(key),
            .identity_key = resource.id.canonical(),
            .resource = resource,
        });
    }

    // Product-deterministic ordering is used only for stable diagnostics and
    // equal-range lookup. identity_key is not an original-runtime tie-break and
    // never selects a winner among comparator-equal provider keys.
    std::sort(
        result.entries_.begin(), result.entries_.end(),
        [](const Entry& left, const Entry& right) {
            if (left.key != right.key) {
                return left.key < right.key;
            }
            return left.identity_key < right.identity_key;
        });

    const auto before_unique = result.entries_.size();
    result.entries_.erase(
        std::unique(
            result.entries_.begin(), result.entries_.end(),
            [](const Entry& left, const Entry& right) {
                return left.key == right.key &&
                    left.identity_key == right.identity_key;
            }),
        result.entries_.end());
    result.receipt_.duplicate_identity_count =
        before_unique - result.entries_.size();
    result.receipt_.indexed_count = result.entries_.size();

    for (std::size_t begin = 0U; begin < result.entries_.size();) {
        std::size_t end = begin + 1U;
        while (end < result.entries_.size() &&
               result.entries_[end].key == result.entries_[begin].key) {
            ++end;
        }
        if (end - begin > 1U) {
            ++result.receipt_.conflict_key_count;
        }
        begin = end;
    }

    return result;
}

ResourceKeyMatchReport ResourceKeyIndex::lookup(
    std::string_view provider_key) const {
    ResourceKeyMatchReport report{
        .provider_key = std::string{provider_key},
        .index_valid = valid(),
        .key_valid = false,
        .matches = {},
    };

    report.key_valid = !provider_key.empty() &&
        ResourcePathNormalizer::c_string_compatible(provider_key) &&
        ResourcePathNormalizer::normalize(
            provider_key, normalization_flags_) == provider_key;
    if (!report.index_valid || !report.key_valid) {
        return report;
    }

    auto iterator = std::lower_bound(
        entries_.begin(), entries_.end(), provider_key,
        [](const Entry& entry, std::string_view key) {
            return entry.key < key;
        });

    while (iterator != entries_.end() && iterator->key == provider_key) {
        report.matches.push_back(iterator->resource);
        ++iterator;
    }
    return report;
}

std::vector<ResourceKeyConflict> ResourceKeyIndex::conflicts() const {
    std::vector<ResourceKeyConflict> result;
    if (!valid() || receipt_.conflict_key_count == 0U) {
        return result;
    }

    result.reserve(receipt_.conflict_key_count);
    for (std::size_t begin = 0U; begin < entries_.size();) {
        std::size_t end = begin + 1U;
        while (end < entries_.size() && entries_[end].key == entries_[begin].key) {
            ++end;
        }

        if (end - begin > 1U) {
            ResourceKeyConflict conflict{
                .provider_key = entries_[begin].key,
                .matches = {},
            };
            conflict.matches.reserve(end - begin);
            for (std::size_t index = begin; index < end; ++index) {
                conflict.matches.push_back(entries_[index].resource);
            }
            result.push_back(std::move(conflict));
        }
        begin = end;
    }

    return result;
}

bool ResourceKeyIndex::valid() const noexcept {
    return receipt_.valid() &&
        receipt_.source_id == source_id_ &&
        receipt_.normalization_flags == normalization_flags_ &&
        receipt_.indexed_count == entries_.size();
}

std::string_view ResourceKeyIndex::source_id() const noexcept {
    return source_id_;
}

std::uint32_t ResourceKeyIndex::normalization_flags() const noexcept {
    return normalization_flags_;
}

const ResourceKeyIndexBuildReceipt& ResourceKeyIndex::receipt() const noexcept {
    return receipt_;
}

} // namespace dmc::rengine::gdspaces
