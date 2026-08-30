#include "dmc_rengine/formats/pnst.hpp"
#include "dmc_rengine/gdspaces/container_expander.hpp"
#include "dmc_rengine/gdspaces/resource_naming_identity.hpp"
#include "dmc_rengine/profiles/dmc3/effect_stored_name_evidence.hpp"
#include "dmc_rengine/profiles/dmc3/naming_pipeline.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

[[nodiscard]] std::vector<std::byte> text_bytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (const unsigned char value : text) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

[[nodiscard]] std::vector<std::byte> make_pnst(
    const std::vector<std::optional<std::vector<std::byte>>>& slots) {
    const auto slot_count = static_cast<std::uint32_t>(slots.size());
    const auto header_size = 8U + slots.size() * 4U;
    std::vector<std::byte> bytes(header_size, std::byte{0});
    bytes[0] = std::byte{'P'};
    bytes[1] = std::byte{'N'};
    bytes[2] = std::byte{'S'};
    bytes[3] = std::byte{'T'};
    put_u32(bytes, 4U, slot_count);

    std::size_t next = header_size;
    for (std::size_t slot = 0U; slot < slots.size(); ++slot) {
        if (!slots[slot].has_value() || slots[slot]->empty()) {
            continue;
        }
        put_u32(
            bytes,
            8U + slot * 4U,
            static_cast<std::uint32_t>(next));
        bytes.insert(bytes.end(), slots[slot]->begin(), slots[slot]->end());
        next += slots[slot]->size();
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_effect_pack() {
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto records = make_pnst({
        text_bytes("record-zero"),
        text_bytes("record-one-longer"),
    });
    const auto manifest = text_bytes("E 17\r\nT 3\r\n# End\r\n");
    auto bytes = make_pnst({manifest, records});
    const auto size = static_cast<std::uint64_t>(bytes.size());

    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-source",
                .logical_path = "GData.afs/room/st001_effect.pac",
                .container_chain = "NBZ[4]/PAC[9]",
                .offset = 0x100000U,
                .size = size,
            },
            .display_name = "st001_effect.pac",
            .format = "pnst",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = true,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
        .enclosing_container_name_evidence = {},
        .semantic_evidence = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ResourcePayload make_index() {
    namespace gdspaces = dmc::rengine::gdspaces;
    auto bytes = text_bytes("effect_000.ukn\r\neffect_001.ukn\r\n");
    const auto size = static_cast<std::uint64_t>(bytes.size());
    return gdspaces::ResourcePayload{
        .resource = gdspaces::ResourceRef{
            .id = gdspaces::ResourceId{
                .source_id = "retail-source",
                .logical_path = "GData.afs/room/st001_effect.index",
                .container_chain = {},
                .offset = 0U,
                .size = size,
            },
            .display_name = "st001_effect.index",
            .format = "index",
            .profile = "dmc3-hd",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = std::move(bytes),
        .diagnostics = {},
        .byte_provenance = std::nullopt,
        .name_evidence = {},
        .enclosing_container_name_evidence = {},
        .semantic_evidence = {},
    };
}

[[nodiscard]] dmc::rengine::gdspaces::ContainerExpansion records_expansion(
    const dmc::rengine::gdspaces::ResourcePayload& enclosing) {
    namespace formats = dmc::rengine::formats;
    namespace gdspaces = dmc::rengine::gdspaces;

    const auto outer = formats::PnstParser::parse(
        std::span<const std::byte>{enclosing.bytes.data(), enclosing.bytes.size()});
    assert(outer.ok());
    const auto expanded_outer = gdspaces::ContainerExpander::expand(enclosing, *outer.document);
    assert(expanded_outer.usable());
    assert(expanded_outer.children.size() == 2U);
    assert(expanded_outer.children[1].entry.slot_index == 1U);

    const auto& records_payload = expanded_outer.children[1].payload;
    const auto inner = formats::PnstParser::parse(
        std::span<const std::byte>{
            records_payload.bytes.data(), records_payload.bytes.size()});
    assert(inner.ok());
    auto expansion = gdspaces::ContainerExpander::expand(records_payload, *inner.document);
    assert(expansion.usable());
    return expansion;
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto enclosing = make_effect_pack();

    // Direct stored names are sealed to the exact enclosing bytes and exact
    // physical target records, then retained separately in the unified snapshot.
    auto expansion = records_expansion(enclosing);
    const auto result = dmc3::Dmc3NamingPipeline::apply(
        expansion, nullptr, nullptr, &enclosing);
    assert(result.ok());
    assert(result.enclosing_stored_names_applied);
    assert(result.snapshot->children.size() == 2U);
    assert(result.snapshot->children[0].physical_slot_index == 0U);
    assert(result.snapshot->children[0].extracted_ordinal == 0U);
    assert(result.snapshot->children[0].enclosing_container_stored_name == "E 17");
    assert(result.snapshot->children[0].enclosing_container_stored_name_evidence.has_value());
    assert(result.snapshot->children[0].canonical_display_name == "E 17");
    assert(!result.snapshot->children[0].external_index_normalized_name().has_value());

    // A later payload edit invalidates the sealed target-byte evidence instead
    // of silently retaining a stale display label.
    expansion.children[0].payload.bytes[0] = std::byte{'X'};
    const auto stale = gdspaces::ResourceNamingIdentityBuilder::build(expansion);
    assert(!stale.ok());

    // Enclosing stored names and external extraction names coexist as distinct
    // domains. `.index` still maps by populated ordinal and remains the display
    // authority for historical extraction naming.
    auto dual = records_expansion(enclosing);
    const auto index = make_index();
    const auto dual_result = dmc3::Dmc3NamingPipeline::apply(
        dual, &index, nullptr, &enclosing);
    assert(dual_result.ok());
    assert(dual_result.snapshot->children[0].extracted_ordinal == 0U);
    assert(
        dual_result.snapshot->children[0].external_index_normalized_name() ==
        std::optional<std::string>{"effect_000.ukn"});
    assert(dual_result.snapshot->children[0].enclosing_container_stored_name == "E 17");
    assert(dual_result.snapshot->children[0].canonical_display_name == "effect_000.ukn");

    // A look-alike PNST with altered parent identity is not allowed to inherit
    // names from the enclosing effect container.
    auto wrong_parent = records_expansion(enclosing);
    wrong_parent.parent.id.logical_path.append(".wrong");
    const auto wrong = dmc3::EffectStoredNameEvidenceBuilder::apply(
        enclosing, wrong_parent);
    assert(!wrong.ok());
    for (const auto& child : wrong_parent.children) {
        assert(child.payload.enclosing_container_name_evidence.empty());
    }

    return 0;
}
