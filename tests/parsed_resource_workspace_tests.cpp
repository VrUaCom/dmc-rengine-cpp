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
    assert(report.byte_source == integration::ParsedByteSource::immutable_source);
    assert(report.byte_revision == 0U);

    const auto* session = project.find_session(resource.id);
    assert(session != nullptr);
    assert(session->binary_document() != nullptr);

    const auto* parsed = session->parsed_resource();
    assert(parsed != nullptr);
    assert(parsed->valid());
    assert(parsed->recognized());
    assert(parsed->parser_id == "formats.hits-record-scanner");
    assert(parsed->byte_source == integration::ParsedByteSource::immutable_source);
    assert(parsed->byte_revision == 0U);

    const auto* hits = parsed->get_if<formats::hits::ScanResult>();
    assert(hits != nullptr);
    assert(hits->recognized);
    assert(hits->triangles.size() == 1U);
    assert(!hits->cells.empty());

    auto parser_events = session->events().by_type(
        integration::WorkspaceEventType::parser_completed);
    assert(parser_events.size() == 1U);
    assert(parser_events[0]->revision == 0U);
    auto parsed_events = session->events().by_type(
        integration::WorkspaceEventType::parsed_resource_attached);
    assert(parsed_events.size() == 1U);
    assert(parsed_events[0]->revision == 0U);
    assert(session->events().by_type(
        integration::WorkspaceEventType::binary_document_attached).size() == 1U);

    // Source re-analysis remains explicitly source@0 even if a WorkingCopy later
    // exists. Parser provenance must never be inferred from session state.
    assert(project.enable_working_copy(resource.id));
    const auto source_06 = bytes[0x06U];
    const auto edit = project.apply_edit(
        resource.id,
        gdspaces::EditOperation{
            .id = "working-copy-parser-revision",
            .base_revision = 0U,
            .offset = 0x06U,
            .expected = {source_06},
            .replacement = {std::byte{0x09}},
            .description = "Create WorkingCopy revision one for parser lineage.",
        },
        gdspaces::ToolTarget::stage_ops);
    assert(edit.applied && edit.revision == 1U);

    const auto source_again = integration::ResourceAnalyzer::analyze(
        project, resource.id);
    assert(source_again.ok());
    assert(source_again.byte_source == integration::ParsedByteSource::immutable_source);
    assert(source_again.byte_revision == 0U);
    session = project.find_session(resource.id);
    assert(session != nullptr);
    parsed = session->parsed_resource();
    assert(parsed != nullptr);
    assert(parsed->byte_source == integration::ParsedByteSource::immutable_source);
    assert(parsed->byte_revision == 0U);
    parser_events = session->events().by_type(
        integration::WorkspaceEventType::parser_completed);
    assert(parser_events.size() == 2U);
    assert(parser_events.back()->revision == 0U);

    // Canonical WorkingCopy analysis uses the same parser adapter but retains an
    // exact working-copy@1 typed result and parser-completion event.
    const auto working_report = integration::ResourceAnalyzer::analyze_working_copy(
        project, resource.id);
    assert(working_report.ok());
    assert(working_report.byte_source == integration::ParsedByteSource::working_copy);
    assert(working_report.byte_revision == 1U);

    session = project.find_session(resource.id);
    assert(session != nullptr);
    parsed = session->parsed_resource();
    assert(parsed != nullptr);
    assert(parsed->recognized());
    assert(parsed->byte_source == integration::ParsedByteSource::working_copy);
    assert(parsed->byte_revision == 1U);
    hits = parsed->get_if<formats::hits::ScanResult>();
    assert(hits != nullptr && hits->triangles.size() == 1U);

    parser_events = session->events().by_type(
        integration::WorkspaceEventType::parser_completed);
    assert(parser_events.size() == 3U);
    assert(parser_events.back()->revision == 1U);
    parsed_events = session->events().by_type(
        integration::WorkspaceEventType::parsed_resource_attached);
    assert(parsed_events.size() == 3U);
    assert(parsed_events.back()->revision == 1U);

    return 0;
}
