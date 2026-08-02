#include "dmc_rengine/core/version.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <cassert>
#include <string>

int main() {
    using dmc::rengine::gdspaces::ResourceId;

    static_assert(dmc::rengine::version() == "0.1.0");

    const ResourceId valid{
        .source_id = "source",
        .logical_path = "dmc3/st001",
        .container_chain = "NBZ/PAC",
        .offset = 16,
        .size = 32,
    };

    assert(valid.valid());
    assert(valid.canonical() == std::string{"source:dmc3/st001#NBZ/PAC@16+32"});

    const ResourceId invalid{};
    assert(!invalid.valid());

    return 0;
}
