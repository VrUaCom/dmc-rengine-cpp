#include "dmc_rengine/gdspaces/resource_graph.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"

#include <cassert>
#include <string>

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;

    // Reproduce P1 from the retained L1 review. Under the legacy delimiter-only
    // encoding these two distinct identities both serialized as:
    // corpus:boot.pac::PAC/slot-0000/a.bin#PAC[0]@64+128
    const gdspaces::ResourceId child{
        .source_id = "corpus",
        .logical_path = "boot.pac::PAC/slot-0000/a.bin",
        .container_chain = "PAC[0]",
        .offset = 64U,
        .size = 128U,
    };
    const gdspaces::ResourceId delimiter_in_path{
        .source_id = "corpus",
        .logical_path = "boot.pac::PAC/slot-0000/a.bin#PAC[0]",
        .container_chain = {},
        .offset = 64U,
        .size = 128U,
    };

    assert(child.valid());
    assert(delimiter_in_path.valid());
    assert(child != delimiter_in_path);
    assert(child.canonical() != delimiter_in_path.canonical());
    assert(child.canonical().starts_with("rid2|"));
    assert(delimiter_in_path.canonical().starts_with("rid2|"));

    // Source/path boundaries are length-bound too; ':' can no longer move the
    // apparent boundary between the two arbitrary string fields.
    const gdspaces::ResourceId source_colon{
        .source_id = "a:b",
        .logical_path = "c",
        .container_chain = {},
        .offset = 1U,
        .size = 2U,
    };
    const gdspaces::ResourceId path_colon{
        .source_id = "a",
        .logical_path = "b:c",
        .container_chain = {},
        .offset = 1U,
        .size = 2U,
    };
    assert(source_colon != path_colon);
    assert(source_colon.canonical() != path_colon.canonical());

    gdspaces::ResourceGraph graph;
    const gdspaces::ResourceRef child_ref{
        .id = child,
        .display_name = "a.bin",
        .format = "unknown",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
    const gdspaces::ResourceRef delimiter_ref{
        .id = delimiter_in_path,
        .display_name = "a.bin#PAC[0]",
        .format = "unknown",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };

    assert(graph.add(child_ref));
    assert(graph.add(delimiter_ref));
    assert(graph.resource_count() == 2U);
    assert(graph.find(child) != nullptr);
    assert(graph.find(delimiter_in_path) != nullptr);
    assert(graph.find(child)->id == child);
    assert(graph.find(delimiter_in_path)->id == delimiter_in_path);

    // Determinism remains unchanged: equal identities always produce exactly
    // the same versioned machine key.
    const auto child_copy = child;
    assert(child_copy == child);
    assert(child_copy.canonical() == child.canonical());

    return 0;
}
