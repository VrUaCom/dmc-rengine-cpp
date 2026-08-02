#include "dmc_rengine/formats/hits_binary.hpp"

#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

namespace dmc::rengine::formats::hits {
namespace {

[[nodiscard]] std::string offset_id(
    std::string_view prefix,
    std::uint64_t offset) {
    std::ostringstream output;
    output << prefix << '-' << std::hex << std::setfill('0')
           << std::setw(8) << offset;
    return output.str();
}

[[nodiscard]] std::string value_id(
    std::string_view record_id,
    std::size_t index) {
    std::ostringstream output;
    output << record_id << "-value-" << std::setfill('0')
           << std::setw(2) << index;
    return output.str();
}

[[nodiscard]] std::string float_text(float value) {
    std::ostringstream output;
    output << std::setprecision(9) << value;
    return output.str();
}

} // namespace

std::optional<binary::Document> build_binary_document(
    gdspaces::ResourceRef resource,
    std::span<const std::byte> bytes,
    const ScanResult& scan) {
    if (!resource.valid() || resource.id.size != bytes.size() ||
        !scan.recognized || bytes.size() < 5U) {
        return std::nullopt;
    }

    binary::Document document(std::move(resource), bytes.size());
    if (!document.add_region(binary::Region{
            .id = "hits-magic",
            .name = "HITS$ magic",
            .range = {.offset = 0U, .size = 5U},
            .kind = binary::RegionKind::header,
            .type_name = "char[5]",
            .evidence_id = {},
        })) {
        return std::nullopt;
    }

    static_cast<void>(document.add_field(binary::Field{
        .id = "hits-magic-field",
        .name = "Magic",
        .range = {.offset = 0U, .size = 5U},
        .kind = binary::FieldKind::string,
        .type_name = "char[5]",
        .display_value = "HITS$",
        .parent_id = {},
        .evidence_id = {},
    }));
    static_cast<void>(document.add_ownership(binary::OwnershipClaim{
        .owner_id = "formats.hits",
        .range = {.offset = 0U, .size = 5U},
        .rationale = "The HITS scanner owns the confirmed magic bytes.",
    }));

    for (const auto& record : scan.records) {
        const auto record_id = offset_id("hits-record", record.offset);
        if (!document.add_region(binary::Region{
                .id = record_id,
                .name = "HITS record",
                .range = {.offset = record.offset, .size = record_size},
                .kind = binary::RegionKind::record,
                .type_name = "HitsRecordRaw",
                .evidence_id = {},
            })) {
            return std::nullopt;
        }

        if (!document.add_field(binary::Field{
                .id = record_id + "-struct",
                .name = "HITS record structure",
                .range = {.offset = record.offset, .size = record_size},
                .kind = binary::FieldKind::structure,
                .type_name = "HitsRecordRaw",
                .display_value = {},
                .parent_id = {},
                .evidence_id = {},
            })) {
            return std::nullopt;
        }

        if (!document.add_field(binary::Field{
                .id = record_id + "-marker",
                .name = "Record marker",
                .range = {.offset = record.offset, .size = 4U},
                .kind = binary::FieldKind::unsigned_integer,
                .type_name = "u32_le",
                .display_value = "0x18060001",
                .parent_id = record_id + "-struct",
                .evidence_id = {},
            })) {
            return std::nullopt;
        }

        for (std::size_t index = 0; index < value_count; ++index) {
            if (!document.add_field(binary::Field{
                    .id = value_id(record_id, index),
                    .name = "Raw float " + std::to_string(index),
                    .range = {
                        .offset = record.offset + 4U + index * 4U,
                        .size = 4U,
                    },
                    .kind = binary::FieldKind::floating_point,
                    .type_name = "f32_le",
                    .display_value = float_text(record.values[index]),
                    .parent_id = record_id + "-struct",
                    .evidence_id = {},
                })) {
                return std::nullopt;
            }
        }

        static_cast<void>(document.add_ownership(binary::OwnershipClaim{
            .owner_id = "formats.hits",
            .range = {.offset = record.offset, .size = record_size},
            .rationale = "The HITS scanner owns the confirmed 56-byte record.",
        }));
    }

    return document;
}

} // namespace dmc::rengine::formats::hits
