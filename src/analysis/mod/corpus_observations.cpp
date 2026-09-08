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

static_assert(MultiCorpusModSummary::em000_mod_count +
              MultiCorpusModSummary::pl000_mod_count +
              MultiCorpusModSummary::id100_mod_count ==
              MultiCorpusModSummary::unique_mod_count);
static_assert(MultiCorpusModSummary::version_082_count +
              MultiCorpusModSummary::version_084_count +
              MultiCorpusModSummary::version_100_count +
              MultiCorpusModSummary::version_101_count ==
              MultiCorpusModSummary::unique_mod_count);
static_assert(MultiCorpusModSummary::unique_mod_count == 38U);
static_assert(MultiCorpusModSummary::object_count == 166U);
static_assert(MultiCorpusModSummary::mesh_count == 180U);
static_assert(MultiCorpusModSummary::vertex_count == 20976U);
static_assert(MultiCorpusModSummary::transform_record_count == 285U);

static_assert(MultiCorpusModSummary::header_08_0f_all_zero);
static_assert(MultiCorpusModSummary::header_18_1f_all_zero);
static_assert(MultiCorpusModSummary::header_28_3f_all_zero);
static_assert(MultiCorpusModSummary::object_04_07_all_zero);
static_assert(MultiCorpusModSummary::object_14_17_all_zero);
static_assert(MultiCorpusModSummary::object_20_2f_all_zero);
static_assert(MultiCorpusModSummary::mesh_0c_all_zero);
static_assert(MultiCorpusModSummary::mesh_38_all_zero);
static_assert(MultiCorpusModSummary::mesh_48_serialized_all_zero);
static_assert(MultiCorpusModSummary::mesh_4c_all_zero);
static_assert(MultiCorpusModSummary::node_domain_10_1f_all_zero);
static_assert(MultiCorpusModSummary::transform_1c_all_zero);
static_assert(MultiCorpusModSummary::blendindices_x_all_zero);

static_assert(MultiCorpusModSummary::node_domain_exact_layout_count ==
              MultiCorpusModSummary::unique_mod_count);
static_assert(MultiCorpusModSummary::topological_hierarchy_count ==
              MultiCorpusModSummary::unique_mod_count);
static_assert(MultiCorpusModSummary::translation_magnitude_match_count ==
              MultiCorpusModSummary::transform_record_count);
static_assert(MultiCorpusModSummary::motion_group_0_count +
              MultiCorpusModSummary::motion_group_1_count +
              MultiCorpusModSummary::motion_group_2_count ==
              MultiCorpusModSummary::transform_record_count);

static_assert(MultiCorpusModSummary::workspace_capacity_match_count ==
              MultiCorpusModSummary::mesh_count);
static_assert(MultiCorpusModSummary::workspace_aligned_count ==
              MultiCorpusModSummary::mesh_count);
static_assert(MultiCorpusModSummary::workspace_start_1212_count ==
              MultiCorpusModSummary::mesh_count);
static_assert(MultiCorpusModSummary::workspace_all_12_count +
              MultiCorpusModSummary::workspace_final_12_then_0000_count ==
              MultiCorpusModSummary::mesh_count);
static_assert(MultiCorpusModSummary::workspace_final_12_then_0000_count ==
              MultiCorpusModSummary::unique_mod_count);
static_assert(MultiCorpusModSummary::file_tail_1212_0000_count ==
              MultiCorpusModSummary::unique_mod_count);
static_assert(MultiCorpusModSummary::workspace_observed_fill_word == 0x1212U);
static_assert(MultiCorpusModSummary::file_observed_terminal_word == 0x0000U);

static_assert(MultiCorpusModSummary::alpha_control_80_count ==
              MultiCorpusModSummary::object_count);
static_assert(MultiCorpusModSummary::source_flag_00100000_count == 45U);
static_assert(MultiCorpusModSummary::source_flag_00200000_count == 7U);
static_assert(MultiCorpusModSummary::populated_parameter18_1c_object_count == 2U);
static_assert(MultiCorpusModSummary::pl000_runtime_metadata_u32 == 217U);
static_assert(MultiCorpusModSummary::id100_runtime_metadata_u32 == 1000000U);

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

// The same arithmetic operation remains lossless for the broader corpus, but
// the resulting components are deliberately NOT treated as universal MOD ABI.
constexpr auto metadata_pl000 = project_runtime_metadata_decimal(
    MultiCorpusModSummary::pl000_runtime_metadata_u32);
static_assert(recompose_runtime_metadata_decimal(metadata_pl000) == 217U);
constexpr auto metadata_id100 = project_runtime_metadata_decimal(
    MultiCorpusModSummary::id100_runtime_metadata_u32);
static_assert(recompose_runtime_metadata_decimal(metadata_id100) == 1000000U);

static_assert(mod_generated_workspace_capacity(0U) == 0U);
static_assert(mod_generated_workspace_capacity(1U) == 0U);
static_assert(mod_generated_workspace_capacity(2U) == 0U);
static_assert(mod_generated_workspace_capacity(3U) == 0x10U);
static_assert(mod_generated_workspace_capacity(34U) == 0xC0U);
static_assert(mod_generated_workspace_capacity(376U) == 0x8D0U);
static_assert(mod_generated_workspace_capacity(773U) == 0x1220U);

static_assert(em000_generated_workspace_capacity(34U) ==
              mod_generated_workspace_capacity(34U));
static_assert(em000_generated_workspace_capacity(376U) ==
              mod_generated_workspace_capacity(376U));

} // namespace
} // namespace dmc::rengine::analysis::mod
