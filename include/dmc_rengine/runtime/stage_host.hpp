#pragma once

#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/runtime/render_device.hpp"
#include "dmc_rengine/runtime/resource_bridge.hpp"
#include "dmc_rengine/runtime/status.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace dmc::rengine::runtime {

/// Geometry a caller has already derived for a stage member.
///
/// The runtime never invents these numbers. Turning MOD/SCM bytes into
/// geometry is upstream work; until an owner supplies a binding, the member
/// simply does not draw.
struct GeometryBinding final {
    std::uint64_t geometry_key{};
    std::uint64_t material_key{};
    std::uint32_t index_count{};

    [[nodiscard]] bool valid() const noexcept {
        return geometry_key != 0U && index_count > 0U;
    }

    friend bool operator==(const GeometryBinding&, const GeometryBinding&) = default;
};

struct StageSlot final {
    gdspaces::StageResourceCategory category{gdspaces::StageResourceCategory::unknown};
    gdspaces::ResourceRef resource{};
    std::string role{};
    bool resident{false};

    friend bool operator==(const StageSlot&, const StageSlot&) = default;
};

struct PreloadReport final {
    std::uint32_t requested{};
    std::uint32_t loaded{};
    std::uint32_t failed{};

    friend bool operator==(const PreloadReport&, const PreloadReport&) = default;
};

/// Runtime mirror of a Stage Ops `StageBundle`.
///
/// It holds no scene truth of its own (Architecture rule 5): the slot list is
/// a straight projection of the bound bundle, in bundle order, and rebinding
/// replaces it wholesale rather than merging.
class StageHost final {
public:
    explicit StageHost(ResourceBridge& bridge) noexcept;

    [[nodiscard]] Status bind(const gdspaces::StageBundle& bundle);
    void release() noexcept;

    [[nodiscard]] bool bound() const noexcept;
    [[nodiscard]] const gdspaces::StageIdentity& identity() const noexcept;
    [[nodiscard]] std::span<const StageSlot> slots() const noexcept;
    [[nodiscard]] std::vector<const StageSlot*> slots_in(
        gdspaces::StageResourceCategory category) const;

    /// Reads every slot of a category through the resource bridge.
    [[nodiscard]] Expected<PreloadReport> preload(gdspaces::StageResourceCategory category);

    [[nodiscard]] Status bind_geometry(const gdspaces::ResourceId& id, GeometryBinding binding);
    [[nodiscard]] std::size_t geometry_bindings() const noexcept;

    /// Deterministic draw list for resident model slots that have a geometry
    /// binding. Order follows the bundle, so two identical bundles always
    /// produce an identical list.
    [[nodiscard]] DrawList build_draw_list() const;

private:
    ResourceBridge* bridge_{};
    gdspaces::StageIdentity identity_{};
    std::vector<StageSlot> slots_{};
    std::map<std::string, GeometryBinding, std::less<>> geometry_{};
    bool bound_{false};
};

} // namespace dmc::rengine::runtime
