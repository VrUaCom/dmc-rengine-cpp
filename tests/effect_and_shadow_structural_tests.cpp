#include "dmc_rengine/formats/efm.hpp"
#include "dmc_rengine/formats/shw.hpp"

#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/profiles/dmc3/efm_contract.hpp"
#include "dmc_rengine/profiles/dmc3/mod_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/scm_contract.hpp"
#include "dmc_rengine/profiles/dmc3/shw_contract.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

// The last two type handlers. Neither has a payload in the supplied corpus, so
// these tests can only prove that the readers implement what the routines
// require — not that a real file looks like this. That distinction is the
// point of several of the assertions below.

namespace {

namespace formats = dmc::rengine::formats;
namespace dmc3 = dmc::rengine::profiles::dmc3;

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
    return (raw + 15U) / 16U * 16U;
}

[[nodiscard]] std::vector<std::byte> make_effect_document(
    const std::vector<std::vector<std::uint16_t>>& groups) {
    using Contract = dmc3::EfmContract;
    using Shell = formats::RelocatedModelShell;

    std::size_t batch_total = 0U;
    for (const auto& group : groups) {
        batch_total += group.size();
    }
    const auto batch_table =
        Shell::group_table_offset + groups.size() * Shell::group_stride;
    auto cursor = aligned(batch_table + batch_total * Contract::batch_stride);

    struct Placed final {
        std::uint64_t batch_offset{};
        std::uint16_t count{};
        std::uint64_t position{};
        std::uint64_t normal{};
        std::uint64_t attribute{};
        std::uint64_t secondary{};
        std::uint64_t control{};
        std::uint64_t extra{};
        std::uint64_t strip{};
    };
    std::vector<std::vector<Placed>> placed;

    std::uint64_t next_batch = batch_table;
    for (const auto& group : groups) {
        std::vector<Placed> records;
        for (const auto count : group) {
            records.push_back(Placed{next_batch, count, 0U, 0U, 0U, 0U, 0U, 0U, 0U});
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
        // The extra array has no recovered element width, so the fixture gives
        // it a base and a generous span and the reader checks only the base.
        for (auto& record : records) {
            record.extra = cursor;
            cursor += aligned(static_cast<std::uint64_t>(record.count) * 16U);
        }
        placed.push_back(std::move(records));
    }
    for (auto& group : placed) {
        for (auto& record : group) {
            record.strip = cursor;
            cursor += aligned(static_cast<std::uint64_t>(record.count) * 8U + 16U);
        }
    }

    std::vector<std::byte> bytes(static_cast<std::size_t>(cursor), std::byte{0});
    bytes[0] = static_cast<std::byte>('E');
    bytes[1] = static_cast<std::byte>('F');
    bytes[2] = static_cast<std::byte>('M');
    bytes[Shell::group_count_offset] = static_cast<std::byte>(groups.size() & 0xFFU);
    put_u64(bytes, Shell::document_pointer_offset, batch_table);

    for (std::size_t index = 0U; index < placed.size(); ++index) {
        const auto group_offset =
            Shell::group_table_offset + index * Shell::group_stride;
        bytes[group_offset + Shell::group_batch_count_offset] =
            static_cast<std::byte>(placed[index].size() & 0xFFU);
        put_u64(
            bytes, group_offset + Shell::group_batch_pointer_offset,
            placed[index].front().batch_offset);

        for (const auto& record : placed[index]) {
            const auto at = static_cast<std::size_t>(record.batch_offset);
            put_u16(bytes, at + Contract::batch_index_count_offset, record.count);
            put_u64(bytes, at + Contract::batch_position_offset, record.position);
            put_u64(bytes, at + Contract::batch_normal_offset, record.normal);
            put_u64(bytes, at + Contract::batch_attribute_offset, record.attribute);
            put_u64(bytes, at + Contract::batch_secondary_offset, record.secondary);
            put_u64(bytes, at + Contract::batch_index_offset, record.control);
            put_u64(bytes, at + Contract::batch_extra_offset, record.extra);
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
                    static_cast<std::uint16_t>(Contract::index_break_mask));
            }
        }
    }
    return bytes;
}

void the_effect_model_is_the_model_plus_one_array() {
    const auto bytes = make_effect_document({{24U, 12U}, {8U}});
    const auto parsed = formats::EfmParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->group_count == 2U);
    assert(parsed.document->batches.size() == 3U);
    assert(parsed.document->total_vertex_count() == 44U);
    for (const auto& batch : parsed.document->batches) {
        assert(batch.break_count == 2U);
        assert(batch.strip_marker_present);
        assert(batch.extra_offset != 0U);
        assert(batch.valid(parsed.document->document_size));
    }

    // Every batch field it shares with the model sits at the same offset. The
    // extra array is the only structural difference between them.
    static_assert(
        dmc3::EfmContract::batch_position_offset ==
        dmc3::ModContract::batch_position_offset);
    static_assert(
        dmc3::EfmContract::batch_index_offset ==
        dmc3::ModContract::batch_index_offset);
    static_assert(
        dmc3::EfmContract::batch_stride == dmc3::ModContract::batch_stride);
    static_assert(dmc3::EfmContract::batch_extra_offset == 0x38U);
    static_assert(!dmc3::EfmContract::extra_array_extent_is_known);
    // The model does not read the mode byte's branch; the effect model has no
    // such branch at all.
    static_assert(!dmc3::EfmContract::reads_document_mode);
}

void the_effect_model_and_the_model_refuse_each_other() {
    const auto effect = make_effect_document({{8U}});
    assert(formats::EfmParser::parse(effect).ok());
    // They share the shell, so the tag is what separates them there.
    const auto as_model = formats::ModParser::parse(effect);
    assert(!as_model.ok());
    assert(as_model.error == formats::ModParseError::shell_rejected);
}

void an_unbounded_array_is_bounded_only_by_its_base() {
    // The routine relocates the extra array and never indexes through it, so
    // its element width is unknown. A base outside the document is still a
    // refusal; a base inside it is all that can be asserted.
    auto bytes = make_effect_document({{8U}});
    const auto batch = formats::RelocatedModelShell::group_table_offset +
        formats::RelocatedModelShell::group_stride;
    put_u64(
        bytes,
        static_cast<std::size_t>(batch) + dmc3::EfmContract::batch_extra_offset,
        bytes.size());
    const auto refused = formats::EfmParser::parse(bytes);
    assert(!refused.ok());
    assert(refused.error == formats::EfmParseError::array_out_of_bounds);
}

[[nodiscard]] std::vector<std::byte> make_shadow_document(std::uint8_t entries) {
    using Contract = dmc3::ShwContract;
    const auto table_end = Contract::entry_table_offset +
        static_cast<std::size_t>(entries) * Contract::entry_stride;
    const auto payload = aligned(table_end);
    std::vector<std::byte> bytes(
        static_cast<std::size_t>(payload + entries * 4U * 16U), std::byte{0});
    bytes[0] = static_cast<std::byte>('S');
    bytes[1] = static_cast<std::byte>('H');
    bytes[2] = static_cast<std::byte>('W');
    bytes[Contract::entry_count_offset] = static_cast<std::byte>(entries);

    auto cursor = payload;
    for (std::size_t entry = 0U; entry < entries; ++entry) {
        const auto at = Contract::entry_table_offset + entry * Contract::entry_stride;
        for (std::size_t pointer = 0U; pointer < Contract::entry_pointer_count; ++pointer) {
            put_u64(bytes, at + pointer * Contract::entry_pointer_stride, cursor);
            cursor += 16U;
        }
    }
    return bytes;
}

void the_shadow_document_does_not_use_the_model_shell() {
    // Three of the four handlers share a shell and this is the fourth. Its
    // table starts where the others put their group table's predecessor, so a
    // reader that assumed the shell would take entry 0 for a group header.
    static_assert(
        dmc3::ShwContract::entry_table_offset !=
        formats::RelocatedModelShell::group_table_offset);
    static_assert(
        dmc3::ShwContract::entry_count_offset ==
        formats::RelocatedModelShell::group_count_offset);
    static_assert(!dmc3::ShwContract::array_extents_are_known);
    static_assert(dmc3::ShwContract::entry_offset(0U) == 0x30U);
    static_assert(dmc3::ShwContract::entry_offset(1U) == 0x70U);

    const auto bytes = make_shadow_document(3U);
    const auto parsed = formats::ShwParser::parse(bytes);
    assert(parsed.ok());
    assert(parsed.document->entry_count == 3U);
    assert(parsed.document->entries.size() == 3U);
    for (std::uint32_t index = 0U; index < 3U; ++index) {
        const auto& entry = parsed.document->entries[index];
        assert(entry.entry_index == index);
        assert(entry.entry_offset == dmc3::ShwContract::entry_table_offset +
            index * dmc3::ShwContract::entry_stride);
        for (const auto offset : entry.array_offsets) {
            assert(offset != 0U);
            assert(offset < parsed.document->document_size);
        }
    }
}

void a_shadow_pointer_outside_the_document_is_refused() {
    auto bytes = make_shadow_document(2U);
    put_u64(bytes, dmc3::ShwContract::entry_offset(1U), bytes.size());
    const auto refused = formats::ShwParser::parse(bytes);
    assert(!refused.ok());
    assert(refused.error == formats::ShwParseError::pointer_out_of_bounds);
}

void every_handler_the_probe_names_is_now_accounted_for() {
    using Types = dmc3::ResourceTypeContract;
    for (const auto& entry : Types::tagged_types) {
        if (entry.tag == "MOD") {
            assert(entry.handler_va == dmc3::ModContract::relocate_va);
        } else if (entry.tag == "EFM") {
            assert(entry.handler_va == dmc3::EfmContract::relocate_va);
        } else if (entry.tag == "SCM") {
            assert(entry.handler_va == dmc3::ScmContract::relocate_va);
        } else if (entry.tag == "SHW") {
            assert(entry.handler_va == dmc3::ShwContract::relocate_va);
        } else {
            // MRP is the one type the runtime records and never handles, so
            // there is no routine to recover and none is claimed.
            assert(entry.tag == "MRP");
            assert(entry.handler_va == 0U);
        }
    }
    static_assert(
        dmc3::EfmContract::canonical_target_sha256 ==
        Types::canonical_target_sha256);
    static_assert(
        dmc3::ShwContract::canonical_target_sha256 ==
        Types::canonical_target_sha256);
}

} // namespace

int main() {
    the_effect_model_is_the_model_plus_one_array();
    the_effect_model_and_the_model_refuse_each_other();
    an_unbounded_array_is_bounded_only_by_its_base();
    the_shadow_document_does_not_use_the_model_shell();
    a_shadow_pointer_outside_the_document_is_refused();
    every_handler_the_probe_names_is_now_accounted_for();
    return 0;
}
