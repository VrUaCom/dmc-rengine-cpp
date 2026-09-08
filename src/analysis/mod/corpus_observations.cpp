#include "dmc_rengine/analysis/mod/corpus_observations.hpp"

namespace dmc::rengine::analysis::mod {
namespace {

static_assert(Em000CorpusSummary::mod_count == 35U);
static_assert(Em000CorpusSummary::top_level_mod_count +
              Em000CorpusSummary::nested_mod_count ==
              Em000CorpusSummary::mod_count);
static_assert(Em000CorpusSummary::one_influence_vertices +
              Em000CorpusSummary::two_influence_vertices +
              Em000CorpusSummary::three_influence_vertices ==
              Em000CorpusSummary::vertex_count);
static_assert(Em000CorpusSummary::motion_group_0_count +
              Em000CorpusSummary::motion_group_1_count +
              Em000CorpusSummary::motion_group_2_count ==
              Em000CorpusSummary::node_position_count);

static_assert(dmc::rengine::formats::mod::is_corpus_confirmed_structural_version(0.82F));
static_assert(dmc::rengine::formats::mod::is_corpus_confirmed_structural_version(0.84F));
static_assert(dmc::rengine::formats::mod::is_corpus_confirmed_structural_version(1.00F));
static_assert(dmc::rengine::formats::mod::is_corpus_confirmed_structural_version(1.01F));
static_assert(!dmc::rengine::formats::mod::is_corpus_confirmed_structural_version(0.83F));

constexpr auto metadata_100407 = project_runtime_metadata_decimal(100407U);
static_assert(metadata_100407.high_component == 1U);
static_assert(metadata_100407.middle_component == 4U);
static_assert(metadata_100407.low_component == 7U);
static_assert(recompose_runtime_metadata_decimal(metadata_100407) == 100407U);

constexpr auto metadata_601715 = project_runtime_metadata_decimal(601715U);
static_assert(metadata_601715.high_component == 6U);
static_assert(metadata_601715.middle_component == 17U);
static_assert(metadata_601715.low_component == 15U);
static_assert(recompose_runtime_metadata_decimal(metadata_601715) == 601715U);

constexpr auto metadata_700601 = project_runtime_metadata_decimal(700601U);
static_assert(metadata_700601.high_component == 7U);
static_assert(metadata_700601.middle_component == 6U);
static_assert(metadata_700601.low_component == 1U);
static_assert(recompose_runtime_metadata_decimal(metadata_700601) == 700601U);

static_assert(em000_generated_workspace_capacity(0U) == 0U);
static_assert(em000_generated_workspace_capacity(1U) == 0U);
static_assert(em000_generated_workspace_capacity(2U) == 0U);
static_assert(em000_generated_workspace_capacity(3U) == 0x10U);
static_assert(em000_generated_workspace_capacity(34U) == 0xC0U);
static_assert(em000_generated_workspace_capacity(376U) == 0x8D0U);

} // namespace
} // namespace dmc::rengine::analysis::mod
