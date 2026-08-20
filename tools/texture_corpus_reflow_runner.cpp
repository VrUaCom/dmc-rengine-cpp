#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) {
        return {};
    }
    const auto end = stream.tellg();
    if (end <= 0) {
        return {};
    }
    const auto size = static_cast<std::size_t>(end);
    std::vector<std::byte> bytes(size);
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
    if (!stream) {
        return {};
    }
    return bytes;
}

[[nodiscard]] bool write_file(
    const std::filesystem::path& path,
    std::span<const std::byte> bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        return false;
    }
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    return stream.good();
}

[[nodiscard]] bool parse_u32(std::string_view text, std::uint32_t& output) noexcept {
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto result = std::from_chars(begin, end, output, 10);
    return result.ec == std::errc{} && result.ptr == end;
}

[[nodiscard]] std::uint32_t payload_size(
    std::uint32_t width,
    std::uint32_t height,
    dmc::rengine::profiles::dmc3::Dmc3DdsCompression compression) noexcept {
    std::uint64_t total = 0U;
    const std::uint32_t block_bytes =
        compression == dmc::rengine::profiles::dmc3::Dmc3DdsCompression::dxt1
        ? 8U
        : 16U;
    while (true) {
        total += static_cast<std::uint64_t>(std::max(1U, (width + 3U) / 4U)) *
            std::max(1U, (height + 3U) / 4U) * block_bytes;
        if (width == 1U && height == 1U) {
            break;
        }
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return static_cast<std::uint32_t>(total);
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

} // namespace

int main(int argc, char** argv) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    if (argc != 6) {
        std::cerr << "usage: dmc-rengine-texture-corpus-reflow <source-slot> <texture-index> <target-width> <target-height> <output-slot>\n";
        return 2;
    }

    std::uint32_t texture_index = 0U;
    std::uint32_t target_width = 0U;
    std::uint32_t target_height = 0U;
    if (!parse_u32(argv[2], texture_index) ||
        !parse_u32(argv[3], target_width) ||
        !parse_u32(argv[4], target_height)) {
        std::cerr << "invalid numeric argument\n";
        return 2;
    }

    const auto source = read_file(argv[1]);
    if (source.empty()) {
        std::cerr << "source read failed\n";
        return 3;
    }

    const auto framing = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{source.data(), source.size()});
    if (!framing.ok() || texture_index >= framing.document.textures.size()) {
        std::cerr << "source framing failed: " << to_string(framing.status) << "\n";
        return 4;
    }

    const auto& entry = framing.document.textures[texture_index];
    const auto compression = entry.compression == dmc3::TextureCompressionKind::dxt1
        ? dmc3::Dmc3DdsCompression::dxt1
        : dmc3::Dmc3DdsCompression::dxt5;
    const auto authored_payload_size = payload_size(
        target_width, target_height, compression);
    std::vector<std::byte> payload(authored_payload_size, std::byte{0});
    for (std::size_t index = 0U; index < payload.size(); ++index) {
        const auto value = static_cast<unsigned>(texture_index) * 17U +
            static_cast<unsigned>(target_width & 0xFFU) +
            static_cast<unsigned>(target_height & 0xFFU) +
            static_cast<unsigned>(index * 29U);
        payload[index] = static_cast<std::byte>(value & 0xFFU);
    }

    const auto authored_dds = dmc3::Dmc3DdsProfile::build(
        target_width,
        target_height,
        compression,
        std::span<const std::byte>{payload.data(), payload.size()});
    if (!authored_dds.ok()) {
        std::cerr << "authored DDS build failed: "
                  << to_string(authored_dds.status) << "\n";
        return 5;
    }

    const auto source_dds = std::span<const std::byte>{
        source.data() + static_cast<std::ptrdiff_t>(entry.dds_offset),
        static_cast<std::size_t>(entry.dds_size)};
    const std::vector<dmc3::AuthoredPackedTextureDds> edits{
        dmc3::AuthoredPackedTextureDds{
            .texture_index = texture_index,
            .expected_source_sha256 = sha256_of(source_dds),
            .bytes = authored_dds.bytes,
        },
    };

    const auto rebuilt = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{source.data(), source.size()}, edits);
    if (!rebuilt.ok()) {
        std::cerr << "texture reflow failed: " << to_string(rebuilt.status)
                  << " detail=" << rebuilt.detail << "\n";
        return 6;
    }

    if (!write_file(
            argv[5],
            std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()})) {
        std::cerr << "output write failed\n";
        return 7;
    }

    const auto output_framing = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{rebuilt.bytes.data(), rebuilt.bytes.size()});
    if (!output_framing.ok() ||
        texture_index >= output_framing.document.textures.size()) {
        std::cerr << "output reparse failed\n";
        return 8;
    }

    const auto& output_entry = output_framing.document.textures[texture_index];
    std::cout
        << "{\"status\":\"ok\","
        << "\"texture_index\":" << texture_index << ','
        << "\"source_slot_size\":" << source.size() << ','
        << "\"output_slot_size\":" << rebuilt.bytes.size() << ','
        << "\"source_width\":" << entry.width << ','
        << "\"source_height\":" << entry.height << ','
        << "\"output_width\":" << output_entry.width << ','
        << "\"output_height\":" << output_entry.height << ','
        << "\"source_sha256\":\""
        << sha256_of(std::span<const std::byte>{source.data(), source.size()})
        << "\",\"output_sha256\":\""
        << sha256_of(std::span<const std::byte>{
               rebuilt.bytes.data(), rebuilt.bytes.size()})
        << "\"}\n";
    return 0;
}
