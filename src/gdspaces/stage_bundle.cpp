#include "dmc_rengine/gdspaces/stage_bundle.hpp"

#include <algorithm>
#include <utility>

namespace dmc::rengine::gdspaces {

StageBundle::StageBundle(StageIdentity identity)
    : identity_(std::move(identity)) {}

const StageIdentity& StageBundle::identity() const noexcept {
    return identity_;
}

bool StageBundle::add(StageMember member) {
    if (!member.valid()) {
        return false;
    }

    const auto key = member.resource.id.canonical();
    const auto duplicate = std::find_if(
        members_.begin(), members_.end(),
        [&key](const StageMember& existing) {
            return existing.resource.id.canonical() == key;
        });

    if (duplicate != members_.end()) {
        return false;
    }

    members_.push_back(std::move(member));
    return true;
}

void StageBundle::add_diagnostic(Diagnostic diagnostic) {
    if (diagnostic.valid()) {
        diagnostics_.push_back(std::move(diagnostic));
    }
}

std::vector<const StageMember*> StageBundle::members(
    StageResourceCategory category) const {
    std::vector<const StageMember*> result;
    for (const auto& member : members_) {
        if (member.category == category) {
            result.push_back(&member);
        }
    }
    return result;
}

const std::vector<StageMember>& StageBundle::all_members() const noexcept {
    return members_;
}

const std::vector<Diagnostic>& StageBundle::diagnostics() const noexcept {
    return diagnostics_;
}

std::size_t StageBundle::size() const noexcept {
    return members_.size();
}

bool StageBundle::valid() const noexcept {
    if (!identity_.valid()) {
        return false;
    }

    return std::none_of(
        diagnostics_.begin(), diagnostics_.end(),
        [](const Diagnostic& diagnostic) {
            return diagnostic.severity == DiagnosticSeverity::error;
        });
}

} // namespace dmc::rengine::gdspaces
