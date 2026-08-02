#include "dmc_rengine/integration/workspace_event.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::integration {

std::uint64_t WorkspaceEventJournal::record(
    WorkspaceEventType type,
    const gdspaces::ResourceId& resource,
    gdspaces::ToolTarget producer,
    std::optional<gdspaces::ToolTarget> consumer,
    std::uint64_t revision,
    std::string subject_id,
    std::string message) {
    if (!resource.valid() || message.empty()) {
        return 0U;
    }

    const auto sequence = next_sequence_++;
    events_.push_back(WorkspaceEvent{
        .sequence = sequence,
        .type = type,
        .resource = resource,
        .producer = producer,
        .consumer = consumer,
        .revision = revision,
        .subject_id = std::move(subject_id),
        .message = std::move(message),
    });
    return sequence;
}

const std::vector<WorkspaceEvent>&
WorkspaceEventJournal::events() const noexcept {
    return events_;
}

std::vector<const WorkspaceEvent*> WorkspaceEventJournal::by_type(
    WorkspaceEventType type) const {
    std::vector<const WorkspaceEvent*> result;
    for (const auto& event : events_) {
        if (event.type == type) {
            result.push_back(&event);
        }
    }
    return result;
}

std::vector<const WorkspaceEvent*> WorkspaceEventJournal::by_tool(
    gdspaces::ToolTarget tool) const {
    std::vector<const WorkspaceEvent*> result;
    for (const auto& event : events_) {
        if (event.producer == tool ||
            (event.consumer.has_value() && *event.consumer == tool)) {
            result.push_back(&event);
        }
    }
    return result;
}

const WorkspaceEvent* WorkspaceEventJournal::latest() const noexcept {
    return events_.empty() ? nullptr : &events_.back();
}

std::size_t WorkspaceEventJournal::size() const noexcept {
    return events_.size();
}

bool WorkspaceEventJournal::empty() const noexcept {
    return events_.empty();
}

} // namespace dmc::rengine::integration
