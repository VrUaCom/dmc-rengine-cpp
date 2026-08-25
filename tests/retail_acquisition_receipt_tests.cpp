#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/profiles/dmc3/retail_acquisition_receipt.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] std::vector<std::byte> bytes(std::string_view text) {
    const auto raw = std::as_bytes(std::span{text.data(), text.size()});
    return {raw.begin(), raw.end()};
}

[[nodiscard]] std::string receipt_for(std::span<const std::byte> member) {
    const auto sha = dmc::rengine::core::Sha256::compute(member).hex();
    std::ostringstream output;
    output
        << "{"
        << "\"schema_version\":2,"
        << "\"evidence_class\":\"artifact-bound-retail-member-acquisition\","
        << "\"game_request\":\"obj/em000.pac\","
        << "\"resolver_status\":\"resolved\","
        << "\"selected_volume_index\":0,"
        << "\"archive\":{\"path\":\"DMC3-0.nbz\",\"size\":960358951,"
        << "\"sha256\":\"" << std::string(64U, 'a') << "\"},"
        << "\"member\":{\"logical_path\":\"obj/em000.pac\","
        << "\"central_index\":7,\"compression_method\":0,\"crc32\":123,"
        << "\"compressed_size\":" << member.size()
        << ",\"uncompressed_size\":" << member.size()
        << ",\"local_header_offset\":10,\"data_offset\":20},"
        << "\"materialized_resource\":{\"size\":" << member.size()
        << ",\"sha256\":\"" << sha
        << "\",\"origin_kind\":\"archive-member\",\"transform\":\"stored\","
        << "\"source_offset\":20,\"stored_size\":" << member.size()
        << ",\"materialized_size\":" << member.size()
        << ",\"provenance_crc32\":123},"
        << "\"output_file\":\"workspace/retail-em000.pac\","
        << "\"publication\":\"atomic-no-replace-per-artifact\""
        << "}";
    return output.str();
}

[[nodiscard]] dmc::rengine::profiles::dmc3::RetailAcquisitionReceiptResult verify(
    std::string_view receipt,
    std::span<const std::byte> member,
    std::string_view request = "obj/em000.pac",
    std::string_view output = "workspace/retail-em000.pac") {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    return dmc3::RetailAcquisitionReceiptVerifier::verify(
        std::as_bytes(std::span{receipt.data(), receipt.size()}),
        dmc3::RetailAcquisitionReceiptExpectation{
            .game_request = request,
            .output_file = output,
            .member_bytes = member,
        });
}

} // namespace

int main() {
    namespace dmc3 = dmc::rengine::profiles::dmc3;
    const auto member = bytes("MEMBER");
    const auto receipt = receipt_for(member);
    assert(verify(receipt, member).ok());

    const auto other_member = bytes("TAMPER");
    assert(verify(receipt, other_member).status ==
        dmc3::RetailAcquisitionReceiptStatus::member_hash_mismatch);
    assert(verify(receipt, member, "obj/em001.pac").status ==
        dmc3::RetailAcquisitionReceiptStatus::request_mismatch);
    assert(verify(receipt, member, "obj/em000.pac", "other/output.pac").status ==
        dmc3::RetailAcquisitionReceiptStatus::output_mismatch);

    auto wrong_schema = receipt;
    wrong_schema.replace(wrong_schema.find("\"schema_version\":2"), 18U,
        "\"schema_version\":3");
    assert(verify(wrong_schema, member).status ==
        dmc3::RetailAcquisitionReceiptStatus::invalid_contract);

    auto wrong_provenance = receipt;
    wrong_provenance.replace(
        wrong_provenance.find("\"source_offset\":20"), 18U,
        "\"source_offset\":21");
    assert(verify(wrong_provenance, member).status ==
        dmc3::RetailAcquisitionReceiptStatus::invalid_contract);

    assert(verify("{\"schema_version\":2,\"schema_version\":2}", member).status ==
        dmc3::RetailAcquisitionReceiptStatus::invalid_json);
    assert(verify("[]", member).status ==
        dmc3::RetailAcquisitionReceiptStatus::invalid_contract);
    return 0;
}
