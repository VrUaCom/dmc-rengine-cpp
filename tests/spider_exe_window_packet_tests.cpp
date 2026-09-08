#include "dmc_rengine/spider/exe_window_packet.hpp"

#include <cassert>
#include <cstddef>
#include <string>

namespace {

constexpr const char* k_sha =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";
constexpr const char* k_window_sha =
    "e61d6a793b42951d4e466a18683567c9011cd840b03559c0cc9e94c761995098";

struct Fixture final {
    bool fail{};
    bool wrong_schema_equivalent{};
};

dmc::rengine::spider::ExeWindowAcquisition acquire(
    void* context,
    const dmc::rengine::spider::ExeWindowRequest& request) {
    auto* fixture = static_cast<Fixture*>(context);
    if (fixture != nullptr && fixture->fail) {
        return {.receipt = std::nullopt, .error = "fixture failure"};
    }

    dmc::rengine::exe::ExeByteWindowReceipt receipt{
        .artifact_sha256 = k_sha,
        .artifact_size = 4096U,
        .image_base = 0x140000000ULL,
        .va = request.va,
        .rva = 0x1000U,
        .file_offset = 0x200U,
        .size = static_cast<std::uint64_t>(request.size),
        .section_name = ".text",
        .window_sha256 = k_window_sha,
    };
    if (fixture != nullptr && fixture->wrong_schema_equivalent) {
        // The native path eliminates JSON schema spoofing. Model the same
        // guardrail as a typed receipt mismatch instead.
        receipt.artifact_size = 4095U;
    }
    return {.receipt = receipt, .error = {}};
}

std::string make_plan(std::string mode, std::string body = {}) {
    std::string json = R"({
  "schema": "dmc-rengine.exe-window-packet-plan.v1",
  "id": "test-l3-writer-plan",
  "artifact_sha256": ")";
    json += k_sha;
    json += R"(",
  "artifact_size": 4096,
  "authority_role": "analysis-reverse",
  "window_size_policy": "Probe coverage only; not a body-boundary assertion.",
  "windows": [
    {
      "id": "writer-probe",
      "va": "0x140001000",
      "size": "0x4",
      "mode": ")";
    json += mode;
    json += "\",";
    if (!body.empty()) {
        json += "\n      \"body_sha256\": \"" + body + "\",";
    }
    json += R"(
      "issues": [88],
      "purpose": "Synthetic guardrail coverage only."
    }
  ]
}
)";
    return json;
}

} // namespace

int main() {
    using namespace dmc::rengine::spider;

    const auto compiled = compile_exe_window_packet(make_plan("probe"));
    assert(compiled.ok());
    assert(compiled.program->summary.window_count == 1U);
    assert(compiled.program->summary.probe_count == 1U);
    assert(compiled.program->summary.known_body_count == 0U);
    assert(!compiled.program->summary.semantic_claim);
    assert(compiled.program->execution.size() == 4U);
    assert(compiled.program->execution.instructions[1].op == OpCode::acquire_window);
    assert(compiled.program->execution.instructions[2].op == OpCode::validate_window);

    Fixture fixture;
    const auto executed = execute_exe_window_packet(
        *compiled.program, k_sha, acquire, &fixture);
    assert(executed.ok());
    assert(executed.receipts.size() == 1U);
    assert(executed.receipts.front().section_name == ".text");

    fixture.fail = true;
    const auto failed = execute_exe_window_packet(
        *compiled.program, k_sha, acquire, &fixture);
    assert(failed.error == ExeWindowPacketError::acquisition_failed);
    fixture.fail = false;

    fixture.wrong_schema_equivalent = true;
    const auto mismatched = execute_exe_window_packet(
        *compiled.program, k_sha, acquire, &fixture);
    assert(mismatched.error == ExeWindowPacketError::invalid_receipt);
    fixture.wrong_schema_equivalent = false;

    const auto known = compile_exe_window_packet(make_plan("known-body", k_window_sha));
    assert(known.ok());
    const auto known_executed = execute_exe_window_packet(
        *known.program, k_sha, acquire, &fixture);
    assert(known_executed.ok());

    const auto wrong_body = compile_exe_window_packet(make_plan(
        "known-body",
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    assert(wrong_body.ok());
    const auto body_mismatch = execute_exe_window_packet(
        *wrong_body.program, k_sha, acquire, &fixture);
    assert(body_mismatch.error == ExeWindowPacketError::known_body_mismatch);

    auto unsafe = make_plan("probe");
    const auto position = unsafe.find("writer-probe");
    assert(position != std::string::npos);
    unsafe.replace(position, std::string{"writer-probe"}.size(), "../unsafe");
    assert(!compile_exe_window_packet(unsafe).ok());

    return 0;
}
