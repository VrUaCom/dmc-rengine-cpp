#include "dmc_rengine/exe/byte_window_receipt.hpp"

#include "dmc_rengine/core/sha256.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <string>

int main() {
    namespace core = dmc::rengine::core;
    namespace exe = dmc::rengine::exe;

    const exe::ExeByteWindowReceipt receipt{
        .artifact_sha256 = std::string(64U, 'a'),
        .artifact_size = 4096U,
        .image_base = 0x140000000ULL,
        .va = 0x140001080ULL,
        .rva = 0x1080U,
        .file_offset = 0x480U,
        .size = 2U,
        .section_name = ".te\"xt",
        .window_sha256 = std::string(64U, 'b'),
    };
    assert(receipt.valid());

    const auto metadata_json = exe::byte_window_receipt_to_json(receipt);
    assert(!metadata_json.empty());
    assert(metadata_json == exe::byte_window_receipt_to_json(receipt));
    assert(metadata_json.find("dmc-rengine.exe-byte-window.v1") != std::string::npos);
    assert(metadata_json.find("\"image_base\": \"0x140000000\"") != std::string::npos);
    assert(metadata_json.find("\"va\": \"0x140001080\"") != std::string::npos);
    assert(metadata_json.find("\"rva\": \"0x1080\"") != std::string::npos);
    assert(metadata_json.find("\"file_offset\": \"0x480\"") != std::string::npos);
    assert(metadata_json.find("\"section\": \".te\\\"xt\"") != std::string::npos);
    assert(metadata_json.find("bytes_hex") == std::string::npos);

    const std::array<std::byte, 2> window_bytes{
        std::byte{0x7F},
        std::byte{0x80},
    };
    auto receipt_with_bytes = receipt;
    receipt_with_bytes.window_sha256 = core::Sha256::compute(
        std::span<const std::byte>{window_bytes}).hex();
    assert(receipt_with_bytes.valid());

    // Raw bytes are accepted only when their canonical lowercase hex payload
    // hashes exactly to the receipt's window SHA-256.
    assert(exe::byte_window_receipt_to_json(receipt, "7f80").empty());
    const auto bytes_json =
        exe::byte_window_receipt_to_json(receipt_with_bytes, "7f80");
    assert(!bytes_json.empty());
    assert(bytes_json.find("\"bytes_hex\": \"7f80\"") != std::string::npos);
    assert(exe::byte_window_receipt_to_json(receipt_with_bytes, "7F80").empty());

    auto bad_sha = receipt;
    bad_sha.artifact_sha256[3U] = 'x';
    assert(!bad_sha.valid());
    assert(exe::byte_window_receipt_to_json(bad_sha).empty());

    auto uppercase_sha = receipt;
    uppercase_sha.artifact_sha256[0U] = 'A';
    assert(!uppercase_sha.valid());

    auto bad_mapping = receipt;
    ++bad_mapping.rva;
    assert(!bad_mapping.valid());

    auto bad_range = receipt;
    bad_range.file_offset = 4095U;
    bad_range.size = 2U;
    assert(!bad_range.valid());

    assert(exe::byte_window_receipt_to_json(receipt_with_bytes, "7f").empty());
    assert(exe::byte_window_receipt_to_json(receipt_with_bytes, "7g80").empty());

    return 0;
}
