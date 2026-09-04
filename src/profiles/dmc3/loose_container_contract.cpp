#include "dmc_rengine/profiles/dmc3/loose_container_contract.hpp"

#include "dmc_rengine/profiles/dmc3/loose_container_list.hpp"

namespace dmc::rengine::profiles::dmc3 {

static_assert(LooseContainerContract::image_base == 0x140000000ULL);
static_assert(LooseContainerContract::canonical_target_sha256.size() == 64U);
static_assert(LooseContainerContract::rva_of(
    LooseContainerContract::representation_selector_va) == 0x001B79E0U);
static_assert(LooseContainerContract::rva_of(
    LooseContainerContract::extension_rewrite_va) == 0x001B9390U);
static_assert(LooseContainerContract::rva_of(
    LooseContainerContract::loose_materializer_va) == 0x001B85C0U);
static_assert(LooseContainerContract::rva_of(
    LooseContainerContract::generic_materializer_va) == 0x002EF4D0U);
static_assert(LooseContainerContract::parser_helper_vas.size() == 5U);

// Precedence, stated as the direction it actually runs.
static_assert(LooseContainerContract::packed_representation_wins);
static_assert(LooseContainerContract::requires_existing_extension);
static_assert(LooseContainerContract::list_extension == ".lst");
static_assert(LooseContainerContract::container_backed_kind16 == 0U);

static_assert(LooseContainerContract::terminates_child_text(0x0DU));
static_assert(LooseContainerContract::terminates_child_text(0x00U));
static_assert(!LooseContainerContract::terminates_child_text(0x0AU));
static_assert(!LooseContainerContract::lf_only_normal_line_is_equivalent);

// The correction that matters: `/` comments, `#` directs.
static_assert(LooseContainerContract::comment_marker == '/');
static_assert(LooseContainerContract::directive_marker == '#');
static_assert(!LooseContainerContract::directive_increments_slot_count);
static_assert(!LooseContainerContract::comment_increments_slot_count);
static_assert(LooseContainerContract::dummy_increments_slot_count);

static_assert(LooseContainerContract::directive_magic_bytes == 4U);
static_assert(!LooseContainerContract::directive_skips_whitespace);
static_assert(LooseContainerContract::default_magic[0] == 'P');
static_assert(LooseContainerContract::default_magic[3] == '\0');

// The product reader reads its bounds from the contract rather than repeating
// them, and these assertions keep that structural.
static_assert(
    LooseContainerListPolicy::scan_limit == LooseContainerContract::scan_limit);
static_assert(
    LooseContainerListPolicy::token_limit == LooseContainerContract::token_limit);
static_assert(
    LooseContainerListPolicy::synthesis_alignment ==
    LooseContainerContract::synthesis_alignment);

} // namespace dmc::rengine::profiles::dmc3
