#include "dmc_rengine/gdspaces/nbz_zip_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"
#include "dmc_rengine/profiles/dmc3/nbz_overlay_writer.hpp"
#include "dmc_rengine/profiles/dmc3/runtime_resource_resolver.hpp"
#include "dmc_rengine/profiles/dmc3/volume_bootstrap_policy.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

class EmptyPhysicalSource final : public dmc::rengine::gdspaces::ISource {
public:
    [[nodiscard]] std::string_view id() const noexcept override {
        return "physical";
    }

    [[nodiscard]] std::string_view kind() const noexcept override {
        return "empty-physical-test";
    }

    [[nodiscard]] std::vector<dmc::rengine::gdspaces::ResourceRef>
    enumerate() const override {
        return {};
    }

    [[nodiscard]] std::optional<dmc::rengine::gdspaces::ResourcePayload> read(
        const dmc::rengine::gdspaces::ResourceId&) const override {
        return std::nullopt;
    }
};

[[nodiscard]] std::vector<std::byte> bytes(std::string_view text) {
    std::vector<std::byte> output;
    output.reserve(text.size());
    for (const char value : text) {
        output.push_back(static_cast<std::byte>(value));
    }
    return output;
}

[[nodiscard]] std::filesystem::path temp_nbz(std::string_view label) {
    const auto nonce = static_cast<unsigned long long>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    return std::filesystem::temp_directory_path() /
        (std::string{"dmc-rengine-"} + std::string{label} + "-" +
         std::to_string(nonce) + ".nbz");
}

void write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> data) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(
        reinterpret_cast<const char*>(data.data()),
        static_cast<std::streamsize>(data.size()));
    assert(stream.good());
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    constexpr std::string_view logical_path = "GDataX360.afs/override-test.pac";

    const auto original_bytes = bytes("ORIGINAL-VOLUME-0");
    const auto modified_bytes = bytes("MODIFIED-VOLUME-1");

    // Build a synthetic first numbered archive. With no existing numbered
    // volumes, the product overlay writer emits DMC3-0.nbz. This authoring
    // decision uses discovery only and does not claim runtime mount success.
    const std::array<std::uint32_t, 0> none{};
    const auto empty_bootstrap = dmc3::VolumeBootstrapPolicy::plan(none);
    assert(empty_bootstrap.valid());
    const std::vector<dmc3::NbzOverlayMember> base_members{
        dmc3::NbzOverlayMember{
            .logical_path = std::string{logical_path},
            .bytes = original_bytes,
        },
    };
    const auto base_archive = dmc3::NbzStoreOverlayWriter::build(
        empty_bootstrap, base_members);
    assert(base_archive.ok());
    assert(base_archive.receipt->volume_index == 0U);
    assert(base_archive.receipt->filename == "DMC3-0.nbz");

    // Given discovered contiguous volume 0, author the next volume containing
    // the exact same logical resource identity with modified bytes.
    constexpr std::array<std::uint32_t, 1> present_zero{0U};
    const auto overlay_bootstrap = dmc3::VolumeBootstrapPolicy::plan(present_zero);
    assert(overlay_bootstrap.valid());
    const std::vector<dmc3::NbzOverlayMember> overlay_members{
        dmc3::NbzOverlayMember{
            .logical_path = std::string{logical_path},
            .bytes = modified_bytes,
        },
    };
    const auto overlay_archive = dmc3::NbzStoreOverlayWriter::build(
        overlay_bootstrap, overlay_members);
    assert(overlay_archive.ok());
    assert(overlay_archive.receipt->volume_index == 1U);
    assert(overlay_archive.receipt->filename == "DMC3-1.nbz");

    const auto base_path = temp_nbz("volume-0");
    const auto overlay_path = temp_nbz("volume-1");
    write_file(base_path, base_archive.bytes);
    write_file(overlay_path, overlay_archive.bytes);

    gdspaces::SourceRegistry registry;
    assert(registry.mount(std::make_unique<EmptyPhysicalSource>()));

    auto volume0 = std::make_unique<gdspaces::NbzZipSource>(
        "archive-0", base_path);
    auto volume1 = std::make_unique<gdspaces::NbzZipSource>(
        "archive-1", overlay_path);
    assert(volume0->valid());
    assert(volume1->valid());
    assert(registry.mount(std::move(volume0)));
    assert(registry.mount(std::move(volume1)));

    constexpr std::array<std::uint32_t, 2> present_both{0U, 1U};
    const auto runtime_discovery = dmc3::VolumeBootstrapPolicy::plan(present_both);
    assert(runtime_discovery.valid());
    assert(runtime_discovery.first_missing_index == 2U);
    assert(runtime_discovery.discovered_archives.size() == 2U);

    // This synthetic integration fixture has directly constructed two valid NBZ
    // sources and mounts both into the product registry, so the test explicitly
    // records those two registration outcomes as success. Runtime precedence is
    // then derived from the recovered prepend rule: 1 -> 0. Filename discovery
    // alone is intentionally insufficient to construct this topology.
    constexpr std::array<std::uint32_t, 2> successful_archives{0U, 1U};
    const auto runtime_topology =
        dmc3::VolumeBootstrapPolicy::successful_mount_topology(
            runtime_discovery,
            true,
            successful_archives);
    assert(runtime_topology.has_value());
    assert(runtime_topology->valid_for(runtime_discovery));
    assert(runtime_topology->mounted_archives.size() == 2U);
    assert(runtime_topology->archive_resolution_order.size() == 2U);
    assert(runtime_topology->archive_resolution_order[0U] == 1U);
    assert(runtime_topology->archive_resolution_order[1U] == 0U);

    const dmc3::RuntimeSourceBindings bindings{
        .physical_source_id = "physical",
        .archives = {
            dmc3::ArchiveSourceBinding{1U, "archive-1"},
            dmc3::ArchiveSourceBinding{0U, "archive-0"},
        },
    };
    assert(bindings.valid_for(*runtime_topology));

    // A directory-qualified game request is reduced to its basename before the
    // six recovered namespace prefixes are tried. The winning first archive
    // candidate is therefore GDataX360.afs/override-test.pac, not a path that
    // preserves the request's synthetic obj/ directory.
    const auto resolved = dmc3::RuntimeResourceResolver::resolve(
        "obj\\override-test.pac", *runtime_topology, bindings, registry);
    assert(resolved.ok());
    assert(resolved.resolved.has_value());
    assert(resolved.resolved->id.source_id == "archive-1");
    assert(resolved.resolved->id.logical_path == logical_path);
    assert(resolved.probes.size() == 1U);
    assert(resolved.probes[0U].candidate == logical_path);
    assert(resolved.probes[0U].archive_volume_index.has_value());
    assert(*resolved.probes[0U].archive_volume_index == 1U);

    const auto materialized = registry.read(resolved.resolved->id);
    assert(materialized.has_value());
    assert(materialized->readable());
    assert(materialized->bytes == modified_bytes);
    assert(materialized->bytes != original_bytes);
    assert(materialized->byte_provenance.has_value());
    assert(
        materialized->byte_provenance->transform ==
        gdspaces::ByteTransform::zip_stored);

    std::error_code error;
    std::filesystem::remove(base_path, error);
    error.clear();
    std::filesystem::remove(overlay_path, error);

    return 0;
}
