#include "dmc_rengine/profiles/dmc3/retail_acquisition_receipt.hpp"

#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"

#include <algorithm>

namespace dmc::rengine::profiles::dmc3 {
namespace {

using Object = core::json::Value::Object;

[[nodiscard]] const core::json::Value* field(
    const Object& object,
    std::string_view name) noexcept {
    const auto iterator = object.find(name);
    return iterator == object.end() ? nullptr : &iterator->second;
}

[[nodiscard]] const Object* object_field(
    const Object& object,
    std::string_view name) noexcept {
    const auto* value = field(object, name);
    return value == nullptr ? nullptr : value->as_object();
}

[[nodiscard]] const std::string* string_field(
    const Object& object,
    std::string_view name) noexcept {
    const auto* value = field(object, name);
    return value == nullptr ? nullptr : value->as_string();
}

[[nodiscard]] const std::uint64_t* u64_field(
    const Object& object,
    std::string_view name) noexcept {
    const auto* value = field(object, name);
    return value == nullptr ? nullptr : value->as_u64();
}

[[nodiscard]] bool valid_sha256(std::string_view text) noexcept {
    return text.size() == 64U &&
        std::all_of(text.begin(), text.end(), [](char value) {
            return (value >= '0' && value <= '9') ||
                (value >= 'a' && value <= 'f');
        });
}

[[nodiscard]] RetailAcquisitionReceiptResult invalid_contract(
    std::string detail) {
    return {
        .status = RetailAcquisitionReceiptStatus::invalid_contract,
        .detail = std::move(detail),
    };
}

} // namespace

RetailAcquisitionReceiptResult RetailAcquisitionReceiptVerifier::verify(
    std::span<const std::byte> receipt_bytes,
    RetailAcquisitionReceiptExpectation expected) {
    const auto text = std::string_view{
        reinterpret_cast<const char*>(receipt_bytes.data()),
        receipt_bytes.size()};
    core::json::ParseLimits limits;
    limits.max_input_bytes = 1024U * 1024U;
    limits.max_depth = 16U;
    limits.max_values = 256U;
    limits.max_array_items = 32U;
    limits.max_object_members = 64U;
    limits.max_string_bytes = 64U * 1024U;
    const auto parsed = core::json::Parser::parse(text, limits);
    if (!parsed.ok()) {
        return {
            .status = RetailAcquisitionReceiptStatus::invalid_json,
            .detail = parsed.errors.empty()
                ? "acquisition receipt is not valid JSON"
                : parsed.errors.front().message,
        };
    }

    const auto* root = parsed.value->as_object();
    if (root == nullptr) {
        return invalid_contract("acquisition receipt root is not an object");
    }
    const auto* schema = u64_field(*root, "schema_version");
    const auto* evidence_class = string_field(*root, "evidence_class");
    const auto* resolver_status = string_field(*root, "resolver_status");
    const auto* publication = string_field(*root, "publication");
    const auto* selected_volume = u64_field(*root, "selected_volume_index");
    if (schema == nullptr || *schema != 2U ||
        evidence_class == nullptr ||
        *evidence_class != "artifact-bound-retail-member-acquisition" ||
        resolver_status == nullptr || *resolver_status != "resolved" ||
        publication == nullptr ||
        *publication != "atomic-no-replace-per-artifact" ||
        selected_volume == nullptr) {
        return invalid_contract(
            "acquisition receipt identity or publication contract is incomplete");
    }

    const auto* request = string_field(*root, "game_request");
    if (request == nullptr || *request != expected.game_request) {
        return {
            .status = RetailAcquisitionReceiptStatus::request_mismatch,
            .detail = "receipt game_request differs from the closure request",
        };
    }
    const auto* output = string_field(*root, "output_file");
    if (output == nullptr || *output != expected.output_file) {
        return {
            .status = RetailAcquisitionReceiptStatus::output_mismatch,
            .detail = "receipt output_file differs from the acquired member path",
        };
    }

    const auto* archive = object_field(*root, "archive");
    const auto* member = object_field(*root, "member");
    const auto* materialized = object_field(*root, "materialized_resource");
    if (archive == nullptr || member == nullptr || materialized == nullptr) {
        return invalid_contract(
            "archive, member or materialized_resource identity is missing");
    }
    const auto* archive_size = u64_field(*archive, "size");
    const auto* archive_sha = string_field(*archive, "sha256");
    const auto* archive_path = string_field(*archive, "path");
    if (archive_size == nullptr || *archive_size == 0U ||
        archive_sha == nullptr || !valid_sha256(*archive_sha) ||
        archive_path == nullptr || archive_path->empty()) {
        return invalid_contract("archive identity is incomplete");
    }

    const auto* logical_path = string_field(*member, "logical_path");
    const auto* central_index = u64_field(*member, "central_index");
    const auto* compression_method = u64_field(*member, "compression_method");
    const auto* crc32 = u64_field(*member, "crc32");
    const auto* compressed_size = u64_field(*member, "compressed_size");
    const auto* uncompressed_size = u64_field(*member, "uncompressed_size");
    const auto* local_header_offset = u64_field(*member, "local_header_offset");
    const auto* data_offset = u64_field(*member, "data_offset");
    if (logical_path == nullptr || logical_path->empty() ||
        central_index == nullptr || compression_method == nullptr ||
        crc32 == nullptr || compressed_size == nullptr ||
        uncompressed_size == nullptr || local_header_offset == nullptr ||
        data_offset == nullptr) {
        return invalid_contract("selected member identity is incomplete");
    }

    const auto actual_size =
        static_cast<std::uint64_t>(expected.member_bytes.size());
    const auto* materialized_size = u64_field(*materialized, "size");
    const auto* materialized_sha = string_field(*materialized, "sha256");
    const auto* origin_kind = string_field(*materialized, "origin_kind");
    const auto* transform = string_field(*materialized, "transform");
    const auto* source_offset = u64_field(*materialized, "source_offset");
    const auto* stored_size = u64_field(*materialized, "stored_size");
    const auto* provenance_size = u64_field(*materialized, "materialized_size");
    const auto* provenance_crc32 = u64_field(*materialized, "provenance_crc32");
    if (materialized_size == nullptr || materialized_sha == nullptr ||
        origin_kind == nullptr || origin_kind->empty() ||
        transform == nullptr || transform->empty() ||
        source_offset == nullptr || stored_size == nullptr ||
        provenance_size == nullptr || provenance_crc32 == nullptr) {
        return invalid_contract("materialized member provenance is incomplete");
    }
    if (*uncompressed_size != actual_size || *materialized_size != actual_size ||
        *provenance_size != actual_size ||
        *compressed_size != *stored_size) {
        return {
            .status = RetailAcquisitionReceiptStatus::member_size_mismatch,
            .detail = "receipt sizes do not describe the acquired member bytes",
        };
    }
    if (*data_offset != *source_offset || *crc32 != *provenance_crc32) {
        return invalid_contract(
            "member metadata and byte provenance do not identify one archive span");
    }

    const auto actual_sha = core::Sha256::compute(expected.member_bytes).hex();
    if (!valid_sha256(*materialized_sha) || *materialized_sha != actual_sha) {
        return {
            .status = RetailAcquisitionReceiptStatus::member_hash_mismatch,
            .detail = "receipt materialized SHA-256 differs from acquired member bytes",
        };
    }
    return {.status = RetailAcquisitionReceiptStatus::verified};
}

} // namespace dmc::rengine::profiles::dmc3
