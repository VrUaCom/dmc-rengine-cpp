#include "dmc_rengine/evidence/json.hpp"

#include "dmc_rengine/evidence/confidence.hpp"

#include <iomanip>
#include <sstream>
#include <string_view>

namespace dmc::rengine::evidence {
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

void write_string_array(
    std::ostringstream& output,
    const std::vector<std::string>& values,
    std::string_view indent) {
    output << "[";
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

std::string to_json(const EvidencePacket& packet) {
    if (!packet.valid()) {
        return {};
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"schema_version\": " << packet.schema_version << ",\n"
           << "  \"id\": ";
    write_string(output, packet.id);
    output << ",\n  \"title\": ";
    write_string(output, packet.title);
    output << ",\n  \"project\": ";
    write_string(output, packet.project);
    output << ",\n  \"artifacts\": [";

    if (!packet.artifacts.empty()) {
        output << '\n';
        for (std::size_t index = 0; index < packet.artifacts.size(); ++index) {
            const auto& artifact = packet.artifacts[index];
            output << "    {\n"
                   << "      \"id\": ";
            write_string(output, artifact.id);
            output << ",\n      \"role\": ";
            write_string(output, artifact.role);
            output << ",\n      \"sha256\": ";
            write_string(output, artifact.sha256);
            output << ",\n      \"size\": " << artifact.size << "\n"
                   << "    }";
            if (index + 1U != packet.artifacts.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }
    output << "],\n  \"records\": [";

    if (!packet.records.empty()) {
        output << '\n';
        for (std::size_t record_index = 0;
             record_index < packet.records.size();
             ++record_index) {
            const auto& record = packet.records[record_index];
            output << "    {\n"
                   << "      \"id\": ";
            write_string(output, record.id);
            output << ",\n      \"claim_id\": ";
            write_string(output, record.claim_id);
            output << ",\n      \"title\": ";
            write_string(output, record.title);
            output << ",\n      \"summary\": ";
            write_string(output, record.summary);
            output << ",\n      \"confidence\": ";
            write_string(output, to_string(record.confidence));
            output << ",\n      \"tags\": ";
            write_string_array(output, record.tags, "      ");
            output << ",\n      \"supersedes\": ";
            write_string_array(output, record.supersedes, "      ");
            output << ",\n      \"locations\": [";

            if (!record.locations.empty()) {
                output << '\n';
                for (std::size_t location_index = 0;
                     location_index < record.locations.size();
                     ++location_index) {
                    const auto& location = record.locations[location_index];
                    output << "        {\n"
                           << "          \"artifact_id\": ";
                    write_string(output, location.artifact_id);
                    if (location.file_offset.has_value()) {
                        output << ",\n          \"file_offset\": "
                               << *location.file_offset;
                    }
                    if (location.size.has_value()) {
                        output << ",\n          \"size\": " << *location.size;
                    }
                    if (location.rva.has_value()) {
                        output << ",\n          \"rva\": " << *location.rva;
                    }
                    if (location.va.has_value()) {
                        output << ",\n          \"va\": " << *location.va;
                    }
                    if (!location.symbol.empty()) {
                        output << ",\n          \"symbol\": ";
                        write_string(output, location.symbol);
                    }
                    if (!location.note.empty()) {
                        output << ",\n          \"note\": ";
                        write_string(output, location.note);
                    }
                    output << "\n        }";
                    if (location_index + 1U != record.locations.size()) {
                        output << ',';
                    }
                    output << '\n';
                }
                output << "      ";
            }
            output << "]\n    }";
            if (record_index + 1U != packet.records.size()) {
                output << ',';
            }
            output << '\n';
        }
        output << "  ";
    }

    output << "]\n}\n";
    return output.str();
}

} // namespace dmc::rengine::evidence
