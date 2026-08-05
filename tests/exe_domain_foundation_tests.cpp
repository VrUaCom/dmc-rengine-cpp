#include "dmc_rengine/exe/project.hpp"

#include <cassert>
#include <string>

int main() {
    using namespace dmc::rengine::exe;

    ExecutableProject project{ExecutableProjectManifest{
        .schema_version = 1U,
        .project_id = "dmc3-hdc-phase-0.01a",
        .phase = "0.01a",
        .language_standard = "C++20",
        .target = ExecutableIdentity{
            .sha256 = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
            .size = 6'356'432U,
            .image_base = 0x140000000ULL,
            .entry_point_va = 0x14034615CULL,
            .profile = "dmc3-hd",
        },
        .first_reverse_pass = 0U,
        .last_reverse_pass = 32U,
        .authorities = {
            ReversePassAuthority{
                .pass = 0U,
                .title = "Baseline executable identity",
                .authority_uri = "drive://dmc3-reverse/pass-000",
                .artifact_sha256 = "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082",
                .status = EntityStatus::confirmed,
            },
            ReversePassAuthority{
                .pass = 32U,
                .title = "PC save record-envelope ABI",
                .authority_uri = "drive://dmc3-reverse/pass-032",
                .artifact_sha256 = "pass32-evidence-packet",
                .status = EntityStatus::implemented,
            },
        },
    }};

    assert(project.add_runtime_range(RuntimeRange{
        .id = "range.dmc3_main",
        .range = AddressRange{Address{AddressSpace::va, 0x1402C5DF0ULL}, 0x100U},
        .unwind_info = std::nullopt,
        .chained = false,
        .exception_handler = false,
        .evidence = {{"ev.phase12.dmc3_main", EntityStatus::confirmed}},
    }));

    assert(project.add_fragment(FunctionFragment{
        .id = "fragment.dmc3_main.body",
        .range = AddressRange{Address{AddressSpace::va, 0x1402C5DF0ULL}, 0x100U},
        .role = "primary-body",
        .runtime_range_ids = {"range.dmc3_main"},
        .evidence = {{"ev.phase12.dmc3_main", EntityStatus::confirmed}},
    }));

    assert(project.add_logical_function(LogicalFunction{
        .id = "function.dmc3_main",
        .recovered_name = "dmc3_main",
        .fragment_ids = {"fragment.dmc3_main.body"},
        .callee_ids = {},
        .global_ids = {},
        .status = EntityStatus::high,
        .evidence = {{"ev.phase12.dmc3_main", EntityStatus::confirmed}},
    }));

    assert(project.add_source_unit(RecoveredSourceUnit{
        .id = "source.bootstrap.dmc3_main",
        .path = "recovered/dmc3/src/bootstrap/dmc3_main.cpp",
        .language = "c++20",
        .logical_function_ids = {"function.dmc3_main"},
        .source_sha256 = "",
        .user_modified = false,
        .status = EntityStatus::candidate,
    }));

    assert(project.graph().add_edge(GraphEdge{
        .id = "edge.main.range",
        .source_id = "function.dmc3_main",
        .target_id = "range.dmc3_main",
        .kind = GraphEdgeKind::maps_to,
        .status = EntityStatus::confirmed,
    }));

    assert(project.graph().add_edge(GraphEdge{
        .id = "edge.source.main",
        .source_id = "source.bootstrap.dmc3_main",
        .target_id = "function.dmc3_main",
        .kind = GraphEdgeKind::maps_to,
        .status = EntityStatus::candidate,
    }));

    assert(project.valid());
    assert(project.graph().node_count() == 4U);
    assert(project.graph().edge_count() == 2U);
    assert(project.graph().outgoing("function.dmc3_main", GraphEdgeKind::maps_to).size() == 1U);

    const auto json = project.manifest_json();
    assert(json.find("\"phase\": \"0.01a\"") != std::string::npos);
    assert(json.find("\"language_standard\": \"C++20\"") != std::string::npos);
    assert(json.find("\"first_reverse_pass\": 0") != std::string::npos);
    assert(json.find("\"last_reverse_pass\": 32") != std::string::npos);

    assert(!project.add_logical_function(LogicalFunction{
        .id = "function.dmc3_main",
        .recovered_name = "duplicate",
    }));

    assert(!project.graph().add_edge(GraphEdge{
        .id = "edge.invalid",
        .source_id = "missing",
        .target_id = "function.dmc3_main",
        .kind = GraphEdgeKind::calls,
    }));

    return 0;
}
