#include "dmc_rengine/binary/document.hpp"

#include <cassert>

int main() {
    dmc::rengine::binary::Document document(
        dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "synthetic",
                .logical_path = "fixture.bin",
                .container_chain = {},
                .offset = 0,
                .size = 100,
            },
            .display_name = "fixture.bin",
            .format = "bin",
            .profile = "unknown",
            .synthetic_name = false,
            .container = false,
        },
        100);

    assert(document.add_region(dmc::rengine::binary::Region{
        .id = "header",
        .name = "Header",
        .range = {.offset = 0, .size = 10},
        .kind = dmc::rengine::binary::RegionKind::header,
        .type_name = "SyntheticHeader",
        .evidence_id = "ev-header",
    }));
    assert(document.add_region(dmc::rengine::binary::Region{
        .id = "table",
        .name = "Table",
        .range = {.offset = 20, .size = 10},
        .kind = dmc::rengine::binary::RegionKind::table,
        .type_name = "SyntheticTable",
        .evidence_id = "ev-table",
    }));
    assert(document.add_region(dmc::rengine::binary::Region{
        .id = "payload",
        .name = "Payload",
        .range = {.offset = 25, .size = 10},
        .kind = dmc::rengine::binary::RegionKind::payload,
        .type_name = "SyntheticPayload",
        .evidence_id = "ev-payload",
    }));

    assert(!document.add_region(dmc::rengine::binary::Region{
        .id = "header",
        .name = "Duplicate",
        .range = {.offset = 50, .size = 5},
        .kind = dmc::rengine::binary::RegionKind::unknown,
        .type_name = {},
        .evidence_id = {},
    }));
    assert(!document.add_region(dmc::rengine::binary::Region{
        .id = "outside",
        .name = "Outside",
        .range = {.offset = 95, .size = 10},
        .kind = dmc::rengine::binary::RegionKind::unknown,
        .type_name = {},
        .evidence_id = {},
    }));

    assert(document.add_ownership(dmc::rengine::binary::OwnershipClaim{
        .owner_id = "parser.synthetic",
        .range = {.offset = 0, .size = 35},
        .rationale = "Synthetic parser owns the known regions.",
    }));
    assert(!document.add_ownership(dmc::rengine::binary::OwnershipClaim{
        .owner_id = "parser.invalid",
        .range = {.offset = 99, .size = 2},
        .rationale = "Outside document.",
    }));

    assert(document.find_region("table") != nullptr);
    assert(document.find_region("missing") == nullptr);
    assert(document.coverage_bytes() == 25U);

    const auto unknown = document.unknown_ranges();
    assert(unknown.size() == 2U);
    assert(unknown[0] == dmc::rengine::binary::ByteRange({.offset = 10, .size = 10}));
    assert(unknown[1] == dmc::rengine::binary::ByteRange({.offset = 35, .size = 65}));

    const auto conflicts = document.conflicts();
    assert(conflicts.size() == 1U);
    assert(conflicts[0].left_region_id == "table");
    assert(conflicts[0].right_region_id == "payload");
    assert(conflicts[0].overlap ==
        dmc::rengine::binary::ByteRange({.offset = 25, .size = 5}));

    return 0;
}
