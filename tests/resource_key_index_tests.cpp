#include "dmc_rengine/gdspaces/resource_key_index.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>

namespace {

dmc::rengine::gdspaces::ResourceRef resource(
    std::string source,
    std::string path,
    std::string chain,
    std::uint64_t offset) {
    using namespace dmc::rengine::gdspaces;
    return ResourceRef{
        .id = ResourceId{
            .source_id = std::move(source),
            .logical_path = std::move(path),
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

} // namespace

int main() {
    using dmc::rengine::gdspaces::ResourceKeyIndex;
    using dmc::rengine::gdspaces::ResourceRef;

    ResourceKeyIndex index;
    assert(index.empty());
    assert(!index.add("", resource("a", "one.bin", "nbz[0]", 0U)));
    assert(!index.add("one", ResourceRef{}));

    const auto first = resource("archive-2", "Room/ST001.PAC", "nbz[10]", 0U);
    const auto second = resource("archive-2", "room\\st001.pac", "nbz[11]", 0U);
    const auto unique = resource("archive-2", "Video/Intro.BIK", "nbz[12]", 0U);

    assert(index.add("room\\st001.pac", first));
    assert(index.add("room\\st001.pac", second));
    assert(index.add("video\\intro.bik", unique));
    assert(!index.empty());

    const auto collision = index.lookup("room\\st001.pac");
    assert(collision.found());
    assert(!collision.unique());
    assert(collision.ambiguous());
    assert(collision.matches.size() == 2U);
    assert(collision.matches[0].id != collision.matches[1].id);

    // Bucket ordering is deterministic product presentation only. It must not
    // be interpreted as original-runtime duplicate winner semantics.
    assert(collision.matches[0].id.canonical() <
        collision.matches[1].id.canonical());

    const auto one = index.lookup("video\\intro.bik");
    assert(one.found());
    assert(one.unique());
    assert(!one.ambiguous());
    assert(one.matches.size() == 1U);
    assert(one.matches[0].id.container_chain == "nbz[12]");

    const auto missing = index.lookup("missing.bin");
    assert(!missing.found());
    assert(!missing.unique());
    assert(!missing.ambiguous());
    assert(missing.matches.empty());

    const auto empty_lookup = index.lookup("");
    assert(!empty_lookup.found());

    // One immutable ResourceId may not be duplicated or assigned to a second
    // key in the same derived index.
    assert(!index.add("room\\st001.pac", first));
    assert(!index.add("different-key", first));

    const auto stats = index.stats();
    assert(stats.indexed_resources == 3U);
    assert(stats.unique_keys == 2U);
    assert(stats.ambiguous_keys == 1U);

    return 0;
}
