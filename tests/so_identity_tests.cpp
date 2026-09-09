// What makes an SO payload an SO payload.
//
// The SO family has three readers, three test files and — until now — no
// caller in classification. That is the project's recurring defect: a
// capability that exists and cannot be reached. Wiring it required first
// answering a question the readers had never been asked, because two of the
// three recognized a payload by its length alone:
//
//   so::link_table::parse   accepted 305 of a 306-payload em000 extraction
//   so::volume_table::parse accepted  28 of the same
//   so::graph::parse        accepted   1
//
// A reader that recognizes everything tells a classifier nothing. What is
// asserted here is what each reader now requires instead, and that a slot
// carrying one of the three is named by it.
//
// Corpus: the complete em000 extraction supplied 2026-09-08, source archive
// SHA-256 306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b,
// slots 38, 39 and 40 of `em000.pac`.

#include "dmc_rengine/formats/so.hpp"

#include "dmc_rengine/gdspaces/classifier.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <span>
#include <vector>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace graph = dmc::rengine::formats::so::graph;
namespace links = dmc::rengine::formats::so::link_table;
namespace volumes = dmc::rengine::formats::so::volume_table;

void put_u16(std::vector<std::byte>& bytes, std::size_t at, std::uint16_t value) {
    bytes[at] = static_cast<std::byte>(value & 0xFFU);
    bytes[at + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, std::size_t at, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[at + index] = static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
    }
}

void put_f32(std::vector<std::byte>& bytes, std::size_t at, float value) {
    std::uint32_t raw{};
    std::memcpy(&raw, &value, sizeof(raw));
    put_u32(bytes, at, raw);
}

// `count` spheres, laid out the way the corpus lays one out.
[[nodiscard]] std::vector<std::byte> make_volumes(std::size_t count) {
    std::vector<std::byte> bytes(count * volumes::record_size, std::byte{0});
    for (std::size_t index = 0U; index < count; ++index) {
        const auto record = index * volumes::record_size;
        put_u32(bytes, record, volumes::sphere_record_type);
        put_f32(bytes, record + volumes::first_vector_offset + 0x0CU,
            volumes::position_w);
        put_f32(bytes, record + 0x20U, 50.0F);
    }
    return bytes;
}

// A leading word and one record per volume, each binding a volume to a node.
[[nodiscard]] std::vector<std::byte> make_links(
    const std::vector<std::uint8_t>& nodes) {
    std::vector<std::byte> bytes(
        links::leading_word_size + nodes.size() * links::record_size,
        std::byte{0});
    bytes[0] = static_cast<std::byte>(links::leading_word_value);
    for (std::size_t index = 0U; index < nodes.size(); ++index) {
        const auto record = links::leading_word_size + index * links::record_size;
        bytes[record] = std::byte{1};
        bytes[record + 1U] = std::byte{9};
        bytes[record + links::node_field_offset] =
            static_cast<std::byte>(nodes[index]);
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_graph() {
    std::vector<std::byte> bytes(0x80U, std::byte{0});
    put_u16(bytes, 0x00U, 6U);     // root type
    put_u16(bytes, 0x02U, 0x40U);  // boundary: where the type-8 block starts
    put_u16(bytes, 0x0EU, 0x12U);  // first entry, closing a one-entry table
    put_u16(bytes, 0x10U, 0x20U);
    put_u16(bytes, 0x40U, 8U);     // the companion's type
    put_u16(bytes, 0x48U, 0x0CU);
    put_u16(bytes, 0x4AU, 0x18U);
    return bytes;
}

void a_volume_record_is_more_than_its_length() {
    assert(volumes::recognizes(make_volumes(23U)));

    // Each invariant, removed on its own. Against the bound corpus each one
    // by itself cuts a size-only match from 28 payloads to one, so each has
    // to be load-bearing here too.
    auto unknown_kind = make_volumes(2U);
    put_u32(unknown_kind, volumes::record_size, 7U);
    assert(!volumes::recognizes(unknown_kind));

    auto dirty_reserved = make_volumes(2U);
    dirty_reserved[volumes::record_size + volumes::reserved_offset] =
        std::byte{1};
    assert(!volumes::recognizes(dirty_reserved));

    // vector0 is a position, so its w is 1. A zero there is what a buffer of
    // the right length looks like, and it is not a volume.
    auto not_a_position = make_volumes(2U);
    put_f32(not_a_position,
        volumes::record_size + volumes::first_vector_offset + 0x0CU, 0.0F);
    assert(!volumes::recognizes(not_a_position));

    // A run of zeros divides by 0x50 and is not a volume table.
    const std::vector<std::byte> empty(4U * volumes::record_size, std::byte{0});
    assert(!volumes::recognizes(empty));
}

void a_link_table_needs_both_of_its_invariants() {
    const auto table = make_links({1U, 2U, 0U, 2U});
    assert(links::recognizes(table));

    // Neither invariant is sufficient alone against the corpus, so neither may
    // be dropped here: without the leading word one effect record survives,
    // without the reserved byte the ten effect-M companions do.
    auto no_leading = table;
    no_leading[0] = std::byte{0};
    assert(!links::recognizes(no_leading));

    auto dirty_reserved = table;
    dirty_reserved[links::leading_word_size + links::record_size +
        links::reserved_field_offset] = std::byte{1};
    assert(!links::recognizes(dirty_reserved));

    // The third byte is a node reference, not the record's own ordinal: it
    // repeats for volumes sharing a node and is zero for those on the root.
    // Reading it as a self-index makes a valid table look corrupt.
    const auto parsed = links::parse(table);
    assert(parsed.ok());
    // The reader returns the leading word as records[0], which is what
    // analysis::so::correlate_companions counts as the header when it checks
    // one header plus one link per volume.
    assert(parsed.records.size() == 5U);
    assert(parsed.records[0].field0 == links::leading_word_value);
    assert(parsed.records[3].field2 == 0U);
    assert(parsed.records[4].field2 == parsed.records[2].field2);
    assert(parsed.records[4].field2 == 2U);
}

void the_classifier_names_all_three_from_a_nameless_slot() {
    const auto so_graph = gdspaces::ResourceClassifier::classify(
        "slot_0038.bin", std::span<const std::byte>{make_graph()});
    assert(so_graph.format == "so-graph");
    assert(so_graph.structural_confirmed);
    assert(!so_graph.magic_confirmed);

    const auto links_bytes = make_links({1U, 2U, 3U});
    const auto so_link = gdspaces::ResourceClassifier::classify(
        "slot_0039.bin", std::span<const std::byte>{links_bytes});
    assert(so_link.format == "so-link");

    const auto volumes_bytes = make_volumes(23U);
    const auto so_volume = gdspaces::ResourceClassifier::classify(
        "slot_0040.bin", std::span<const std::byte>{volumes_bytes});
    assert(so_volume.format == "so-volume");

    // And a payload that is none of the three keeps falling through, rather
    // than being swept up by the weakest gate.
    const std::vector<std::byte> filler(64U, std::byte{0xC3});
    assert(
        gdspaces::ResourceClassifier::classify(
            "slot_0031.bin", std::span<const std::byte>{filler}).format !=
        "so-link");
}

} // namespace

int main() {
    a_volume_record_is_more_than_its_length();
    a_link_table_needs_both_of_its_invariants();
    the_classifier_names_all_three_from_a_nameless_slot();
    std::cout << "so_identity_tests: all assertions held\n";
    return EXIT_SUCCESS;
}
