#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/exe/process_memory_window_receipt.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <string>

int main() {
    using dmc::rengine::exe::ProcessMemoryWindowReceipt;
    using dmc::rengine::exe::ProcessMemoryWindowReceiptV2;
    using dmc::rengine::exe::process_memory_window_receipt_to_json;
    using dmc::rengine::exe::process_memory_window_receipt_v2_to_json;

    const std::array<std::byte, 2> mz{
        std::byte{0x4d}, std::byte{0x5a}};
    const auto window_sha = dmc::rengine::core::Sha256::compute(
        std::span<const std::byte>{mz}).hex();
    const std::string artifact_sha(64U, 'a');
    const std::string canonical_artifact_sha(64U, 'b');

    ProcessMemoryWindowReceipt receipt{
        .artifact_sha256 = artifact_sha,
        .artifact_size = 6567320U,
        .image_path = "C:/Games/DMC3/dmc3.exe",
        .preferred_image_base = 0x140000000ULL,
        .pid = 1234U,
        .module_base = 0x7FF600000000ULL,
        .rva = 0x1000U,
        .runtime_va = 0x7FF600001000ULL,
        .size = 2U,
        .section_name = ".text",
        .window_sha256 = window_sha,
        .expected_window_artifact_sha256 = canonical_artifact_sha,
        .expected_window_sha256 = window_sha,
    };

    assert(receipt.valid());
    assert(receipt.has_mapping_expectation());
    assert(receipt.matches_expected_window());

    const auto json = process_memory_window_receipt_to_json(receipt, "4d5a");
    assert(!json.empty());
    assert(json.find("dmc-rengine.exe-process-window.v1") != std::string::npos);
    assert(json.find("process_creation_filetime") == std::string::npos);
    assert(json.find("\"matches_expected_window\": true") != std::string::npos);
    assert(json.find(canonical_artifact_sha) != std::string::npos);

    auto mismatch = receipt;
    mismatch.expected_window_sha256 = std::string(64U, '0');
    assert(mismatch.valid());
    assert(!mismatch.matches_expected_window());
    const auto mismatch_json = process_memory_window_receipt_to_json(mismatch);
    assert(mismatch_json.find("\"matches_expected_window\": false") !=
           std::string::npos);

    auto half_bound = receipt;
    half_bound.expected_window_sha256.reset();
    assert(!half_bound.valid());
    assert(process_memory_window_receipt_to_json(half_bound).empty());

    auto bad_runtime_va = receipt;
    ++bad_runtime_va.runtime_va;
    assert(!bad_runtime_va.valid());

    assert(process_memory_window_receipt_to_json(receipt, "4d00").empty());

    ProcessMemoryWindowReceiptV2 receipt_v2{
        .artifact_sha256 = artifact_sha,
        .artifact_size = 6567320U,
        .image_path = "C:/Games/DMC3/dmc3.exe",
        .preferred_image_base = 0x140000000ULL,
        .pid = 1234U,
        .process_creation_filetime = 133800000000000000ULL,
        .module_base = 0x7FF600000000ULL,
        .rva = 0x1000U,
        .runtime_va = 0x7FF600001000ULL,
        .size = 2U,
        .section_name = ".text",
        .window_sha256 = window_sha,
        .expected_window_artifact_sha256 = canonical_artifact_sha,
        .expected_window_sha256 = window_sha,
    };
    assert(receipt_v2.valid());
    assert(receipt_v2.has_mapping_expectation());
    assert(receipt_v2.matches_expected_window());

    const auto json_v2 = process_memory_window_receipt_v2_to_json(
        receipt_v2, "4d5a");
    assert(!json_v2.empty());
    assert(json_v2.find("dmc-rengine.exe-process-window.v2") != std::string::npos);
    assert(json_v2.find("\"process_creation_filetime\": 133800000000000000") !=
           std::string::npos);
    assert(json_v2.find("\"matches_expected_window\": true") !=
           std::string::npos);

    auto no_instance = receipt_v2;
    no_instance.process_creation_filetime = 0U;
    assert(!no_instance.valid());
    assert(process_memory_window_receipt_v2_to_json(no_instance).empty());

    auto v2_bad_runtime_va = receipt_v2;
    ++v2_bad_runtime_va.runtime_va;
    assert(!v2_bad_runtime_va.valid());

    assert(process_memory_window_receipt_v2_to_json(receipt_v2, "4d00").empty());

    return 0;
}
