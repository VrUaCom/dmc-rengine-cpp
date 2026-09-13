#include "dmc_rengine/runtime/stage_host.hpp"

#include <string>
#include <utility>

namespace dmc::rengine::runtime {

StageHost::StageHost(ResourceBridge& bridge) noexcept : bridge_(&bridge) {}

Status StageHost::bind(const gdspaces::StageBundle& bundle) {
    if (!bundle.valid()) {
        return fail(make_error(RuntimeErrorCode::invalid_argument, "stage-host",
                               "stage bundle is not valid"));
    }

    // A rebind replaces the mirror wholesale. Merging would create a second
    // scene truth that Stage Ops never authored.
    release();

    identity_ = bundle.identity();
    slots_.reserve(bundle.all_members().size());
    for (const auto& member : bundle.all_members()) {
        if (!member.valid()) {
            continue;
        }
        slots_.push_back(StageSlot{member.category, member.resource, member.role, false});
    }

    bound_ = true;
    return ok();
}

void StageHost::release() noexcept {
    identity_ = gdspaces::StageIdentity{};
    slots_.clear();
    geometry_.clear();
    bound_ = false;
}

bool StageHost::bound() const noexcept {
    return bound_;
}

const gdspaces::StageIdentity& StageHost::identity() const noexcept {
    return identity_;
}

std::span<const StageSlot> StageHost::slots() const noexcept {
    return std::span<const StageSlot>{slots_};
}

std::vector<const StageSlot*> StageHost::slots_in(
    gdspaces::StageResourceCategory category) const {
    std::vector<const StageSlot*> matched;
    for (const auto& slot : slots_) {
        if (slot.category == category) {
            matched.push_back(&slot);
        }
    }
    return matched;
}

Expected<PreloadReport> StageHost::preload(gdspaces::StageResourceCategory category) {
    if (!bound_) {
        return fail(make_error(RuntimeErrorCode::not_initialized, "stage-host",
                               "preload before bind"));
    }

    PreloadReport report{};
    for (auto& slot : slots_) {
        if (slot.category != category) {
            continue;
        }

        ++report.requested;
        if (bridge_->bytes(slot.resource.id).has_value()) {
            slot.resident = true;
            ++report.loaded;
        } else {
            slot.resident = false;
            ++report.failed;
        }
    }

    return report;
}

Status StageHost::bind_geometry(const gdspaces::ResourceId& id, GeometryBinding binding) {
    if (!id.valid()) {
        return fail(make_error(RuntimeErrorCode::invalid_argument, "stage-host",
                               "geometry binding needs a valid resource id"));
    }
    if (!binding.valid()) {
        return fail(make_error(RuntimeErrorCode::invalid_argument, "stage-host",
                               "geometry binding needs a key and a non-zero index count"));
    }

    geometry_[id.canonical()] = binding;
    return ok();
}

std::size_t StageHost::geometry_bindings() const noexcept {
    return geometry_.size();
}

DrawList StageHost::build_draw_list() const {
    DrawList draws;
    for (const auto& slot : slots_) {
        if (slot.category != gdspaces::StageResourceCategory::models || !slot.resident) {
            continue;
        }

        const auto binding = geometry_.find(slot.resource.id.canonical());
        if (binding == geometry_.end()) {
            continue;
        }

        draws.push_back(DrawCall{binding->second.geometry_key, binding->second.material_key,
                                 binding->second.index_count, slot.resource.display_name});
    }
    return draws;
}

} // namespace dmc::rengine::runtime
