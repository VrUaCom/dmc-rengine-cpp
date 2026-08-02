#include "dmc_rengine/binary/manifest.hpp"

#include <sstream>
#include <string_view>

namespace dmc::rengine::binary {
namespace {

[[nodiscard]] std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";

    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00"
                       << hex[(character >> 4U) & 0x0FU]
                       << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }

    return output.str();
}

void write_string(std::ostringstream& output, std::string_view value) {
    output << '"' << escape_json(value) << '"';
}

void write_range(
    std::ostringstream& output,
    const ByteRange& range,
    std::string_view indent) {
    output << "{\n"
           << indent << "  \"offset\": " << range.offset << ",\n"
           << indent << "  \"size\": " << range.size << "\n"
           << indent << '}';
}

void write_string_array(
    std::ostringstream& output,
    const std::vector<std::string>& values,
    std::string_view indent) {
    output << '[';
    if (!values.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < values.size(); ++index) {
            output << indent << "  ";
            write_string(output, values[index]);
            if (index + 1U != values.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << indent;
    }
    output << ']';
}

} // namespace

std::string manifest_json(const Document& document) {
    if (!document.resource().valid()) {
        return {};
    }

    const auto& resource = document.resource();
    const auto unknown = document.unknown_ranges();
    const auto region_conflicts = document.conflicts();
    const auto owner_conflicts = document.ownership_conflicts();

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": 1,\n"
           << "  \"resource\": {\n"
           << "    \"canonical_id\": ";
    write_string(output, resource.id.canonical());
    output << ",\n    \"source_id\": ";
    write_string(output, resource.id.source_id);
    output << ",\n    \"logical_path\": ";
    write_string(output, resource.id.logical_path);
    output << ",\n    \"container_chain\": ";
    write_string(output, resource.id.container_chain);
    output << ",\n    \"source_offset\": " << resource.id.offset
           << ",\n    \"source_size\": " << resource.id.size
           << ",\n    \"display_name\": ";
    write_string(output, resource.display_name);
    output << ",\n    \"format\": ";
    write_string(output, resource.format);
    output << ",\n    \"profile\": ";
    write_string(output, resource.profile);
    output << ",\n    \"synthetic_name\": "
           << (resource.synthetic_name ? "true" : "false")
           << ",\n    \"container\": "
           << (resource.container ? "true" : "false")
           << "\n  },\n"
           << "  \"byte_size\": " << document.byte_size() << ",\n"
           << "  \"coverage_bytes\": " << document.coverage_bytes() << ",\n"
           << "  \"regions\": [";

    const auto& regions = document.regions();
    if (!regions.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < regions.size(); ++index) {
            const auto& region = regions[index];
            output << "    {\n      \"id\": ";
            write_string(output, region.id);
            output << ",\n      \"name\": ";
            write_string(output, region.name);
            output << ",\n      \"range\": ";
            write_range(output, region.range, "      ");
            output << ",\n      \"kind\": ";
            write_string(output, to_string(region.kind));
            output << ",\n      \"type_name\": ";
            write_string(output, region.type_name);
            output << ",\n      \"evidence_id\": ";
            write_string(output, region.evidence_id);
            output << "\n    }";
            if (index + 1U != regions.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"fields\": [";

    const auto& fields = document.fields();
    if (!fields.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < fields.size(); ++index) {
            const auto& field = fields[index];
            output << "    {\n      \"id\": ";
            write_string(output, field.id);
            output << ",\n      \"name\": ";
            write_string(output, field.name);
            output << ",\n      \"range\": ";
            write_range(output, field.range, "      ");
            output << ",\n      \"kind\": ";
            write_string(output, to_string(field.kind));
            output << ",\n      \"type_name\": ";
            write_string(output, field.type_name);
            output << ",\n      \"display_value\": ";
            write_string(output, field.display_value);
            output << ",\n      \"parent_id\": ";
            write_string(output, field.parent_id);
            output << ",\n      \"evidence_id\": ";
            write_string(output, field.evidence_id);
            output << "\n    }";
            if (index + 1U != fields.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"ownership\": [";

    const auto& ownership = document.ownership();
    if (!ownership.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < ownership.size(); ++index) {
            const auto& claim = ownership[index];
            output << "    {\n      \"owner_id\": ";
            write_string(output, claim.owner_id);
            output << ",\n      \"range\": ";
            write_range(output, claim.range, "      ");
            output << ",\n      \"rationale\": ";
            write_string(output, claim.rationale);
            output << "\n    }";
            if (index + 1U != ownership.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"annotations\": [";

    const auto& annotations = document.annotations();
    if (!annotations.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < annotations.size(); ++index) {
            const auto& annotation = annotations[index];
            output << "    {\n      \"id\": ";
            write_string(output, annotation.id);
            output << ",\n      \"range\": ";
            write_range(output, annotation.range, "      ");
            output << ",\n      \"text\": ";
            write_string(output, annotation.text);
            output << ",\n      \"evidence_id\": ";
            write_string(output, annotation.evidence_id);
            output << ",\n      \"tags\": ";
            write_string_array(output, annotation.tags, "      ");
            output << "\n    }";
            if (index + 1U != annotations.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"unknown_ranges\": [";

    if (!unknown.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < unknown.size(); ++index) {
            output << "    ";
            write_range(output, unknown[index], "    ");
            if (index + 1U != unknown.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"region_conflicts\": [";

    if (!region_conflicts.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < region_conflicts.size(); ++index) {
            const auto& conflict = region_conflicts[index];
            output << "    {\n      \"left_region_id\": ";
            write_string(output, conflict.left_region_id);
            output << ",\n      \"right_region_id\": ";
            write_string(output, conflict.right_region_id);
            output << ",\n      \"overlap\": ";
            write_range(output, conflict.overlap, "      ");
            output << "\n    }";
            if (index + 1U != region_conflicts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"ownership_conflicts\": [";

    if (!owner_conflicts.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < owner_conflicts.size(); ++index) {
            const auto& conflict = owner_conflicts[index];
            output << "    {\n      \"left_owner_id\": ";
            write_string(output, conflict.left_owner_id);
            output << ",\n      \"right_owner_id\": ";
            write_string(output, conflict.right_owner_id);
            output << ",\n      \"overlap\": ";
            write_range(output, conflict.overlap, "      ");
            output << "\n    }";
            if (index + 1U != owner_conflicts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::binary
