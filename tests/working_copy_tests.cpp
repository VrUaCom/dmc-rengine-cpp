#include "dmc_rengine/gdspaces/working_copy.hpp"

#include <cassert>
#include <cstddef>
#include <string>

int main() {
    dmc::rengine::gdspaces::ResourcePayload payload{
        .resource = dmc::rengine::gdspaces::ResourceRef{
            .id = dmc::rengine::gdspaces::ResourceId{
                .source_id = "synthetic",
                .logical_path = "fixture.bin",
                .container_chain = {},
                .offset = 0,
                .size = 4,
            },
            .display_name = "fixture.bin",
            .format = "bin",
            .profile = "unknown",
            .synthetic_name = false,
            .container = false,
        },
        .bytes = {
            std::byte{0x01},
            std::byte{0x02},
            std::byte{0x03},
            std::byte{0x04},
        },
        .diagnostics = {},
    };

    dmc::rengine::gdspaces::WorkingCopy copy(std::move(payload));
    assert(copy.revision() == 0U);
    assert(!copy.dirty());
    assert(copy.history().empty());
    assert(copy.source_sha256().size() == 64U);

    auto result = copy.apply(dmc::rengine::gdspaces::EditOperation{
        .id = "insert-byte",
        .base_revision = 0,
        .offset = 1,
        .expected = {std::byte{0x02}},
        .replacement = {std::byte{0x09}, std::byte{0x08}},
        .description = "Replace one byte with two synthetic bytes",
    });
    assert(result.applied);
    assert(result.revision == 1U);
    assert(copy.dirty());
    assert(copy.bytes().size() == 5U);
    assert(copy.bytes()[1] == std::byte{0x09});
    assert(copy.bytes()[2] == std::byte{0x08});

    const auto stale = copy.apply(dmc::rengine::gdspaces::EditOperation{
        .id = "stale-edit",
        .base_revision = 0,
        .offset = 0,
        .expected = {std::byte{0x01}},
        .replacement = {std::byte{0xFF}},
        .description = "Stale edit",
    });
    assert(!stale.applied);
    assert(!stale.error.empty());

    assert(copy.undo_last());
    assert(copy.revision() == 2U);
    assert(!copy.dirty());
    assert(copy.bytes().size() == 4U);
    assert(copy.history().empty());
    assert(!copy.undo_last());

    copy.reset();
    assert(copy.revision() == 0U);
    assert(!copy.dirty());

    return 0;
}
