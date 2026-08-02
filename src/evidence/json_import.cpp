#include "dmc_rengine/evidence/json_import.hpp"

#include "dmc_rengine/evidence/confidence.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>

namespace dmc::rengine::evidence {
namespace {

class EvidenceImporter final {
public:
    EvidenceImporter(
        const core::json::Value& root,
        EvidenceImportLimits limits)
        : root_(root), limits_(limits) {}

    [[nodiscard]] EvidenceImportResult run() {
        EvidenceImportResult result;
        const auto* root_object = root_.as_object();
        if (root_object == nullptr) {
            fail("$", "Evidence Packet root must be a JSON object.");
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        validate_keys(
            *root_object,
            {"schema_version", "id", "title", "project", "artifacts", "records"},
            "$");

        EvidencePacket packet;
        const auto schema = required_u32(*root_object, "schema_version", "$");
        const auto id = required_string(
            *root_object, "id", "$", limits_.max_id_bytes, false);
        const auto title = required_string(
            *root_object, "title", "$", limits_.max_title_bytes, false);
        const auto project = required_string(
            *root_object, "project", "$", limits_.max_title_bytes, false);
        const auto* artifacts_value = required(*root_object, "artifacts", "$");
        const auto* records_value = required(*root_object, "records", "$");

        if (!schema.has_value() || !id.has_value() || !title.has_value() ||
            !project.has_value() || artifacts_value == nullptr ||
            records_value == nullptr) {
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        if (*schema != 1U) {
            fail("$.schema_version", "Only Evidence Packet schema version 1 is supported.");
        }

        packet.schema_version = *schema;
        packet.id = *id;
        packet.title = *title;
        packet.project = *project;

        parse_artifacts(*artifacts_value, packet);
        parse_records(*records_value, packet);

        if (!diagnostics_.empty()) {
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        if (!packet.valid()) {
            fail(
                "$",
                "Evidence Packet failed final cross-reference or structural validation.");
            result.diagnostics = std::move(diagnostics_);
            return result;
        }

        result.packet = std::move(packet);
        return result;
    }

private:
    void parse_artifacts(
        const core::json::Value& value,
        EvidencePacket& packet) {
        const auto* array = value.as_array();
        if (array == nullptr) {
            fail("$.artifacts", "Artifacts must be a JSON array.");
            return;
        }
        if (array->size() > limits_.max_artifacts) {
            fail("$.artifacts", "Artifact count exceeds the configured limit.");
            return;
        }

        std::set<std::string> ids;
        packet.artifacts.reserve(array->size());
        for (std::size_t index = 0; index < array->size(); ++index) {
            const auto path = "$.artifacts[" + std::to_string(index) + "]";
            const auto* object = (*array)[index].as_object();
            if (object == nullptr) {
                fail(path, "Artifact must be a JSON object.");
                continue;
            }

            validate_keys(*object, {"id", "role", "sha256", "size"}, path);
            const auto id = required_string(
                *object, "id", path, limits_.max_id_bytes, false);
            const auto role = required_string(
                *object, "role", path, limits_.max_title_bytes, false);
            const auto sha256 = required_string(
                *object, "sha256", path, 64U, false);
            const auto size = required_u64(*object, "size", path);
            if (!id.has_value() || !role.has_value() ||
                !sha256.has_value() || !size.has_value()) {
                continue;
            }

            if (!ids.insert(*id).second) {
                fail(path + ".id", "Duplicate artifact ID.");
                continue;
            }

            auto normalized_hash = *sha256;
            std::transform(
                normalized_hash.begin(), normalized_hash.end(),
                normalized_hash.begin(),
                [](unsigned char character) {
                    return static_cast<char>(std::tolower(character));
                });

            ArtifactIdentity artifact{
                .id = *id,
                .role = *role,
                .sha256 = std::move(normalized_hash),
                .size = *size,
            };
            if (!artifact.valid()) {
                fail(path, "Artifact identity is invalid.");
                continue;
            }

            packet.artifacts.push_back(std::move(artifact));
        }
    }

    void parse_records(
        const core::json::Value& value,
        EvidencePacket& packet) {
        const auto* array = value.as_array();
        if (array == nullptr) {
            fail("$.records", "Records must be a JSON array.");
            return;
        }
        if (array->size() > limits_.max_records) {
            fail("$.records", "Record count exceeds the configured limit.");
            return;
        }

        std::set<std::string> artifact_ids;
        for (const auto& artifact : packet.artifacts) {
            artifact_ids.insert(artifact.id);
        }

        std::set<std::string> record_ids;
        packet.records.reserve(array->size());
        for (std::size_t index = 0; index < array->size(); ++index) {
            const auto path = "$.records[" + std::to_string(index) + "]";
            const auto* object = (*array)[index].as_object();
            if (object == nullptr) {
                fail(path, "Evidence record must be a JSON object.");
                continue;
            }

            validate_keys(
                *object,
                {"id", "claim_id", "title", "summary", "confidence",
                 "tags", "supersedes", "locations"},
                path);

            const auto id = required_string(
                *object, "id", path, limits_.max_id_bytes, false);
            const auto claim_id = required_string(
                *object, "claim_id", path, limits_.max_id_bytes, false);
            const auto title = required_string(
                *object, "title", path, limits_.max_title_bytes, false);
            const auto summary = required_string(
                *object, "summary", path, limits_.max_summary_bytes, false);
            const auto confidence_text = required_string(
                *object, "confidence", path, 32U, false);
            const auto* tags_value = required(*object, "tags", path);
            const auto* supersedes_value = required(*object, "supersedes", path);
            const auto* locations_value = required(*object, "locations", path);

            if (!id.has_value() || !claim_id.has_value() ||
                !title.has_value() || !summary.has_value() ||
                !confidence_text.has_value() || tags_value == nullptr ||
                supersedes_value == nullptr || locations_value == nullptr) {
                continue;
            }

            if (!record_ids.insert(*id).second) {
                fail(path + ".id", "Duplicate evidence record ID.");
                continue;
            }

            const auto confidence = confidence_from_string(*confidence_text);
            if (!confidence.has_value()) {
                fail(path + ".confidence", "Unknown evidence confidence value.");
                continue;
            }

            auto tags = string_array(
                *tags_value,
                path + ".tags",
                limits_.max_tags_per_record,
                limits_.max_id_bytes);
            auto supersedes = string_array(
                *supersedes_value,
                path + ".supersedes",
                limits_.max_supersedes_per_record,
                limits_.max_id_bytes);
            auto locations = parse_locations(
                *locations_value,
                path + ".locations",
                artifact_ids);
            if (!tags.has_value() || !supersedes.has_value() ||
                !locations.has_value()) {
                continue;
            }

            EvidenceRecord record{
                .id = *id,
                .claim_id = *claim_id,
                .title = *title,
                .summary = *summary,
                .confidence = *confidence,
                .locations = std::move(*locations),
                .tags = std::move(*tags),
                .supersedes = std::move(*supersedes),
            };
            if (!record.valid()) {
                fail(path, "Evidence record is structurally invalid.");
                continue;
            }

            packet.records.push_back(std::move(record));
        }
    }

    [[nodiscard]] std::optional<std::vector<EvidenceLocation>> parse_locations(
        const core::json::Value& value,
        const std::string& path,
        const std::set<std::string>& artifact_ids) {
        const auto* array = value.as_array();
        if (array == nullptr) {
            fail(path, "Locations must be a JSON array.");
            return std::nullopt;
        }
        if (array->size() > limits_.max_locations_per_record) {
            fail(path, "Location count exceeds the configured limit.");
            return std::nullopt;
        }

        std::vector<EvidenceLocation> locations;
        locations.reserve(array->size());
        auto valid = true;
        for (std::size_t index = 0; index < array->size(); ++index) {
            const auto item_path = path + "[" + std::to_string(index) + "]";
            const auto* object = (*array)[index].as_object();
            if (object == nullptr) {
                fail(item_path, "Evidence location must be a JSON object.");
                valid = false;
                continue;
            }

            validate_keys(
                *object,
                {"artifact_id", "file_offset", "size", "rva", "va",
                 "symbol", "note"},
                item_path);

            const auto artifact_id = required_string(
                *object, "artifact_id", item_path, limits_.max_id_bytes, false);
            if (!artifact_id.has_value()) {
                valid = false;
                continue;
            }
            if (!artifact_ids.contains(*artifact_id)) {
                fail(
                    item_path + ".artifact_id",
                    "Evidence location references an undeclared artifact ID.");
                valid = false;
                continue;
            }

            const auto file_offset = optional_u64(
                *object, "file_offset", item_path);
            const auto size = optional_u64(*object, "size", item_path);
            const auto rva = optional_u64(*object, "rva", item_path);
            const auto va = optional_u64(*object, "va", item_path);
            const auto symbol = optional_string(
                *object, "symbol", item_path, limits_.max_title_bytes);
            const auto note = optional_string(
                *object, "note", item_path, limits_.max_summary_bytes);

            if (!file_offset.valid || !size.valid || !rva.valid || !va.valid ||
                !symbol.valid || !note.valid) {
                valid = false;
                continue;
            }

            EvidenceLocation location{
                .artifact_id = *artifact_id,
                .file_offset = file_offset.value,
                .size = size.value,
                .rva = rva.value,
                .va = va.value,
                .symbol = symbol.value.value_or(std::string{}),
                .note = note.value.value_or(std::string{}),
            };
            if (!location.valid()) {
                fail(item_path, "Evidence location is structurally invalid.");
                valid = false;
                continue;
            }
            locations.push_back(std::move(location));
        }

        return valid ? std::optional{std::move(locations)} : std::nullopt;
    }

    struct OptionalU64 final {
        bool valid{true};
        std::optional<std::uint64_t> value;
    };

    struct OptionalString final {
        bool valid{true};
        std::optional<std::string> value;
    };

    [[nodiscard]] OptionalU64 optional_u64(
        const core::json::Value::Object& object,
        std::string_view key,
        const std::string& path) {
        const auto iterator = object.find(key);
        if (iterator == object.end()) {
            return {};
        }

        const auto* value = iterator->second.as_u64();
        if (value == nullptr) {
            fail(path + "." + std::string{key}, "Expected an unsigned 64-bit integer.");
            return {.valid = false, .value = std::nullopt};
        }
        return {.valid = true, .value = *value};
    }

    [[nodiscard]] OptionalString optional_string(
        const core::json::Value::Object& object,
        std::string_view key,
        const std::string& path,
        std::size_t max_bytes) {
        const auto iterator = object.find(key);
        if (iterator == object.end()) {
            return {};
        }

        const auto* value = iterator->second.as_string();
        if (value == nullptr) {
            fail(path + "." + std::string{key}, "Expected a JSON string.");
            return {.valid = false, .value = std::nullopt};
        }
        if (value->size() > max_bytes) {
            fail(path + "." + std::string{key}, "String exceeds the configured field limit.");
            return {.valid = false, .value = std::nullopt};
        }
        return {.valid = true, .value = *value};
    }

    [[nodiscard]] std::optional<std::vector<std::string>> string_array(
        const core::json::Value& value,
        const std::string& path,
        std::size_t max_items,
        std::size_t max_string_bytes) {
        const auto* array = value.as_array();
        if (array == nullptr) {
            fail(path, "Expected a JSON string array.");
            return std::nullopt;
        }
        if (array->size() > max_items) {
            fail(path, "String array item count exceeds the configured limit.");
            return std::nullopt;
        }

        std::vector<std::string> result;
        result.reserve(array->size());
        for (std::size_t index = 0; index < array->size(); ++index) {
            const auto* text = (*array)[index].as_string();
            const auto item_path = path + "[" + std::to_string(index) + "]";
            if (text == nullptr) {
                fail(item_path, "Expected a JSON string.");
                return std::nullopt;
            }
            if (text->empty()) {
                fail(item_path, "String array items must not be empty.");
                return std::nullopt;
            }
            if (text->size() > max_string_bytes) {
                fail(item_path, "String array item exceeds the configured limit.");
                return std::nullopt;
            }
            result.push_back(*text);
        }
        return result;
    }

    [[nodiscard]] const core::json::Value* required(
        const core::json::Value::Object& object,
        std::string_view key,
        const std::string& path) {
        const auto iterator = object.find(key);
        if (iterator == object.end()) {
            fail(path + "." + std::string{key}, "Required field is missing.");
            return nullptr;
        }
        return &iterator->second;
    }

    [[nodiscard]] std::optional<std::string> required_string(
        const core::json::Value::Object& object,
        std::string_view key,
        const std::string& path,
        std::size_t max_bytes,
        bool allow_empty) {
        const auto* value = required(object, key, path);
        if (value == nullptr) {
            return std::nullopt;
        }
        const auto* text = value->as_string();
        if (text == nullptr) {
            fail(path + "." + std::string{key}, "Expected a JSON string.");
            return std::nullopt;
        }
        if (!allow_empty && text->empty()) {
            fail(path + "." + std::string{key}, "String field must not be empty.");
            return std::nullopt;
        }
        if (text->size() > max_bytes) {
            fail(path + "." + std::string{key}, "String field exceeds the configured limit.");
            return std::nullopt;
        }
        return *text;
    }

    [[nodiscard]] std::optional<std::uint64_t> required_u64(
        const core::json::Value::Object& object,
        std::string_view key,
        const std::string& path) {
        const auto* value = required(object, key, path);
        if (value == nullptr) {
            return std::nullopt;
        }
        const auto* number = value->as_u64();
        if (number == nullptr) {
            fail(path + "." + std::string{key}, "Expected an unsigned 64-bit integer.");
            return std::nullopt;
        }
        return *number;
    }

    [[nodiscard]] std::optional<std::uint32_t> required_u32(
        const core::json::Value::Object& object,
        std::string_view key,
        const std::string& path) {
        const auto value = required_u64(object, key, path);
        if (!value.has_value()) {
            return std::nullopt;
        }
        if (*value > std::numeric_limits<std::uint32_t>::max()) {
            fail(path + "." + std::string{key}, "Integer exceeds the unsigned 32-bit range.");
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(*value);
    }

    void validate_keys(
        const core::json::Value::Object& object,
        std::initializer_list<std::string_view> allowed,
        const std::string& path) {
        for (const auto& [key, ignored] : object) {
            static_cast<void>(ignored);
            if (std::find(allowed.begin(), allowed.end(), key) == allowed.end()) {
                fail(path + "." + key, "Unknown field is not allowed by the strict schema.");
            }
        }
    }

    void fail(std::string path, std::string message) {
        diagnostics_.push_back(EvidenceImportDiagnostic{
            .path = std::move(path),
            .message = std::move(message),
        });
    }

    const core::json::Value& root_;
    EvidenceImportLimits limits_;
    std::vector<EvidenceImportDiagnostic> diagnostics_;
};

} // namespace

EvidenceImportResult evidence_packet_from_json(
    std::string_view input,
    EvidenceImportLimits limits) {
    const auto parsed = core::json::Parser::parse(input, limits.json);
    if (!parsed.ok()) {
        EvidenceImportResult result;
        for (const auto& error : parsed.errors) {
            result.diagnostics.push_back(EvidenceImportDiagnostic{
                .path = "$@" + std::to_string(error.offset),
                .message = error.message,
            });
        }
        return result;
    }

    return EvidenceImporter(*parsed.value, limits).run();
}

} // namespace dmc::rengine::evidence
