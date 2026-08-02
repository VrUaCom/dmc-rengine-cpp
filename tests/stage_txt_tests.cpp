#include "dmc_rengine/formats/stage_txt.hpp"
#include "dmc_rengine/formats/stage_txt_binary.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> bytes(std::string_view text) {
    std::vector<std::byte> result;
    result.reserve(text.size());
    for (const auto character : text) {
        result.push_back(static_cast<std::byte>(character));
    }
    return result;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "stage-txt-test",
            .logical_path = "room/st001cfg_004.txt",
            .container_chain = "NBZ[0]/PAC[4]",
            .offset = 12288U,
            .size = size,
        },
        .display_name = "st001cfg_004.txt",
        .format = "txt",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

} // namespace

int main() {
    using dmc::rengine::formats::stage_txt::Lexer;
    using dmc::rengine::formats::stage_txt::TokenKind;
    using dmc::rengine::formats::stage_txt::build_binary_document;

    const auto input = bytes(
        "#SET STAY\n"
        "DOOR BoxIn NextRoom \"st002\" 12 -3.5 // ignored\n"
        "/* block comment */ ORBREAK\n");
    const auto lex = Lexer::scan(std::span<const std::byte>{input});
    assert(lex.recognized);
    assert(lex.ok());
    assert(lex.diagnostics.empty());
    assert(lex.tokens.size() == 9U);
    assert(lex.tokens[0].kind == TokenKind::directive_set);
    assert(lex.tokens[0].value == "#SET");
    assert(lex.tokens[0].line == 1U);
    assert(lex.tokens[0].column == 1U);
    assert(lex.tokens[1].kind == TokenKind::stage_set_value);
    assert(lex.tokens[1].value == "STAY");
    assert(lex.tokens[2].kind == TokenKind::door);
    assert(lex.tokens[3].kind == TokenKind::box_in);
    assert(lex.tokens[4].kind == TokenKind::next_room);
    assert(lex.tokens[5].kind == TokenKind::string_literal);
    assert(lex.tokens[5].value == "st002");
    assert(lex.tokens[6].kind == TokenKind::number);
    assert(lex.tokens[6].value == "12");
    assert(lex.tokens[7].kind == TokenKind::number);
    assert(lex.tokens[7].value == "-3.5");
    assert(lex.tokens[8].kind == TokenKind::stage_set_value);
    assert(lex.tokens[8].value == "ORBREAK");
    assert(lex.by_kind(TokenKind::stage_set_value).size() == 2U);

    auto document = build_binary_document(
        resource(input.size()),
        std::span<const std::byte>{input},
        lex);
    assert(document.has_value());
    assert(document->regions().size() == 1U);
    assert(document->fields().size() == 9U);
    assert(document->ownership().size() == 1U);
    assert(document->coverage_bytes() == input.size());
    assert(document->unknown_ranges().empty());
    assert(document->fields()[0].evidence_id ==
        "ev-dmc3-stageset-token-classifier");
    assert(document->fields()[1].evidence_id ==
        "ev-dmc3-stageset-token-classifier");
    assert(document->fields()[2].evidence_id.empty());

    auto nul = input;
    nul.push_back(std::byte{0});
    const auto binary = Lexer::scan(std::span<const std::byte>{nul});
    assert(!binary.recognized);
    assert(!binary.ok());
    assert(binary.diagnostics[0].code == "stage-txt.binary-null");

    const auto unterminated_string = bytes("#SET \"STAY");
    const auto string_error = Lexer::scan(
        std::span<const std::byte>{unterminated_string});
    assert(string_error.recognized);
    assert(!string_error.ok());
    assert(!string_error.diagnostics.empty());

    const auto unterminated_comment = bytes("#SET STAY /* missing");
    const auto comment_error = Lexer::scan(
        std::span<const std::byte>{unterminated_comment});
    assert(comment_error.recognized);
    assert(!comment_error.ok());
    assert(comment_error.diagnostics[0].code ==
        "stage-txt.unterminated-comment");

    const std::vector<std::byte> empty;
    const auto empty_result = Lexer::scan(
        std::span<const std::byte>{empty});
    assert(!empty_result.recognized);
    assert(!empty_result.ok());
    assert(empty_result.diagnostics[0].code == "stage-txt.empty");

    return 0;
}
