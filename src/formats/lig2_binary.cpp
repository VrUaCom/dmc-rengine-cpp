#include "dmc_rengine/formats/lig2_binary.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::formats::lig2 {
namespace {

[[nodiscard]] std::string record_id(std::size_t index) {
    std::ostringstream output;
    output << "lig2-record-" << std::setfill('0') << std::setw(4) << index;
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
            .id = "lig2-header",
            .name = "LIG2 header",
            .range = {.offset = 0U, .size = header_size},
            .kind = binary::RegionKind::header,
            .type_name = "Lig2HeaderRaw",
            .evidence_id = {},
        })) {
        return std::nullopt;
    }
    if (!document.add_field(binary::Field{
            .id = "lig2-header-raw",
            .name = "Raw LIG2 header",
            .range = {.offset = 0U, .size = header_size},
            .kind = binary::FieldKind::bytes,
            .type_name = "byte[0x20]",
            .display_value = {},
            .parent_id = {},
            .evidence_id = {},
        })) {
        return std::nullopt;
    }
    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.lig2",
        .range = {.offset = 0U, .size = header_size},
        .rationale = "The LIG2 scanner owns the confirmed header range while preserving unknown fields.",
    }));

    for (std::size_t index = 0; index < scan.records.size(); ++index) {
        const auto& record = scan.records[index];
        const auto id = record_id(index);
        if (!document.add_region(binary::Region{
                .id = id,
                .name = "LIG2 record",
                .range = {.offset = record.offset, .size = record.size},
                .kind = binary::RegionKind::record,
                .type_name = "Lig2RecordRaw",
                .evidence_id = {},
            })) {
            return std::nullopt;
        }
        if (!document.add_field(binary::Field{
                .id = id + "-raw",
                .name = "Raw LIG2 record",
                .range = {.offset = record.offset, .size = record.size},
                .kind = binary::FieldKind::structure,
                .type_name = "byte[0x30]",
                .display_value = {},
                .parent_id = {},
                .evidence_id = {},
            })) {
            return std::nullopt;
        }
        static_cast<void>(document.add_ownership(binary::OwnershipClaim{
            .owner_id = "formats.lig2",
            .range = {.offset = record.offset, .size = record.size},
            .rationale = "The LIG2 scanner owns the confirmed fixed-size record while preserving unknown semantics.",
        }));
    }
    return document;
}

} // namespace dmc::rengine::formats::lig2
