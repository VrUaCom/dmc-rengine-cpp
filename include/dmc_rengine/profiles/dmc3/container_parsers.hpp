#pragma once

#include "dmc_rengine/formats/container_parser_registry.hpp"

namespace dmc::rengine::profiles::dmc3 {

// Production DMC3 container-parser set promoted from evidence-backed runtime
// schemas. Parsers are read-only and do not own resolution or source lookup.
[[nodiscard]] formats::ContainerParserRegistry make_container_parser_registry();

} // namespace dmc::rengine::profiles::dmc3
