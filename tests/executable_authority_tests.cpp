#include "dmc_rengine/profiles/dmc3/executable_authority.hpp"

#include <cassert>
#include <string>

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto& analysis = dmc3::canonical_analysis_executable();
    const auto& distribution = dmc3::protected_distribution_executable();

    assert(analysis.valid());
    assert(distribution.valid());
    assert(analysis.sha256 != distribution.sha256);
    assert(analysis.file_size != distribution.file_size);

    assert(analysis.role == dmc3::ExecutableAuthorityRole::analysis_reverse);
    assert(analysis.instruction_reverse_authority);
    assert(!analysis.distribution_provenance_authority);
    assert(!analysis.original_execution_candidate);

    assert(distribution.role == dmc3::ExecutableAuthorityRole::protected_distribution);
    assert(!distribution.instruction_reverse_authority);
    assert(distribution.distribution_provenance_authority);
    assert(distribution.original_execution_candidate);

    const auto analysis_match = dmc3::classify_executable_authority(
        analysis.sha256, analysis.file_size);
    assert(analysis_match.recognized());
    assert(analysis_match.authority == &analysis);

    std::string uppercase_analysis{analysis.sha256};
    for (auto& ch : uppercase_analysis) {
        if (ch >= 'a' && ch <= 'f') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
    }
    const auto uppercase_match = dmc3::classify_executable_authority(
        uppercase_analysis, analysis.file_size);
    assert(uppercase_match.recognized());
    assert(uppercase_match.authority == &analysis);

    const auto distribution_match = dmc3::classify_executable_authority(
        distribution.sha256, distribution.file_size);
    assert(distribution_match.recognized());
    assert(distribution_match.authority == &distribution);

    const auto mismatch = dmc3::classify_executable_authority(
        distribution.sha256, distribution.file_size + 1U);
    assert(mismatch.status ==
        dmc3::ExecutableAuthorityMatchStatus::known_hash_size_mismatch);
    assert(mismatch.authority == &distribution);
    assert(!mismatch.recognized());

    const auto unknown = dmc3::classify_executable_authority(
        "0000000000000000000000000000000000000000000000000000000000000000",
        123U);
    assert(unknown.status == dmc3::ExecutableAuthorityMatchStatus::unknown);
    assert(unknown.authority == nullptr);
    assert(!unknown.recognized());

    return 0;
}
