#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include "dmc_rengine/formats/dmc3/pac_container_parser.hpp"

#include <memory>

namespace dmc::rengine::profiles::dmc3 {

formats::ContainerParserRegistry make_container_parser_registry() {
    formats::ContainerParserRegistry registry;
    static_cast<void>(registry.register_parser(
        std::make_unique<formats::dmc3::PacContainerParser>()));
    return registry;
}

} // namespace dmc::rengine::profiles::dmc3
