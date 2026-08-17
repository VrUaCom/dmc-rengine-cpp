#pragma once

#include "dmc_rengine/formats/container_parser_registry.hpp"

namespace dmc::rengine::profiles::dmc3 {

[[nodiscard]] formats::ContainerParserRegistry make_container_parser_registry();

} // namespace dmc::rengine::profiles::dmc3
