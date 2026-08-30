#include "dmc_rengine/binary/manifest.hpp"
#include "dmc_rengine/core/json.hpp"

#include <cassert>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::binary::Document make_document() {
    return dmc::rengine::binary::Document(
        dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "synthetic",
                .logical_path = "manifest-fixture.bin",
                .container_chain = "SLTC[1]",
                .offset = 100,
                .size = 32,
            },
            .display_name = "manifest-fixture.bin",
            .format = "bin",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        32);
}

} // namespace

int main() {
    auto document = make_document();
    assert(document.add_region(dmc::rengine::binary::Region{
        .id = "header",
        .name = "Header",
        .range = {.offset = 0, .size = 8},
        .kind = dmc::rengine::binary::RegionKind::header,
        .type_name = "HeaderV1",
        .evidence_id = "ev-header",
    }));
    assert(document.add_region(dmc::rengine::binary::Region{
        .id = "overlap",
        .name = "Overlap",
        .range = {.offset = 6, .size = 6},
        .kind = dmc::rengine::binary::RegionKind::record,
        .type_name = "Record",
        .evidence_id = {},
    }));
    assert(document.add_field(dmc::rengine::binary::Field{
        .id = "header-struct",
        .name = "Header structure",
        .range = {.offset = 0, .size = 8},
        .kind = dmc::rengine::binary::FieldKind::structure,
        .type_name = "HeaderV1",
        .display_value = {},
        .parent_id = {},
        .evidence_id = "ev-header",
    }));
    assert(document.add_field(dmc::rengine::binary::Field{
        .id = "version",
        .name = "Version",
        .range = {.offset = 4, .size = 2},
        .kind = dmc::rengine::binary::FieldKind::unsigned_integer,
        .type_name = "u16_le",
        .display_value = "1",
        .parent_id = "header-struct",
        .evidence_id = "ev-version",
    }));
    assert(document.add_ownership(dmc::rengine::binary::OwnershipClaim{
        .owner_id = "parser.header",
        .range = {.offset = 0, .size = 8},
        .rationale = "Owns the header.",
    }));
    assert(document.add_ownership(dmc::rengine::binary::OwnershipClaim{
        .owner_id = "parser.record",
        .range = {.offset = 6, .size = 6},
        .rationale = "Owns the record.",
    }));
    assert(document.add_annotation(dmc::rengine::binary::Annotation{
        .id = "note",
        .range = {.offset = 4, .size = 2},
        .text = "Version \"one\".\nConfirmed.",
        .evidence_id = "ev-version",
        .tags = {"confirmed", "version"},
    }));

    const auto first = dmc::rengine::binary::manifest_json(document);
    const auto second = dmc::rengine::binary::manifest_json(document);
    assert(!first.empty());
    assert(first == second);
    assert(first.find("\"schema_version\": 1") != std::string::npos);
    assert(first.find("\"canonical_id\": \"rid2|9:synthetic|20:manifest-fixture.bin|7:SLTC[1]|100|32\"") !=
        std::string::npos);
    assert(first.find("\"coverage_bytes\": 12") != std::string::npos);
    assert(first.find("\"unknown_ranges\"") != std::string::npos);
    assert(first.find("\"region_conflicts\"") != std::string::npos);
    assert(first.find("\"ownership_conflicts\"") != std::string::npos);
    assert(first.find("Version \\\"one\\\".\\nConfirmed.") != std::string::npos);

    const auto parsed = dmc::rengine::core::json::Parser::parse(first);
    assert(parsed.ok());
    const auto* root = parsed.value->as_object();
    assert(root != nullptr);
    assert(*root->at("byte_size").as_u64() == 32U);
    assert(*root->at("coverage_bytes").as_u64() == 12U);
    assert(root->at("regions").as_array()->size() == 2U);
    assert(root->at("fields").as_array()->size() == 2U);
    assert(root->at("ownership").as_array()->size() == 2U);
    assert(root->at("annotations").as_array()->size() == 1U);
    assert(root->at("unknown_ranges").as_array()->size() == 1U);
    assert(root->at("region_conflicts").as_array()->size() == 1U);
    assert(root->at("ownership_conflicts").as_array()->size() == 1U);

    const dmc::rengine::binary::Document invalid({}, 0U);
    assert(dmc::rengine::binary::manifest_json(invalid).empty());

    return 0;
}
