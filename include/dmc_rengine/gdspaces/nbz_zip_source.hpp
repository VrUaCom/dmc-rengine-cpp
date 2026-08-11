#pragma once

#include "dmc_rengine/gdspaces/diagnostic.hpp"
#include "dmc_rengine/gdspaces/source.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dmc::rengine::gdspaces {

struct NbzZipEntry final {
    std::uint32_t central_index{};
    std::string logical_path;
    std::uint16_t flags{};
    std::uint16_t compression_method{};
    std::uint32_t crc32{};
    std::uint32_t compressed_size{};
    std::uint32_t uncompressed_size{};
    std::uint32_t local_header_offset{};
    std::uint64_t data_offset{};

    [[nodiscard]] bool directory() const noexcept;
    [[nodiscard]] bool encrypted() const noexcept;
};

class NbzZipSource final : public ISource {
public:
    NbzZipSource(
        std::string source_id,
        std::filesystem::path archive_path,
        std::string profile = "dmc3-hd");

    [[nodiscard]] std::string_view id() const noexcept override;
    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::vector<ResourceRef> enumerate() const override;
    [[nodiscard]] std::optional<ResourcePayload> read(
        const ResourceId& resource) const override;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const std::filesystem::path& archive_path() const noexcept;
    [[nodiscard]] const std::vector<NbzZipEntry>& entries() const noexcept;
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept;

private:
    std::string source_id_;
    std::filesystem::path archive_path_;
    std::string profile_;
    std::uint64_t archive_size_{};
    std::vector<NbzZipEntry> entries_;
    std::vector<Diagnostic> diagnostics_;

    void build_index();
    [[nodiscard]] const NbzZipEntry* find_entry(
        const ResourceId& resource) const noexcept;
};

} // namespace dmc::rengine::gdspaces
