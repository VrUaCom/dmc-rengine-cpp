// The L2 original-selection chain, natively. Carries over the guardrails of
// scripts/reverse/test_normalize_l2_original_selection_candidate.py and
// test_verify_l2_original_selection_evidence.py, and pins Python's json.dumps
// output for the Tarantula writer they share.

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/spider/l2_original_selection.hpp"
#include "dmc_rengine/spider/python_json.hpp"
#include "spider_l2_runtime_mapping_test_cases.hpp"

#include <cassert>
#include <chrono>
#include <map>
#include <optional>
#include <tuple>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>

namespace fs = std::filesystem;
namespace spider = dmc::rengine::spider;
using spider::OrderedJson;

namespace {

void python_json() {
    // json.dumps({"a": [1, 2.5, None, True], "b": {}, "c": [], "é": "x\u00e9\U0001f600\x7f\n"}, indent=2)
    auto value = OrderedJson::object();
    value.set("a", OrderedJson::array().push(1).push(2.5).push(nullptr).push(true))
        .set("b", OrderedJson::object())
        .set("c", OrderedJson::array())
        .set("\xC3\xA9", "x\xC3\xA9\xF0\x9F\x98\x80\x7F\n");
    assert(spider::dump_python(value) ==
           "{\n  \"a\": [\n    1,\n    2.5,\n    null,\n    true\n  ],\n  \"b\": {},\n  \"c\": [],\n"
           "  \"\\u00e9\": \"x\\u00e9\\ud83d\\ude00\\u007f\\n\"\n}");
    // repr() of floats: positional up to 1e16, then scientific.
    assert(spider::dump_python(OrderedJson{1.0}) == "1.0");
    assert(spider::dump_python(OrderedJson{1e16}) == "1e+16");
    assert(spider::dump_python(OrderedJson{1.5e-05}) == "1.5e-05");
    assert(spider::dump_python(OrderedJson{0.1}) == "0.1");
    assert(spider::dump_python(OrderedJson{-0.0}) == "-0.0");
    assert(spider::dump_python(OrderedJson{std::uint64_t{18446744073709551615ULL}}) == "18446744073709551615");
    assert(spider::python_length("a\xC3\xA9\xF0\x9F\x98\x80") == 3U);

    const auto parse = [](std::string_view text) { return *dmc::rengine::core::json::Parser::parse(text).value; };
    assert(spider::json_equal(parse(R"({"a":[1,{"b":null}]})"), parse(R"({ "a" : [1.0, {"b": null}] })")));
    assert(!spider::json_equal(parse("[true]"), parse("[1]")));
    assert(!spider::json_equal(parse(R"({"a":1})"), parse(R"({"a":1,"b":2})")));
    assert(spider::contains_key(parse(R"({"x":[{"y":{"bytes_hex":"00"}}]})"), "bytes_hex"));
}

/// The legacy selection the normalizer test used, as Python wrote it.
OrderedJson legacy() {
    auto probe = OrderedJson::object();
    probe.set("sequence_index", 0).set("lookup_attempt_index", 6).set("provider", "physical")
        .set("candidate", "GDataX360.afs/st001.pac").set("provider_key", "GDataX360.afs\\st001.pac")
        .set("archive_volume_index", nullptr).set("outcome", "selected");
    auto selected = OrderedJson::object();
    selected.set("provider", "physical").set("lookup_attempt_index", 6).set("candidate", "GDataX360.afs/st001.pac")
        .set("provider_key", "GDataX360.afs\\st001.pac").set("archive_volume_index", nullptr)
        .set("archive_member_path", "").set("physical_relative_path", "GDataX360.afs/st001.pac");
    auto value = OrderedJson::object();
    value.set("schema", spider::k_l2_legacy_selection_schema)
        .set("evidence_class", "original-process-observation")
        .set("executable_sha256", std::string(64U, '1')).set("executable_size", 1)
        .set("runtime_mapping_packet_sha256", std::string(64U, '2'))
        .set("observer_id", "observer").set("observer_version", "v1")
        .set("observer_build_sha256", std::string(64U, '3'))
        .set("trace_complete", true).set("dropped_event_count", 0).set("pid", 1).set("module_base", "0x1")
        .set("flags", 1).set("request", "scr\\st001.pac").set("basename", "st001.pac")
        .set("first_missing_archive_volume", 0).set("archives", OrderedJson::array())
        .set("probes", OrderedJson::array().push(probe)).set("selected", selected)
        .set("proves", OrderedJson::array()
                           .push("original-process-provider-traversal-prefix")
                           .push("original-process-selected-resource-identity"))
        .set("does_not_prove", OrderedJson::array().push("retail-archive-collision-freedom"));
    return value;
}

/// `value` with member `key` replaced (or added), the way the Python tests copied dicts.
OrderedJson with(OrderedJson value, const std::string& key, OrderedJson replacement) {
    for (auto& [name, item] : std::get<OrderedJson::Members>(value.data)) {
        if (name == key) {
            item = std::move(replacement);
            return value;
        }
    }
    value.set(key, std::move(replacement));
    return value;
}

OrderedJson without(OrderedJson value, const std::string& key) {
    auto& members = std::get<OrderedJson::Members>(value.data);
    std::erase_if(members, [&key](const auto& member) { return member.first == key; });
    return value;
}

OrderedJson& member(OrderedJson& value, const std::string& key) {
    for (auto& [name, item] : std::get<OrderedJson::Members>(value.data)) {
        if (name == key) return item;
    }
    std::abort();
}

void normalizer_rejects(const OrderedJson& value, std::string_view contains) {
    const auto result = spider::normalize_l2_selection_candidate(spider::dump_python(value));
    assert(!result.ok());
    if (result.error.find(contains) == std::string::npos) {
        std::cerr << "expected '" << contains << "' in '" << result.error << "'\n";
        std::abort();
    }
}

void normalizer() {
    const auto result = spider::normalize_l2_selection_candidate(spider::dump_python(legacy()));
    assert(result.ok());
    const auto text = spider::dump_python(*result.value);
    assert(text.find("\"schema\": \"dmc-rengine.gdspaces-l2-original-selection-candidate.v1\"") != std::string::npos);
    assert(text.find("\"promotion_eligible\": false,\n  \"trusted_capture_bound\": false,\n"
                     "  \"legacy_schema_normalized\": true") != std::string::npos);
    assert(text.find("original-process-selected-resource-identity") == std::string::npos);
    assert(text.find("\"does_not_prove\": [\n    \"retail-archive-collision-freedom\",\n"
                     "    \"trusted-observer-execution-or-trace-origin\",\n"
                     "    \"original-process-selected-provider-identity\",\n"
                     "    \"promotion-eligibility\"\n  ]") != std::string::npos);
    // Selected and probe fields come out in the recovered order, whatever order they came in.
    assert(text.find("\"selected\": {\n    \"provider\": \"physical\",\n    \"lookup_attempt_index\": 6,") !=
           std::string::npos);
    // Determinism: the same input gives the same bytes.
    assert(spider::dump_python(*spider::normalize_l2_selection_candidate(spider::dump_python(legacy())).value) == text);

    normalizer_rejects(with(legacy(), "schema", "other"), "schema mismatch");
    normalizer_rejects(with(legacy(), "evidence_class", "other"), "evidence class mismatch");
    normalizer_rejects(with(legacy(), "promotion_eligible", true), "unsupported field(s): promotion_eligible");
    normalizer_rejects(with(legacy(), "trusted_origin", true), "unsupported field(s): trusted_origin");
    auto nested_extra = legacy();
    member(nested_extra, "selected").set("trusted_origin", true);
    normalizer_rejects(nested_extra, "legacy selected contains unsupported field(s): trusted_origin");
    auto nested_raw = legacy();
    member(nested_raw, "selected").set("debug", OrderedJson::object().set("bytes_hex", "00"));
    normalizer_rejects(nested_raw, "forbidden raw bytes_hex");
    normalizer_rejects(without(legacy(), "observer_build_sha256"), "missing required field observer_build_sha256");
    assert(!spider::normalize_l2_selection_candidate("[]").ok());
    assert(!spider::normalize_l2_selection_candidate("{").ok());
}

std::string sha(std::string_view bytes) {
    return dmc::rengine::core::Sha256::compute(std::as_bytes(std::span<const char>{bytes.data(), bytes.size()})).hex();
}

std::string write(const fs::path& path, std::string_view text) {
    std::ofstream{path, std::ios::binary | std::ios::trunc} << text;
    return std::string{text};
}

std::string write_json(const fs::path& path, const OrderedJson& value) {
    return write(path, spider::dump_python(value) + "\n");
}

OrderedJson candidate(const std::string& mapping_sha, const std::string& observer_sha, const std::string& a0_sha,
                      int a0_size, const std::string& a1_sha, int a1_size) {
    const auto archive = [](int index, const std::string& digest, int size) {
        return OrderedJson::object().set("volume_index", index).set("filename", "DMC3-" + std::to_string(index) + ".nbz")
            .set("sha256", digest).set("size", size);
    };
    auto value = OrderedJson::object();
    value.set("schema", spider::k_l2_selection_candidate_schema)
        .set("evidence_class", "original-process-observation-candidate")
        .set("promotion_eligible", false).set("trusted_capture_bound", false).set("legacy_schema_normalized", true)
        .set("executable_sha256", dmc::rengine::tests::k_mapping_protected_sha).set("executable_size", 6567320)
        .set("runtime_mapping_packet_sha256", mapping_sha)
        .set("observer_id", "dmc-rengine-l2-observer").set("observer_version", "synthetic-contract-test")
        .set("observer_build_sha256", observer_sha)
        .set("trace_complete", true).set("dropped_event_count", 0).set("pid", 4242)
        .set("module_base", "0x7FF600000000").set("flags", 1)
        .set("request", "scr\\st001.pac").set("basename", "st001.pac")
        .set("first_missing_archive_volume", 2)
        .set("archives", OrderedJson::array().push(archive(0, a0_sha, a0_size)).push(archive(1, a1_sha, a1_size)))
        .set("probes", OrderedJson::array().push(
            OrderedJson::object().set("sequence_index", 0).set("lookup_attempt_index", 0).set("provider", "archive")
                .set("candidate", "GDataX360.afs/st001.pac").set("provider_key", "gdatax360.afs\\st001.pac")
                .set("archive_volume_index", 1).set("outcome", "selected")))
        .set("selected", OrderedJson::object().set("provider", "archive").set("lookup_attempt_index", 0)
                             .set("candidate", "GDataX360.afs/st001.pac").set("provider_key", "gdatax360.afs\\st001.pac")
                             .set("archive_volume_index", 1).set("archive_member_path", "GDataX360.afs/ST001.PAC")
                             .set("physical_relative_path", ""))
        .set("proves", OrderedJson::array().push("self-authored-selection-content-has-candidate-shape-only"))
        .set("does_not_prove", OrderedJson::array()
                                   .push("trusted-observer-execution-or-trace-origin")
                                   .push("original-process-selected-provider-identity")
                                   .push("promotion-eligibility"));
    return value;
}

void binder() {
    const auto root = fs::temp_directory_path() / ("dmc-l2-selection-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::remove_all(root);
    fs::create_directories(root);

    std::vector<fs::path> children;
    for (const auto& [name, rva, digit] : {std::tuple{"open-game.json", 0x0002FCA0ULL, '1'},
                                          std::tuple{"resolve.json", 0x00327430ULL, '2'},
                                          std::tuple{"final-open.json", 0x00327800ULL, '3'}}) {
        children.push_back(root / name);
        write(children.back(), dmc::rengine::tests::make_mapping_receipt(rva, digit));
    }
    std::vector<std::string> receipts;
    for (const auto& child : children) {
        std::ifstream stream{child, std::ios::binary};
        receipts.emplace_back(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{});
    }
    const auto built = dmc::rengine::tests::build_mapping(receipts);
    assert(built.ok());
    const auto mapping_path = root / "mapping.json";
    const auto mapping_sha = sha(write(mapping_path, spider::l2_runtime_mapping_v1_to_json(*built.packet)));

    const auto observer = root / "observer.bin";
    const auto observer_sha = sha(write(observer, "observer-build-v1"));
    const auto archive0 = root / "DMC3-0.nbz";
    const auto archive1 = root / "DMC3-1.nbz";
    const auto a0 = sha(write(archive0, "archive-zero"));
    const auto a1 = sha(write(archive1, "archive-one"));
    const auto valid = candidate(mapping_sha, observer_sha, a0, 12, a1, 11);
    const auto selection_path = root / "selection.json";
    write_json(selection_path, valid);

    const auto inputs = [&](const fs::path& mapping, const fs::path& selection,
                            std::map<std::uint32_t, fs::path> archives) {
        return spider::L2SelectionBindingInputs{.mapping = mapping, .selection = selection,
                                                .mapping_children = children, .observer_artifact = observer,
                                                .archive_artifacts = std::move(archives)};
    };
    const std::map<std::uint32_t, fs::path> archives{{0U, archive0}, {1U, archive1}};

    const auto bound = spider::bind_l2_selection_candidate(inputs(mapping_path, selection_path, archives));
    if (!bound.ok()) std::cerr << bound.error << '\n';
    assert(bound.ok());
    const auto packet = spider::dump_python(*bound.value);
    assert(packet.find("\"status\": \"bound_candidate\"") != std::string::npos);
    assert(packet.find("\"runtime_mapping_packet_sha256\": \"" + mapping_sha + "\"") != std::string::npos);
    assert(packet.find("\"rva\": \"0x2FCA0\"") < packet.find("\"rva\": \"0x327430\""));
    assert(packet.find("\"sha256\": \"" + observer_sha + "\",\n    \"size\": 17") != std::string::npos);
    assert(packet.find("\"module_base\": \"0x7FF600000000\"") != std::string::npos);
    assert(packet.find("\"archive_volume_index\": 1,\n    \"archive_member_path\"") != std::string::npos);

    int case_number = 0;
    const auto rejects = [&](const OrderedJson& selection, std::string_view contains,
                             std::map<std::uint32_t, fs::path> artifacts, std::optional<fs::path> mapping = {}) {
        const auto path = root / ("case-" + std::to_string(++case_number) + ".json");
        write_json(path, selection);
        const auto result = spider::bind_l2_selection_candidate(inputs(mapping.value_or(mapping_path), path, artifacts));
        assert(!result.ok());
        if (result.error.find(contains) == std::string::npos) {
            std::cerr << "expected '" << contains << "' in '" << result.error << "'\n";
            std::abort();
        }
    };
    rejects(with(valid, "runtime_mapping_packet_sha256", std::string(64U, '0')), "does not hash-bind", archives);
    rejects(with(valid, "promotion_eligible", true), "may not predeclare promotion eligibility", archives);
    rejects(with(valid, "trusted_capture_bound", true), "may not predeclare trusted capture", archives);
    rejects(with(valid, "legacy_schema_normalized", false), "lacks legacy-normalizer provenance marker", archives);
    rejects(with(valid, "trusted_origin", true), "unsupported field(s): trusted_origin", archives);
    auto extra_selected = valid;
    member(extra_selected, "selected").set("trusted_origin", true);
    rejects(extra_selected, "selected identity contains unsupported field(s): trusted_origin", archives);
    rejects(with(valid, "observer_build_sha256", std::string(64U, 'f')), "observer artifact SHA", archives);

    const auto tampered = root / "tampered-DMC3-1.nbz";
    write(tampered, "tampered");
    rejects(valid, "does not match claimed SHA/size", {{0U, archive0}, {1U, tampered}});
    rejects(valid, "does not exactly match selection volume set", {{1U, archive1}});

    auto backend_failure = valid;
    member(std::get<OrderedJson::Array>(member(backend_failure, "probes").data)[0], "outcome") = "provider_failure";
    rejects(backend_failure, "provider/backend failure is fail-closed", archives);
    rejects(with(valid, "trace_complete", false), "not marked complete", archives);
    rejects(with(valid, "dropped_event_count", 1), "dropped events", archives);
    auto nested_raw = valid;
    member(nested_raw, "selected").set("debug", OrderedJson::object().set("bytes_hex", "00"));
    rejects(nested_raw, "forbidden raw bytes_hex", archives);
    // Python's True == 1 let a bool through; the port does not.
    rejects(with(valid, "flags", true), "flags=1", archives);
    rejects(with(valid, "executable_size", 6567320.0), "executable size mismatch", archives);

    // A mapping that does not rebuild from its children is refused, even when the selection binds it.
    auto forged = spider::l2_runtime_mapping_v1_to_json(*built.packet);
    const auto first_window = forged.find(std::string(64U, '1'));
    forged.replace(first_window, 64U, std::string(64U, 'f'));
    const auto forged_path = root / "forged-mapping.json";
    const auto forged_sha = sha(write(forged_path, forged));
    rejects(with(valid, "runtime_mapping_packet_sha256", forged_sha), "does not exactly match reconstruction",
            archives, forged_path);

    // Too few children.
    auto short_inputs = inputs(mapping_path, selection_path, archives);
    short_inputs.mapping_children.pop_back();
    assert(spider::bind_l2_selection_candidate(short_inputs).error ==
           "at least 3 mapping child receipts are required");

    std::map<std::uint32_t, fs::path> parsed;
    assert(spider::parse_l2_archive_artifacts({"0=/a", "1=/b"}, parsed).empty() && parsed.size() == 2U);
    parsed.clear();
    assert(spider::parse_l2_archive_artifacts({"0"}, parsed) == "--archive-artifact must use INDEX=PATH");
    assert(spider::parse_l2_archive_artifacts({"x=/a"}, parsed) == "--archive-artifact INDEX must be decimal");
    parsed.clear();
    assert(spider::parse_l2_archive_artifacts({"0=/a", "0=/b"}, parsed) ==
           "--archive-artifact index/path is invalid or duplicate");
    parsed.clear();
    assert(spider::parse_l2_archive_artifacts({"0="}, parsed) == "--archive-artifact index/path is invalid or duplicate");

    fs::remove_all(root);
}

} // namespace

int main() {
    python_json();
    normalizer();
    binder();
    std::cout << "spider_l2_original_selection_tests: ok\n";
    return 0;
}
