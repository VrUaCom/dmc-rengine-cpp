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

    ResourceKeyIndex invalid_index{""};
    assert(!invalid_index.valid());
    assert(invalid_index.empty());
    assert(!invalid_index.add(
        "one", resource("archive-2", "one.bin", "nbz[0]", 0U)));

    ResourceKeyIndex index{"archive-2"};
    assert(index.valid());
    assert(index.source_id() == "archive-2");
    assert(index.empty());
    assert(!index.add("", resource("archive-2", "one.bin", "nbz[0]", 0U)));
    assert(!index.add("one", ResourceRef{}));
    assert(!index.add(
        "one", resource("archive-1", "one.bin", "nbz[0]", 0U)));

    const auto first = resource("archive-2", "Room/ST001.PAC", "nbz[10]", 0U);
    const auto second = resource("archive-2", "room\\st001.pac", "nbz[11]", 0U);
    const auto unique = resource("archive-2", "Video/Intro.BIK", "nbz[12]", 0U);

    assert(index.add("room\\st001.pac", first));
    assert(index.add("room\\st001.pac", second));
    assert(index.add("video\\intro.bik", unique));

    const auto collision = index.lookup("room\\st001.pac");
    assert(collision.found());
    assert(!collision.unique());
    assert(collision.ambiguous());
    assert(collision.matches.size() == 2U);
    assert(collision.matches[0].id != collision.matches[1].id);
    assert(collision.matches[0].id.source_id == "archive-2");
    assert(collision.matches[1].id.source_id == "archive-2");

    // Deterministic bucket order is product presentation only. It is not an
    // original-runtime duplicate winner rule; Pass 22 establishes CRT-dependent
    // ambiguity for comparator-equal normalized keys.
    assert(collision.matches[0].id.canonical() <
        collision.matches[1].id.canonical());

    const auto one = index.lookup("video\\intro.bik");
    assert(one.unique());
    assert(one.matches[0].id.container_chain == "nbz[12]");

    assert(!index.lookup("missing.bin").found());
    assert(!index.lookup("").found());

    // One immutable ResourceId cannot be duplicated or assigned to another key
    // in the same source-bound derived index.
    assert(!index.add("room\\st001.pac", first));
    assert(!index.add("different-key", first));

    const auto stats = index.stats();
    assert(stats.indexed_resources == 3U);
    assert(stats.key_count == 2U);
    assert(stats.ambiguous_keys == 1U);

    return 0;
}
