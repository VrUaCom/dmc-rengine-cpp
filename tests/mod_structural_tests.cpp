#include "dmc_rengine/formats/mod.hpp"

#include "dmc_rengine/formats/scm.hpp"
#include "dmc_rengine/profiles/dmc3/mod_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/scm_contract.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

// The model payload, read from its own relocation routine. It shares a
// document shell with the scene model and nothing else, and these check both
// halves of that: that the shell really is common, and that the batch layouts
// really are not interchangeable.

namespace {

namespace formats = dmc::rengine::formats;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Contract = dmc3::ModContract;

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

using GroupSpec = std::vector<std::uint16_t>;

// Builds a document the way the file lays one out: batches indexed from the
// group pointer, arrays grouped by kind across the group and 16-byte aligned,
// each batch's strip buffer marked, and the first two vertices of every batch
// carrying the strip-break bit.
[[nodiscard]] std::vector<std::byte> make_document(
    const std::vector<GroupSpec>& groups,
    std::uint8_t document_mode = 2U) {
    std::size_t batch_total = 0U;
    for (const auto& group : groups) {
        batch_total += group.size();
    }
    const auto batch_table = dmc3::ModContract::batch_stride == 0U
        ? 0U
        : formats::RelocatedModelShell::group_table_offset +
              groups.size() * formats::RelocatedModelShell::group_stride;
    auto cursor = aligned(batch_table + batch_total * Contract::batch_stride);

    struct Placed final {
        std::uint64_t batch_offset{};
        std::uint16_t count{};
        std::uint64_t position{};
        std::uint64_t normal{};
        std::uint64_t attribute{};
        std::uint64_t secondary{};
        std::uint64_t control{};
        std::uint64_t strip{};
    };
    std::vector<std::vector<Placed>> placed;

    std::uint64_t next_batch = batch_table;
    for (const auto& group : groups) {
        std::vector<Placed> records;
        for (const auto count : group) {
            records.push_back(Placed{next_batch, count, 0U, 0U, 0U, 0U, 0U, 0U});
            next_batch += Contract::batch_stride;
        }
        const std::pair<std::uint64_t Placed::*, std::uint64_t> kinds[]{
            {&Placed::position, Contract::position_element_bytes},
            {&Placed::normal, Contract::normal_element_bytes},
            {&Placed::attribute, Contract::attribute_element_bytes},
            {&Placed::secondary, Contract::secondary_element_bytes},
            {&Placed::control, Contract::index_element_bytes},
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
    bytes[0] = static_cast<std::byte>('M');
    bytes[1] = static_cast<std::byte>('O');
    bytes[2] = static_cast<std::byte>('D');
    bytes[3] = static_cast<std::byte>(' ');
    bytes[formats::RelocatedModelShell::group_count_offset] =
        static_cast<std::byte>(groups.size() & 0xFFU);
    bytes[formats::RelocatedModelShell::document_mode_offset] =
        static_cast<std::byte>(document_mode);
    put_u64(bytes, formats::RelocatedModelShell::document_pointer_offset, batch_table);

    for (std::size_t index = 0U; index < placed.size(); ++index) {
        const auto group_offset = formats::RelocatedModelShell::group_table_offset +
            index * formats::RelocatedModelShell::group_stride;
        bytes[group_offset + formats::RelocatedModelShell::group_batch_count_offset] =
            static_cast<std::byte>(placed[index].size() & 0xFFU);
        put_u64(
            bytes,
            group_offset + formats::RelocatedModelShell::group_batch_pointer_offset,
            placed[index].front().batch_offset);

        for (const auto& record : placed[index]) {
            const auto at = static_cast<std::size_t>(record.batch_offset);
            put_u16(bytes, at + Contract::batch_index_count_offset, record.count);
            put_u64(bytes, at + Contract::batch_position_offset, record.position);
            put_u64(bytes, at + Contract::batch_normal_offset, record.normal);
            put_u64(bytes, at + Contract::batch_attribute_offset, record.attribute);
            put_u64(bytes, at + Contract::batch_secondary_offset, record.secondary);
            put_u64(bytes, at + Contract::batch_index_offset, record.control);
            // Stored relative to the batch, which is the one field in this
            // structure that does not use the document base.
            put_u64(
                bytes, at + Contract::batch_strip_offset,
                record.strip - record.batch_offset);
            put_u16(
                bytes, static_cast<std::size_t>(record.strip),
                Contract::strip_buffer_marker);

            for (std::uint16_t vertex = 0U; vertex < 2U && vertex < record.count; ++vertex) {
                put_u16(
                    bytes,
                    static_cast<std::size_t>(
                        record.control + vertex * Contract::index_element_bytes),
                    static_cast<std::uint16_t>(Contract::index_break_mask | 0x1FU));
            }
        }
    }
    return bytes;
}

void a_document_reads_the_way_the_routine_walks_it() {
    // Seven single-batch groups of four vertices each is the shape of the real
    // HUD model in the corpus: one quad per group.
    const auto bytes = make_document({{4U}, {4U}, {4U}, {4U}, {4U}, {4U}, {4U}});
    const auto parsed = formats::ModParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->group_count == 7U);
    assert(parsed.document->batches.size() == 7U);
    assert(parsed.document->total_vertex_count() == 28U);
    assert(parsed.document->document_mode == 2U);
    for (const auto& batch : parsed.document->batches) {
        assert(batch.vertex_count == 4U);
        assert(batch.break_count == 2U);
        assert(batch.strip_marker_present);
        assert(batch.valid(parsed.document->document_size));
    }
}

void batches_are_indexed_not_chained() {
    // A group with several batches is where the two model formats part. The
    // model reaches batch n at `first + n * 0x50`; the scene model follows a
    // stored step. Reading this format the scene model's way would take an
    // array pointer for a stride.
    const auto bytes = make_document({{16U, 12U, 9U}});
    const auto parsed = formats::ModParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->batches.size() == 3U);
    for (std::size_t index = 0U; index < 3U; ++index) {
        assert(parsed.document->batches[index].batch_offset ==
            parsed.document->batches[0].batch_offset +
                index * Contract::batch_stride);
    }
    // Arrays interleave by kind, so the second batch's positions come before
    // the first batch's normals.
    assert(parsed.document->batches[1].position_offset >
        parsed.document->batches[0].position_offset);
    assert(parsed.document->batches[0].normal_offset >
        parsed.document->batches[2].position_offset);
}

void the_control_word_is_two_bytes_here() {
    // The scene model's per-index word is four bytes with the break flag in
    // byte 3; the model's is two with the flag in the high bit. That single
    // difference moves every array after it, so the contracts must not drift
    // into agreeing.
    static_assert(Contract::index_element_bytes == 2U);
    static_assert(dmc3::ScmContract::index_element_bytes == 4U);
    static_assert(Contract::index_break_mask == 0x8000U);
    static_assert(Contract::break_flag_cleared_after_rebuild);
    static_assert(Contract::batch_index_offset != dmc3::ScmContract::batch_index_offset);
    // The scene model uses this field for the step to the next batch.
    static_assert(
        Contract::batch_secondary_offset == dmc3::ScmContract::batch_next_stride_offset);
    static_assert(Contract::aligned_array_bytes(4U, 2U) == 16U);
    static_assert(Contract::aligned_array_bytes(9U, 12U) == 112U);
}

void the_two_formats_refuse_each_other() {
    const auto model = make_document({{4U}});
    assert(formats::ModParser::parse(model).ok());
    // The shell is common, so the scene model gets past the header shape and
    // is stopped by the tag — which is the only thing that separates them at
    // that level.
    const auto as_scene = formats::ScmParser::parse(model);
    assert(!as_scene.ok());
    assert(as_scene.error == formats::ScmParseError::invalid_magic);
}

void a_broken_document_is_refused_not_repaired() {
    auto out_of_range = make_document({{8U}});
    const auto batch = formats::RelocatedModelShell::group_table_offset +
        formats::RelocatedModelShell::group_stride;
    put_u64(
        out_of_range,
        static_cast<std::size_t>(batch) + Contract::batch_position_offset,
        out_of_range.size());
    const auto refused = formats::ModParser::parse(out_of_range);
    assert(!refused.ok());
    assert(refused.error == formats::ModParseError::array_out_of_bounds ||
        refused.error == formats::ModParseError::array_packing_mismatch);

    // A batch array moved off the group's packing is a walk that found
    // something plausible instead of the real thing.
    auto misplaced = make_document({{16U, 12U}});
    const auto second = formats::RelocatedModelShell::group_table_offset +
        formats::RelocatedModelShell::group_stride + Contract::batch_stride;
    std::uint64_t stored = 0U;
    for (std::size_t index = 0U; index < 8U; ++index) {
        stored |= std::to_integer<std::uint64_t>(
            misplaced[static_cast<std::size_t>(second) +
                Contract::batch_position_offset + index]) << (8U * index);
    }
    put_u64(
        misplaced,
        static_cast<std::size_t>(second) + Contract::batch_position_offset,
        stored + Contract::array_alignment);
    const auto packing = formats::ModParser::parse(misplaced);
    assert(!packing.ok());
    assert(packing.error == formats::ModParseError::array_packing_mismatch);
}

void the_contract_agrees_with_the_type_probe() {
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
    static_assert(
        Contract::canonical_target_sha256 ==
        dmc3::ScmContract::canonical_target_sha256);
}

} // namespace

int main() {
    a_document_reads_the_way_the_routine_walks_it();
    batches_are_indexed_not_chained();
    the_control_word_is_two_bytes_here();
    the_two_formats_refuse_each_other();
    a_broken_document_is_refused_not_repaired();
    the_contract_agrees_with_the_type_probe();
    return 0;
}
