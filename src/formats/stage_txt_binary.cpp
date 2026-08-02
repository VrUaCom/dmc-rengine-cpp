#include "dmc_rengine/formats/stage_txt_binary.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::formats::stage_txt {
namespace {

[[nodiscard]] std::string token_id(std::size_t index) {
    std::ostringstream output;
    output << "stage-txt-token-" << std::setfill('0')
           << std::setw(5) << index;
    return output.str();
}

[[nodiscard]] binary::FieldKind field_kind(const Token& token) noexcept {
    switch (token.kind) {
    case TokenKind::number:
        return token.value.find_first_of(".eE") == std::string::npos
            ? binary::FieldKind::signed_integer
            : binary::FieldKind::floating_point;
    case TokenKind::string_literal:
    case TokenKind::identifier:
        return binary::FieldKind::string;
    case TokenKind::directive_set:
    case TokenKind::door:
    case TokenKind::box_in:
    case TokenKind::next_room:
    case TokenKind::stage_set_value:
    case TokenKind::symbol:
        return binary::FieldKind::enumeration;
    }
    return binary::FieldKind::unknown;
}

[[nodiscard]] std::string evidence_id(TokenKind kind) {
    if (kind == TokenKind::directive_set ||
        kind == TokenKind::stage_set_value) {
        return "ev-dmc3-stageset-token-classifier";
    }
    return {};
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const LexResult& lex) {
    if (!resource.valid() || resource.id.size != bytes.size() ||
        !lex.recognized || bytes.empty()) {
        return std::nullopt;
    }

    binary::Document document(std::move(resource), bytes.size());
    if (!document.add_region(binary::Region{
            .id = "stage-txt-document",
            .name = "Stage text document",
            .range = {.offset = 0U, .size = bytes.size()},
            .kind = binary::RegionKind::payload,
            .type_name = "StageTextRaw",
            .evidence_id = {},
        })) {
        return std::nullopt;
    }
    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.stage-txt",
        .range = {.offset = 0U, .size = bytes.size()},
        .rationale = "The stage text lexer owns lexical token ranges while preserving unknown semantics.",
    }));

    for (std::size_t index = 0; index < lex.tokens.size(); ++index) {
        const auto& token = lex.tokens[index];
        if (!document.add_field(binary::Field{
                .id = token_id(index),
                .name = std::string(to_string(token.kind)),
                .range = {.offset = token.offset, .size = token.size},
                .kind = field_kind(token),
                .type_name = std::string(to_string(token.kind)),
                .display_value = token.value,
                .parent_id = {},
                .evidence_id = evidence_id(token.kind),
            })) {
            return std::nullopt;
        }
    }
    return document;
}

} // namespace dmc::rengine::formats::stage_txt
