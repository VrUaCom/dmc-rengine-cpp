#include "dmc_rengine/core/version.hpp"

namespace dmc::rengine {

static_assert(!version().empty());
static_assert(!architecture_name().empty());

} // namespace dmc::rengine
