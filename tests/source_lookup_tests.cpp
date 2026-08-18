#include "dmc_rengine/gdspaces/resource_key_index.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string source_id,
    std::string logical_path,
    std::string chain,
    std::uint64_t offset) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = std::move(logical_path),
            .container_chain = std::move(chain),
            .offset = offset,
            .size = 16U,
        },
        .display_name = "test.bin",
        .format = "unknown",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] bool has_chain(
    const std::vector<dmc::rengine::gdspaces::ResourceRef>& matches,
    std::string_view chain) {
    for (const auto& match : matches) {
        if (match.id.container_chain == chain) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::ResourceKeyIndex;
    using dmc::rengine::profiles::dmc3::ResourcePathPolicy;

    const auto first = resource(
        "archive-2", "Room/ST001.PAC", "nbz[10]", 0U);
    const auto second = resource(
        "archive-2", "room\\st001.pac", "nbz[11]", 16U);
    const auto video = resource(
        "archive-2", "Video/Intro.BIK", "nbz[12]", 32U);

    auto embedded_nul_path = std::string{"room/st001"};
    embedded_nul_path.push_back('\0');
    embedded_nul_path += "shadow.pac";

    auto invalid = resource(
        "archive-2", "ROOM/st001.pac", "nbz[invalid]", 48U);
    invalid.display_name.clear();
    assert(!invalid.valid());

    std::vector<dmc::rengine::gdspaces::ResourceRef> resources{
        first,
        second,
        video,
        second, // exact duplicate identity must not create false ambiguity
        std::move(invalid),
        resource("archive-3", "room/st001.pac", "nbz[wrong-source]", 64U),
        resource("archive-2", embedded_nul_path, "nbz[nul]", 80U),
        resource("archive-2", "////", "nbz[empty-key]", 96U),
    };

    const auto archive_index = ResourceKeyIndex::build(
        "archive-2",
        ResourcePathPolicy::archive_flags,
        resources);
    assert(archive_index.valid());
    assert(archive_index.source_id() == "archive-2");
    assert(archive_index.normalization_flags() == ResourcePathPolicy::archive_flags);

    const auto& receipt = archive_index.receipt();
    assert(receipt.valid());
    assert(receipt.input_count == 8U);
    assert(receipt.indexed_count == 3U);
    assert(receipt.rejected_invalid_resource_count == 1U);
    assert(receipt.rejected_source_mismatch_count == 1U);
    assert(receipt.rejected_c_string_count == 1U);
    assert(receipt.rejected_empty_key_count == 1U);
    assert(receipt.duplicate_identity_count == 1U);
    assert(receipt.conflict_key_count == 1U);

    // Two different physical identities collapse under archive flags 0x0E.
    // Product lookup preserves both and does not invent a winner.
    const auto ambiguous = archive_index.lookup("room\\st001.pac");
    assert(ambiguous.index_valid);
    assert(ambiguous.key_valid);
    assert(ambiguous.found());
    assert(!ambiguous.unique());
    assert(ambiguous.ambiguous());
    assert(ambiguous.matches.size() == 2U);
    assert(has_chain(ambiguous.matches, "nbz[10]"));
    assert(has_chain(ambiguous.matches, "nbz[11]"));
    assert(!has_chain(ambiguous.matches, "nbz[invalid]"));
    assert(!has_chain(ambiguous.matches, "nbz[wrong-source]"));

    const auto unique = archive_index.lookup("video\\intro.bik");
    assert(unique.index_valid);
    assert(unique.key_valid);
    assert(unique.unique());
    assert(unique.matches.size() == 1U);
    assert(unique.matches[0].id.container_chain == "nbz[12]");

    const auto absent = archive_index.lookup("does\\not\\exist.pac");
    assert(absent.index_valid);
    assert(absent.key_valid);
    assert(!absent.found());
    assert(absent.matches.empty());

    // The index accepts only a provider key already canonical under its flags.
    const auto raw_key = archive_index.lookup("Room/ST001.PAC");
    assert(raw_key.index_valid);
    assert(!raw_key.key_valid);
    assert(!raw_key.found());

    auto embedded_nul_key = std::string{"room\\st001"};
    embedded_nul_key.push_back('\0');
    embedded_nul_key += "evil.pac";
    const auto nul_key = archive_index.lookup(embedded_nul_key);
    assert(nul_key.index_valid);
    assert(!nul_key.key_valid);
    assert(!nul_key.found());

    // A source-bound index with no source identity is itself invalid; this is
    // distinct from a valid index returning no hit.
    const auto invalid_index = ResourceKeyIndex::build(
        "",
        ResourcePathPolicy::archive_flags,
        resources);
    assert(!invalid_index.valid());
    const auto invalid_index_query = invalid_index.lookup("room\\st001.pac");
    assert(!invalid_index_query.index_valid);
    assert(invalid_index_query.key_valid);
    assert(!invalid_index_query.found());

    // Physical normalization 0x0C preserves case. Provider policy belongs to
    // the derived index, not to ISource implementations.
    const std::vector<dmc::rengine::gdspaces::ResourceRef> physical_resources{
        first,
        second,
    };
    const auto physical_index = ResourceKeyIndex::build(
        "archive-2",
        ResourcePathPolicy::physical_flags,
        physical_resources);
    assert(physical_index.valid());

    const auto physical_exact = physical_index.lookup("Room\\ST001.PAC");
    assert(physical_exact.unique());
    assert(physical_exact.matches[0].id.container_chain == "nbz[10]");

    const auto physical_wrong_case = physical_index.lookup("room\\ST001.PAC");
    assert(physical_wrong_case.key_valid);
    assert(!physical_wrong_case.found());

    return 0;
}
