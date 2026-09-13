// The occurrence census, on synthetic containers.
//
// What a per-file corpus sweep cannot answer is how many of the SCMs it saw are
// the same payload. A stage set repeats scene models across its containers, so
// "16 at version 0.90" means two very different things depending on whether it
// counts occurrences or payloads, and every statement about how common a
// version or a family class is rests on that distinction.
//
// The mechanism under test is also the reason the census exists at all: the
// reader refuses a span that does not terminate exactly where the canonical
// layout ends, which is a condition no payload embedded in a container can
// meet. So the scan computes the extent from the layout authority first and
// judges the exact span second — and that extent is what makes the digest
// identify a payload rather than the tail of the file it happens to sit in.
//
// SafeProductValidation: every fixture here is synthetic. Nothing below asserts
// anything about a real retail container.

#include "scm_occurrence_census_commands.hpp"

#include "dmc_rengine/formats/scm_layout.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <span>
#include <string>
#include <vector>

namespace {

namespace scm = dmc::rengine::formats::scm;
using dmc::rengine::cli::scm_occurrence_detail::scan_container;

template <typename T>
void put(std::vector<std::byte>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

/// A minimal but complete SCM, built by the canonical layout authority.
[[nodiscard]] std::vector<std::byte> make_scm(
    float version,
    std::uint32_t resource_code,
    std::uint8_t lighting_node,
    std::uint16_t vertices) {
    scm::ObjectShape shape;
    shape.mesh_vertex_counts = {vertices};
    const std::vector<scm::ObjectShape> shapes{shape};
    const auto layout = scm::build_serialized_layout(
        std::span<const scm::ObjectShape>{shapes}, 1U);

    std::vector<std::byte> bytes(
        static_cast<std::size_t>(layout.file_size), std::byte{0});
    bytes[0] = std::byte{'S'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'M'};
    bytes[3] = std::byte{' '};
    put<float>(bytes, 0x04U, version);
    bytes[0x10U] = std::byte{1};
    bytes[0x11U] = std::byte{1};
    bytes[0x12U] = std::byte{1};
    bytes[0x13U] = static_cast<std::byte>(lighting_node);
    put<std::uint32_t>(bytes, 0x14U, resource_code);
    put<std::uint64_t>(bytes, 0x20U, layout.scene.block_offset);

    const auto& object_layout = layout.objects[0];
    const auto object_at =
        static_cast<std::size_t>(object_layout.record_offset);
    bytes[object_at] = std::byte{1};
    bytes[object_at + 0x01U] = std::byte{0x80};
    put<std::uint16_t>(bytes, object_at + 0x02U, vertices);
    put<std::uint64_t>(
        bytes, object_at + 0x08U, object_layout.mesh_table_offset);

    const auto& mesh_layout = object_layout.meshes[0];
    const auto mesh_at = static_cast<std::size_t>(mesh_layout.record_offset);
    put<std::uint16_t>(bytes, mesh_at + 0x00U, vertices);
    put<std::uint64_t>(bytes, mesh_at + 0x10U, mesh_layout.positions_offset);
    put<std::uint64_t>(bytes, mesh_at + 0x18U, mesh_layout.normals_offset);
    put<std::uint64_t>(bytes, mesh_at + 0x20U, mesh_layout.uv_offset);
    put<std::uint64_t>(bytes, mesh_at + 0x38U, mesh_layout.color_flags_offset);
    put<std::uint64_t>(
        bytes, mesh_at + 0x40U,
        mesh_layout.index_workspace_offset - mesh_layout.record_offset);
    put<std::uint16_t>(
        bytes, static_cast<std::size_t>(mesh_layout.index_workspace_offset),
        scm::index_workspace_sentinel);

    const auto scene_at = static_cast<std::size_t>(layout.scene.block_offset);
    put<std::uint32_t>(bytes, scene_at + 0x00U, layout.scene.parent_rel);
    put<std::uint32_t>(bytes, scene_at + 0x04U, layout.scene.order_rel);
    put<std::uint32_t>(
        bytes, scene_at + 0x08U, layout.scene.object_binding_rel);
    put<std::uint32_t>(bytes, scene_at + 0x0CU, layout.scene.transform_rel);
    bytes[scene_at + layout.scene.parent_rel] = std::byte{0xFF};
    return bytes;
}

/// Payloads laid into one file at 16-byte alignment, with filler between them.
[[nodiscard]] std::filesystem::path write_container(
    std::string_view name,
    const std::vector<std::vector<std::byte>>& payloads) {
    std::vector<std::byte> container(0x40U, std::byte{0xAB});
    for (const auto& payload : payloads) {
        while (container.size() % 16U != 0U) {
            container.push_back(std::byte{0xAB});
        }
        container.insert(container.end(), payload.begin(), payload.end());
        // Filler that is not a multiple of 16, so alignment is re-established
        // rather than accidental.
        container.insert(container.end(), 37U, std::byte{0xCD});
    }
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(
        reinterpret_cast<const char*>(container.data()),
        static_cast<std::streamsize>(container.size()));
    return path;
}

// The reason the scan is two passes. A payload followed by more bytes is what
// every embedded SCM looks like, and a single-pass reader rejects all of them.
void an_embedded_payload_is_found_despite_the_bytes_after_it() {
    const auto path = write_container(
        "dmc-rengine-census-embedded.bin",
        {make_scm(1.01F, 400115U, 0U, 3U)});
    const auto found = scan_container(path);
    assert(found.size() == 1U);
    assert(found[0].offset == 0x40U);
    assert(found[0].version > 1.0F && found[0].version < 1.02F);
    assert(found[0].resource_code == 400115U);
    assert(found[0].family_class == 4U);
    std::filesystem::remove(path);
}

// The statistic that was missing: occurrences against unique payloads.
void repeated_payloads_are_counted_once_by_digest() {
    const auto repeated = make_scm(1.01F, 400115U, 0U, 3U);
    const auto other = make_scm(0.90F, 730507U, 0U, 4U);
    const auto path = write_container(
        "dmc-rengine-census-repeats.bin", {repeated, other, repeated});

    const auto found = scan_container(path);
    assert(found.size() == 3U);

    std::set<std::string> unique;
    for (const auto& occurrence : found) unique.insert(occurrence.sha256);
    assert(unique.size() == 2U);

    // Byte-identical payloads at different offsets hash the same. Hashing to
    // end-of-file instead of to the computed extent would make all three
    // distinct and the deduplication would report nothing.
    assert(found[0].sha256 == found[2].sha256);
    assert(found[0].sha256 != found[1].sha256);
    assert(found[0].offset != found[2].offset);

    std::filesystem::remove(path);
}

// A legacy revision reviving a preservation domain is the single most
// important thing a scan of new containers could find, so the counters have to
// move when one does — a scan that always reports zero would look identical to
// a clean corpus.
void a_revived_preservation_lane_is_counted() {
    auto clean = make_scm(1.01F, 400115U, 0U, 3U);
    const auto clean_path =
        write_container("dmc-rengine-census-clean.bin", {clean});
    const auto clean_found = scan_container(clean_path);
    assert(clean_found.size() == 1U);
    assert(!clean_found[0].header_preservation_nonzero);
    assert(clean_found[0].mesh_preservation_nonzero == 0U);
    std::filesystem::remove(clean_path);

    // Header +0x18 and mesh +0x0C, both terminal preservation-only lanes.
    auto revived = clean;
    put<std::uint64_t>(revived, 0x18U, 0x1122334455667788ULL);
    const auto layout = scm::build_serialized_layout(
        std::span<const scm::ObjectShape>{
            std::vector<scm::ObjectShape>{scm::ObjectShape{{3U}}}},
        1U);
    const auto mesh_at = static_cast<std::size_t>(
        layout.objects[0].meshes[0].record_offset);
    put<std::uint32_t>(revived, mesh_at + 0x0CU, 0xDEADBEEFU);

    const auto revived_path =
        write_container("dmc-rengine-census-revived.bin", {revived});
    const auto revived_found = scan_container(revived_path);
    assert(revived_found.size() == 1U);
    assert(revived_found[0].header_preservation_nonzero);
    assert(revived_found[0].mesh_preservation_nonzero == 1U);
    std::filesystem::remove(revived_path);
}

// The lighting reference is read as a field now, not as a reserved byte.
void the_lighting_reference_node_is_carried_per_occurrence() {
    const auto path = write_container(
        "dmc-rengine-census-lighting.bin",
        {make_scm(1.01F, 400115U, 0U, 3U)});
    const auto found = scan_container(path);
    assert(found.size() == 1U);
    assert(found[0].lighting_reference_node == 0U);
    assert(found[0].scene_node_count == 1U);
    std::filesystem::remove(path);
}

// The magic is not the test. A container carrying the four bytes inside
// unrelated data must not contribute an occurrence.
void a_bare_magic_match_is_not_an_occurrence() {
    std::vector<std::byte> junk(0x200U, std::byte{0x5A});
    junk[0x40U] = std::byte{'S'};
    junk[0x41U] = std::byte{'C'};
    junk[0x42U] = std::byte{'M'};
    junk[0x43U] = std::byte{' '};
    const auto path =
        std::filesystem::temp_directory_path() / "dmc-rengine-census-junk.bin";
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(
            reinterpret_cast<const char*>(junk.data()),
            static_cast<std::streamsize>(junk.size()));
    }
    assert(scan_container(path).empty());
    std::filesystem::remove(path);
}

[[nodiscard]] int run(const std::vector<std::string>& arguments) {
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (const auto& argument : arguments) {
        argv.push_back(const_cast<char*>(argument.c_str()));
    }
    return dmc::rengine::cli::try_run_scm_occurrence_census_command(
        static_cast<int>(argv.size()), argv.data());
}

void the_command_declines_what_is_not_its_own() {
    assert(run({"dmc-rengine", "verify-scm-corpus", "."}) == -1);
    assert(run({"dmc-rengine"}) == -1);
    assert(run({"dmc-rengine", "census-scm-occurrences"}) == 1);
    assert(run({"dmc-rengine", "census-scm-occurrences",
                "definitely-not-a-real-container.pac"}) == 2);
}

} // namespace

int main() {
    an_embedded_payload_is_found_despite_the_bytes_after_it();
    repeated_payloads_are_counted_once_by_digest();
    a_revived_preservation_lane_is_counted();
    the_lighting_reference_node_is_carried_per_occurrence();
    a_bare_magic_match_is_not_an_occurrence();
    the_command_declines_what_is_not_its_own();
    return 0;
}
