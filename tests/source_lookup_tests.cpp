#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/resource_path_policy.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class StaticSource final : public dmc::rengine::gdspaces::ISource {
public:
    StaticSource(
        std::string source_id,
        std::vector<dmc::rengine::gdspaces::ResourceRef> resources)
        : source_id_(std::move(source_id)),
          resources_(std::move(resources)) {}

    [[nodiscard]] std::string_view id() const noexcept override {
        return source_id_;
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "static-test";
    }

    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef>
    enumerate() const override {
        return resources_;
    }

    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId&) const override {
        return std::nullopt;
    }

private:
    std::string source_id_;
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources_;
};

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string source_id,
    std::string logical_path,
    std::string chain,
    std::uint64_t offset) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = std::move(logical_path),
            .container_chain = std::move(chain),
            .offset = offset,
            .size = 16U,
        },
        .display_name = "test.bin",
        .format = "unknown",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

} // namespace

int main() {
    using dmc::rengine::gdspaces::SourceRegistry;
    using dmc::rengine::profiles::dmc3::ResourcePathPolicy;

    SourceRegistry registry;
    std::vector<dmc::rengine::gdspaces::ResourceRef> resources{
        resource("archive-2", "Room/ST001.PAC", "nbz[10]", 0U),
        resource("archive-2", "room\\st001.pac", "nbz[11]", 0U),
        resource("archive-2", "Video/Intro.BIK", "nbz[12]", 0U),
    };
    assert(registry.mount(std::make_unique<StaticSource>(
        "archive-2", std::move(resources))));

    const auto unavailable = registry.lookup(
        "missing-source",
        "room\\st001.pac",
        ResourcePathPolicy::archive_flags);
    assert(!unavailable.source_available);
    assert(!unavailable.found());
    assert(!unavailable.unique());
    assert(!unavailable.ambiguous());
    assert(unavailable.matches.empty());

    const auto absent = registry.lookup(
        "archive-2",
        "does\\not\\exist.pac",
        ResourcePathPolicy::archive_flags);
    assert(absent.source_available);
    assert(!absent.found());
    assert(!absent.unique());
    assert(!absent.ambiguous());
    assert(absent.matches.empty());

    // Both source names normalize to the same archive key under 0x0E. The
    // registry must preserve both physical identities instead of picking one.
    const auto ambiguous = registry.lookup(
        "archive-2",
        "room\\st001.pac",
        ResourcePathPolicy::archive_flags);
    assert(ambiguous.source_available);
    assert(ambiguous.found());
    assert(!ambiguous.unique());
    assert(ambiguous.ambiguous());
    assert(ambiguous.matches.size() == 2U);
    assert(ambiguous.matches[0].id.container_chain == "nbz[10]");
    assert(ambiguous.matches[1].id.container_chain == "nbz[11]");
    assert(ambiguous.matches[0].id != ambiguous.matches[1].id);

    const auto unique = registry.lookup(
        "archive-2",
        "video\\intro.bik",
        ResourcePathPolicy::archive_flags);
    assert(unique.source_available);
    assert(unique.found());
    assert(unique.unique());
    assert(!unique.ambiguous());
    assert(unique.matches.size() == 1U);
    assert(unique.matches[0].id.container_chain == "nbz[12]");

    // Physical normalization 0x0C preserves case. This proves that provider
    // semantics come from the caller and are not hard-coded by ISource.
    const auto physical_exact = registry.lookup(
        "archive-2",
        "Room\\ST001.PAC",
        ResourcePathPolicy::physical_flags);
    assert(physical_exact.unique());
    assert(physical_exact.matches[0].id.container_chain == "nbz[10]");

    const auto physical_wrong_case = registry.lookup(
        "archive-2",
        "room\\ST001.PAC",
        ResourcePathPolicy::physical_flags);
    assert(!physical_wrong_case.found());

    return 0;
}
