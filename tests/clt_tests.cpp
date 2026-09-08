#include "dmc_rengine/formats/clt.hpp"

#include <cassert>
#include <cstddef>
#include <string_view>
#include <vector>

namespace clt = dmc::rengine::formats::clt;

namespace {

[[nodiscard]] std::vector<std::byte> bytes_with_padding(
    std::string_view text,
    std::size_t padding = 8U) {
    std::vector<std::byte> out;
    out.reserve(text.size() + padding);
    for (const char ch : text) {
        out.push_back(static_cast<std::byte>(static_cast<unsigned char>(ch)));
    }
    out.insert(out.end(), padding, std::byte{0});
    return out;
}

} // namespace

int main() {
    const auto valid = bytes_with_padding(
        ";em002_01.clt\r\n"
        "\r\n"
        "ClothNum 1\r\n"
        "ClothNo 0\r\n"
        "ClothId 0\r\n"
        "Gravity 0.000000 -0.050000 0.000000\r\n"
        "SpringForce 0.060000\r\n"
        "MaxSpeed 30.000000\r\n"
        "Stiffness 0.150000\r\n"
        "Wind 0.000000 0.000000 0.000000\r\n"
        "WindLocal 0\r\n"
        "WindParent 9\r\n"
        "WindType 0\r\n"
        "Bone 2 NY\r\n"
        "Bone 3 NY\r\n"
        "Bone 4 NY\r\n"
        "Bone 5 NY\r\n"
        "End\r\n"
        "$\r\n");

    const auto parsed = clt::Parser::parse(valid);
    assert(parsed.ok());
    assert(parsed.document->valid());
    assert(parsed.document->source_bytes == valid);
    assert(parsed.document->embedded_name == "em002_01.clt");
    assert(parsed.document->cloth_num == 1U);
    assert(parsed.document->cloth_no == 0U);
    assert(parsed.document->cloth_id == 0U);
    assert(parsed.document->wind_local_raw == 0U);
    assert(parsed.document->wind_parent_raw == 9U);
    assert(parsed.document->wind_type_raw == 0U);
    assert(!parsed.document->limit_length_raw.has_value());
    assert(parsed.document->bones.size() == 4U);
    assert(parsed.document->bones.front().node_index == 2U);
    assert(parsed.document->bones.front().axis_token == "NY");
    assert(parsed.document->bones.back().node_index == 5U);
    assert(parsed.document->zero_padding_size == 8U);

    const auto with_limit = bytes_with_padding(
        ";em005_02.clt\n"
        "ClothNum 1\n"
        "ClothNo 0\n"
        "ClothId 0\n"
        "WindLocal 1\n"
        "WindParent 0\n"
        "WindType 1\n"
        "Bone 2 Z\n"
        "Bone 3 Z\n"
        "Bone 4 Z\n"
        "LimitLength 0\n"
        "End\n"
        "$\n");
    const auto limit_result = clt::Parser::parse(with_limit);
    assert(limit_result.ok());
    assert(limit_result.document->limit_length_raw == 0U);
    assert(limit_result.document->bones.size() == 3U);
    assert(limit_result.document->bones[0].axis_token == "Z");

    auto bad_tail = valid;
    bad_tail.back() = std::byte{1};
    const auto tail_result = clt::Parser::parse(bad_tail);
    assert(!tail_result.ok());
    assert(tail_result.error == clt::ParseError::nonzero_tail);

    const auto bad_bone = bytes_with_padding(
        ";em000_01.clt\n"
        "Bone not-a-node Y\n"
        "$\n");
    const auto bone_result = clt::Parser::parse(bad_bone);
    assert(!bone_result.ok());
    assert(bone_result.error == clt::ParseError::invalid_bone_directive);

    const auto wrong_identity = bytes_with_padding(
        ";em000_01.txt\n"
        "Bone 2 Y\n"
        "$\n");
    const auto identity_result = clt::Parser::parse(wrong_identity);
    assert(!identity_result.ok());
    assert(identity_result.error == clt::ParseError::missing_clt_identity);

    return 0;
}
