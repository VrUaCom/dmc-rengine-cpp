#include "dmc_rengine/binary/document.hpp"

#include <cassert>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::binary::Document make_document() {
    return dmc::rengine::binary::Document(
        dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "synthetic",
                .logical_path = "field-fixture.bin",
                .container_chain = {},
                .offset = 0,
                .size = 64,
            },
            .display_name = "field-fixture.bin",
            .format = "bin",
            .profile = "unknown",
            .synthetic_name = false,
            .container = false,
        },
        64);
}

} // namespace

int main() {
    using dmc::rengine::binary::Annotation;
    using dmc::rengine::binary::ByteRange;
    using dmc::rengine::binary::Field;
    using dmc::rengine::binary::FieldKind;
    using dmc::rengine::binary::OwnershipClaim;
    using dmc::rengine::binary::Region;
    using dmc::rengine::binary::RegionKind;

    auto document = make_document();

    assert(document.add_region(Region{
        .id = "header",
        .name = "Header",
        .range = {.offset = 0, .size = 16},
        .kind = RegionKind::header,
        .type_name = "HeaderV1",
        .evidence_id = "ev-header",
    }));

    assert(document.add_field(Field{
        .id = "header-struct",
        .name = "Header structure",
        .range = {.offset = 0, .size = 16},
        .kind = FieldKind::structure,
        .type_name = "HeaderV1",
        .display_value = {},
        .parent_id = {},
        .evidence_id = "ev-header",
    }));
    assert(document.add_field(Field{
        .id = "version",
        .name = "Version",
        .range = {.offset = 4, .size = 4},
        .kind = FieldKind::unsigned_integer,
        .type_name = "u32_le",
        .display_value = "1",
        .parent_id = "header-struct",
        .evidence_id = "ev-version",
    }));
    assert(document.add_field(Field{
        .id = "flags",
        .name = "Flags",
        .range = {.offset = 8, .size = 2},
        .kind = FieldKind::enumeration,
        .type_name = "HeaderFlags",
        .display_value = "enabled",
        .parent_id = "header-struct",
        .evidence_id = {},
    }));

    assert(!document.add_field(Field{
        .id = "outside-child",
        .name = "Outside child",
        .range = {.offset = 15, .size = 4},
        .kind = FieldKind::bytes,
        .type_name = {},
        .display_value = {},
        .parent_id = "header-struct",
        .evidence_id = {},
    }));
    assert(!document.add_field(Field{
        .id = "version",
        .name = "Duplicate",
        .range = {.offset = 20, .size = 1},
        .kind = FieldKind::unknown,
        .type_name = {},
        .display_value = {},
        .parent_id = {},
        .evidence_id = {},
    }));
    assert(!document.add_field(Field{
        .id = "orphan",
        .name = "Orphan",
        .range = {.offset = 20, .size = 1},
        .kind = FieldKind::unknown,
        .type_name = {},
        .display_value = {},
        .parent_id = "missing-parent",
        .evidence_id = {},
    }));

    assert(document.add_ownership(OwnershipClaim{
        .owner_id = "parser.header",
        .range = {.offset = 0, .size = 12},
        .rationale = "Header parser owns the decoded prefix.",
    }));
    assert(document.add_ownership(OwnershipClaim{
        .owner_id = "parser.flags",
        .range = {.offset = 8, .size = 8},
        .rationale = "Flags parser claims the overlapping suffix.",
    }));
    assert(document.add_ownership(OwnershipClaim{
        .owner_id = "parser.header",
        .range = {.offset = 12, .size = 4},
        .rationale = "Same owner may hold another range.",
    }));
    assert(!document.add_ownership(OwnershipClaim{
        .owner_id = "parser.header",
        .range = {.offset = 12, .size = 4},
        .rationale = "Same owner may hold another range.",
    }));

    assert(document.add_annotation(Annotation{
        .id = "note-version",
        .range = {.offset = 4, .size = 4},
        .text = "Version is independently confirmed.",
        .evidence_id = "ev-version",
        .tags = {"confirmed", "header", "confirmed", ""},
    }));
    assert(!document.add_annotation(Annotation{
        .id = "note-version",
        .range = {.offset = 20, .size = 1},
        .text = "Duplicate ID",
        .evidence_id = {},
        .tags = {},
    }));

    assert(document.find_field("version") != nullptr);
    assert(document.find_field("missing") == nullptr);
    assert(document.find_annotation("note-version") != nullptr);
    assert(document.find_annotation("missing") == nullptr);

    const auto children = document.child_fields("header-struct");
    assert(children.size() == 2U);
    assert(children[0]->id == "version");
    assert(children[1]->id == "flags");

    const auto fields_at_five = document.fields_at(5U);
    assert(fields_at_five.size() == 2U);
    assert(fields_at_five[0]->id == "header-struct");
    assert(fields_at_five[1]->id == "version");

    const auto owners_at_nine = document.owners_at(9U);
    assert(owners_at_nine.size() == 2U);
    const auto annotations_at_five = document.annotations_at(5U);
    assert(annotations_at_five.size() == 1U);
    assert(annotations_at_five[0]->tags.size() == 2U);
    assert(annotations_at_five[0]->tags[0] == "confirmed");
    assert(annotations_at_five[0]->tags[1] == "header");

    const auto selection = document.selection_at(5U);
    assert(selection.regions.size() == 1U);
    assert(selection.fields.size() == 2U);
    assert(selection.owners.size() == 1U);
    assert(selection.annotations.size() == 1U);

    const auto ownership_conflicts = document.ownership_conflicts();
    assert(ownership_conflicts.size() == 2U);
    assert(ownership_conflicts[0].left_owner_id == "parser.header");
    assert(ownership_conflicts[0].right_owner_id == "parser.flags");
    assert(ownership_conflicts[0].overlap == ByteRange({.offset = 8, .size = 4}));
    assert(ownership_conflicts[1].left_owner_id == "parser.flags");
    assert(ownership_conflicts[1].right_owner_id == "parser.header");
    assert(ownership_conflicts[1].overlap == ByteRange({.offset = 12, .size = 4}));

    assert(ByteRange({.offset = 4, .size = 4}).contains(4U));
    assert(ByteRange({.offset = 4, .size = 4}).contains(7U));
    assert(!ByteRange({.offset = 4, .size = 4}).contains(8U));
    assert(ByteRange({.offset = 0, .size = 16}).contains(
        ByteRange({.offset = 4, .size = 4})));
    assert(!ByteRange({.offset = 0, .size = 8}).contains(
        ByteRange({.offset = 6, .size = 4})));

    return 0;
}
