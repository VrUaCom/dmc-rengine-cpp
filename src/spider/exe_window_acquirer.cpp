#include "dmc_rengine/spider/exe_window_acquirer.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/byte_window.hpp"
#include "dmc_rengine/exe/pe_reader.hpp"
#include "dmc_rengine/gdspaces/local_directory_source.hpp"
#include "dmc_rengine/gdspaces/source_registry.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmc::rengine::spider {
namespace {

[[nodiscard]] std::optional<std::string> normalize_sha256(
    std::string_view text) {
    if (text.size() != 64U) {
        return std::nullopt;
    }

    std::string normalized;
    normalized.reserve(64U);
    for (const auto character : text) {
        const auto value = static_cast<unsigned char>(character);
        if (std::isxdigit(value) == 0) {
            return std::nullopt;
        }
        normalized.push_back(static_cast<char>(std::tolower(value)));
    }
    return normalized;
}

[[nodiscard]] std::optional<std::vector<std::byte>> load_through_gdspaces(
    const std::filesystem::path& path) {
    std::error_code error;
    const auto absolute = std::filesystem::absolute(path, error);
    if (error || !std::filesystem::is_regular_file(absolute, error) || error) {
        return std::nullopt;
    }

    const auto raw_size = std::filesystem::file_size(absolute, error);
    if (error ||
        raw_size > static_cast<std::uintmax_t>(
            std::numeric_limits<std::uint64_t>::max())) {
        return std::nullopt;
    }

    constexpr std::string_view source_id = "spider-exe-window-source";
    gdspaces::SourceRegistry registry;
    if (!registry.mount(std::make_unique<gdspaces::LocalDirectorySource>(
            std::string{source_id}, absolute.parent_path(), false))) {
        return std::nullopt;
    }

    const gdspaces::ResourceId resource{
        .source_id = std::string{source_id},
        .logical_path = absolute.filename().generic_string(),
        .container_chain = {},
        .offset = 0U,
        .size = static_cast<std::uint64_t>(raw_size),
    };
    auto payload = registry.read(resource);
    if (!payload || !payload->readable()) {
        return std::nullopt;
    }
    return std::move(payload->bytes);
}

} // namespace

NativeExeWindowSource::NativeExeWindowSource(
    std::vector<std::byte> bytes,
    exe::PeImage image,
    std::string artifact_sha256)
    : bytes_(std::move(bytes)),
      image_(std::move(image)),
      artifact_sha256_(std::move(artifact_sha256)) {}

std::unique_ptr<NativeExeWindowSource> NativeExeWindowSource::open(
    const std::filesystem::path& path,
    std::string_view expected_sha256,
    std::string& error) {
    error.clear();
    const auto expected = normalize_sha256(expected_sha256);
    if (!expected) {
        error = "expected SHA-256 must be exactly 64 hexadecimal characters";
        return nullptr;
    }

    auto bytes = load_through_gdspaces(path);
    if (!bytes) {
        error = "could not read executable through GDSpaces";
        return nullptr;
    }

    const auto file_bytes = std::span<const std::byte>{*bytes};
    const auto actual_sha = core::Sha256::compute(file_bytes).hex();
    if (actual_sha != *expected) {
        error = "executable SHA-256 mismatch";
        return nullptr;
    }

    auto pe = exe::PeReader::read(file_bytes);
    if (!pe.ok()) {
        error = "PE inspection failed";
        return nullptr;
    }

    return std::unique_ptr<NativeExeWindowSource>(new NativeExeWindowSource(
        std::move(*bytes), std::move(*pe.image), actual_sha));
}

ExeWindowAcquisition NativeExeWindowSource::acquire(
    const ExeWindowRequest& request) const {
    const auto extracted = exe::ExeByteWindowExtractor::extract(
        std::span<const std::byte>{bytes_}, image_, request.va, request.size);
    if (!extracted.ok()) {
        return {
            .receipt = std::nullopt,
            .error = std::string{"executable window extraction failed ["} +
                exe::to_string(extracted.error) + "]: " + extracted.message,
        };
    }

    const auto& window = *extracted.window;
    const auto window_sha = core::Sha256::compute(
        std::span<const std::byte>{window.bytes}).hex();
    exe::ExeByteWindowReceipt receipt{
        .artifact_sha256 = artifact_sha256_,
        .artifact_size = static_cast<std::uint64_t>(bytes_.size()),
        .image_base = image_.image_base,
        .va = window.va,
        .rva = window.rva,
        .file_offset = window.file_offset,
        .size = static_cast<std::uint64_t>(window.bytes.size()),
        .section_name = window.section_name,
        .window_sha256 = window_sha,
    };
    if (!receipt.valid()) {
        return {
            .receipt = std::nullopt,
            .error = "canonical byte-window extraction produced an invalid receipt",
        };
    }
    return {.receipt = std::move(receipt), .error = {}};
}

ExeWindowAcquisition acquire_native_exe_window(
    void* context,
    const ExeWindowRequest& request) {
    if (context == nullptr) {
        return {.receipt = std::nullopt, .error = "native EXE source is not bound"};
    }
    return static_cast<NativeExeWindowSource*>(context)->acquire(request);
}

} // namespace dmc::rengine::spider
