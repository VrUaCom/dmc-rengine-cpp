#include "dmc_rengine/exe/dmc3_seed.hpp"

#include <utility>

namespace dmc::rengine::exe {
namespace {

[[nodiscard]] EvidenceLink evidence(std::string id, EntityStatus status) {
    return EvidenceLink{.record_id = std::move(id), .status = status};
}

void add_edge(
    ExecutableProject& project,
    std::string id,
    std::string source,
    std::string target,
    GraphEdgeKind kind,
    EntityStatus status) {
    (void)project.graph().add_edge(GraphEdge{
        .id = std::move(id),
        .source_id = std::move(source),
        .target_id = std::move(target),
        .kind = kind,
        .status = status,
    });
}

} // namespace

ExecutableProjectManifest dmc3_phase_001a_manifest() {
    return ExecutableProjectManifest{
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
            ReversePassAuthority{0U, "Canonical executable baseline", "drive://dmc3/pass-000", "target-sha256", EntityStatus::confirmed},
            ReversePassAuthority{12U, "Whole-EXE architecture assessment", "drive://dmc3/pass-012", "phase12-architecture", EntityStatus::confirmed},
            ReversePassAuthority{13U, "Runtime topology", "drive://dmc3/pass-013", "phase13-runtime", EntityStatus::confirmed},
            ReversePassAuthority{14U, "Resource I/O architecture", "drive://dmc3/pass-014", "phase14-resource-io", EntityStatus::confirmed},
            ReversePassAuthority{15U, "Container runtime", "drive://dmc3/pass-015", "phase15-container", EntityStatus::confirmed},
            ReversePassAuthority{16U, "Texture resource architecture", "drive://dmc3/pass-016", "phase16-texture", EntityStatus::confirmed},
            ReversePassAuthority{17U, "Texture lifecycle", "drive://dmc3/pass-017", "phase17-texture-lifecycle", EntityStatus::high},
            ReversePassAuthority{32U, "Reviewed PC save promotion", "drive://dmc3/pass-032", "pass32-save", EntityStatus::implemented},
        },
    };
}

ExecutableProject build_dmc3_phase_001a_seed() {
    ExecutableProject project{dmc3_phase_001a_manifest()};

    (void)project.add_runtime_range(RuntimeRange{
        .id = "range.pe_entry",
        .range = {Address{AddressSpace::va, 0x14034615CULL}, 0x80U},
        .evidence = {evidence("pass12.pe-entry", EntityStatus::confirmed)},
    });
    (void)project.add_runtime_range(RuntimeRange{
        .id = "range.dmc3_main",
        .range = {Address{AddressSpace::va, 0x1402C5DF0ULL}, 0x300U},
        .evidence = {evidence("pass12.dmc3-main", EntityStatus::confirmed)},
    });
    (void)project.add_runtime_range(RuntimeRange{
        .id = "range.asyncio_worker",
        .range = {Address{AddressSpace::va, 0x14002FAE0ULL}, 0x190U},
        .evidence = {evidence("pass13.asyncio-worker", EntityStatus::confirmed)},
    });
    (void)project.add_runtime_range(RuntimeRange{
        .id = "range.open_game_resource",
        .range = {Address{AddressSpace::va, 0x14002FCA0ULL}, 0x300U},
        .evidence = {evidence("pass14.open-game-resource", EntityStatus::high)},
    });
    (void)project.add_runtime_range(RuntimeRange{
        .id = "range.ptx_finalize",
        .range = {Address{AddressSpace::va, 0x140331A80ULL}, 0x140U},
        .evidence = {evidence("pass17.ptx-finalize", EntityStatus::high)},
    });

    (void)project.add_fragment(FunctionFragment{
        .id = "fragment.pe_entry.primary",
        .range = {Address{AddressSpace::va, 0x14034615CULL}, 0x80U},
        .role = "primary-body",
        .runtime_range_ids = {"range.pe_entry"},
        .evidence = {evidence("pass12.pe-entry", EntityStatus::confirmed)},
    });
    (void)project.add_fragment(FunctionFragment{
        .id = "fragment.dmc3_main.primary",
        .range = {Address{AddressSpace::va, 0x1402C5DF0ULL}, 0x300U},
        .role = "primary-body",
        .runtime_range_ids = {"range.dmc3_main"},
        .evidence = {evidence("pass12.dmc3-main", EntityStatus::confirmed)},
    });

    (void)project.add_logical_function(LogicalFunction{
        .id = "function.pe_entry",
        .recovered_name = "Dmc3PeEntry",
        .fragment_ids = {"fragment.pe_entry.primary"},
        .callee_ids = {"function.dmc3_main"},
        .status = EntityStatus::confirmed,
        .evidence = {evidence("pass12.startup-chain", EntityStatus::confirmed)},
    });
    (void)project.add_logical_function(LogicalFunction{
        .id = "function.dmc3_main",
        .recovered_name = "dmc3_main",
        .fragment_ids = {"fragment.dmc3_main.primary"},
        .status = EntityStatus::high,
        .evidence = {evidence("pass12.main-loop", EntityStatus::confirmed)},
    });
    (void)project.add_logical_function(LogicalFunction{
        .id = "function.asyncio_worker",
        .recovered_name = "AsyncIOThread::WorkerLoop",
        .status = EntityStatus::high,
        .evidence = {evidence("pass13.asyncio-worker", EntityStatus::confirmed)},
    });
    (void)project.add_logical_function(LogicalFunction{
        .id = "function.open_game_resource",
        .recovered_name = "OpenGameResource",
        .status = EntityStatus::high,
        .evidence = {evidence("pass14.resource-open", EntityStatus::high)},
    });
    (void)project.add_logical_function(LogicalFunction{
        .id = "function.ptx_finalize",
        .recovered_name = "FinalizePtxRuntimeBundle",
        .status = EntityStatus::high,
        .evidence = {evidence("pass17.ptx-finalize", EntityStatus::high)},
    });

    (void)project.add_memory_arena(MemoryArenaModel{
        .id = "memory.application-arena",
        .name = "DMC3 application arena",
        .size = 0x10400000ULL,
        .alignment = 0x1000U,
        .lifetime = "process",
        .status = EntityStatus::confirmed,
        .evidence = {evidence("pass12.memory-arena", EntityStatus::confirmed)},
    });
    (void)project.add_memory_arena(MemoryArenaModel{
        .id = "memory.audio-heap",
        .name = "FMOD private heap",
        .size = 35ULL * 1024ULL * 1024ULL,
        .alignment = 16U,
        .lifetime = "audio-subsystem",
        .status = EntityStatus::confirmed,
        .evidence = {evidence("pass12.fmod-heap", EntityStatus::confirmed)},
    });

    (void)project.add_class(ClassModel{
        .id = "class.CPlayer",
        .name = "CPlayer",
        .minimum_size = 0x7540U,
        .base_class_ids = {"class.CActor"},
        .vtable_ids = {"vtable.CPlayer.primary", "vtable.CPlayer.sub60", "vtable.CPlayer.subD0", "vtable.CPlayer.sub110"},
        .evidence = {evidence("pass12.cplayer-layout", EntityStatus::high)},
    });
    (void)project.add_class(ClassModel{
        .id = "class.CPtxManager",
        .name = "CPtxManager",
        .minimum_size = 0x4308U,
        .base_class_ids = {"class.IPtxManager"},
        .vtable_ids = {"vtable.CPtxManager"},
        .fields = {
            ClassField{"cache", 0x08U, 32U * 0x218U, "PtxCacheEntryV1[32]", EntityStatus::confirmed},
        },
        .evidence = {evidence("pass16.cptxmanager", EntityStatus::confirmed)},
    });
    (void)project.add_vtable(VtableModel{
        .id = "vtable.CPtxManager",
        .address = {AddressSpace::va, 0x140507B60ULL},
        .subobject_offset = 0,
        .evidence = {evidence("pass16.cptxmanager-vtable", EntityStatus::confirmed)},
    });

    (void)project.add_global_initializer(GlobalInitializerModel{
        .id = "global-init.CPtxManager",
        .function = {AddressSpace::va, 0x140000000ULL},
        .target_global = Address{AddressSpace::va, 0x140D5B860ULL},
        .semantic_role = "construct global CPtxManager",
        .registers_destructor = true,
        .status = EntityStatus::high,
        .evidence = {evidence("pass12.global-init", EntityStatus::high)},
    });

    (void)project.add_subsystem(SubsystemModel{
        .id = "subsystem.startup",
        .name = "CRT and application bootstrap",
        .logical_function_ids = {"function.pe_entry", "function.dmc3_main"},
        .status = EntityStatus::confirmed,
    });
    (void)project.add_subsystem(SubsystemModel{
        .id = "subsystem.resource-io",
        .name = "GD-compatible resource resolver and AsyncIO",
        .logical_function_ids = {"function.open_game_resource", "function.asyncio_worker"},
        .status = EntityStatus::high,
    });
    (void)project.add_subsystem(SubsystemModel{
        .id = "subsystem.renderer-texture",
        .name = "PTX/TIM2 and D3D11 texture runtime",
        .logical_function_ids = {"function.ptx_finalize"},
        .class_ids = {"class.CPtxManager"},
        .status = EntityStatus::high,
    });
    (void)project.add_subsystem(SubsystemModel{
        .id = "subsystem.save",
        .name = "DMC3 PC save and Steam persistence",
        .status = EntityStatus::implemented,
    });

    (void)project.add_resource_binding(ResourceBindingModel{
        .id = "binding.stage-table",
        .logical_path = "stage-table[stage][script|room|effect|sound]",
        .container_identity = "dmc3.exe@0x1405C4AA8",
        .slot_expression = "row*4+column",
        .owner_class_id = "subsystem.resource-io",
        .owner_field_offset = 0U,
        .status = EntityStatus::confirmed,
        .evidence = {evidence("pass12.stage-table", EntityStatus::confirmed)},
    });

    add_edge(project, "edge.pe-entry.main", "function.pe_entry", "function.dmc3_main", GraphEdgeKind::calls, EntityStatus::confirmed);
    add_edge(project, "edge.pe-entry.range", "function.pe_entry", "range.pe_entry", GraphEdgeKind::maps_to, EntityStatus::confirmed);
    add_edge(project, "edge.main.range", "function.dmc3_main", "range.dmc3_main", GraphEdgeKind::maps_to, EntityStatus::confirmed);
    add_edge(project, "edge.async.range", "function.asyncio_worker", "range.asyncio_worker", GraphEdgeKind::maps_to, EntityStatus::confirmed);
    add_edge(project, "edge.open.range", "function.open_game_resource", "range.open_game_resource", GraphEdgeKind::maps_to, EntityStatus::high);
    add_edge(project, "edge.ptx.range", "function.ptx_finalize", "range.ptx_finalize", GraphEdgeKind::maps_to, EntityStatus::high);
    add_edge(project, "edge.startup.main", "subsystem.startup", "function.dmc3_main", GraphEdgeKind::contains, EntityStatus::confirmed);
    add_edge(project, "edge.resource.open", "subsystem.resource-io", "function.open_game_resource", GraphEdgeKind::contains, EntityStatus::high);
    add_edge(project, "edge.resource.async", "subsystem.resource-io", "function.asyncio_worker", GraphEdgeKind::contains, EntityStatus::high);
    add_edge(project, "edge.texture.manager", "subsystem.renderer-texture", "class.CPtxManager", GraphEdgeKind::contains, EntityStatus::confirmed);
    add_edge(project, "edge.texture.finalize", "subsystem.renderer-texture", "function.ptx_finalize", GraphEdgeKind::contains, EntityStatus::high);
    add_edge(project, "edge.ptx.vtable", "class.CPtxManager", "vtable.CPtxManager", GraphEdgeKind::installs, EntityStatus::confirmed);
    add_edge(project, "edge.ptx.global-init", "class.CPtxManager", "global-init.CPtxManager", GraphEdgeKind::initialized_by, EntityStatus::high);
    add_edge(project, "edge.resource.stage-table", "subsystem.resource-io", "binding.stage-table", GraphEdgeKind::binds, EntityStatus::confirmed);

    return project;
}

} // namespace dmc::rengine::exe
