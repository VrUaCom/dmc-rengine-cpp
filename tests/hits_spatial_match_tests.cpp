#include "dmc_rengine/hits/spatial_match.hpp"

#include <cassert>
#include <cstdint>

namespace {

[[nodiscard]] dmc::rengine::formats::hits::Triangle triangle(
    std::uint32_t flags,
    float x) {
    using dmc::rengine::formats::hits::Triangle;
    using dmc::rengine::formats::hits::Vec3;
    return Triangle{
        .offset = 0U,
        .flags = flags,
        .point_a = Vec3{x, 0.0F, 0.0F},
        .point_b = Vec3{x + 1.0F, 0.0F, 0.0F},
        .point_c = Vec3{x, 0.0F, 1.0F},
        .normal = Vec3{0.0F, -1.0F, 0.0F},
        .plane_d = 0.0F,
    };
}

[[nodiscard]] dmc::rengine::formats::hits::ScanResult scan() {
    dmc::rengine::formats::hits::ScanResult result;
    result.recognized = true;
    result.header.triangle_count = 2U;
    result.triangles = {
        triangle(0x18060001U, -2.0F),
        triangle(0x00000001U, 3.0F),
    };
    return result;
}

} // namespace

int main() {
    using namespace dmc::rengine::hits::spatial_match;

    const auto original = scan();
    const auto same = match_unique_geometry(original, original, 100U);
    assert(same.ok());
    assert(same.status == GeometryMatchStatus::matched);
    assert(same.mappings.size() == 2U);
    assert(same.mappings[0].stable_id == 100U);
    assert(same.mappings[0].original_triangle_byte_offset == 0U);
    assert(same.mappings[0].candidate_triangle_byte_offset == 0U);
    assert(same.mappings[1].stable_id == 101U);
    assert(same.mappings[1].original_triangle_byte_offset == 0x38U);
    assert(same.mappings[1].candidate_triangle_byte_offset == 0x38U);

    auto reordered = original;
    std::swap(reordered.triangles[0], reordered.triangles[1]);
    const auto reordered_match = match_unique_geometry(
        original, reordered, 100U);
    assert(reordered_match.ok());
    assert(reordered_match.mappings[0].candidate_triangle_byte_offset ==
           0x38U);
    assert(reordered_match.mappings[1].candidate_triangle_byte_offset ==
           0U);

    auto changed = original;
    changed.triangles[0].point_a.x += 0.25F;
    const auto unmatched = match_unique_geometry(original, changed);
    assert(!unmatched.ok());
    assert(unmatched.status == GeometryMatchStatus::unmatched_geometry);

    auto duplicate_original = original;
    duplicate_original.triangles[1] = duplicate_original.triangles[0];
    auto duplicate_candidate = duplicate_original;
    std::swap(
        duplicate_candidate.triangles[0],
        duplicate_candidate.triangles[1]);
    const auto ambiguous = match_unique_geometry(
        duplicate_original, duplicate_candidate);
    assert(!ambiguous.ok());
    assert(ambiguous.status == GeometryMatchStatus::ambiguous_geometry);

    auto shorter = original;
    shorter.triangles.pop_back();
    shorter.header.triangle_count = 1U;
    const auto count_mismatch = match_unique_geometry(original, shorter);
    assert(!count_mismatch.ok());
    assert(count_mismatch.status ==
           GeometryMatchStatus::triangle_count_mismatch);

    auto invalid = original;
    invalid.recognized = false;
    const auto invalid_original = match_unique_geometry(invalid, original);
    assert(invalid_original.status ==
           GeometryMatchStatus::invalid_original_scan);
    const auto invalid_candidate = match_unique_geometry(original, invalid);
    assert(invalid_candidate.status ==
           GeometryMatchStatus::invalid_candidate_scan);

    const auto invalid_first_id = match_unique_geometry(
        original, original, 0U);
    assert(invalid_first_id.status ==
           GeometryMatchStatus::stable_id_overflow);

    return 0;
}
