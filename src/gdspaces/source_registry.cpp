#include "dmc_rengine/gdspaces/source_registry.hpp"

#include "dmc_rengine/gdspaces/resource_path_normalizer.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

namespace dmc::rengine::gdspaces {

bool SourceRegistry::mount(std::unique_ptr<ISource> source) {
    if (!source || source->id().empty() || find(source->id()) != nullptr) {
        return false;
    }

    sources_.push_back(std::move(source));
    return true;
}

const ISource* SourceRegistry::find(std::string_view source_id) const noexcept {
    const auto iterator = std::find_if(
        sources_.begin(), sources_.end(),
        [source_id](const std::unique_ptr<ISource>& source) {
            return source->id() == source_id;
        });

    return iterator == sources_.end() ? nullptr : iterator->get();
}

std::vector<ResourceRef> SourceRegistry::enumerate_all() const {
    std::vector<ResourceRef> result;
    for (const auto& source : sources_) {
        auto resources = source->enumerate();
        result.insert(
            result.end(),
            std::make_move_iterator(resources.begin()),
            std::make_move_iterator(resources.end()));
    }

    std::sort(
        result.begin(), result.end(),
        [](const ResourceRef& left, const ResourceRef& right) {
            return left.id.canonical() < right.id.canonical();
        });

    return result;
}

SourceLookupReport SourceRegistry::lookup(
    std::string_view source_id,
    std::string_view provider_key,
    std::uint32_t normalization_flags) const {
    SourceLookupReport report{
        .source_id = std::string{source_id},
        .provider_key = std::string{provider_key},
        .normalization_flags = normalization_flags,
        .provider_key_valid = false,
        .source_available = false,
        .matches = {},
    };

    report.provider_key_valid =
        !provider_key.empty() &&
        ResourcePathNormalizer::c_string_compatible(provider_key) &&
        ResourcePathNormalizer::normalize(provider_key, normalization_flags) ==
            provider_key;

    const auto* source = find(source_id);
    if (source == nullptr) {
        return report;
    }
    report.source_available = true;

    if (!report.provider_key_valid) {
        return report;
    }

    report.matches = source->lookup(provider_key, normalization_flags);
    return report;
}

std::optional<ResourcePayload> SourceRegistry::read(
    const ResourceId& resource) const {
    const auto* source = find(resource.source_id);
    return source == nullptr ? std::nullopt : source->read(resource);
}

std::size_t SourceRegistry::size() const noexcept {
    return sources_.size();
}

bool SourceRegistry::empty() const noexcept {
    return sources_.empty();
}

} // namespace dmc::rengine::gdspaces
