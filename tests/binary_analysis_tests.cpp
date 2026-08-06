#include "dmc_rengine/binary/analysis.hpp"
#include "dmc_rengine/binary/document.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::binary::Document make_document() {
    return dmc::rengine::binary::Document(
        dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "synthetic",
                .logical_path = "analysis-fixture.bin",
                .container_chain = {},
                .offset = 0,
                .size = 64,
            },
            .display_name = "analysis-fixture.bin",
            .format = "bin",
            .profile = "unknown",
            .synthetic_name = false,
            .container = false,
        },
        64U);
}

[[nodiscard]] bool near(double left, double right) {
    return std::abs(left - right) < 0.000001;
}

} // namespace

int main() {
    using namespace dmc::rengine::binary;

    const std::array left{
        std::byte{0x00}, std::byte{0x01}, std::byte{0x02},
        std::byte{0x03}, std::byte{0x04},
    };
    const std::array right{
        std::byte{0x00}, std::byte{0x01}, std::byte{0xFF},
        std::byte{0x03}, std::byte{0x04}, std::byte{0x05},
    };

    const auto diff = aligned_byte_diff(left, right);
    assert(!diff.identical());
    assert(diff.left_size == 5U);
    assert(diff.right_size == 6U);
    assert(diff.equal_bytes == 4U);
    assert(diff.modified_bytes == 1U);
    assert(diff.inserted_bytes == 1U);
    assert(diff.removed_bytes == 0U);
    assert(diff.spans.size() == 4U);
    assert(diff.spans[0].kind == DiffKind::equal);
    assert(diff.spans[0].left_range == ByteRange({.offset = 0, .size = 2}));
    assert(diff.spans[1].kind == DiffKind::modified);
    assert(diff.spans[1].right_range == ByteRange({.offset = 2, .size = 1}));
    assert(diff.spans[3].kind == DiffKind::inserted);
    assert(!diff.spans[3].left_range.has_value());
    assert(diff.spans[3].right_range == ByteRange({.offset = 5, .size = 1}));

    const auto reverse_diff = aligned_byte_diff(right, left);
    assert(reverse_diff.removed_bytes == 1U);
    assert(reverse_diff.inserted_bytes == 0U);
    assert(reverse_diff.spans.back().kind == DiffKind::removed);

    const auto identical = aligned_byte_diff(left, left);
    assert(identical.identical());
    assert(identical.spans.size() == 1U);
    assert(identical.spans.front().kind == DiffKind::equal);

    std::array<std::byte, 256U> zeros{};
    const auto zero_entropy = entropy_map(zeros);
    assert(zero_entropy.size() == 1U);
    assert(near(zero_entropy[0].bits_per_byte, 0.0));
    assert(near(zero_entropy[0].zero_ratio, 1.0));
    assert(zero_entropy[0].unique_byte_count == 1U);
    assert(zero_entropy[0].band == EntropyBand::zero_fill);

    std::array<std::byte, 256U> uniform{};
    for (std::size_t index = 0U; index < uniform.size(); ++index) {
        uniform[index] = static_cast<std::byte>(index);
    }
    const auto uniform_entropy = entropy_map(uniform);
    assert(uniform_entropy.size() == 1U);
    assert(near(uniform_entropy[0].bits_per_byte, 8.0));
    assert(uniform_entropy[0].unique_byte_count == 256U);
    assert(uniform_entropy[0].band == EntropyBand::high);

    const std::array partial{
        std::byte{0x00}, std::byte{0x00}, std::byte{0x01},
        std::byte{0x01}, std::byte{0x02}, std::byte{0x02},
    };
    const auto partial_entropy = entropy_map(
        partial,
        EntropyOptions{
            .window_size = 4U,
            .step_size = 4U,
            .include_partial_window = true,
        });
    assert(partial_entropy.size() == 2U);
    assert(partial_entropy[0].range == ByteRange({.offset = 0, .size = 4}));
    assert(partial_entropy[1].range == ByteRange({.offset = 4, .size = 2}));

    const auto no_partial_entropy = entropy_map(
        partial,
        EntropyOptions{
            .window_size = 4U,
            .step_size = 4U,
            .include_partial_window = false,
        });
    assert(no_partial_entropy.size() == 1U);
    assert(entropy_map(partial, EntropyOptions{.window_size = 0U}).empty());

    auto document = make_document();
    assert(document.add_region(Region{
        .id = "header",
        .name = "Header",
        .range = {.offset = 0, .size = 16},
        .kind = RegionKind::header,
        .type_name = "Header",
        .evidence_id = "ev-header",
    }));
    assert(document.add_region(Region{
        .id = "payload",
        .name = "Payload",
        .range = {.offset = 16, .size = 32},
        .kind = RegionKind::payload,
        .type_name = "Payload",
        .evidence_id = "ev-payload",
    }));
    assert(document.add_field(Field{
        .id = "header-field",
        .name = "Header field",
        .range = {.offset = 8, .size = 8},
        .kind = FieldKind::unsigned_integer,
        .type_name = "u64_le",
        .display_value = "0",
        .parent_id = {},
        .evidence_id = "ev-field",
    }));
    assert(document.add_ownership(OwnershipClaim{
        .owner_id = "parser.header",
        .range = {.offset = 0, .size = 20},
        .rationale = "Owns the header and payload prefix.",
    }));
    assert(document.add_annotation(Annotation{
        .id = "boundary-note",
        .range = {.offset = 15, .size = 2},
        .text = "Crosses the structure boundary.",
        .evidence_id = "ev-boundary",
        .tags = {"boundary"},
    }));

    const auto range_selection = document.selection_for_range(
        ByteRange{.offset = 14, .size = 4});
    assert(range_selection.regions.size() == 2U);
    assert(range_selection.fields.size() == 1U);
    assert(range_selection.owners.size() == 1U);
    assert(range_selection.annotations.size() == 1U);
    assert(document.regions_at(17U).size() == 1U);

    const auto invalid_selection = document.selection_for_range(
        ByteRange{.offset = 63, .size = 2});
    assert(invalid_selection.regions.empty());
    assert(invalid_selection.fields.empty());
    assert(invalid_selection.owners.empty());
    assert(invalid_selection.annotations.empty());

    return 0;
}
