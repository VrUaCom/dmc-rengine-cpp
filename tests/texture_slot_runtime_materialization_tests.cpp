#include "dmc_rengine/profiles/dmc3/texture_slot_runtime_materialization.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t i = 0; i < 4U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (i * 8U)) & 0xFFU);
    }
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t i = 0; i < 8U; ++i) {
        bytes[offset + i] = static_cast<std::byte>((value >> (i * 8U)) & 0xFFU);
    }
}

std::vector<std::byte> make_entry() {
    std::vector<std::byte> bytes(0xF0U, std::byte{0});
    put_u16(bytes, 0x10U, 256U);
    put_u16(bytes, 0x12U, 128U);

    // Canonical original-runtime relative fixups:
    // address(entry+0x20) + 0x40 = entry+0x60 descriptor
    // address(entry+0x68) + 0x08 = entry+0x70 DDS
    put_u64(bytes, 0x20U, 0x40U);
    put_u32(bytes, 0x64U, 0x80U);
    put_u64(bytes, 0x68U, 0x08U);

    bytes[0x70U] = std::byte{'D'};
    bytes[0x71U] = std::byte{'D'};
    bytes[0x72U] = std::byte{'S'};
    bytes[0x73U] = std::byte{' '};
    put_u32(bytes, 0x74U, 0x7CU);
    put_u32(bytes, 0xBCU, 0x20U); // DDS + 0x4C
    return bytes;
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;

    const auto source = make_entry();
    const auto ok = dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
        source, 0U, source.size());
    assert(ok.ok());
    assert(ok.materialization.cpu_payload_descriptor_offset == 0x60U);
    assert(ok.materialization.dds_offset == 0x70U);
    assert(ok.materialization.dds_byte_size == 0x80U);
    assert(ok.materialization.width == 256U);
    assert(ok.materialization.height == 128U);

    auto bad_vtable = source;
    put_u64(bad_vtable, 0x00U, 1U);
    assert(dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
        bad_vtable, 0U, bad_vtable.size()).status ==
        dmc3::TextureSlotRuntimeMaterializationStatus::source_vtable_not_zero);

    auto bad_descriptor = source;
    put_u64(bad_descriptor, 0x20U, 0x1000U);
    assert(dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
        bad_descriptor, 0U, bad_descriptor.size()).status ==
        dmc3::TextureSlotRuntimeMaterializationStatus::descriptor_pointer_out_of_bounds);

    auto bad_dds = source;
    bad_dds[0x70U] = std::byte{'X'};
    assert(dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
        bad_dds, 0U, bad_dds.size()).status ==
        dmc3::TextureSlotRuntimeMaterializationStatus::dds_magic_mismatch);

    auto shifted = std::vector<std::byte>(0x20U, std::byte{0});
    shifted.insert(shifted.end(), source.begin(), source.end());
    const auto shifted_result = dmc3::TextureSlotRuntimeMaterializationInspector::inspect(
        shifted, 0x20U, source.size());
    assert(shifted_result.ok());
    assert(shifted_result.materialization.cpu_payload_descriptor_offset == 0x80U);
    assert(shifted_result.materialization.dds_offset == 0x90U);

    return 0;
}
