#include "dmc_rengine/profiles/dmc3/legacy_index_replay.hpp"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;

[[nodiscard]] gdspaces::ResourceId resource_id(
    std::string_view path,
    std::uint64_t offset = 0U,
    std::uint64_t size = 1U) {
    return gdspaces::ResourceId{
        .source_id = "retail-source",
        .logical_path = std::string{path},
        .container_chain = {},
        .offset = offset,
        .size = size,
    };
}

[[nodiscard]] gdspaces::ResourceNamingIdentity child(
    std::uint32_t slot,
    bool populated,
    std::optional<std::size_t> ordinal,
    std::optional<std::string> raw_label,
    std::optional<std::string> index_name,
    bool folder = false) {
    return gdspaces::ResourceNamingIdentity{
        .resource_id = resource_id(
            "GData.afs/st001.pac::PAC/slot-" + std::to_string(slot),
            0x100U + static_cast<std::uint64_t>(slot) * 0x100U,
            populated ? 0x80U : 0U),
        .physical_slot_index = slot,
        .populated = populated,
        .extracted_ordinal = ordinal,
        .external_index_raw_label = std::move(raw_label),
        .external_index_name = std::move(index_name),
        .external_index_folder = folder,
        .embedded_alias = std::nullopt,
        .semantic_format = populated ? "unknown" : "empty",
        .canonical_extension = {},
        .canonical_display_name = "slot-" + std::to_string(slot),
    };
}

[[nodiscard]] gdspaces::ContainerNamingIdentitySnapshot valid_snapshot() {
    gdspaces::ContainerNamingIdentitySnapshot snapshot;
    snapshot.parent_resource = resource_id("GData.afs/st001.pac", 0U, 0x4000U);
    snapshot.external_index_evidence = gdspaces::ContainerIndexNamingEvidence{
        .manifest_resource = resource_id("GData.afs/st001.index", 0U, 64U),
        .manifest_sha256 = std::string(64U, 'a'),
        .directive = "PNST",
        .entry_count = 2U,
    };
    snapshot.children = {
        child(
            0U,
            true,
            0U,
            std::string{"st001_000.ukn"},
            std::string{"st001_000.ukn"}),
        child(1U, false, std::nullopt, std::nullopt, std::nullopt),
        child(
            2U,
            true,
            1U,
            std::string{"st001_001 folder"},
            std::string{"st001_001"},
            true),
    };
    return snapshot;
}

[[nodiscard]] bool has_code(
    const std::vector<gdspaces::Diagnostic>& diagnostics,
    std::string_view code) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.code == code) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    {
        const auto snapshot = valid_snapshot();
        assert(snapshot.ok());

        const auto replay = dmc3::LegacyIndexReplayPlanner::build(snapshot);
        assert(replay.ok());
        assert(replay.plan.valid());
        assert(replay.plan.parent_resource == snapshot.parent_resource);
        assert(replay.plan.manifest_resource ==
               snapshot.external_index_evidence->manifest_resource);
        assert(replay.plan.manifest_sha256 == std::string(64U, 'a'));
        assert(replay.plan.directive == "PNST");
        assert(replay.plan.raw_entries.size() == 2U);
        assert(replay.plan.raw_entries[0] == "st001_000.ukn");
        assert(replay.plan.raw_entries[1] == "st001_001 folder");
        assert(replay.plan.exact_labels_from_external_index);
        assert(
            replay.plan.render_crlf() ==
            "PNST\r\nst001_000.ukn\r\nst001_001 folder\r\n");
    }

    {
        auto snapshot = valid_snapshot();
        snapshot.external_index_evidence.reset();
        for (auto& identity : snapshot.children) {
            identity.external_index_raw_label.reset();
            identity.external_index_name.reset();
            identity.external_index_folder = false;
        }
        assert(snapshot.ok());

        const auto replay = dmc3::LegacyIndexReplayPlanner::build(snapshot);
        assert(!replay.ok());
        assert(has_code(
            replay.diagnostics,
            "gdspaces.dmc3-index-replay.no-external-index"));
        assert(replay.plan.render_crlf().empty());
    }

    {
        auto snapshot = valid_snapshot();
        snapshot.children[1].extracted_ordinal = 1U;
        snapshot.children[1].external_index_raw_label = "illegal_001.ukn";
        snapshot.children[1].external_index_name = "illegal_001.ukn";
        assert(!snapshot.ok());

        const auto replay = dmc3::LegacyIndexReplayPlanner::build(snapshot);
        assert(!replay.ok());
        assert(has_code(
            replay.diagnostics,
            "gdspaces.dmc3-index-replay.invalid-snapshot"));
    }

    {
        auto snapshot = valid_snapshot();
        snapshot.children[2].extracted_ordinal = 2U;
        assert(!snapshot.ok());

        const auto replay = dmc3::LegacyIndexReplayPlanner::build(snapshot);
        assert(!replay.ok());
        assert(has_code(
            replay.diagnostics,
            "gdspaces.dmc3-index-replay.invalid-snapshot"));
    }

    {
        auto snapshot = valid_snapshot();
        snapshot.external_index_evidence->directive = "NOT-PNST";
        assert(!snapshot.ok());

        const auto replay = dmc3::LegacyIndexReplayPlanner::build(snapshot);
        assert(!replay.ok());
        assert(has_code(
            replay.diagnostics,
            "gdspaces.dmc3-index-replay.invalid-snapshot"));
    }

    return 0;
}
