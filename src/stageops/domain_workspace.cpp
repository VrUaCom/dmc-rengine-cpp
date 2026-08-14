#include "dmc_rengine/stageops/domain_workspace.hpp"

#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/hits.hpp"
#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/stage_txt.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace dmc::rengine::stageops {
namespace {

[[nodiscard]] std::string domain_id(
    StageDomainKind kind,
    std::string_view resource_id) {
    return "stage-domain:" + std::string{to_string(kind)} + ":" +
        std::string{resource_id};
}

[[nodiscard]] std::string token_domain_id(
    StageDomainKind kind,
    std::string_view resource_id,
    std::uint64_t offset) {
    return domain_id(kind, resource_id) + ":offset/" +
        std::to_string(offset);
}

[[nodiscard]] bool parsed_result_current(
    const integration::ParsedResourceResult& parsed,
    const integration::ResourceWorkspaceSession& session) noexcept {
    const auto* working = session.working_copy();
    if (parsed.byte_source == integration::ParsedByteSource::immutable_source) {
        // Source parse remains byte-current while no dirty WorkingCopy exists.
        // A clean WorkingCopy may have a nonzero history revision after undo,
        // but its bytes equal the immutable source again.
        return working == nullptr || !working->dirty();
    }

    return working != nullptr &&
        parsed.byte_revision == working->revision();
}

void add_object(
    StageDomainWorkspace& workspace,
    StageDomainKind kind,
    const integration::ParsedResourceResult& parsed,
    std::string resource_id,
    bool current,
    std::map<std::string, std::string, std::less<>> attributes) {
    workspace.objects.push_back(StageDomainObject{
        .id = domain_id(kind, resource_id),
        .kind = kind,
        .resource_id = std::move(resource_id),
        .parser_id = parsed.parser_id,
        .byte_source = parsed.byte_source,
        .byte_revision = parsed.byte_revision,
        .current_for_active_bytes = current,
        .attributes = std::move(attributes),
    });
}

[[nodiscard]] std::optional<StageDomainKind> marker_kind(
    formats::stage_txt::TokenKind kind) noexcept {
    using formats::stage_txt::TokenKind;
    switch (kind) {
    case TokenKind::directive_set:
        return StageDomainKind::stage_set_directive_token;
    case TokenKind::stage_set_value:
        return StageDomainKind::stage_set_value_token;
    case TokenKind::door:
        return StageDomainKind::door_token;
    case TokenKind::box_in:
        return StageDomainKind::box_in_token;
    case TokenKind::next_room:
        return StageDomainKind::next_room_token;
    case TokenKind::identifier:
    case TokenKind::number:
    case TokenKind::string_literal:
    case TokenKind::symbol:
        return std::nullopt;
    }
    return std::nullopt;
}

void add_stage_txt_marker(
    StageDomainWorkspace& workspace,
    const integration::ParsedResourceResult& parsed,
    std::string_view resource_id,
    bool current,
    const formats::stage_txt::Token& token,
    StageDomainKind kind) {
    workspace.objects.push_back(StageDomainObject{
        .id = token_domain_id(kind, resource_id, token.offset),
        .kind = kind,
        .resource_id = std::string{resource_id},
        .parser_id = parsed.parser_id,
        .byte_source = parsed.byte_source,
        .byte_revision = parsed.byte_revision,
        .current_for_active_bytes = current,
        .attributes = {
            {"lexical_authority", "stage-txt-token"},
            {"token_kind", std::string{formats::stage_txt::to_string(token.kind)}},
            {"value", token.value},
            {"offset", std::to_string(token.offset)},
            {"size", std::to_string(token.size)},
            {"line", std::to_string(token.line)},
            {"column", std::to_string(token.column)},
            {"runtime_object_claim", "false"},
        },
    });
}

void project_stage_txt(
    StageDomainWorkspace& workspace,
    const integration::ParsedResourceResult& parsed,
    std::string resource_id,
    bool current,
    const formats::stage_txt::LexResult& script) {
    add_object(
        workspace,
        StageDomainKind::stage_script_tokens,
        parsed,
        resource_id,
        current,
        {
            {"token_count", std::to_string(script.tokens.size())},
            {"directive_set_count", std::to_string(
                script.by_kind(
                    formats::stage_txt::TokenKind::directive_set).size())},
            {"stage_set_value_count", std::to_string(
                script.by_kind(
                    formats::stage_txt::TokenKind::stage_set_value).size())},
            {"door_count", std::to_string(
                script.by_kind(formats::stage_txt::TokenKind::door).size())},
            {"box_in_count", std::to_string(
                script.by_kind(formats::stage_txt::TokenKind::box_in).size())},
            {"next_room_count", std::to_string(
                script.by_kind(
                    formats::stage_txt::TokenKind::next_room).size())},
        });

    // These are exact lexer-emitted structural markers. No adjacency or grammar
    // relationship is promoted here: e.g. `#SET STAY` remains two lexical facts
    // until a grammar/runtime consumer contract separately proves their relation.
    for (const auto& token : script.tokens) {
        const auto kind = marker_kind(token.kind);
        if (!kind.has_value()) {
            continue;
        }
        add_stage_txt_marker(
            workspace,
            parsed,
            resource_id,
            current,
            token,
            *kind);
    }
}

void project_typed_result(
    StageDomainWorkspace& workspace,
    const integration::ParsedResourceResult& parsed,
    std::string resource_id,
    bool current) {
    if (const auto* hits = parsed.get_if<formats::hits::ScanResult>();
        hits != nullptr) {
        add_object(
            workspace,
            StageDomainKind::hits_collision,
            parsed,
            std::move(resource_id),
            current,
            {
                {"cell_count", std::to_string(hits->cells.size())},
                {"triangle_count", std::to_string(hits->triangles.size())},
                {"grid_count_x", std::to_string(hits->header.grid_count_x)},
                {"grid_count_y", std::to_string(hits->header.grid_count_y)},
                {"grid_count_z", std::to_string(hits->header.grid_count_z)},
            });
        return;
    }

    if (const auto* dca = parsed.get_if<formats::dca::ScanResult>();
        dca != nullptr) {
        add_object(
            workspace,
            StageDomainKind::dca_records,
            parsed,
            std::move(resource_id),
            current,
            {{"record_count", std::to_string(dca->records.size())}});
        return;
    }

    if (const auto* lighting = parsed.get_if<formats::lig2::ScanResult>();
        lighting != nullptr) {
        add_object(
            workspace,
            StageDomainKind::lighting_records,
            parsed,
            std::move(resource_id),
            current,
            {{"record_count", std::to_string(lighting->records.size())}});
        return;
    }

    if (const auto* script = parsed.get_if<formats::stage_txt::LexResult>();
        script != nullptr) {
        project_stage_txt(
            workspace,
            parsed,
            std::move(resource_id),
            current,
            *script);
    }
}

} // namespace

std::vector<const StageDomainObject*> StageDomainWorkspace::by_kind(
    StageDomainKind kind) const {
    std::vector<const StageDomainObject*> result;
    for (const auto& object : objects) {
        if (object.kind == kind) {
            result.push_back(&object);
        }
    }
    return result;
}

StageDomainWorkspace StageDomainAssembler::assemble(
    const StageAssemblyWorkspace& assembly,
    const integration::ProjectWorkspace& project,
    std::uint64_t source_stage_revision) {
    StageDomainWorkspace workspace;
    if (!assembly.valid()) {
        return workspace;
    }

    workspace.identity = assembly.identity;
    workspace.source_stage_revision = source_stage_revision;

    for (const auto& resource : assembly.resources) {
        const auto* session = project.find_session(resource.resource.id);
        if (session == nullptr) {
            ++workspace.missing_project_session_count;
            continue;
        }

        const auto* parsed = session->parsed_resource();
        if (parsed == nullptr || !parsed->valid()) {
            ++workspace.missing_typed_result_count;
            continue;
        }
        if (!parsed->recognized()) {
            ++workspace.unrecognized_typed_result_count;
            continue;
        }

        const auto current = parsed_result_current(*parsed, *session);
        if (!current) {
            ++workspace.stale_typed_result_count;
        }
        project_typed_result(
            workspace,
            *parsed,
            resource.resource.id.canonical(),
            current);
    }

    std::sort(
        workspace.objects.begin(), workspace.objects.end(),
        [](const StageDomainObject& left, const StageDomainObject& right) {
            return left.id < right.id;
        });
    return workspace;
}

} // namespace dmc::rengine::stageops
