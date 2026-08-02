#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <algorithm>
#include <iterator>
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
