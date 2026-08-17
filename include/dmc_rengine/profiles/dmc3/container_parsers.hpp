#pragma once

#include "dmc_rengine/formats/container_parser_registry.hpp"

namespace dmc::rengine::profiles::dmc3 {

// Canonical product-side DMC3 container parser set. The adapters registered
// here call the clean format authorities (PacParser/PnstParser); they do not
// implement independent format decoding.
[[nodiscard]] formats::ContainerParserRegistry make_container_parser_registry();

} // namespace dmc::rengine::profiles::dmc3
