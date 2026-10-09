#include "dmc_rengine/spider/l2_original_selection.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/spider/l2_runtime_mapping.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace dmc::rengine::spider {
namespace {

using core::json::Value;

constexpr std::string_view k_legacy_evidence_class = "original-process-observation";
constexpr std::string_view k_candidate_evidence_class = "original-process-observation-candidate";
constexpr std::string_view k_process_window_schema = "dmc-rengine.exe-process-window.v1";
constexpr std::string_view k_protected_sha256 = "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";
constexpr std::uint64_t k_protected_size = 6567320U;
constexpr std::size_t k_min_mapping_children = 3U;
constexpr std::array<std::string_view, 6> k_prefixes{
    "GDataX360.afs/", "GData.afs/", "Video/", "afs/sound/", "SAVEDATA/", ""};

constexpr std::array<std::string_view, 21> k_legacy_top_fields{
    "schema", "evidence_class", "executable_sha256", "executable_size",
    "runtime_mapping_packet_sha256", "observer_id", "observer_version",
    "observer_build_sha256", "trace_complete", "dropped_event_count", "pid",
    "module_base", "flags", "request", "basename", "first_missing_archive_volume",
    "archives", "probes", "selected", "proves", "does_not_prove"};
constexpr std::array<std::string_view, 24> k_candidate_fields{
    "schema", "evidence_class", "promotion_eligible", "trusted_capture_bound",
    "legacy_schema_normalized", "executable_sha256", "executable_size",
    "runtime_mapping_packet_sha256", "observer_id", "observer_version",
    "observer_build_sha256", "trace_complete", "dropped_event_count", "pid",
    "module_base", "flags", "request", "basename", "first_missing_archive_volume",
    "archives", "probes", "selected", "proves", "does_not_prove"};
constexpr std::array<std::string_view, 4> k_archive_fields{"volume_index", "filename", "sha256", "size"};
constexpr std::array<std::string_view, 7> k_probe_fields{
    "sequence_index", "lookup_attempt_index", "provider", "candidate",
    "provider_key", "archive_volume_index", "outcome"};
constexpr std::array<std::string_view, 7> k_selected_fields{
    "provider", "lookup_attempt_index", "candidate", "provider_key",
    "archive_volume_index", "archive_member_path", "physical_relative_path"};

/// A rejection, carrying the Python script's message.
struct Rejected final {
    std::string message;
};

[[noreturn]] void reject(std::string message) {
    throw Rejected{std::move(message)};
}

template <std::size_t N>
[[nodiscard]] bool listed(const std::array<std::string_view, N>& fields, std::string_view key) {
    return std::find(fields.begin(), fields.end(), key) != fields.end();
}

[[nodiscard]] std::string joined(const std::vector<std::string>& names) {
    std::string text;
    for (const auto& name : names) {
        if (!text.empty()) text += ", ";
        text += name;
    }
    return text;
}

/// Python's `_exact_keys`; `missing_too` is the binder's stricter form.
template <std::size_t N>
const Value::Object& exact_keys(const Value& value, const std::array<std::string_view, N>& allowed,
                                const std::string& context, bool missing_too) {
    const auto* object = value.as_object();
    if (object == nullptr) reject(context + " must be an object");
    std::vector<std::string> extra;
    for (const auto& [key, item] : *object) {
        if (!listed(allowed, key)) extra.push_back(key);
    }
    if (!extra.empty()) reject(context + " contains unsupported field(s): " + joined(extra));
    if (missing_too) {
        std::vector<std::string> missing;
        for (const auto field : allowed) {
            if (object->find(field) == object->end()) missing.emplace_back(field);
        }
        std::sort(missing.begin(), missing.end());
        if (!missing.empty()) reject(context + " is missing field(s): " + joined(missing));
    }
    return *object;
}

[[nodiscard]] const Value* member(const Value::Object& object, std::string_view key) {
    const auto found = object.find(key);
    return found == object.end() ? nullptr : &found->second;
}

[[nodiscard]] const Value& at(const Value::Object& object, std::string_view key) {
    static const Value null_value{};
    const auto* value = member(object, key);
    return value == nullptr ? null_value : *value;
}

[[nodiscard]] bool is_text(const Value& value, std::string_view text) {
    const auto* held = value.as_string();
    return held != nullptr && *held == text;
}

[[nodiscard]] bool is_bool(const Value& value, bool expected) {
    const auto* held = value.as_bool();
    return held != nullptr && *held == expected;
}

[[nodiscard]] std::optional<std::int64_t> integer(const Value& value) {
    if (const auto* held = value.as_i64()) return *held;
    if (const auto* held = value.as_u64()) {
        if (*held <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return static_cast<std::int64_t>(*held);
        }
    }
    return std::nullopt;
}

[[nodiscard]] bool is_integer_equal(const Value& value, std::int64_t expected) {
    const auto held = integer(value);
    return held.has_value() && *held == expected;
}

[[nodiscard]] std::string sha256_hex(std::span<const std::byte> bytes) {
    return core::Sha256::compute(bytes).hex();
}

[[nodiscard]] std::span<const std::byte> bytes_of(std::string_view text) {
    return std::as_bytes(std::span<const char>{text.data(), text.size()});
}

struct FileDigest final {
    std::string sha256;
    std::uint64_t size{};
};

[[nodiscard]] FileDigest sha256_file(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) reject("could not read artifact " + path.string() + ": cannot open");
    core::Sha256Accumulator accumulator;
    std::vector<char> chunk(1024U * 1024U);
    while (stream) {
        stream.read(chunk.data(), static_cast<std::streamsize>(chunk.size()));
        const auto got = static_cast<std::size_t>(stream.gcount());
        if (got == 0U) break;
        if (!accumulator.update(std::as_bytes(std::span<const char>{chunk.data(), got}))) {
            reject("could not read artifact " + path.string() + ": hash failed");
        }
    }
    if (stream.bad()) reject("could not read artifact " + path.string() + ": read failed");
    const auto digest = accumulator.finalize();
    if (!digest.has_value()) reject("could not read artifact " + path.string() + ": hash failed");
    return FileDigest{.sha256 = digest->hex(), .size = accumulator.byte_count()};
}

struct JsonFile final {
    Value value;
    std::string raw;
};

[[nodiscard]] JsonFile read_json(const std::filesystem::path& path) {
    std::ifstream stream{path, std::ios::binary};
    if (!stream) reject("could not read JSON " + path.string() + ": cannot open");
    std::string raw{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    auto parsed = core::json::Parser::parse(raw);
    if (!parsed.ok()) {
        reject("could not read JSON " + path.string() + ": " +
               (parsed.errors.empty() ? std::string{"invalid JSON"} : parsed.errors.front().message));
    }
    if (parsed.value->as_object() == nullptr) reject(path.string() + " must contain one JSON object");
    if (contains_key(*parsed.value, "bytes_hex")) reject(path.string() + " contains forbidden raw bytes_hex");
    return JsonFile{.value = std::move(*parsed.value), .raw = std::move(raw)};
}

[[nodiscard]] const std::string& sha(const Value& value, const std::string& field) {
    const auto* text = value.as_string();
    const bool canonical = text != nullptr && text->size() == 64U &&
        std::all_of(text->begin(), text->end(), [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
    if (!canonical) reject(field + " must be canonical lowercase SHA-256");
    return *text;
}

[[nodiscard]] std::uint64_t hex_u64(const Value& value, const std::string& field) {
    const auto* text = value.as_string();
    if (text == nullptr || !text->starts_with("0x")) reject(field + " must be 0x-prefixed hexadecimal");
    const std::string_view digits = std::string_view{*text}.substr(2U);
    if (digits.empty()) reject(field + " is invalid hexadecimal");
    std::uint64_t parsed = 0U;
    bool overflow = false;
    for (const char c : digits) {
        std::uint64_t nibble = 0U;
        if (c >= '0' && c <= '9') nibble = static_cast<std::uint64_t>(c - '0');
        else if (c >= 'a' && c <= 'f') nibble = static_cast<std::uint64_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') nibble = static_cast<std::uint64_t>(c - 'A' + 10);
        else reject(field + " is invalid hexadecimal");
        if (parsed > (std::numeric_limits<std::uint64_t>::max() >> 4U)) overflow = true;
        parsed = (parsed << 4U) | nibble;
    }
    if (overflow) reject(field + " is outside uint64 range");
    return parsed;
}

[[nodiscard]] std::string upper_hex(std::uint64_t value) {
    std::ostringstream output;
    output << "0x" << std::uppercase << std::hex << value;
    return output.str();
}

/// The recovered resolver's key normalization (flags 0x01 upper, 0x02 lower, 0x04/0x08 trim).
[[nodiscard]] std::string normalize_key(std::string_view path, unsigned flags) {
    if (path.find('\0') != std::string_view::npos) reject("path contains embedded NUL");
    const auto separator = [](char c) { return c == '/' || c == '\\'; };
    std::size_t begin = 0U;
    std::size_t end = path.size();
    if ((flags & 0x04U) != 0U) {
        while (begin < end && separator(path[begin])) ++begin;
    }
    if ((flags & 0x08U) != 0U) {
        while (end > begin && separator(path[end - 1U])) --end;
    }
    const bool upper = (flags & 0x01U) != 0U;
    const bool lower = (flags & 0x02U) != 0U && !upper;
    std::string output;
    bool previous_separator = false;
    for (auto c : path.substr(begin, end - begin)) {
        if (upper && c >= 'a' && c <= 'z') c = static_cast<char>(c - 32);
        else if (lower && c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
        if (separator(c)) {
            if (!previous_separator) output.push_back('\\');
            previous_separator = true;
        } else {
            output.push_back(c);
            previous_separator = false;
        }
    }
    return output;
}

[[nodiscard]] std::string basename_of(std::string_view request) {
    if (request.empty() || request.find('\0') != std::string_view::npos) return {};
    const auto position = request.find_last_of("/\\");
    return std::string{position == std::string_view::npos ? request : request.substr(position + 1U)};
}

/// The second character Python sees in UTF-8 `text`, when it is one byte.
[[nodiscard]] bool second_character_is_colon(std::string_view text) {
    if (text.empty()) return false;
    const auto lead = static_cast<unsigned char>(text[0]);
    const std::size_t width = lead < 0x80U ? 1U : lead < 0xE0U ? 2U : lead < 0xF0U ? 3U : 4U;
    return width < text.size() && text[width] == ':';
}

[[nodiscard]] OrderedJson fields_in_order(const Value::Object& object, std::span<const std::string_view> fields) {
    auto result = OrderedJson::object();
    for (const auto field : fields) {
        if (const auto* value = member(object, field)) result.set(std::string{field}, to_ordered(*value));
    }
    return result;
}

[[nodiscard]] OrderedJson normalize(const Value& value) {
    exact_keys(value, k_legacy_top_fields, "legacy selection", false);
    const auto& object = *value.as_object();
    if (!is_text(at(object, "schema"), k_l2_legacy_selection_schema)) reject("legacy selection schema mismatch");
    if (!is_text(at(object, "evidence_class"), k_legacy_evidence_class)) {
        reject("legacy selection evidence class mismatch");
    }
    if (contains_key(value, "bytes_hex")) reject("legacy selection contains forbidden raw bytes_hex");
    for (const auto field : k_legacy_top_fields) {
        if (field == "schema" || field == "evidence_class" || field == "proves" || field == "does_not_prove") continue;
        if (member(object, field) == nullptr) reject("legacy selection missing required field " + std::string{field});
    }

    const auto sanitize_list = [&object](std::string_view key, std::string_view item_name,
                                         std::span<const std::string_view> fields, auto allowed) {
        const auto* items = at(object, key).as_array();
        if (items == nullptr) reject("legacy " + std::string{key} + " must be an array");
        auto result = OrderedJson::array();
        for (std::size_t index = 0U; index < items->size(); ++index) {
            const auto& entry = exact_keys((*items)[index], allowed,
                                           "legacy " + std::string{item_name} + "[" + std::to_string(index) + "]", false);
            result.push(fields_in_order(entry, fields));
        }
        return result;
    };

    auto normalized = OrderedJson::object();
    normalized.set("schema", k_l2_selection_candidate_schema)
        .set("evidence_class", k_candidate_evidence_class)
        .set("promotion_eligible", false)
        .set("trusted_capture_bound", false)
        .set("legacy_schema_normalized", true);
    for (const auto field : {"executable_sha256", "executable_size", "runtime_mapping_packet_sha256", "observer_id",
                             "observer_version", "observer_build_sha256", "trace_complete", "dropped_event_count",
                             "pid", "module_base", "flags", "request", "basename", "first_missing_archive_volume"}) {
        normalized.set(field, to_ordered(at(object, field)));
    }
    normalized.set("archives", sanitize_list("archives", "archive", k_archive_fields, k_archive_fields));
    normalized.set("probes", sanitize_list("probes", "probe", k_probe_fields, k_probe_fields));
    const auto& selected = exact_keys(at(object, "selected"), k_selected_fields, "legacy selected", false);
    normalized.set("selected", fields_in_order(selected, k_selected_fields));
    normalized.set("proves", OrderedJson::array().push("self-authored-selection-content-has-candidate-shape-only"));

    std::vector<std::string> nonclaims;
    if (const auto* old = at(object, "does_not_prove").as_array()) {
        for (const auto& item : *old) {
            if (const auto* text = item.as_string()) nonclaims.push_back(*text);
        }
    }
    for (const auto* item : {"trusted-observer-execution-or-trace-origin",
                             "original-process-selected-provider-identity", "promotion-eligibility"}) {
        if (std::find(nonclaims.begin(), nonclaims.end(), item) == nonclaims.end()) nonclaims.emplace_back(item);
    }
    auto list = OrderedJson::array();
    for (auto& item : nonclaims) list.push(std::move(item));
    normalized.set("does_not_prove", std::move(list));
    return normalized;
}

struct ChildReceipt final {
    std::uint64_t rva{};
    std::string rva_text;
    std::string receipt_sha256;
};

[[nodiscard]] Value reconstruct_mapping(const std::vector<std::filesystem::path>& children,
                                        std::vector<ChildReceipt>& receipts) {
    if (children.size() < k_min_mapping_children) {
        reject("at least " + std::to_string(k_min_mapping_children) + " mapping child receipts are required");
    }
    std::vector<std::string> raws;
    for (const auto& path : children) {
        auto child = read_json(path);
        const auto& object = *child.value.as_object();
        if (!is_text(at(object, "schema"), k_process_window_schema)) {
            reject("mapping child " + path.string() + " has unsupported schema");
        }
        const auto* rva = at(object, "rva").as_string();
        if (rva == nullptr) reject("mapping child " + path.string() + " has invalid RVA");
        receipts.push_back(ChildReceipt{.rva = 0U, .rva_text = *rva, .receipt_sha256 = sha256_hex(bytes_of(child.raw))});
        raws.push_back(std::move(child.raw));
    }

    std::vector<std::string_view> views{raws.begin(), raws.end()};
    const auto built = build_l2_runtime_mapping_v1(views);
    if (!built.ok()) {
        std::string message;
        for (const auto& error : built.errors) message += (message.empty() ? "" : "; ") + error;
        reject(message.empty() ? std::string{"mapping child receipts do not build a mapping packet"} : message);
    }
    for (auto& receipt : receipts) {
        Value rva;
        rva.data = receipt.rva_text;
        receipt.rva = hex_u64(rva, "mapping child rva");
    }
    std::stable_sort(receipts.begin(), receipts.end(), [](const auto& l, const auto& r) { return l.rva < r.rva; });

    auto parsed = core::json::Parser::parse(l2_runtime_mapping_v1_to_json(*built.packet));
    if (!parsed.ok()) reject("native mapping serializer failed");
    return std::move(*parsed.value);
}

struct MappingSession final {
    std::int64_t pid{};
    std::uint64_t module_base{};
};

[[nodiscard]] MappingSession mapping_session(const Value::Object& mapping) {
    if (!is_text(at(mapping, "schema"), k_l2_runtime_mapping_v1_schema)) reject("mapping packet schema mismatch");
    if (!is_text(at(mapping, "status"), "bounded_match")) reject("mapping packet is not bounded_match");
    if (!is_text(at(mapping, "protected_artifact_sha256"), k_protected_sha256)) {
        reject("mapping protected artifact SHA mismatch");
    }
    if (!is_integer_equal(at(mapping, "protected_artifact_size"), static_cast<std::int64_t>(k_protected_size))) {
        reject("mapping protected artifact size mismatch");
    }
    const auto pid = integer(at(mapping, "pid"));
    if (!pid.has_value() || *pid <= 0) reject("mapping pid is invalid");
    return MappingSession{.pid = *pid, .module_base = hex_u64(at(mapping, "module_base"), "mapping module_base")};
}

using ArchiveClaims = std::map<std::uint32_t, const Value::Object*>;

[[nodiscard]] ArchiveClaims validate_archives(const Value::Object& selection) {
    const auto first_missing = integer(at(selection, "first_missing_archive_volume"));
    if (!first_missing.has_value() || *first_missing < 0 || *first_missing > 0x7FFFFFFF) {
        reject("first_missing_archive_volume is invalid");
    }
    const auto* archives = at(selection, "archives").as_array();
    if (archives == nullptr || static_cast<std::int64_t>(archives->size()) != *first_missing) {
        reject("archive identity census is not contiguous");
    }
    ArchiveClaims claims;
    for (std::size_t position = 0U; position < archives->size(); ++position) {
        const auto& item = exact_keys((*archives)[position], k_archive_fields,
                                      "archive[" + std::to_string(position) + "]", true);
        const auto index = integer(at(item, "volume_index"));
        if (!index.has_value() || *index < 0 || *index >= *first_missing ||
            claims.contains(static_cast<std::uint32_t>(*index))) {
            reject("archive volume index is invalid/duplicate");
        }
        if (!is_text(at(item, "filename"), "DMC3-" + std::to_string(*index) + ".nbz")) {
            reject("archive filename does not match runtime volume identity");
        }
        (void)sha(at(item, "sha256"), "archive " + std::to_string(*index) + " SHA");
        const auto size = integer(at(item, "size"));
        if (!size.has_value() || *size <= 0) reject("archive size is invalid");
        claims.emplace(static_cast<std::uint32_t>(*index), &item);
    }
    if (static_cast<std::int64_t>(claims.size()) != *first_missing) reject("archive volume identity set has a gap");
    return claims;
}

struct ExpectedProbe final {
    std::int64_t attempt{};
    std::string_view provider;
    std::string candidate;
    std::optional<std::int64_t> volume;
};

[[nodiscard]] ArchiveClaims validate_candidate(const Value& value, std::string_view mapping_sha,
                                               const MappingSession& session) {
    const auto& selection = exact_keys(value, k_candidate_fields, "selection candidate", true);
    if (!is_text(at(selection, "schema"), k_l2_selection_candidate_schema)) reject("selection receipt schema mismatch");
    if (!is_text(at(selection, "evidence_class"), k_candidate_evidence_class)) {
        reject("selection evidence class mismatch");
    }
    if (!is_bool(at(selection, "promotion_eligible"), false)) {
        reject("selection candidate may not predeclare promotion eligibility");
    }
    if (!is_bool(at(selection, "trusted_capture_bound"), false)) {
        reject("selection candidate may not predeclare trusted capture");
    }
    if (!is_bool(at(selection, "legacy_schema_normalized"), true)) {
        reject("selection candidate lacks legacy-normalizer provenance marker");
    }
    if (sha(at(selection, "executable_sha256"), "selection executable SHA") != k_protected_sha256) {
        reject("selection is not bound to protected DMC3 executable");
    }
    if (!is_integer_equal(at(selection, "executable_size"), static_cast<std::int64_t>(k_protected_size))) {
        reject("selection executable size mismatch");
    }
    if (sha(at(selection, "runtime_mapping_packet_sha256"), "mapping packet SHA") != mapping_sha) {
        reject("selection does not hash-bind the supplied mapping packet");
    }
    if (!is_integer_equal(at(selection, "pid"), session.pid)) reject("selection and mapping pid differ");
    if (hex_u64(at(selection, "module_base"), "selection module_base") != session.module_base) {
        reject("selection and mapping module base differ");
    }
    if (!is_integer_equal(at(selection, "flags"), 1)) reject("selection must use recovered direct-call flags=1 mode");
    for (const auto* field : {"observer_id", "observer_version"}) {
        const auto* text = at(selection, field).as_string();
        if (text == nullptr || text->empty() || python_length(*text) > 128U ||
            text->find('\0') != std::string::npos) {
            reject("selection " + std::string{field} + " is invalid");
        }
    }
    (void)sha(at(selection, "observer_build_sha256"), "observer build SHA");
    if (!is_bool(at(selection, "trace_complete"), true)) reject("selection trace is not marked complete");
    if (!is_integer_equal(at(selection, "dropped_event_count"), 0)) reject("selection trace reports dropped events");

    const auto* request = at(selection, "request").as_string();
    const auto* basename = at(selection, "basename").as_string();
    if (request == nullptr || basename == nullptr || basename->empty() || basename_of(*request) != *basename) {
        reject("selection request/basename binding is invalid");
    }
    if (python_length(std::string{k_prefixes[0]} + *basename) >= 0x400U) {
        reject("selection request violates recovered 0x400 first-candidate bound");
    }

    auto claims = validate_archives(selection);
    const auto first_missing = *integer(at(selection, "first_missing_archive_volume"));

    const auto* probes = at(selection, "probes").as_array();
    if (probes == nullptr || probes->empty()) reject("selection probes are missing");
    const auto& selected = exact_keys(at(selection, "selected"), k_selected_fields, "selected identity", true);

    std::vector<ExpectedProbe> expected;
    for (std::int64_t attempt = 0; attempt < 12; ++attempt) {
        const bool archive = attempt < 6;
        const auto candidate = std::string{k_prefixes[static_cast<std::size_t>(attempt % 6)]} + *basename;
        if (archive) {
            for (auto volume = first_missing - 1; volume >= 0; --volume) {
                expected.push_back(ExpectedProbe{attempt, "archive", candidate, volume});
            }
        } else {
            expected.push_back(ExpectedProbe{attempt, "physical", candidate, std::nullopt});
        }
    }
    if (probes->size() > expected.size()) reject("selection contains more probes than recovered policy permits");

    bool selected_seen = false;
    for (std::size_t sequence = 0U; sequence < probes->size(); ++sequence) {
        const auto& probe = exact_keys((*probes)[sequence], k_probe_fields,
                                       "probe[" + std::to_string(sequence) + "]", true);
        const auto& want = expected[sequence];
        if (!is_integer_equal(at(probe, "sequence_index"), static_cast<std::int64_t>(sequence)) ||
            !is_integer_equal(at(probe, "lookup_attempt_index"), want.attempt)) {
            reject("probe sequence/attempt order mismatch");
        }
        if (!is_text(at(probe, "provider"), want.provider) || !is_text(at(probe, "candidate"), want.candidate)) {
            reject("probe provider/candidate order mismatch");
        }
        const auto& volume = at(probe, "archive_volume_index");
        const bool volume_matches = want.volume.has_value() ? is_integer_equal(volume, *want.volume) : volume.is_null();
        if (!volume_matches) reject("probe archive volume precedence mismatch");
        if (!is_text(at(probe, "provider_key"), normalize_key(want.candidate, want.provider == "archive" ? 0x0EU : 0x0CU))) {
            reject("probe provider key mismatch");
        }
        const auto& outcome = at(probe, "outcome");
        if (!is_text(outcome, "miss") && !is_text(outcome, "selected")) {
            reject("v1 selection candidate supports only clean miss/selected outcomes; "
                   "provider/backend failure is fail-closed");
        }
        if (is_text(outcome, "selected")) {
            if (sequence != probes->size() - 1U) reject("selected probe must terminate trace");
            selected_seen = true;
        }
    }
    if (!selected_seen) reject("trace has no selected probe");

    const auto& terminal = *probes->back().as_object();
    for (const auto* key : {"provider", "lookup_attempt_index", "candidate", "provider_key", "archive_volume_index"}) {
        if (!json_equal(at(selected, key), at(terminal, key))) {
            reject("selected identity disagrees with terminal probe field " + std::string{key});
        }
    }

    const auto* selected_key = at(selected, "provider_key").as_string();
    if (selected_key == nullptr || selected_key->empty()) reject("selected provider key is invalid");
    const auto& provider = at(selected, "provider");
    if (is_text(provider, "archive")) {
        const auto* member_path = at(selected, "archive_member_path").as_string();
        if (member_path == nullptr || member_path->empty() || normalize_key(*member_path, 0x0EU) != *selected_key) {
            reject("selected archive member identity does not match provider key");
        }
        if (!is_text(at(selected, "physical_relative_path"), "")) reject("archive selection carries physical identity");
    } else if (is_text(provider, "physical")) {
        const auto* relative = at(selected, "physical_relative_path").as_string();
        if (relative == nullptr || relative->empty() || relative->front() == '/' || relative->front() == '\\' ||
            second_character_is_colon(*relative)) {
            reject("physical identity must be mounted-root-relative");
        }
        if (normalize_key(*relative, 0x0CU) != *selected_key) {
            reject("selected physical identity does not match provider key");
        }
        if (!is_text(at(selected, "archive_member_path"), "")) {
            reject("physical selection carries archive member identity");
        }
    } else {
        reject("selected provider is invalid");
    }
    return claims;
}

[[nodiscard]] OrderedJson bind(const L2SelectionBindingInputs& inputs) {
    const auto mapping = read_json(inputs.mapping);
    const auto selection = read_json(inputs.selection);

    std::vector<ChildReceipt> receipts;
    const auto rebuilt = reconstruct_mapping(inputs.mapping_children, receipts);
    if (!json_equal(mapping.value, rebuilt)) {
        reject("supplied mapping packet does not exactly match reconstruction from mapping child receipts");
    }

    const auto mapping_sha = sha256_hex(bytes_of(mapping.raw));
    const auto session = mapping_session(*mapping.value.as_object());
    const auto claims = validate_candidate(selection.value, mapping_sha, session);
    const auto& candidate = *selection.value.as_object();

    const auto observer = sha256_file(inputs.observer_artifact);
    if (observer.sha256 != *at(candidate, "observer_build_sha256").as_string()) {
        reject("observer artifact SHA does not match selection receipt");
    }

    std::set<std::uint32_t> claimed;
    std::set<std::uint32_t> supplied;
    for (const auto& [index, claim] : claims) claimed.insert(index);
    for (const auto& [index, path] : inputs.archive_artifacts) supplied.insert(index);
    if (claimed != supplied) reject("archive artifact set does not exactly match selection volume set");
    auto archives = OrderedJson::array();
    for (const auto& [index, claim] : claims) {
        const auto actual = sha256_file(inputs.archive_artifacts.at(index));
        if (!is_text(at(*claim, "sha256"), actual.sha256) ||
            !is_integer_equal(at(*claim, "size"), static_cast<std::int64_t>(actual.size))) {
            reject("archive artifact " + std::to_string(index) + " does not match claimed SHA/size");
        }
        archives.push(OrderedJson::object()
                          .set("volume_index", static_cast<std::int64_t>(index))
                          .set("filename", to_ordered(at(*claim, "filename")))
                          .set("sha256", actual.sha256)
                          .set("size", actual.size));
    }

    auto children = OrderedJson::array();
    for (const auto& receipt : receipts) {
        children.push(OrderedJson::object().set("rva", receipt.rva_text).set("receipt_sha256", receipt.receipt_sha256));
    }

    auto packet = OrderedJson::object();
    packet.set("schema", k_l2_selection_bound_schema)
        .set("status", "bound_candidate")
        .set("evidence_class", k_candidate_evidence_class)
        .set("promotion_eligible", false)
        .set("trusted_capture_bound", false)
        .set("protected_artifact_sha256", k_protected_sha256)
        .set("protected_artifact_size", k_protected_size)
        .set("runtime_mapping_packet_sha256", mapping_sha)
        .set("runtime_mapping_child_receipts", std::move(children))
        .set("selection_candidate_sha256", sha256_hex(bytes_of(selection.raw)))
        .set("observer", OrderedJson::object()
                             .set("id", to_ordered(at(candidate, "observer_id")))
                             .set("version", to_ordered(at(candidate, "observer_version")))
                             .set("sha256", observer.sha256)
                             .set("size", observer.size))
        .set("archives", std::move(archives))
        .set("pid", session.pid)
        .set("module_base", upper_hex(session.module_base))
        .set("trace_complete", true)
        .set("dropped_event_count", 0)
        .set("request", to_ordered(at(candidate, "request")))
        .set("selected", fields_in_order(*at(candidate, "selected").as_object(), k_selected_fields));
    auto proves = OrderedJson::array();
    for (const auto* item : {"selection-candidate-structure-matches-recovered-clean-path-policy",
                             "mapping-reconstructs-from-supplied-process-window-receipts",
                             "candidate-hash-binds-exact-mapping-file", "observer-artifact-matches-declared-build-sha",
                             "numbered-archive-artifacts-match-declared-sha-and-size"}) {
        proves.push(item);
    }
    auto nonclaims = OrderedJson::array();
    for (const auto* item : {"trusted-observer-execution-or-trace-origin",
                             "original-process-selected-provider-identity", "retail-archive-collision-freedom",
                             "global-build-equivalence", "layer-1-or-layer-3-completion"}) {
        nonclaims.push(item);
    }
    packet.set("proves", std::move(proves)).set("does_not_prove", std::move(nonclaims));
    return packet;
}

} // namespace

L2SelectionResult normalize_l2_selection_candidate(std::string_view legacy_json) {
    try {
        const auto parsed = core::json::Parser::parse(legacy_json);
        if (!parsed.ok()) {
            return {.value = std::nullopt,
                    .error = "could not read legacy selection JSON: " +
                        (parsed.errors.empty() ? std::string{"invalid JSON"} : parsed.errors.front().message)};
        }
        if (parsed.value->as_object() == nullptr) {
            return {.value = std::nullopt, .error = "legacy selection must contain one JSON object"};
        }
        return {.value = normalize(*parsed.value), .error = {}};
    } catch (const Rejected& rejected) {
        return {.value = std::nullopt, .error = rejected.message};
    }
}

L2SelectionResult bind_l2_selection_candidate(const L2SelectionBindingInputs& inputs) {
    try {
        return {.value = bind(inputs), .error = {}};
    } catch (const Rejected& rejected) {
        return {.value = std::nullopt, .error = rejected.message};
    }
}

std::string parse_l2_archive_artifacts(const std::vector<std::string>& values,
                                       std::map<std::uint32_t, std::filesystem::path>& out) {
    for (const auto& value : values) {
        const auto equals = value.find('=');
        if (equals == std::string::npos) return "--archive-artifact must use INDEX=PATH";
        const auto index_text = std::string_view{value}.substr(0U, equals);
        const auto path_text = std::string_view{value}.substr(equals + 1U);
        std::uint64_t index = 0U;
        const bool decimal = !index_text.empty() && index_text.size() <= 10U &&
            std::all_of(index_text.begin(), index_text.end(), [](char c) { return c >= '0' && c <= '9'; });
        if (!decimal) return "--archive-artifact INDEX must be decimal";
        for (const char c : index_text) index = index * 10U + static_cast<std::uint64_t>(c - '0');
        if (index > 0x7FFFFFFFU || path_text.empty() || out.contains(static_cast<std::uint32_t>(index))) {
            return "--archive-artifact index/path is invalid or duplicate";
        }
        out.emplace(static_cast<std::uint32_t>(index), std::filesystem::path{std::string{path_text}});
    }
    return {};
}

} // namespace dmc::rengine::spider
