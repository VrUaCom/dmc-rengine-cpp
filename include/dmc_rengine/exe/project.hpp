#pragma once

#include "dmc_rengine/exe/knowledge_graph.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::exe {

struct ExecutableIdentity {
    std::string sha256;
    std::uint64_t size{};
    std::uint64_t image_base{};
    std::uint64_t entry_point_va{};
    std::string profile;
};

struct ReversePassAuthority {
    std::uint32_t pass{};
    std::string title;
    std::string authority_uri;
    std::string artifact_sha256;
    EntityStatus status{EntityStatus::confirmed};
};

struct ExecutableProjectManifest {
    std::uint32_t schema_version{1U};
    std::string project_id;
    std::string phase{"0.01a"};
    std::string language_standard{"C++20"};
    ExecutableIdentity target;
    std::uint32_t first_reverse_pass{};
    std::uint32_t last_reverse_pass{32U};
    std::vector<ReversePassAuthority> authorities;
};

class ExecutableProject {
public:
    explicit ExecutableProject(ExecutableProjectManifest manifest);

    [[nodiscard]] const ExecutableProjectManifest& manifest() const noexcept {
        return manifest_;
    }

    [[nodiscard]] ExeKnowledgeGraph& graph() noexcept { return graph_; }
    [[nodiscard]] const ExeKnowledgeGraph& graph() const noexcept { return graph_; }

    [[nodiscard]] bool add_runtime_range(RuntimeRange value);
    [[nodiscard]] bool add_fragment(FunctionFragment value);
    [[nodiscard]] bool add_logical_function(LogicalFunction value);
    [[nodiscard]] bool add_source_unit(RecoveredSourceUnit value);
    [[nodiscard]] bool add_source_mapping(SourceBinaryMapping value);
    [[nodiscard]] bool add_class(ClassModel value);
    [[nodiscard]] bool add_vtable(VtableModel value);
    [[nodiscard]] bool add_global_initializer(GlobalInitializerModel value);
    [[nodiscard]] bool add_memory_arena(MemoryArenaModel value);
    [[nodiscard]] bool add_subsystem(SubsystemModel value);
    [[nodiscard]] bool add_resource_binding(ResourceBindingModel value);

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::string manifest_json() const;

private:
    template <typename T>
    [[nodiscard]] static bool insert_unique(
        std::vector<T>& values,
        T value,
        std::string_view id);

    ExecutableProjectManifest manifest_;
    ExeKnowledgeGraph graph_;
    std::vector<RuntimeRange> runtime_ranges_;
    std::vector<FunctionFragment> fragments_;
    std::vector<LogicalFunction> logical_functions_;
    std::vector<RecoveredSourceUnit> source_units_;
    std::vector<SourceBinaryMapping> source_mappings_;
    std::vector<ClassModel> classes_;
    std::vector<VtableModel> vtables_;
    std::vector<GlobalInitializerModel> global_initializers_;
    std::vector<MemoryArenaModel> memory_arenas_;
    std::vector<SubsystemModel> subsystems_;
    std::vector<ResourceBindingModel> resource_bindings_;
};

} // namespace dmc::rengine::exe
