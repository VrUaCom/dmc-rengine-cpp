#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/gdspaces/stage_bundle.hpp"
#include "dmc_rengine/runtime/resource_bridge.hpp"
#include "dmc_rengine/runtime/stage_host.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using dmc::rengine::gdspaces::ISource;
using dmc::rengine::gdspaces::ResourceId;
using dmc::rengine::gdspaces::ResourcePayload;
using dmc::rengine::gdspaces::ResourceRef;
using dmc::rengine::gdspaces::SourceRegistry;
using dmc::rengine::gdspaces::StageBundle;
using dmc::rengine::gdspaces::StageIdentity;
using dmc::rengine::gdspaces::StageMember;
using dmc::rengine::gdspaces::StageResourceCategory;
using dmc::rengine::runtime::GeometryBinding;
using dmc::rengine::runtime::ResourceBridge;
using dmc::rengine::runtime::RuntimeErrorCode;
using dmc::rengine::runtime::StageHost;

/// Synthetic source that serves any resource it was told about.
class KnownSource final : public ISource {
public:
    void allow(const std::string& logical_path) { known_.insert(logical_path); }

    [[nodiscard]] std::string_view id() const noexcept override { return "stage-source"; }
    [[nodiscard]] std::string_view kind() const noexcept override { return "scripted"; }

    [[nodiscard]] std::vector<ResourceRef> enumerate() const override { return {}; }

    [[nodiscard]] std::optional<ResourcePayload> read(const ResourceId& resource) const override {
        if (known_.find(resource.logical_path) == known_.end()) {
            return std::nullopt;
        }

        ResourcePayload payload;
        payload.resource.id = resource;
        payload.resource.display_name = resource.logical_path;
        payload.resource.format = "synthetic";
        payload.bytes.assign(16U, std::byte{0x11});
        return payload;
    }

private:
    std::set<std::string> known_;
};

[[nodiscard]] ResourceRef make_ref(const std::string& logical_path) {
    ResourceRef ref;
    ref.id = ResourceId{"stage-source", logical_path, {}, 0U, 16U};
    ref.display_name = logical_path;
    ref.format = "synthetic";
    return ref;
}

struct Fixture final {
    SourceRegistry registry;
    KnownSource* source{};

    Fixture() {
        auto owned = std::make_unique<KnownSource>();
        source = owned.get();
        const bool mounted = registry.mount(std::move(owned));
        assert(mounted);
    }

    [[nodiscard]] StageBundle bundle() const {
        StageBundle built{StageIdentity{"dmc3", "st001", "Stage 001", "evidence-1"}};
        const bool a = built.add(StageMember{StageResourceCategory::models,
                                             make_ref("st001/hero.mod"), "hero"});
        const bool b = built.add(StageMember{StageResourceCategory::models,
                                             make_ref("st001/prop.mod"), "prop"});
        const bool c = built.add(StageMember{StageResourceCategory::textures,
                                             make_ref("st001/atlas.ptx"), "atlas"});
        assert(a && b && c);
        return built;
    }
};

void binding_mirrors_the_bundle_in_order() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};

    assert(!host.bound());
    assert(host.bind(fixture.bundle()).has_value());
    assert(host.bound());
    assert(host.identity().stage_id == "st001");
    assert(host.slots().size() == 3U);
    assert(host.slots()[0].role == "hero");
    assert(host.slots()[2].category == StageResourceCategory::textures);
    assert(host.slots_in(StageResourceCategory::models).size() == 2U);
    assert(host.slots_in(StageResourceCategory::sounds).empty());
}

void an_invalid_bundle_is_refused() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};

    const StageBundle nameless{StageIdentity{}};
    const auto result = host.bind(nameless);
    assert(!result.has_value());
    assert(result.error().code == RuntimeErrorCode::invalid_argument);
    assert(!host.bound());
}

void preload_reports_per_category_success_and_failure() {
    Fixture fixture;
    fixture.source->allow("st001/hero.mod");
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};
    assert(host.bind(fixture.bundle()).has_value());

    const auto models = host.preload(StageResourceCategory::models);
    assert(models.has_value());
    assert(models->requested == 2U);
    assert(models->loaded == 1U);
    assert(models->failed == 1U);

    assert(host.slots()[0].resident);
    assert(!host.slots()[1].resident);

    // The texture slot was never requested, so it stays non-resident.
    assert(!host.slots()[2].resident);
}

void preload_before_bind_is_refused() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};

    const auto result = host.preload(StageResourceCategory::models);
    assert(!result.has_value());
    assert(result.error().code == RuntimeErrorCode::not_initialized);
}

void draw_list_needs_residency_and_a_geometry_binding() {
    Fixture fixture;
    fixture.source->allow("st001/hero.mod");
    fixture.source->allow("st001/prop.mod");
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};
    assert(host.bind(fixture.bundle()).has_value());

    // Resident but unbound geometry draws nothing: the runtime never invents
    // an index count.
    assert(host.preload(StageResourceCategory::models).has_value());
    assert(host.build_draw_list().empty());

    assert(host.bind_geometry(make_ref("st001/hero.mod").id, GeometryBinding{7U, 3U, 120U})
               .has_value());
    assert(host.geometry_bindings() == 1U);

    const auto draws = host.build_draw_list();
    assert(draws.size() == 1U);
    assert(draws[0].geometry_key == 7U);
    assert(draws[0].material_key == 3U);
    assert(draws[0].index_count == 120U);
    assert(draws[0].debug_name == "st001/hero.mod");
}

void draw_list_order_is_deterministic() {
    Fixture fixture;
    fixture.source->allow("st001/hero.mod");
    fixture.source->allow("st001/prop.mod");
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};
    assert(host.bind(fixture.bundle()).has_value());
    assert(host.preload(StageResourceCategory::models).has_value());

    // Bind in reverse bundle order; the draw list must still follow the bundle.
    assert(host.bind_geometry(make_ref("st001/prop.mod").id, GeometryBinding{2U, 0U, 30U})
               .has_value());
    assert(host.bind_geometry(make_ref("st001/hero.mod").id, GeometryBinding{1U, 0U, 60U})
               .has_value());

    const auto first = host.build_draw_list();
    const auto second = host.build_draw_list();
    assert(first == second);
    assert(first.size() == 2U);
    assert(first[0].geometry_key == 1U);
    assert(first[1].geometry_key == 2U);
}

void malformed_geometry_bindings_are_refused() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};

    const auto no_id = host.bind_geometry(ResourceId{}, GeometryBinding{1U, 0U, 3U});
    assert(!no_id.has_value());
    assert(no_id.error().code == RuntimeErrorCode::invalid_argument);

    const auto no_indices =
        host.bind_geometry(make_ref("st001/hero.mod").id, GeometryBinding{1U, 0U, 0U});
    assert(!no_indices.has_value());
    assert(host.geometry_bindings() == 0U);
}

void rebinding_replaces_rather_than_merges() {
    Fixture fixture;
    fixture.source->allow("st001/hero.mod");
    ResourceBridge bridge{fixture.registry};
    StageHost host{bridge};
    assert(host.bind(fixture.bundle()).has_value());
    assert(host.preload(StageResourceCategory::models).has_value());
    assert(host.bind_geometry(make_ref("st001/hero.mod").id, GeometryBinding{1U, 0U, 60U})
               .has_value());
    assert(host.build_draw_list().size() == 1U);

    StageBundle smaller{StageIdentity{"dmc3", "st002", "Stage 002", "evidence-2"}};
    const bool added =
        smaller.add(StageMember{StageResourceCategory::models, make_ref("st002/only.mod"), "only"});
    assert(added);
    assert(host.bind(smaller).has_value());

    assert(host.identity().stage_id == "st002");
    assert(host.slots().size() == 1U);
    assert(host.geometry_bindings() == 0U);
    assert(host.build_draw_list().empty());

    host.release();
    assert(!host.bound());
    assert(host.slots().empty());
}

} // namespace

int main() {
    binding_mirrors_the_bundle_in_order();
    an_invalid_bundle_is_refused();
    preload_reports_per_category_success_and_failure();
    preload_before_bind_is_refused();
    draw_list_needs_residency_and_a_geometry_binding();
    draw_list_order_is_deterministic();
    malformed_geometry_bindings_are_refused();
    rebinding_replaces_rather_than_merges();
    return 0;
}
