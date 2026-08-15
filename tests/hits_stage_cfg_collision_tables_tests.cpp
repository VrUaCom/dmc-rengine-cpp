#include "dmc_rengine/profiles/dmc3/hits_stage_cfg_collision_tables.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace formats = dmc::rengine::formats;

namespace {

void write_entry(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint8_t flags,
    std::uint8_t transform_selector,
    std::uint16_t descriptor_index) {
    bytes[offset] = static_cast<std::byte>(flags);
    bytes[offset + 1U] = static_cast<std::byte>(transform_selector);
    bytes[offset + 2U] = static_cast<std::byte>(descriptor_index & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((descriptor_index >> 8U) & 0xFFU);
}

formats::ContainerDocument make_document(
    std::size_t slot_count,
    std::size_t container_size) {
    formats::ContainerDocument document;
    document.format = "PAC";
    document.schema_version = 1U;
    document.declared_slot_count = static_cast<std::uint32_t>(slot_count);
    document.container_size = static_cast<std::uint64_t>(container_size);
    document.entries.resize(slot_count);
    for (std::size_t index = 0; index < slot_count; ++index) {
        document.entries[index].slot_index = static_cast<std::uint32_t>(index);
    }
    return document;
}

void set_slot(
    formats::ContainerDocument& document,
    std::size_t slot,
    std::size_t offset,
    std::size_t size) {
    auto& entry = document.entries[slot];
    entry.offset = static_cast<std::uint64_t>(offset);
    entry.size = static_cast<std::uint64_t>(size);
    entry.logical_name = "synthetic-slot-" + std::to_string(slot);
    entry.populated = true;
}

} // namespace

int main() {
    namespace stage = dmc::rengine::profiles::dmc3::hits_stage_cfg_collision_tables;

    constexpr std::string_view canonical_sha =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
    constexpr std::string_view packed_sha =
        "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

    std::vector<std::byte> modern_bytes(0x800U, std::byte{0});
    auto modern_document = make_document(41U, modern_bytes.size());
    set_slot(modern_document, 39U, 0x200U, 12U);
    set_slot(modern_document, 40U, 0x300U, 2U * stage::k_primitive_descriptor_stride);
    assert(modern_document.valid());

    write_entry(modern_bytes, 0x200U, 0x06U, 250U, 0U);
    write_entry(modern_bytes, 0x204U, 0x02U, 3U, 1U);
    write_entry(modern_bytes, 0x208U, 0x04U, 4U, 1U);
    modern_bytes[0x300U] = std::byte{2};
    modern_bytes[0x300U + stage::k_primitive_descriptor_stride] = std::byte{5};

    const auto modern = stage::View::open(
        canonical_sha,
        modern_bytes,
        modern_document,
        stage::SlotGeneration::modern_cem_stage_cfg);
    assert(modern.has_value());
    assert(modern->slots().entry_table_slot == 39U);
    assert(modern->slots().primitive_descriptor_table_slot == 40U);
    assert(modern->entry_count() == 3U);
    assert(modern->primitive_descriptor_count() == 2U);
    assert(modern->all_descriptor_references_valid());
    assert(!modern->transform_selector_bounds_available());

    const auto first = modern->entry(0U);
    assert(first.has_value());
    assert(first->flags == 0x06U);
    assert(first->transform_selector == 250U);
    assert(first->descriptor_index == 0U);
    assert(modern->primitive_type(0U).has_value() && *modern->primitive_type(0U) == 2U);
    assert(modern->primitive_type(1U).has_value() && *modern->primitive_type(1U) == 5U);

    const auto census = modern->referenced_descriptor_census();
    assert(census.total_entry_count == 3U);
    assert(census.valid_reference_count == 3U);
    assert(census.all_references_valid());
    assert(census.invalid_entry_indices.empty());
    assert(census.referenced_descriptors.size() == 2U);
    assert(census.referenced_descriptors[0U].descriptor_index == 0U);
    assert(census.referenced_descriptors[0U].primitive_type == 2U);
    assert(census.referenced_descriptors[0U].reference_count == 1U);
    assert(census.referenced_descriptors[1U].descriptor_index == 1U);
    assert(census.referenced_descriptors[1U].primitive_type == 5U);
    assert(census.referenced_descriptors[1U].reference_count == 2U);
    assert(census.referenced_descriptor_count_for_type(5U) == 1U);
    assert(census.reference_count_for_type(5U) == 2U);
    assert(census.referenced_descriptor_count_for_type(6U) == 0U);
    assert(census.reference_count_for_type(6U) == 0U);

    // The raw transform selector is intentionally preserved even when no
    // transform table has been proven for this Stage-CFG slot path.
    const auto first_validation = modern->validate_descriptor_reference(0U);
    assert(first_validation.has_value());
    assert(first_validation->valid());

    assert(!stage::View::open(
                packed_sha,
                modern_bytes,
                modern_document,
                stage::SlotGeneration::modern_cem_stage_cfg)
                .has_value());

    auto bad_descriptor_bytes = modern_bytes;
    write_entry(bad_descriptor_bytes, 0x208U, 0U, 0U, 2U);
    const auto bad_descriptor = stage::View::open(
        canonical_sha,
        bad_descriptor_bytes,
        modern_document,
        stage::SlotGeneration::modern_cem_stage_cfg);
    assert(bad_descriptor.has_value());
    assert(!bad_descriptor->all_descriptor_references_valid());

    const auto bad_census = bad_descriptor->referenced_descriptor_census();
    assert(bad_census.total_entry_count == 3U);
    assert(bad_census.valid_reference_count == 2U);
    assert(!bad_census.all_references_valid());
    assert(bad_census.invalid_entry_indices.size() == 1U);
    assert(bad_census.invalid_entry_indices[0U] == 2U);
    assert(bad_census.referenced_descriptors.size() == 2U);
    assert(bad_census.reference_count_for_type(5U) == 1U);

    auto malformed_document = modern_document;
    malformed_document.entries[40U].size =
        static_cast<std::uint64_t>(2U * stage::k_primitive_descriptor_stride + 1U);
    assert(!stage::View::open(
                canonical_sha,
                modern_bytes,
                malformed_document,
                stage::SlotGeneration::modern_cem_stage_cfg)
                .has_value());

    std::vector<std::byte> legacy_bytes(0x600U, std::byte{0});
    auto legacy_document = make_document(24U, legacy_bytes.size());
    set_slot(legacy_document, 22U, 0x100U, 4U);
    set_slot(legacy_document, 23U, 0x200U, stage::k_primitive_descriptor_stride);
    assert(legacy_document.valid());
    write_entry(legacy_bytes, 0x100U, 0x01U, 7U, 0U);
    legacy_bytes[0x200U] = std::byte{4};

    const auto legacy = stage::View::open(
        canonical_sha,
        legacy_bytes,
        legacy_document,
        stage::SlotGeneration::legacy_cem008_stage_cfg);
    assert(legacy.has_value());
    assert(legacy->slots().entry_table_slot == 22U);
    assert(legacy->slots().primitive_descriptor_table_slot == 23U);
    assert(legacy->entry_count() == 1U);
    assert(legacy->primitive_descriptor_count() == 1U);
    assert(legacy->primitive_type(0U).has_value() && *legacy->primitive_type(0U) == 4U);
    assert(legacy->all_descriptor_references_valid());

    const auto legacy_census = legacy->referenced_descriptor_census();
    assert(legacy_census.all_references_valid());
    assert(legacy_census.total_entry_count == 1U);
    assert(legacy_census.valid_reference_count == 1U);
    assert(legacy_census.referenced_descriptors.size() == 1U);
    assert(legacy_census.referenced_descriptors[0U].descriptor_index == 0U);
    assert(legacy_census.referenced_descriptors[0U].primitive_type == 4U);
    assert(legacy_census.referenced_descriptors[0U].reference_count == 1U);

    assert(!stage::View::open(
                canonical_sha,
                legacy_bytes,
                legacy_document,
                stage::SlotGeneration::modern_cem_stage_cfg)
                .has_value());

    return 0;
}
