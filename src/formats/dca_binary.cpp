#include "dmc_rengine/formats/dca_binary.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::formats::dca {
namespace {

[[nodiscard]] std::string record_id(std::size_t index) {
    std::ostringstream output;
    output << "dca-record-" << std::setfill('0') << std::setw(4) << index;
    return output.str();
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ScanResult& scan) {
    if (!resource.valid() || resource.id.size != bytes.size() ||
        !scan.recognized || bytes.size() < header_size) {
        return std::nullopt;
    }

    binary::Document document(std::move(resource), bytes.size());
    if (!document.add_region(binary::Region{
            .id = "dca-header",
            .name = "DCA header",
            .range = {.offset = 0U, .size = header_size},
            .kind = binary::RegionKind::header,
            .type_name = "DcaHeaderRaw",
            .evidence_id = {},
        })) {
        return std::nullopt;
    }
    if (!document.add_field(binary::Field{
            .id = "dca-magic",
            .name = "Magic",
            .range = {.offset = 0U, .size = 4U},
            .kind = binary::FieldKind::string,
            .type_name = "char[4]",
            .display_value = "DCA\\0",
            .parent_id = {},
            .evidence_id = {},
        })) {
        return std::nullopt;
    }
    if (!document.add_field(binary::Field{
            .id = "dca-header-unknown",
            .name = "Unknown header bytes",
            .range = {.offset = 4U, .size = header_size - 4U},
            .kind = binary::FieldKind::bytes,
            .type_name = "byte[12]",
            .display_value = {},
            .parent_id = {},
            .evidence_id = {},
        })) {
        return std::nullopt;
    }
    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.dca",
        .range = {.offset = 0U, .size = header_size},
        .rationale = "The DCA scanner owns the confirmed header range while preserving unknown fields.",
    }));

    for (std::size_t index = 0; index < scan.records.size(); ++index) {
        const auto& record = scan.records[index];
        const auto id = record_id(index);
        if (!document.add_region(binary::Region{
                .id = id,
                .name = "DCA record",
                .range = {.offset = record.offset, .size = record.size},
                .kind = binary::RegionKind::record,
                .type_name = "DcaRecordRaw",
                .evidence_id = {},
            })) {
            return std::nullopt;
        }
        if (!document.add_field(binary::Field{
                .id = id + "-raw",
                .name = "Raw DCA record",
                .range = {.offset = record.offset, .size = record.size},
                .kind = binary::FieldKind::structure,
                .type_name = "byte[0x410]",
                .display_value = {},
                .parent_id = {},
                .evidence_id = {},
            })) {
            return std::nullopt;
        }
        static_cast<void>(document.add_ownership(binary::OwnershipClaim{
            .owner_id = "formats.dca",
            .range = {.offset = record.offset, .size = record.size},
            .rationale = "The DCA scanner owns the confirmed fixed-size record while preserving unknown semantics.",
        }));
    }
    return document;
}

} // namespace dmc::rengine::formats::dca
