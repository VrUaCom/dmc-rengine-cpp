#include "dmc_rengine/profiles/dmc3/hits_collision_triplet.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void write_entry(
    std::vector<std::byte>& entries,
    std::size_t index,
    std::uint8_t flags,
    std::uint8_t transform_selector,
    std::uint16_t descriptor_index) {
    const auto offset = index * 4U;
    entries[offset] = static_cast<std::byte>(flags);
    entries[offset + 1U] = static_cast<std::byte>(transform_selector);
    entries[offset + 2U] = static_cast<std::byte>(descriptor_index & 0xFFU);
    entries[offset + 3U] = static_cast<std::byte>((descriptor_index >> 8U) & 0xFFU);
}

} // namespace

int main() {
    namespace triplet = dmc::rengine::profiles::dmc3::hits_collision_triplet;

    // Synthetic table dimensions mirror the preserved Phase-15 em000.pac
    // collision triplet, but contain no proprietary game bytes.
    std::vector<std::byte> transforms(96U * triplet::k_transform_stride, std::byte{0});
    std::vector<std::byte> entries(24U * triplet::k_entry_stride, std::byte{0});
    std::vector<std::byte> descriptors(
        23U * triplet::k_primitive_descriptor_stride, std::byte{0});

    write_entry(entries, 0U, 0x06U, 0U, 0U);
    write_entry(entries, 1U, 0x02U, 3U, 1U);
    write_entry(entries, 2U, 0x01U, 9U, 2U);
    write_entry(entries, 3U, 0x04U, 17U, 3U);
    for (std::size_t index = 4U; index < 24U; ++index) {
        write_entry(entries, index, 0U, 0U, 0U);
    }

    descriptors[0U * triplet::k_primitive_descriptor_stride] = std::byte{2};
    descriptors[1U * triplet::k_primitive_descriptor_stride] = std::byte{3};
    descriptors[2U * triplet::k_primitive_descriptor_stride] = std::byte{4};
    descriptors[3U * triplet::k_primitive_descriptor_stride] = std::byte{6};

    const auto view = triplet::View::open(transforms, entries, descriptors);
    assert(view.has_value());
    assert(view->transform_count() == 96U);
    assert(view->entry_count() == 24U);
    assert(view->primitive_descriptor_count() == 23U);
    assert(view->all_entry_references_valid());

    const auto first = view->entry(0U);
    assert(first.has_value());
    assert(first->flags == 0x06U);
    assert(first->transform_selector == 0U);
    assert(first->descriptor_index == 0U);

    const auto fourth = view->entry(3U);
    assert(fourth.has_value());
    assert(fourth->flags == 0x04U);
    assert(fourth->transform_selector == 17U);
    assert(fourth->descriptor_index == 3U);

    assert(view->primitive_type(0U).has_value() && *view->primitive_type(0U) == 2U);
    assert(view->primitive_type(1U).has_value() && *view->primitive_type(1U) == 3U);
    assert(view->primitive_type(2U).has_value() && *view->primitive_type(2U) == 4U);
    assert(view->primitive_type(3U).has_value() && *view->primitive_type(3U) == 6U);
    assert(!view->entry(24U).has_value());
    assert(!view->transform_record(96U).has_value());
    assert(!view->primitive_descriptor(23U).has_value());
    assert(!view->primitive_type(23U).has_value());

    auto bad_transform_entries = entries;
    write_entry(bad_transform_entries, 0U, 0U, 96U, 0U);
    const auto bad_transform_view =
        triplet::View::open(transforms, bad_transform_entries, descriptors);
    assert(bad_transform_view.has_value());
    const auto bad_transform_validation = bad_transform_view->validate_entry(0U);
    assert(bad_transform_validation.has_value());
    assert(!bad_transform_validation->transform_selector_in_range);
    assert(bad_transform_validation->descriptor_index_in_range);
    assert(!bad_transform_validation->valid());
    assert(!bad_transform_view->all_entry_references_valid());

    auto bad_descriptor_entries = entries;
    write_entry(bad_descriptor_entries, 0U, 0U, 0U, 23U);
    const auto bad_descriptor_view =
        triplet::View::open(transforms, bad_descriptor_entries, descriptors);
    assert(bad_descriptor_view.has_value());
    const auto bad_descriptor_validation = bad_descriptor_view->validate_entry(0U);
    assert(bad_descriptor_validation.has_value());
    assert(bad_descriptor_validation->transform_selector_in_range);
    assert(!bad_descriptor_validation->descriptor_index_in_range);
    assert(!bad_descriptor_validation->valid());

    std::vector<std::byte> malformed_entries(entries.size() + 1U, std::byte{0});
    assert(!triplet::View::open(transforms, malformed_entries, descriptors).has_value());

    // Mirrors the Phase-15 id100.pac slot-40 size: 4096 is not divisible by
    // 0x50 and therefore must not be accepted as a primitive descriptor table.
    std::vector<std::byte> id100_like_slot40(4096U, std::byte{0});
    assert(!triplet::View::open(transforms, entries, id100_like_slot40).has_value());

    return 0;
}
