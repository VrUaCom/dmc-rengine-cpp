#include "dmc_rengine/core/version.hpp"
#include "dmc_rengine/gdspaces/resource_id.hpp"

#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    using dmc::rengine::architecture_name;
    using dmc::rengine::version;

    if (argc > 1 && std::string_view{argv[1]} == "--version") {
        std::cout << "DMC Rengine " << version() << '\n';
        return 0;
    }

    const dmc::rengine::gdspaces::ResourceId example{
        .source_id = "local-game",
        .logical_path = "dmc3/stage/st001",
        .container_chain = "NBZ/AFS/PAC",
        .offset = 0,
        .size = 0,
    };

    std::cout << "DMC Rengine " << version() << '\n'
              << architecture_name() << '\n'
              << "Resource identity example: " << example.canonical() << '\n';

    return 0;
}
