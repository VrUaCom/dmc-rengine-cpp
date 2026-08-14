#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include "hits_test_fixture.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

int main() {
    using namespace dmc::rengine;

    const auto bytes = tests::hits_fixture::make_minimal_hits();
    const gdspaces::ResourceRef resource{
        .id = gdspaces::ResourceId{
            .source_id = "parsed-resource-workspace-test",
            .logical_path = "room/st001cfg_006.hits",
            .container_chain = "PAC[6]",
            .offset = 0x1000U,
            .size = static_cast<std::uint64_t>(bytes.size()),
        },
        .display_name = "HITS fixture",
        .format = "hits",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    integration::ProjectWorkspace project;
    assert(project.create_session(
        gdspaces::ResourcePayload{
            .resource = resource,
            .bytes = bytes,
            .diagnostics = {},
            .byte_provenance = gdspaces::ByteProvenance{
                .kind = gdspaces::ByteOriginKind::direct_source_span,
                .authority_id = "parsed-resource-workspace-source",
                .offset = resource.id.offset,
                .stored_size = static_cast<std::uint64_t>(bytes.size()),
                .materialized_size = static_cast<std::uint64_t>(bytes.size()),
                .transform = gdspaces::ByteTransform::none,
                .crc32 = std::nullopt,
            },
        },
        integration::WorkspaceContext{
            .stage_context = true,
            .menu_context = false,
            .evidence_context = true,
        }));

    const auto report = integration::ResourceAnalyzer::analyze(
        project, resource.id);
    assert(report.ok());

    const auto* session = project.find_session(resource.id);
    assert(session != nullptr);
    assert(session->binary_document() != nullptr);

    const auto* parsed = session->parsed_resource();
    assert(parsed != nullptr);
    assert(parsed->valid());
    assert(parsed->recognized());
    assert(parsed->parser_id == "formats.hits-record-scanner");

    const auto* hits = parsed->get_if<formats::hits::ScanResult>();
    assert(hits != nullptr);
    assert(hits->recognized);
    assert(hits->triangles.size() == 1U);
    assert(!hits->cells.empty());

    assert(session->events().by_type(
        integration::WorkspaceEventType::parser_completed).size() == 1U);
    assert(session->events().by_type(
        integration::WorkspaceEventType::parsed_resource_attached).size() == 1U);
    assert(session->events().by_type(
        integration::WorkspaceEventType::binary_document_attached).size() == 1U);

    // Re-analysis may refresh the canonical typed value because source bytes are
    // immutable, but it must remain one shared session value rather than forcing
    // Stage Ops to parse the resource independently.
    const auto second = integration::ResourceAnalyzer::analyze(
        project, resource.id);
    assert(second.ok());
    session = project.find_session(resource.id);
    assert(session != nullptr);
    parsed = session->parsed_resource();
    assert(parsed != nullptr);
    hits = parsed->get_if<formats::hits::ScanResult>();
    assert(hits != nullptr && hits->triangles.size() == 1U);
    assert(session->events().by_type(
        integration::WorkspaceEventType::parsed_resource_attached).size() == 2U);

    return 0;
}
