#include "dmc_rengine/formats/scm.hpp"

#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/scm_contract.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// The scene model, read from the recovered relocation routine rather than by
// inspection. What is checked here is exactly what that routine relies on, and
// nothing about what the payload means.

namespace {

namespace formats = dmc::rengine::formats;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Contract = dmc3::ScmContract;

void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
    }
}

[[nodiscard]] std::uint64_t aligned(std::uint64_t raw) {
    return (raw + Contract::array_alignment - 1U) /
        Contract::array_alignment * Contract::array_alignment;
}

struct GroupSpec final {
    std::vector<std::uint16_t> batch_index_counts;
};

// Builds a document the way the file lays one out: arrays grouped by kind
// across the whole group, 16-byte aligned, each batch's strip buffer marked.
[[nodiscard]] std::vector<std::byte> make_document(
    const std::vector<GroupSpec>& groups) {
    std::size_t batch_total = 0U;
    for (const auto& group : groups) {
        batch_total += group.batch_index_counts.size();
    }

    const auto batch_table = Contract::group_table_offset +
        groups.size() * Contract::group_stride;
    auto cursor = aligned(batch_table + batch_total * Contract::batch_stride);

    struct Placed final {
        std::uint64_t batch_offset{};
        std::uint16_t count{};
        std::uint64_t position{};
        std::uint64_t normal{};
        std::uint64_t attribute{};
        std::uint64_t index{};
        std::uint64_t strip{};
    };
    std::vector<std::vector<Placed>> placed;

    std::uint64_t next_batch = batch_table;
    for (const auto& group : groups) {
        std::vector<Placed> records;
        for (const auto count : group.batch_index_counts) {
            records.push_back(Placed{next_batch, count, 0U, 0U, 0U, 0U, 0U});
            next_batch += Contract::batch_stride;
        }
        const std::pair<std::uint64_t Placed::*, std::uint64_t> kinds[]{
            {&Placed::position, Contract::position_element_bytes},
            {&Placed::normal, Contract::normal_element_bytes},
            {&Placed::attribute, Contract::attribute_element_bytes},
            {&Placed::index, Contract::index_element_bytes},
        };
        for (const auto& [field, element_bytes] : kinds) {
            for (auto& record : records) {
                record.*field = cursor;
                cursor += aligned(
                    static_cast<std::uint64_t>(record.count) * element_bytes);
            }
        }
        placed.push_back(std::move(records));
    }

    for (auto& group : placed) {
        for (auto& record : group) {
            record.strip = cursor;
            cursor += aligned(
                static_cast<std::uint64_t>(record.count) * 8U + 16U);
        }
    }

    std::vector<std::byte> bytes(static_cast<std::size_t>(cursor), std::byte{0});
    bytes[0] = static_cast<std::byte>('S');
    bytes[1] = static_cast<std::byte>('C');
    bytes[2] = static_cast<std::byte>('M');
    bytes[3] = static_cast<std::byte>(' ');
    bytes[Contract::group_count_offset] =
        static_cast<std::byte>(groups.size() & 0xFFU);
    put_u64(bytes, Contract::document_pointer_offset, batch_table);

    for (std::size_t index = 0U; index < placed.size(); ++index) {
        const auto group_offset = Contract::group_table_offset +
            index * Contract::group_stride;
        bytes[group_offset + Contract::group_batch_count_offset] =
            static_cast<std::byte>(placed[index].size() & 0xFFU);
        put_u64(
            bytes,
            group_offset + Contract::group_batch_pointer_offset,
            placed[index].front().batch_offset);

        for (const auto& record : placed[index]) {
            const auto at = static_cast<std::size_t>(record.batch_offset);
            put_u16(bytes, at + Contract::batch_index_count_offset, record.count);
            put_u64(bytes, at + Contract::batch_position_offset, record.position);
            put_u64(bytes, at + Contract::batch_normal_offset, record.normal);
            put_u64(bytes, at + Contract::batch_attribute_offset, record.attribute);
            put_u64(bytes, at + Contract::batch_index_offset, record.index);
            put_u64(bytes, at + Contract::batch_next_stride_offset, Contract::batch_stride);
            // Stored relative to the batch, which is the one field in this
            // structure that does not use the document base.
            put_u64(
                bytes, at + Contract::batch_strip_offset,
                record.strip - record.batch_offset);
            put_u16(
                bytes, static_cast<std::size_t>(record.strip),
                Contract::strip_buffer_marker);

            // Two skip-flagged indices per batch, as the corpus has at the
            // start of every strip.
            for (std::uint16_t element = 0U; element < 2U && element < record.count; ++element) {
                bytes[static_cast<std::size_t>(
                    record.index + element * Contract::index_element_bytes +
                    Contract::index_flag_byte)] =
                    static_cast<std::byte>(Contract::index_skip_mask);
            }
        }
    }
    return bytes;
}

void a_document_reads_the_way_the_routine_walks_it() {
    const auto bytes = make_document({
        GroupSpec{{167U}},
        GroupSpec{{96U, 90U}},
        GroupSpec{{376U}},
    });
    const auto parsed = formats::ScmParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->group_count == 3U);
    assert(parsed.document->groups.size() == 3U);
    assert(parsed.document->batches.size() == 4U);
    assert(parsed.document->total_index_count() == 167U + 96U + 90U + 376U);

    // Two flagged indices per batch, counted the way the strip rebuild reads
    // them: byte 3, mask 2.
    for (const auto& batch : parsed.document->batches) {
        assert(batch.skipped_index_count == 2U);
        assert(batch.valid(parsed.document->document_size));
    }

    // The second group's two batches sit one stride apart and their arrays
    // interleave by kind, which is the case a per-batch reader gets wrong.
    const auto& first = parsed.document->batches[1];
    const auto& second = parsed.document->batches[2];
    assert(second.batch_offset == first.batch_offset + Contract::batch_stride);
    assert(second.position_offset > first.position_offset);
    assert(first.normal_offset > second.position_offset);
}

void the_group_count_is_a_byte() {
    // The routine reads `byte ptr [rax+0x10]`. The rest of that dword is other
    // fields, and a reader that took the whole word would invent millions of
    // groups out of them.
    auto bytes = make_document({GroupSpec{{8U}}});
    bytes[Contract::group_count_offset + 1U] = std::byte{0xFF};
    bytes[Contract::group_count_offset + 2U] = std::byte{0xFF};
    bytes[Contract::group_count_offset + 3U] = std::byte{0xFF};

    const auto parsed = formats::ScmParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->group_count == 1U);
}

void the_tag_is_three_bytes() {
    // `SCM ` is what stage files store, but the runtime compares three bytes,
    // so a payload whose fourth byte is something else is still a scene model.
    auto bytes = make_document({GroupSpec{{8U}}});
    bytes[3] = static_cast<std::byte>('X');
    assert(formats::ScmParser::parse(bytes).ok());

    bytes[2] = static_cast<std::byte>('X');
    const auto refused = formats::ScmParser::parse(bytes);
    assert(!refused.ok());
    assert(refused.error == formats::ScmParseError::invalid_magic);
}

void a_broken_document_is_refused_not_repaired() {
    // An offset that leaves the document is the failure the relocation routine
    // cannot survive: it would add the base and hand the renderer a pointer
    // into someone else's memory.
    auto out_of_range = make_document({GroupSpec{{32U}}});
    put_u64(
        out_of_range,
        static_cast<std::size_t>(
            Contract::group_table_offset + Contract::group_stride) +
            Contract::batch_position_offset,
        out_of_range.size());
    const auto refused = formats::ScmParser::parse(out_of_range);
    assert(!refused.ok());
    assert(refused.error == formats::ScmParseError::array_out_of_bounds ||
        refused.error == formats::ScmParseError::array_packing_mismatch);

    // A strip buffer without the marker means the file does not work the way
    // the recovered routine expects. Guessing past that would be inventing a
    // second format.
    auto unmarked = make_document({GroupSpec{{32U}}});
    const auto batch = Contract::group_table_offset + Contract::group_stride;
    std::uint64_t relative = 0U;
    for (std::size_t index = 0U; index < 8U; ++index) {
        relative |= std::to_integer<std::uint64_t>(
            unmarked[batch + Contract::batch_strip_offset + index]) << (8U * index);
    }
    put_u16(unmarked, static_cast<std::size_t>(batch + relative), 0U);
    const auto no_marker = formats::ScmParser::parse(unmarked);
    assert(!no_marker.ok());
    assert(no_marker.error == formats::ScmParseError::missing_strip_marker);
}

void the_contract_agrees_with_the_type_probe() {
    // The handler this layout came from is the one the type probe names for
    // `SCM`. If those two ever disagree, one of them is describing a different
    // function than it claims.
    using Types = dmc3::ResourceTypeContract;
    bool found = false;
    for (const auto& entry : Types::tagged_types) {
        if (entry.tag == Contract::magic) {
            assert(entry.handler_va == Contract::relocate_va);
            found = true;
        }
    }
    assert(found);
    static_assert(Contract::magic_bytes == Types::content_tag_bytes);
    static_assert(
        Contract::canonical_target_sha256 == Types::canonical_target_sha256);
    static_assert(Contract::aligned_array_bytes(167U, 12U) == 2016U);
    static_assert(Contract::aligned_array_bytes(376U, 12U) == 4512U);
}

} // namespace

int main() {
    a_document_reads_the_way_the_routine_walks_it();
    the_group_count_is_a_byte();
    the_tag_is_three_bytes();
    a_broken_document_is_refused_not_repaired();
    the_contract_agrees_with_the_type_probe();
    return 0;
}
