#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

enum class RetailAcquisitionReceiptStatus : std::uint8_t {
    verified,
    invalid_json,
    invalid_contract,
    request_mismatch,
    output_mismatch,
    member_size_mismatch,
    member_hash_mismatch,
};

[[nodiscard]] constexpr std::string_view to_string(
    RetailAcquisitionReceiptStatus status) noexcept {
    switch (status) {
    case RetailAcquisitionReceiptStatus::verified: return "verified";
    case RetailAcquisitionReceiptStatus::invalid_json: return "invalid-json";
    case RetailAcquisitionReceiptStatus::invalid_contract: return "invalid-contract";
    case RetailAcquisitionReceiptStatus::request_mismatch: return "request-mismatch";
    case RetailAcquisitionReceiptStatus::output_mismatch: return "output-mismatch";
    case RetailAcquisitionReceiptStatus::member_size_mismatch:
        return "member-size-mismatch";
    case RetailAcquisitionReceiptStatus::member_hash_mismatch:
        return "member-hash-mismatch";
    }
    return "invalid-contract";
}

struct RetailAcquisitionReceiptExpectation final {
    std::string_view game_request;
    std::string_view output_file;
    std::span<const std::byte> member_bytes;
};

struct RetailAcquisitionReceiptResult final {
    RetailAcquisitionReceiptStatus status{
        RetailAcquisitionReceiptStatus::invalid_contract};
    std::string detail;

    [[nodiscard]] bool ok() const noexcept {
        return status == RetailAcquisitionReceiptStatus::verified;
    }
};

class RetailAcquisitionReceiptVerifier final {
public:
    // Verifies that the acquisition sidecar is not merely present, but
    // semantically describes the exact request, output path and materialized
    // member bytes consumed by the L1 closure.
    [[nodiscard]] static RetailAcquisitionReceiptResult verify(
        std::span<const std::byte> receipt_bytes,
        RetailAcquisitionReceiptExpectation expected);
};

} // namespace dmc::rengine::profiles::dmc3
