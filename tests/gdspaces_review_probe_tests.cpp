// Characterization probe for the GDSpaces Layer-1 review (revision 2).
//
// IMPORTANT: these assertions pin CURRENT behaviour, not desired behaviour.
// Two of them document confirmed defects. When a defect is fixed, the matching
// assertion is expected to flip, and the fix should invert it here rather than
// delete it.
//
//   P1  ResourceId::canonical() is not injective. Distinct declared identities
//       can produce one key, and ResourceGraph then drops the second.
//   P2  NbzZipSerializationSnapshot::valid() does not verify that the preserved
//       local regions account for every source byte between prefix_size and
//       computed_central_start.
//   P3  NbzZipSerializationEntry::valid() does not re-derive
//       46 + name + extra + comment from the preserved central record's own
//       header fields.
//
// P2/P3 are unchanged by the #150 artifact binder: it forbids forging a
// snapshot, and it re-derives the LOCAL prefix length exactly, but it never
// checks local-region coverage and only bounds the central record length with
// an inequality. See docs note in the review document.

#include "dmc_rengine/gdspaces/nbz_zip_serialization.hpp"
#include "dmc_rengine/gdspaces/resource_graph.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using namespace dmc::rengine::gdspaces;

[[nodiscard]] ResourceRef make_ref(
    std::string logical_path,
    std::string container_chain) {
    return ResourceRef{
        .id = ResourceId{
            .source_id = "corpus",
            .logical_path = std::move(logical_path),
            .container_chain = std::move(container_chain),
            .offset = 64U,
            .size = 128U,
        },
        .display_name = "a.bin",
        .format = "bin",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

void put_u32_le(
    std::vector<std::byte>& output,
    std::size_t at,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        output[at + index] = static_cast<std::byte>(
            (value >> (8U * static_cast<unsigned>(index))) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> signed_record(
    std::size_t size,
    std::uint32_t signature) {
    std::vector<std::byte> record(size, std::byte{0});
    put_u32_le(record, 0U, signature);
    return record;
}

// P1: two distinct ResourceIds, differing only in where the same text is split
// between logical_path and container_chain, collapse to one canonical key.
void probe_canonical_identity_collision() {
    const auto child = make_ref("boot.pac::PAC/slot-0000/a.bin", "PAC[0]");
    const auto loose = make_ref("boot.pac::PAC/slot-0000/a.bin#PAC[0]", "");

    assert(child.valid());
    assert(loose.valid());

    // The identities are genuinely distinct under structural equality...
    assert(!(child.id == loose.id));

    // ...yet they serialize to the same canonical key. DEFECT P1.
    assert(child.id.canonical() == loose.id.canonical());

    // Consequence: ResourceGraph keys on that string, so the second declared
    // identity is silently refused and its edges are never created. This
    // contradicts contract identity rules 3, 4 and 5.
    ResourceGraph graph;
    assert(graph.add(child));
    assert(!graph.add(loose));
    assert(graph.resource_count() == 1U);

    // Worse than a dropped node: because both identities resolve to one key,
    // connect() finds "both" endpoints and succeeds, recording a self-edge in
    // which the surviving resource contains itself.
    assert(graph.connect(child.id, loose.id, ResourceRelation::contains));
    assert(graph.edge_count() == 1U);

    const auto contained = graph.related(child.id, ResourceRelation::contains);
    assert(contained.size() == 1U);
    assert(contained.front()->id == child.id);
}

// Builds one structurally well-formed single-member snapshot whose layout is:
//   [0, 12)    opaque prefix bytes belonging to no entry
//   [12, 54)   local region (30 header + 4 name + 8 stored data)
//   [54, 100)  central record, ending exactly at EOCD
//   [100, 122) EOCD
[[nodiscard]] NbzZipSerializationSnapshot make_truthful_snapshot() {
    constexpr std::uint64_t local_at = 12U;
    constexpr std::uint64_t central_at = 54U;
    constexpr std::uint64_t eocd_at = 100U;

    NbzZipSerializationEntry entry{
        .central_index = 0U,
        .central_record_offset = central_at,
        .central_record_bytes = signed_record(46U, 0x02014B50U),
        .local_record_offset = local_at,
        .local_prefix_bytes = signed_record(34U, 0x04034B50U),
        .data_offset = local_at + 34U,
        .stored_data_size = 8U,
        .local_region_size = central_at - local_at,
        .uses_data_descriptor = false,
    };

    return NbzZipSerializationSnapshot{
        .source_id = "corpus",
        .archive_size = eocd_at + 22U,
        .prefix_size = local_at,
        .computed_central_start = central_at,
        .eocd_offset = eocd_at,
        .eocd_bytes = signed_record(22U, 0x06054B50U),
        .entries = {entry},
    };
}

// P2: valid() accepts snapshots that fail to account for every source byte.
void probe_snapshot_coverage_gap() {
    const auto truthful = make_truthful_snapshot();
    assert(truthful.valid());

    // Claims there is no opaque prefix: 12 source bytes go unaccounted.
    auto dropped_prefix = truthful;
    dropped_prefix.prefix_size = 0U;
    assert(dropped_prefix.valid()); // DEFECT P2

    // Claims the whole pre-central region is prefix, orphaning the member.
    auto swallowed_member = truthful;
    swallowed_member.prefix_size = truthful.computed_central_start;
    assert(swallowed_member.valid()); // DEFECT P2

    // Shrinks the local region to exactly 30 + 4 + 8, orphaning the tail gap.
    auto shrunk_region = truthful;
    shrunk_region.entries[0].local_region_size = 42U;
    assert(shrunk_region.valid()); // DEFECT P2
}

// P3: the central record's own name/extra/comment fields are never used to
// confirm the preserved record length, so a wrong length still validates as
// long as the walk still lands exactly on EOCD.
void probe_central_record_length_gap() {
    auto snapshot = make_truthful_snapshot();

    // Declare a 4-byte filename in the central header (offset 28) while the
    // preserved record stays 46 bytes. A faithful record would be 50 bytes.
    auto& entry = snapshot.entries[0];
    entry.central_record_bytes[28] = std::byte{4};
    entry.central_record_bytes[29] = std::byte{0};

    assert(entry.central_record_bytes.size() == 46U);
    assert(snapshot.valid()); // DEFECT P3
}

} // namespace

int main() {
    probe_canonical_identity_collision();
    probe_snapshot_coverage_gap();
    probe_central_record_length_gap();
    return 0;
}
