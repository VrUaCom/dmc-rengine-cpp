#pragma once

#include "dmc_rengine/gdspaces/source.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
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
    bool directory{false};

    [[nodiscard]] bool valid(std::uint64_t archive_size) const noexcept;
};

// Product-only bounds for hostile or malformed user-supplied archives. These
// limits are not claims about the original DMC3 loader acceptance behavior.
struct NbzZipLimits final {
    std::uint32_t max_central_entries{1U << 20U};
    std::uint64_t max_stored_member_bytes{512ULL * 1024ULL * 1024ULL};
    std::uint64_t max_materialized_member_bytes{512ULL * 1024ULL * 1024ULL};
};

// Keeps recovered-walk observations separate from product hardening. The
// original DMC3 central walk is reconstructed from EOCD position + central
// size; EOCD-declared offset/count remain useful receipt/validation fields.
struct NbzZipIndexReceipt final {
    std::uint64_t archive_size{};
    std::uint64_t eocd_offset{};
    std::uint64_t computed_central_start{};
    std::uint32_t declared_central_offset{};
    std::uint32_t declared_central_size{};
    std::uint16_t declared_entry_count{};
    std::uint32_t walked_entry_count{};
    bool central_offset_matches{};
    bool entry_count_matches{};

    [[nodiscard]] bool valid() const noexcept;
};

class NbzZipSource final : public ISource {
public:
    NbzZipSource(
        std::string source_id,
        std::filesystem::path archive_path,
        NbzZipLimits limits = {});

    [[nodiscard]] std::string_view id() const noexcept override;
    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::vector<ResourceRef> enumerate() const override;
    [[nodiscard]] std::optional<ResourcePayload> read(
        const ResourceId& resource) const override;

    [[nodiscard]] const std::filesystem::path& archive_path() const noexcept;
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] const std::vector<NbzZipEntry>& entries() const noexcept;
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept;
    [[nodiscard]] const std::optional<NbzZipIndexReceipt>& index_receipt() const noexcept;

private:
    void build_index();
    [[nodiscard]] const NbzZipEntry* find_entry(
        const ResourceId& resource) const noexcept;

    std::string source_id_;
    std::filesystem::path archive_path_;
    NbzZipLimits limits_{};
    std::uint64_t archive_size_{};
    std::vector<NbzZipEntry> entries_;
    // Canonical resource key -> index into entries_. Built once during
    // indexing so member reads do not rescan (and re-serialize) the whole
    // central directory. Retail volumes carry thousands of members, which
    // made the previous linear scan quadratic across a full archive walk.
    std::unordered_map<std::string, std::size_t> entry_index_;
    std::vector<Diagnostic> diagnostics_;
    std::optional<NbzZipIndexReceipt> index_receipt_;
};

} // namespace dmc::rengine::gdspaces
