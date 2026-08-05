#pragma once

#include "dmc_rengine/exe/address.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace dmc::rengine::exe {

enum class EntityStatus : std::uint8_t {
    hypothesis,
    candidate,
    low,
    medium,
    high,
    confirmed,
    corrected,
    rejected,
    implemented,
};

struct EvidenceLink {
    std::string record_id;
    EntityStatus status{EntityStatus::hypothesis};
};

struct RuntimeRange {
    std::string id;
    AddressRange range;
    std::optional<Address> unwind_info;
    bool chained{};
    bool exception_handler{};
    std::vector<EvidenceLink> evidence;
};

struct FunctionFragment {
    std::string id;
    AddressRange range;
    std::string role;
    std::vector<std::string> runtime_range_ids;
    std::vector<EvidenceLink> evidence;
};

struct LogicalFunction {
    std::string id;
    std::string recovered_name;
    std::vector<std::string> fragment_ids;
    std::vector<std::string> callee_ids;
    std::vector<std::string> global_ids;
    EntityStatus status{EntityStatus::candidate};
    std::vector<EvidenceLink> evidence;
};

struct RecoveredSourceUnit {
    std::string id;
    std::string path;
    std::string language{"c++20"};
    std::vector<std::string> logical_function_ids;
    std::string source_sha256;
    bool user_modified{};
    EntityStatus status{EntityStatus::candidate};
};

struct SourceBinaryMapping {
    std::string source_unit_id;
    std::uint32_t line_begin{};
    std::uint32_t line_end{};
    AddressRange output_range;
    std::string logical_function_id;
    EntityStatus status{EntityStatus::candidate};
};

struct VtableSlot {
    std::uint32_t index{};
    std::string logical_function_id;
    std::string semantic_name;
    EntityStatus status{EntityStatus::candidate};
};

struct VtableModel {
    std::string id;
    Address address;
    std::int64_t subobject_offset{};
    std::vector<VtableSlot> slots;
    std::vector<EvidenceLink> evidence;
};

struct ClassField {
    std::string name;
    std::uint64_t offset{};
    std::uint64_t size{};
    std::string type_name;
    EntityStatus status{EntityStatus::candidate};
};

struct ClassModel {
    std::string id;
    std::string name;
    std::uint64_t minimum_size{};
    std::vector<std::string> base_class_ids;
    std::vector<std::string> vtable_ids;
    std::vector<ClassField> fields;
    std::vector<EvidenceLink> evidence;
};

struct GlobalInitializerModel {
    std::string id;
    Address function;
    std::optional<Address> target_global;
    std::string semantic_role;
    bool registers_destructor{};
    EntityStatus status{EntityStatus::candidate};
    std::vector<EvidenceLink> evidence;
};

struct MemoryArenaModel {
    std::string id;
    std::string name;
    std::uint64_t size{};
    std::uint64_t alignment{};
    std::string lifetime;
    EntityStatus status{EntityStatus::candidate};
    std::vector<EvidenceLink> evidence;
};

struct SubsystemModel {
    std::string id;
    std::string name;
    std::vector<std::string> logical_function_ids;
    std::vector<std::string> class_ids;
    std::vector<std::string> resource_binding_ids;
    EntityStatus status{EntityStatus::candidate};
};

struct ResourceBindingModel {
    std::string id;
    std::string logical_path;
    std::string container_identity;
    std::string slot_expression;
    std::string owner_class_id;
    std::uint64_t owner_field_offset{};
    EntityStatus status{EntityStatus::candidate};
    std::vector<EvidenceLink> evidence;
};

} // namespace dmc::rengine::exe
