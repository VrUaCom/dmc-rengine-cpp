#include "dmc_rengine/exe/byte_window_receipt.hpp"

#include <cassert>
#include <string>

int main() {
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

    const auto bytes_json = exe::byte_window_receipt_to_json(receipt, "7f80");
    assert(!bytes_json.empty());
    assert(bytes_json.find("\"bytes_hex\": \"7f80\"") != std::string::npos);

    auto bad_sha = receipt;
    bad_sha.artifact_sha256[3U] = 'x';
    assert(!bad_sha.valid());
    assert(exe::byte_window_receipt_to_json(bad_sha).empty());

    auto bad_mapping = receipt;
    ++bad_mapping.rva;
    assert(!bad_mapping.valid());

    auto bad_range = receipt;
    bad_range.file_offset = 4095U;
    bad_range.size = 2U;
    assert(!bad_range.valid());

    assert(exe::byte_window_receipt_to_json(receipt, "7f").empty());
    assert(exe::byte_window_receipt_to_json(receipt, "7g80").empty());

    return 0;
}
