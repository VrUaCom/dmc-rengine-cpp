#include "dmc_rengine/formats/mod_writer_corpus.hpp"

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

void put_u8(std::vector<std::byte>& bytes,
            std::size_t offset,
            std::uint8_t value) {
    bytes[offset] = static_cast<std::byte>(value);
}

void put_u16(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint16_t value) {
    put_u8(bytes, offset + 0U,
           static_cast<std::uint8_t>(value & 0xFFU));
    put_u8(bytes, offset + 1U,
           static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

void put_u32(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(
                   (value >> (index * 8U)) & 0xFFU));
    }
}

void put_u64(std::vector<std::byte>& bytes,
             std::size_t offset,
             std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(
                   (value >> (index * 8U)) & 0xFFU));
    }
}

void put_f32(std::vector<std::byte>& bytes,
             std::size_t offset,
             float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

void put_ascii(std::vector<std::byte>& bytes,
               std::size_t offset,
               std::string_view text) {
    for (std::size_t index = 0U; index < text.size(); ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>(text[index]));
    }
}

std::vector<std::byte> make_valid_mod() {
    std::vector<std::byte> bytes(0x260U, std::byte{0});
    put_ascii(bytes, 0x00U, "MOD ");
    put_f32(bytes, 0x04U, 1.01F);
    put_u8(bytes, 0x10U, 1U);
    put_u8(bytes, 0x11U, 1U);
    put_u8(bytes, 0x12U, 8U);
    put_u8(bytes, 0x13U, 0x5AU);
    put_u32(bytes, 0x14U, 0x12345678U);
    put_u64(bytes, 0x20U, 0x200U);

    put_u8(bytes, 0x40U, 1U);
    put_u8(bytes, 0x41U, 0x90U);
    put_u16(bytes, 0x42U, 1U);
    put_u64(bytes, 0x48U, 0x80U);
    put_u32(bytes, 0x50U, 0x00004000U);
    put_f32(bytes, 0x70U, 10.0F);
    put_f32(bytes, 0x74U, -20.0F);
    put_f32(bytes, 0x78U, 30.0F);
    put_f32(bytes, 0x7CU, 42.5F);

    put_u16(bytes, 0x80U, 1U);
    put_u16(bytes, 0x82U, 7U);
    put_u16(bytes, 0x84U, 1U);
    put_u16(bytes, 0x86U, 2U);
    put_u16(bytes, 0x88U, 3U);
    put_u16(bytes, 0x8AU, 4U);
    put_u64(bytes, 0x90U, 0xD0U);
    put_u64(bytes, 0x98U, 0xE0U);
    put_u64(bytes, 0xA0U, 0xF0U);
    put_u64(bytes, 0xA8U, 0x100U);
    put_u64(bytes, 0xB0U, 0x110U);
    put_u64(bytes, 0xB8U, 0U);
    put_u64(bytes, 0xC0U, 0xA0U);
    put_u32(bytes, 0xC8U, 0U);
    put_u32(bytes, 0xCCU, 0U);

    put_f32(bytes, 0xD0U, 1.0F);
    put_f32(bytes, 0xD4U, 2.0F);
    put_f32(bytes, 0xD8U, 3.0F);
    put_f32(bytes, 0xE0U, 0.0F);
    put_f32(bytes, 0xE4U, 1.0F);
    put_f32(bytes, 0xE8U, 0.0F);
    put_u16(bytes, 0xF0U, 4096U);
    put_u16(bytes, 0xF2U, 2048U);
    put_u16(bytes, 0x110U, 0x001FU);

    put_u32(bytes, 0x200U, 0x20U);
    put_u32(bytes, 0x204U, 0x24U);
    put_u32(bytes, 0x208U, 0x28U);
    put_u32(bytes, 0x20CU, 0x30U);
    put_u8(bytes, 0x220U, 0xFFU);
    put_u8(bytes, 0x224U, 0U);
    put_u8(bytes, 0x228U, 0U);
    return bytes;
}

void write_file(const std::filesystem::path& path,
                const std::vector<std::byte>& bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    assert(stream);
    if (!bytes.empty()) {
        stream.write(reinterpret_cast<const char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()));
    }
    stream.flush();
    assert(stream);
}

} // namespace

int main() {
    namespace mod = dmc::rengine::formats::mod;

    std::error_code error;
    const auto root = std::filesystem::temp_directory_path(error) /
        "dmc-rengine-mod-writer-corpus-tests";
    assert(!error);
    std::filesystem::remove_all(root, error);
    error.clear();
    assert(std::filesystem::create_directories(root / "nested", error));
    assert(!error);

    const auto source = make_valid_mod();
    write_file(root / "a.mod", source);
    write_file(root / "nested" / "B.MOD", source);
    write_file(root / "ignore.txt", source);

    {
        const auto receipt = mod::WriterCorpusRunner::run(root);
        assert(receipt.ok());
        assert(receipt.scan_completed);
        assert(receipt.mod_file_count == 2U);
        assert(receipt.passed_file_count == 2U);
        assert(receipt.failed_file_count == 0U);
        assert(receipt.total_bytes ==
               static_cast<std::uint64_t>(source.size() * 2U));
        assert(receipt.entries.size() == 2U);
        assert(receipt.entries[0].relative_path == "a.mod");
        assert(receipt.entries[1].relative_path == "nested/B.MOD");
        for (const auto& entry : receipt.entries) {
            assert(entry.passed());
            assert(entry.status == mod::CorpusEntryStatus::passed);
            assert(entry.source_sha256.size() == 64U);
            assert(entry.source_sha256 == entry.output_sha256);
            assert(entry.modified_byte_count == 0U);
        }
        const auto json = receipt.to_json();
        assert(json.find(
            "dmc-rengine.mod-writer-corpus-receipt.v1") !=
            std::string::npos);
        assert(json.find("\"all_passed\": true") !=
               std::string::npos);
        assert(json.find("nested/B.MOD") != std::string::npos);
    }

    write_file(root / "bad.mod",
               std::vector<std::byte>{
                   std::byte{'M'}, std::byte{'O'},
                   std::byte{'D'}, std::byte{' '}});

    {
        const auto receipt = mod::WriterCorpusRunner::run(root);
        assert(!receipt.ok());
        assert(receipt.scan_completed);
        assert(receipt.mod_file_count == 3U);
        assert(receipt.passed_file_count == 2U);
        assert(receipt.failed_file_count == 1U);

        bool saw_bad = false;
        for (const auto& entry : receipt.entries) {
            if (entry.relative_path == "bad.mod") {
                saw_bad = true;
                assert(entry.status == mod::CorpusEntryStatus::parse_failed);
                assert(!entry.parse_ok);
                assert(!entry.passed());
            }
        }
        assert(saw_bad);
    }

    std::filesystem::remove_all(root, error);
    return 0;
}
