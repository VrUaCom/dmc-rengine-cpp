#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/resource_payload.hpp"
#include "dmc_rengine/gdspaces/source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/runtime/resource_bridge.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using dmc::rengine::gdspaces::Diagnostic;
using dmc::rengine::gdspaces::DiagnosticSeverity;
using dmc::rengine::gdspaces::ISource;
using dmc::rengine::gdspaces::ResourceId;
using dmc::rengine::gdspaces::ResourcePayload;
using dmc::rengine::gdspaces::ResourceRef;
using dmc::rengine::gdspaces::SourceRegistry;
using dmc::rengine::runtime::ResourceBridge;
using dmc::rengine::runtime::RuntimeErrorCode;

/// Synthetic source. It models only what the bridge contract needs: a name to
/// resolve, bytes to hand back, an optional error diagnostic, and a read
/// counter so cache behavior is observable. No DMC3 layout is implied.
class ScriptedSource final : public ISource {
public:
    struct Entry final {
        std::vector<std::byte> bytes;
        bool poisoned{false};
    };

    explicit ScriptedSource(std::string id) : id_(std::move(id)) {}

    void add(const std::string& logical_path, std::size_t byte_count, bool poisoned = false) {
        Entry entry;
        entry.bytes.assign(byte_count, std::byte{0x5A});
        entry.poisoned = poisoned;
        entries_.emplace(logical_path, std::move(entry));
    }

    [[nodiscard]] std::string_view id() const noexcept override { return id_; }
    [[nodiscard]] std::string_view kind() const noexcept override { return "scripted"; }

    [[nodiscard]] std::vector<ResourceRef> enumerate() const override {
        std::vector<ResourceRef> refs;
        for (const auto& [path, entry] : entries_) {
            refs.push_back(make_ref(path, entry.bytes.size()));
        }
        return refs;
    }

    [[nodiscard]] std::optional<ResourcePayload> read(const ResourceId& resource) const override {
        const auto entry = entries_.find(resource.logical_path);
        if (entry == entries_.end()) {
            return std::nullopt;
        }

        ++reads_;

        ResourcePayload payload;
        payload.resource = make_ref(resource.logical_path, entry->second.bytes.size());
        payload.bytes = entry->second.bytes;
        if (entry->second.poisoned) {
            payload.diagnostics.push_back(
                Diagnostic{DiagnosticSeverity::error, "scripted.unreadable",
                           "synthetic read failure", resource});
        }
        return payload;
    }

    [[nodiscard]] std::size_t reads() const noexcept { return reads_; }

    [[nodiscard]] ResourceId id_for(const std::string& logical_path) const {
        const auto entry = entries_.find(logical_path);
        const std::size_t size = entry == entries_.end() ? 0U : entry->second.bytes.size();
        return ResourceId{id_, logical_path, {}, 0U, static_cast<std::uint64_t>(size)};
    }

private:
    [[nodiscard]] ResourceRef make_ref(const std::string& logical_path,
                                       std::size_t size) const {
        ResourceRef ref;
        ref.id = ResourceId{id_, logical_path, {}, 0U, static_cast<std::uint64_t>(size)};
        ref.display_name = logical_path;
        ref.format = "synthetic";
        return ref;
    }

    std::string id_;
    std::map<std::string, Entry> entries_;
    mutable std::size_t reads_{};
};

struct Fixture final {
    SourceRegistry registry;
    ScriptedSource* source{};

    Fixture() {
        auto owned = std::make_unique<ScriptedSource>("scripted");
        source = owned.get();
        source->add("stage/model.mod", 64U);
        source->add("stage/broken.mod", 32U, true);
        const bool mounted = registry.mount(std::move(owned));
        assert(mounted);
    }
};

void reads_flow_through_gdspaces_and_are_cached() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};

    const auto id = fixture.source->id_for("stage/model.mod");
    const auto first = bridge.bytes(id);
    assert(first.has_value());
    assert(first->size() == 64U);
    assert(fixture.source->reads() == 1U);

    const auto second = bridge.bytes(id);
    assert(second.has_value());
    assert(second->size() == 64U);

    // The second read must be served from residency, not re-fetched.
    assert(fixture.source->reads() == 1U);
    assert(bridge.stats().hits == 1U);
    assert(bridge.stats().misses == 1U);
    assert(bridge.resident(id));
    assert(bridge.resident_resources() == 1U);
    assert(bridge.resident_bytes() == 64U);
}

void missing_resources_are_reported_as_missing() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};

    const ResourceId absent{"scripted", "stage/nothing.mod", {}, 0U, 0U};
    const auto result = bridge.bytes(absent);
    assert(!result.has_value());
    assert(result.error().code == RuntimeErrorCode::resource_missing);
    assert(bridge.stats().failures == 1U);
    assert(bridge.resident_resources() == 0U);
}

void error_diagnostics_make_a_payload_unreadable() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};

    const auto id = fixture.source->id_for("stage/broken.mod");
    const auto result = bridge.bytes(id);
    assert(!result.has_value());
    assert(result.error().code == RuntimeErrorCode::resource_unreadable);
    assert(result.error().message.find("scripted.unreadable") != std::string::npos);

    // A rejected payload must not become resident.
    assert(!bridge.resident(id));
    assert(bridge.resident_bytes() == 0U);
}

void invalid_identities_are_refused_without_touching_the_registry() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};

    const auto result = bridge.bytes(ResourceId{});
    assert(!result.has_value());
    assert(result.error().code == RuntimeErrorCode::invalid_argument);
    assert(fixture.source->reads() == 0U);
}

void eviction_releases_residency_and_forces_a_refetch() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};

    const auto id = fixture.source->id_for("stage/model.mod");
    assert(bridge.bytes(id).has_value());
    assert(bridge.resident_bytes() == 64U);

    assert(bridge.evict(id));
    assert(!bridge.evict(id));
    assert(bridge.resident_bytes() == 0U);
    assert(bridge.resident_resources() == 0U);

    assert(bridge.bytes(id).has_value());
    assert(fixture.source->reads() == 2U);

    bridge.clear();
    assert(bridge.resident_resources() == 0U);
    assert(bridge.resident_bytes() == 0U);
}

void payload_access_exposes_gdspaces_metadata() {
    Fixture fixture;
    ResourceBridge bridge{fixture.registry};

    const auto id = fixture.source->id_for("stage/model.mod");
    const auto payload = bridge.payload(id);
    assert(payload.has_value());
    assert((*payload)->resource.display_name == "stage/model.mod");
    assert((*payload)->readable());
}

} // namespace

int main() {
    reads_flow_through_gdspaces_and_are_cached();
    missing_resources_are_reported_as_missing();
    error_diagnostics_make_a_payload_unreadable();
    invalid_identities_are_refused_without_touching_the_registry();
    eviction_releases_residency_and_forces_a_refetch();
    payload_access_exposes_gdspaces_metadata();
    return 0;
}
