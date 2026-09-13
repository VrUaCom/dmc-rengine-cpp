#include "dmc_rengine/profiles/dmc3/hits_evidence_catalog.hpp"

namespace dmc::rengine::profiles::dmc3::hits_catalog {
namespace {

// Compile/link smoke probe for the complete modular HITS evidence surface.
// Runtime truth remains exposed through canonical_catalog_consistent(); this
// initializer intentionally does not promote any evidence status.
[[maybe_unused]] const bool catalog_link_probe = canonical_catalog_consistent();

} // namespace
} // namespace dmc::rengine::profiles::dmc3::hits_catalog
