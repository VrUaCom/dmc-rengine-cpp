#pragma once

#include "dmc_rengine/gdspaces/direct_path_source.hpp"
#include "dmc_rengine/gdspaces/source.hpp"

#include <filesystem>
#include <string>

namespace dmc::rengine::gdspaces {

class LocalDirectorySource final : public ISource, public IDirectPathSource {
public:
    LocalDirectorySource(
        std::string source_id,
        std::filesystem::path root,
        bool recursive = true);

    [[nodiscard]] std::string_view id() const noexcept override;
    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::vector<ResourceRef> enumerate() const override;
    [[nodiscard]] std::optional<ResourcePayload> read(
        const ResourceId& resource) const override;
    [[nodiscard]] DirectPathLookupResult lookup_direct_path(
        std::string_view logical_path) const override;

    [[nodiscard]] const std::filesystem::path& root() const noexcept;
    [[nodiscard]] bool recursive() const noexcept;

private:
    [[nodiscard]] ResourceRef describe(
        const std::filesystem::path& path,
        std::uint64_t size) const;
    [[nodiscard]] bool contains(const std::filesystem::path& path) const;

    std::string source_id_;
    std::filesystem::path root_;
    bool recursive_{true};
};

} // namespace dmc::rengine::gdspaces
