#include "dmc_rengine/exe/dmc3_seed.hpp"

#include <cassert>
#include <string>

int main() {
    using namespace dmc::rengine::exe;

    const auto manifest = dmc3_phase_001a_manifest();
    assert(manifest.phase == "0.01a");
    assert(manifest.language_standard == "C++20");
    assert(manifest.first_reverse_pass == 0U);
    assert(manifest.last_reverse_pass == 32U);
    assert(manifest.target.sha256 ==
           "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082");

    auto project = build_dmc3_phase_001a_seed();
    assert(project.valid());
    assert(project.graph().node_count() >= 20U);
    assert(project.graph().edge_count() >= 14U);

    const auto* main = project.graph().find_node("function.dmc3_main");
    assert(main != nullptr);
    assert(main->kind == GraphNodeKind::logical_function);

    const auto* arena = project.graph().find_node("memory.application-arena");
    assert(arena != nullptr);
    assert(arena->kind == GraphNodeKind::memory_arena);

    const auto* texture = project.graph().find_node("subsystem.renderer-texture");
    assert(texture != nullptr);
    const auto texture_edges = project.graph().outgoing(
        "subsystem.renderer-texture",
        GraphEdgeKind::contains);
    assert(texture_edges.size() == 2U);

    const auto* stage_binding = project.graph().find_node("binding.stage-table");
    assert(stage_binding != nullptr);
    assert(stage_binding->kind == GraphNodeKind::resource_binding);

    const auto json = project.manifest_json();
    assert(json.find("dmc3-hdc-phase-0.01a") != std::string::npos);
    assert(json.find("\"authority_count\": 8") != std::string::npos);
    return 0;
}
