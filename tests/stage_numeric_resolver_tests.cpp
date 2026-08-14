#include "dmc_rengine/profiles/dmc3/stage_numeric_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/stage_table.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace {
void write_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t i = 0U; i < 8U; ++i) {
        bytes[offset + i] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(i * 8U)) & 0xFFU);
    }
}
}

int main() {
    using namespace dmc::rengine::profiles::dmc3;
    constexpr std::uint64_t image_base = 0x140000000ULL;
    constexpr std::uint32_t section_rva = 0x005C1000U;
    constexpr std::uint32_t section_raw = 0x400U;
    constexpr std::uint32_t section_size = 0x6000U;

    std::vector<std::byte> bytes(
        static_cast<std::size_t>(section_raw) + section_size,
        std::byte{0});
    const dmc::rengine::exe::PeImage image{
        .kind = dmc::rengine::exe::PeKind::pe32_plus,
        .machine = dmc::rengine::exe::PeMachine::amd64,
        .section_count = 1U,
        .image_base = image_base,
        .entry_point_rva = 0x1000U,
        .size_of_image = section_rva + section_size,
        .size_of_headers = section_raw,
        .subsystem = 2U,
        .sections = {dmc::rengine::exe::PeSection{
            .name = ".data",
            .virtual_size = section_size,
            .virtual_address = section_rva,
            .raw_size = section_size,
            .raw_offset = section_raw,
            .characteristics = 0U,
        }},
    };
    const auto file_offset_for_va = [=](std::uint64_t va) {
        return static_cast<std::size_t>(section_raw) +
            static_cast<std::size_t>(va - image_base - section_rva);
    };

    const auto& topology = wave2_stage_descriptor_universe();
    const auto& banks = wave3_stage_resource_banks();
    const auto selector_prefix = topology.selector_table_va - 8ULL;

    write_u64(bytes, file_offset_for_va(topology.group_base_table_va), selector_prefix);
    write_u64(bytes, file_offset_for_va(selector_prefix), banks[0].va);
    write_u64(bytes, file_offset_for_va(topology.selector_table_va),
        banks[0].va + banks[0].row_size_bytes());

    const auto group6_base = topology.selector_table_va + 20ULL * 8ULL;
    write_u64(bytes, file_offset_for_va(topology.group_base_table_va + 6ULL * 8ULL), group6_base);
    write_u64(bytes, file_offset_for_va(group6_base),
        banks[1].va + 55ULL * banks[1].row_size_bytes());

    // An otherwise unused family can alias the st000 selector base.
    write_u64(bytes, file_offset_for_va(topology.group_base_table_va + 5ULL * 8ULL), selector_prefix);

    const auto st000 = StageNumericResolver::resolve(
        std::span<const std::byte>{bytes}, image, 0U);
    assert(st000.resolved());
    assert(st000.descriptor_primary_stage_id == 0U);
    assert(!st000.aliases_another_primary_stage());

    const auto st001 = StageNumericResolver::resolve(
        std::span<const std::byte>{bytes}, image, 1U);
    assert(st001.resolved());
    assert(st001.descriptor_primary_stage_id == 1U);

    const auto st600 = StageNumericResolver::resolve(
        std::span<const std::byte>{bytes}, image, 600U);
    assert(st600.resolved());
    assert(st600.source_table_id == banks[1].id);
    assert(st600.source_row_index == 55U);
    assert(st600.descriptor_primary_stage_id == 600U);

    const auto fallback500 = StageNumericResolver::resolve(
        std::span<const std::byte>{bytes}, image, 500U);
    assert(fallback500.resolved());
    assert(fallback500.descriptor_primary_stage_id == 0U);
    assert(fallback500.aliases_another_primary_stage());

    const auto invalid = StageNumericResolver::resolve(
        std::span<const std::byte>{bytes}, image, 1000U);
    assert(!invalid.resolved());
    assert(!invalid.diagnostics.empty());
    return 0;
}
