#include "dmc_rengine/gdspaces/index_manifest.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>

namespace {

[[nodiscard]] dmc::rengine::formats::ContainerEntry entry(
    std::uint32_t slot,
    std::uint64_t offset,
    std::uint64_t size,
    bool populated) {
    return dmc::rengine::formats::ContainerEntry{
        .slot_index = slot,
        .offset = offset,
        .size = size,
        .logical_name = populated
            ? "slot_" + std::to_string(slot) + ".bin"
            : "slot_" + std::to_string(slot) + ".empty",
        .populated = populated,
        .synthetic_name = true,
    };
}

[[nodiscard]] dmc::rengine::formats::ContainerDocument dense_document() {
    return dmc::rengine::formats::ContainerDocument{
        .format = "pac",
        .schema_version = 1U,
        .declared_slot_count = 3U,
        .container_size = 64U,
        .entries = {
            entry(0U, 16U, 16U, true),
            entry(1U, 32U, 16U, true),
            entry(2U, 48U, 16U, true),
        },
    };
}

[[nodiscard]] dmc::rengine::formats::ContainerDocument sparse_document() {
    dmc::rengine::formats::ContainerDocument document{
        .format = "pnst",
        .schema_version = 1U,
        .declared_slot_count = 11U,
        .container_size = 112U,
        .entries = {},
    };
    document.entries.reserve(11U);
    document.entries.push_back(entry(0U, 64U, 32U, true));
    for (std::uint32_t slot = 1U; slot < 10U; ++slot) {
        document.entries.push_back(entry(slot, 0U, 0U, false));
    }
    document.entries.push_back(entry(10U, 96U, 16U, true));
    return document;
}

[[nodiscard]] bool has_code(
    const std::vector<dmc::rengine::gdspaces::Diagnostic>& diagnostics,
    const std::string& code) {
    return std::any_of(
        diagnostics.begin(), diagnostics.end(),
        [&code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::link_index_manifest;
    using dmc::rengine::gdspaces::parse_index_manifest;

    const auto dense_manifest = parse_index_manifest(
        "st001_000.ukn\r\n"
        "st001_001 folder\r\n"
        "st001_002.scm\r\n");
    assert(!dense_manifest.pnst_directive);
    assert(dense_manifest.entries.size() == 3U);
    assert(dense_manifest.entries[1].folder);
    assert(dense_manifest.entries[1].raw_line == "st001_001 folder");

    const auto dense = dense_document();
    assert(dense.valid());
    const auto dense_links = link_index_manifest(dense, dense_manifest);
    assert(dense_links.safe_positional_mapping);
    assert(dense_links.links.size() == 3U);
    assert(dense_links.links[0].slot_index == 0U);
    assert(dense_links.links[1].slot_index == 1U);
    assert(dense_links.links[1].label == "st001_001");
    assert(dense_links.links[1].folder);
    assert(dense_links.links[2].slot_index == 2U);

    const auto sparse_manifest = parse_index_manifest(
        "PNST\n"
        "crystal_a.mod\n"
        "crystal_b.mod\n");
    assert(sparse_manifest.pnst_directive);
    assert(sparse_manifest.entries.size() == 2U);

    const auto sparse = sparse_document();
    assert(sparse.valid());
    const auto sparse_links = link_index_manifest(sparse, sparse_manifest);
    assert(!sparse_links.safe_positional_mapping);
    assert(sparse_links.links.empty());
    assert(has_code(
        sparse_links.diagnostics,
        "gdspaces.index.sparse-slot-mapping-unresolved"));

    // Even a full number of labels does not override binary sparse identity.
    const auto eleven_labels = parse_index_manifest(
        "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    const auto still_sparse = link_index_manifest(sparse, eleven_labels);
    assert(!still_sparse.safe_positional_mapping);
    assert(still_sparse.links.empty());

    const auto short_manifest = parse_index_manifest("a\nb\n");
    const auto mismatch = link_index_manifest(dense, short_manifest);
    assert(!mismatch.safe_positional_mapping);
    assert(has_code(
        mismatch.diagnostics,
        "gdspaces.index.entry-count-mismatch"));

    const auto unknown_tokens = parse_index_manifest("thing.bin folder extra\n");
    assert(unknown_tokens.entries.size() == 1U);
    assert(unknown_tokens.entries[0].folder);
    assert(has_code(unknown_tokens.diagnostics, "gdspaces.index.unknown-token"));
    assert(has_code(unknown_tokens.diagnostics, "gdspaces.index.extra-tokens"));

    return 0;
}
