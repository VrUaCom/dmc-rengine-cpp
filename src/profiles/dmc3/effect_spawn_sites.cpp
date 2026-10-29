#include "dmc_rengine/profiles/dmc3/effect_spawn_sites.hpp"

#include <algorithm>
#include <array>
#include <iterator>

namespace dmc::rengine::profiles::dmc3::effect_spawn_sites {
namespace {

struct Row final {
    std::uint64_t address;
    std::uint64_t function;
    const char* owner;
    char kind;
    std::uint16_t id;
    std::uint8_t call;
    std::int16_t mode;
};

constexpr Row kRows[] = {
#include "effect_spawn_sites.inc"
};

constexpr auto kSites = [] {
    std::array<SpawnSite, std::size(kRows)> sites{};
    for (std::size_t i = 0U; i < std::size(kRows); ++i) {
        const auto& r = kRows[i];
        sites[i] = {r.address, r.function, r.owner, r.kind, r.id, static_cast<SpawnCall>(r.call), r.mode};
    }
    return sites;
}();

} // namespace

std::span<const SpawnSite> spawn_sites() noexcept { return kSites; }

std::vector<std::string_view> owners(char kind, std::uint16_t id) {
    std::vector<std::string_view> out;
    for (const auto& site : kSites) {
        if (site.kind != kind || site.id != id) continue;
        const std::string_view owner = site.owner;
        if (std::find(out.begin(), out.end(), owner) == out.end()) out.push_back(owner);
    }
    return out;
}

std::uint64_t call_target(const SpawnSite& site) noexcept {
    switch (site.call) {
    case SpawnCall::spawn: return spawn_va;
    case SpawnCall::spawn_flags: return spawn_flags_va;
    case SpawnCall::spawn_raised: return spawn_raised_va;
    case SpawnCall::factory: break;
    }
    switch (site.kind) {
    case 'P': return factory_p_va;
    case 'E': return factory_e_va;
    case 'G': return factory_g_va;
    case 'V': return factory_v_va;
    default: return 0U;
    }
}

} // namespace dmc::rengine::profiles::dmc3::effect_spawn_sites
