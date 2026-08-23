#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/dds_profile.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_framing.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_packed_reflow_writer.hpp"
#include "dmc_rengine/profiles/dmc3/texture_slot_runtime_materialization.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

void put_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    bytes[offset + 0U] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::byte>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::byte>((value >> 24U) & 0xFFU);
}

void put_u64(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0U; index < sizeof(std::uint64_t); ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> (index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::uint32_t payload_size(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5) {
    std::uint32_t total = 0U;
    while (true) {
        total += std::max(1U, (width + 3U) / 4U) *
            std::max(1U, (height + 3U) / 4U) * (dxt5 ? 16U : 8U);
        if (width == 1U && height == 1U) {
            break;
        }
        width = std::max(1U, width / 2U);
        height = std::max(1U, height / 2U);
    }
    return total;
}

[[nodiscard]] std::vector<std::byte> make_dds(
    std::uint32_t width,
    std::uint32_t height,
    bool dxt5,
    std::uint8_t seed) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    std::vector<std::byte> payload(
        payload_size(width, height, dxt5), std::byte{0});
    for (std::size_t index = 0U; index < payload.size(); ++index) {
        payload[index] = static_cast<std::byte>(
            (static_cast<unsigned>(seed) + index * 29U) & 0xFFU);
    }

    const auto built = dmc3::Dmc3DdsProfile::build(
        width,
        height,
        dxt5 ? dmc3::Dmc3DdsCompression::dxt5
             : dmc3::Dmc3DdsCompression::dxt1,
        std::span<const std::byte>{payload.data(), payload.size()});
    assert(built.ok());
    return built.bytes;
}

[[nodiscard]] std::vector<std::byte> make_descriptor(
    const std::vector<std::byte>& dds,
    bool secondary_half) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto parsed = dmc3::Dmc3DdsProfile::parse(
        std::span<const std::byte>{dds.data(), dds.size()});
    assert(parsed.ok());
    const auto& doc = parsed.document;
    const bool dxt5 = doc.compression == dmc3::Dmc3DdsCompression::dxt5;
    const auto secondary_width = secondary_half ? doc.width / 2U : doc.width;
    const auto secondary_height = secondary_half ? doc.height / 2U : doc.height;

    std::vector<std::byte> descriptor(
        dmc3::TextureSlotFramingParser::k_descriptor_size,
        std::byte{0});
    put_u32(
        descriptor,
        0x08U,
        0x20000U | (doc.mip_map_count << 8U) | (dxt5 ? 0x88U : 0x86U));
    put_u32(descriptor, 0x0CU, 0xAAE4U);
    put_u32(descriptor, 0x10U, (doc.height << 16U) | doc.width);
    put_u32(descriptor, 0x14U, 1U);
    put_u32(descriptor, 0x18U, doc.width * (dxt5 ? 4U : 2U));
    put_u32(descriptor, 0x20U, 0x40U);
    put_u32(descriptor, 0x38U, doc.payload_size);
    put_u32(
        descriptor,
        0x44U,
        (secondary_height << 16U) | secondary_width);
    put_u32(
        descriptor,
        0x48U,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_width)));
    put_u32(
        descriptor,
        0x4CU,
        std::bit_cast<std::uint32_t>(
            1.0F / static_cast<float>(secondary_height)));
    put_u32(descriptor, 0x60U, dxt5 ? 4U : 0U);
    put_u32(descriptor, 0x64U, doc.total_size);
    put_u32(descriptor, 0x68U, 8U);
    return descriptor;
}

struct SourceTexture final {
    std::vector<std::byte> dds;
    bool secondary_half{};
};

[[nodiscard]] std::vector<std::byte> make_bundle(
    const std::vector<SourceTexture>& textures) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    std::vector<std::byte> output(
        dmc3::TextureSlotFramingParser::k_bundle_header_size,
        std::byte{0});
    put_u32(output, 0U, static_cast<std::uint32_t>(textures.size()));

    for (std::size_t index = 0U; index < textures.size(); ++index) {
        auto descriptor = make_descriptor(
            textures[index].dds,
            textures[index].secondary_half);
        const auto record_size = descriptor.size() + textures[index].dds.size();
        const auto sectors = static_cast<std::uint32_t>(
            (record_size + dmc3::TextureSlotFramingParser::k_sector_size - 1U) /
            dmc3::TextureSlotFramingParser::k_sector_size);
        put_u32(output, 4U + index * 4U, sectors);
        output.insert(output.end(), descriptor.begin(), descriptor.end());
        output.insert(output.end(), textures[index].dds.begin(), textures[index].dds.end());
        const auto padded_size =
            static_cast<std::size_t>(sectors) *
            dmc3::TextureSlotFramingParser::k_sector_size;
        output.insert(output.end(), padded_size - record_size, std::byte{0});
    }
    return output;
}

[[nodiscard]] std::string sha256_of(std::span<const std::byte> bytes) {
    return dmc::rengine::core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::size_t entry_span(
    const dmc::rengine::profiles::dmc3::TextureSlotFramingDocument& document,
    std::size_t index,
    std::size_t total_size) {
    const auto begin = static_cast<std::size_t>(
        document.textures[index].descriptor_offset);
    const auto end = index + 1U < document.textures.size()
        ? static_cast<std::size_t>(
              document.textures[index + 1U].descriptor_offset)
        : total_size;
    assert(begin < end);
    return end - begin;
}

void assert_runtime_materialization(
    std::span<const std::byte> bytes,
    const dmc::rengine::profiles::dmc3::TextureSlotFramingDocument& document,
    std::size_t index) {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto& entry = document.textures[index];
    const auto offset = static_cast<std::size_t>(entry.descriptor_offset);
    const auto inspected = dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
        bytes,
        offset,
        entry_span(document, index, bytes.size()));
    assert(inspected.ok());

    const auto& runtime = inspected.materialization;
    assert(runtime.entry_offset == entry.descriptor_offset);
    assert(runtime.source_vtable_placeholder == 0U);
    assert(runtime.cpu_payload_descriptor_relative_delta == 0x40U);
    assert(runtime.cpu_payload_descriptor_offset == entry.descriptor_offset + 0x60U);
    assert(runtime.dds_byte_size == entry.dds_size);
    assert(runtime.dds_relative_delta == 8U);
    assert(runtime.dds_offset == entry.dds_offset);
    assert(runtime.dds_offset == entry.descriptor_offset + 0x70U);
    assert(runtime.width == entry.width);
    assert(runtime.height == entry.height);
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto first_dds = make_dds(256U, 256U, true, 0x21U);
    const auto second_dds = make_dds(128U, 128U, false, 0x42U);
    const auto source = make_bundle({
        SourceTexture{.dds = first_dds, .secondary_half = true},
        SourceTexture{.dds = second_dds, .secondary_half = false},
    });

    const auto source_parsed = dmc3::TextureSlotFramingParser::parse(
        std::span<const std::byte>{source.data(), source.size()});
    assert(source_parsed.ok());
    assert(source_parsed.document.textures.size() == 2U);
    assert_runtime_materialization(
        std::span<const std::byte>{source.data(), source.size()},
        source_parsed.document,
        0U);
    assert_runtime_materialization(
        std::span<const std::byte>{source.data(), source.size()},
        source_parsed.document,
        1U);

    const auto authored_dds = make_dds(512U, 512U, true, 0x63U);
    const auto& source_first = source_parsed.document.textures[0U];
    const auto source_first_dds = std::span<const std::byte>{
        source.data() + static_cast<std::ptrdiff_t>(source_first.dds_offset),
        static_cast<std::size_t>(source_first.dds_size)};
    const dmc3::AuthoredPackedTextureDds authored{
        .texture_index = 0U,
        .expected_source_sha256 = sha256_of(source_first_dds),
        .bytes = authored_dds,
    };
    const std::vector<dmc3::AuthoredPackedTextureDds> authored_set{authored};

    const auto rebuilt = dmc3::TextureSlotPackedReflowWriter::rebuild(
        std::span<const std::byte>{source.data(), source.size()},
        authored_set);
    assert(rebuilt.ok());
    assert(rebuilt.bytes.size() > source.size());

    const auto rebuilt_span = std::span<const std::byte>{
        rebuilt.bytes.data(), rebuilt.bytes.size()};
    const auto output_parsed = dmc3::TextureSlotFramingParser::parse(rebuilt_span);
    assert(output_parsed.ok());
    assert(output_parsed.document.textures.size() == 2U);

    // The first authored record grows and therefore moves the untouched second
    // record. Both output records must still satisfy canonical relative-pointer
    // materialization without authoring any absolute runtime address.
    assert(output_parsed.document.textures[0U].width == 512U);
    assert(output_parsed.document.textures[1U].descriptor_offset !=
        source_parsed.document.textures[1U].descriptor_offset);
    assert_runtime_materialization(rebuilt_span, output_parsed.document, 0U);
    assert_runtime_materialization(rebuilt_span, output_parsed.document, 1U);

    // Negative control: the canonical source-file representation requires the
    // pre-placement vtable qword to be zero.
    auto bad_vtable = rebuilt.bytes;
    const auto first_offset = static_cast<std::size_t>(
        output_parsed.document.textures[0U].descriptor_offset);
    put_u64(bad_vtable, first_offset, 1U);
    const auto bad_vtable_result =
        dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
            std::span<const std::byte>{bad_vtable.data(), bad_vtable.size()},
            first_offset,
            entry_span(output_parsed.document, 0U, bad_vtable.size()));
    assert(!bad_vtable_result.ok());
    assert(bad_vtable_result.status ==
        dmc3::TextureSlotRuntimeMaterializationStatus::source_vtable_not_zero);

    // Negative control: a relocation delta that escapes its record must fail
    // closed before any DDS interpretation.
    auto bad_delta = rebuilt.bytes;
    put_u64(bad_delta, first_offset + 0x20U, 0xFFFFFFFFFFFFFFFFULL);
    const auto bad_delta_result =
        dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
            std::span<const std::byte>{bad_delta.data(), bad_delta.size()},
            first_offset,
            entry_span(output_parsed.document, 0U, bad_delta.size()));
    assert(!bad_delta_result.ok());
    assert(bad_delta_result.status ==
        dmc3::TextureSlotRuntimeMaterializationStatus::descriptor_pointer_out_of_bounds);

    return 0;
}
