#include "dmc_rengine/spider/exe_window_packet_publication.hpp"
#include "dmc_rengine/core/json.hpp"
#include "dmc_rengine/core/sha256.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>

namespace {
namespace spider = dmc::rengine::spider;
namespace core = dmc::rengine::core;
constexpr const char* k_sha =
    "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

std::string read(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    assert(in);
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

std::string hash(std::string_view text) {
    return core::Sha256::compute(
        std::as_bytes(std::span<const char>{text.data(), text.size()})).hex();
}

std::string plan(std::string_view id = "probe") {
    return std::string{R"({
      "schema":"dmc-rengine.exe-window-packet-plan.v1",
      "id":"packet \"test\"\nline",
      "artifact_sha256":")"} + k_sha + R"(",
      "artifact_size":4096,
      "authority_role":"synthetic-only",
      "window_size_policy":"probe",
      "windows":[{"id":")" + std::string{id} + R"(","va":"0x140001000",
        "size":4,"mode":"probe","issues":[88],"purpose":"test\tcontrol"}]
    }
    )";
}

struct Source {
    std::size_t calls{};
    bool fail{};
    bool invalid{};
    bool throws{};
};

spider::ExeWindowAcquisition acquire(void* context, const spider::ExeWindowRequest& request) {
    auto& source = *static_cast<Source*>(context);
    ++source.calls;
    if (source.throws) throw std::runtime_error("injected acquisition exception");
    if (source.fail) return {.receipt = std::nullopt, .error = "injected failure"};
    return {
        .receipt = dmc::rengine::exe::ExeByteWindowReceipt{
            .artifact_sha256 = k_sha,
            .artifact_size = source.invalid ? 4095U : 4096U,
            .image_base = 0x140000000ULL,
            .va = request.va,
            .rva = 0x1000U,
            .file_offset = 0x200U,
            .size = request.size,
            .section_name = ".text",
            .window_sha256 = std::string(64U, 'a'),
        },
        .error = {},
    };
}
}

int main() {
    const auto root = std::filesystem::temp_directory_path() /
        ("dmc-spider-publication-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    assert(std::filesystem::create_directory(root));
    struct Cleanup {
        std::filesystem::path path;
        ~Cleanup() { std::error_code error; std::filesystem::remove_all(path, error); }
    } cleanup{root};

    Source source;
    const auto raw = plan();
    const auto output = root / "new-parent" / "packet";
    const auto published = spider::publish_exe_window_packet(raw, k_sha, output, acquire, &source);
    assert(published.ok());
    assert(source.calls == 1U);
    assert(published.receipt_path == output / "packet.receipt.json");
    assert(read(output / "packet.plan.json") == raw);
    const auto child = read(output / "probe.receipt.json");
    const auto manifest = core::json::Parser::parse(read(published.receipt_path));
    assert(manifest.ok());
    const auto& object = *manifest.value->as_object();
    assert(*object.at("plan_sha256").as_string() == hash(raw));
    assert(!*object.at("raw_bytes_included").as_bool());
    assert(!*object.at("semantic_claim").as_bool());
    assert(*object.at("plan_id").as_string() == "packet \"test\"\nline");
    const auto& window = *object.at("windows").as_array()->front().as_object();
    assert(*window.at("receipt_sha256").as_string() == hash(child));
    assert(*window.at("purpose").as_string() == "test\tcontrol");
    assert(child.find("bytes_hex") == std::string::npos);
    assert(std::distance(std::filesystem::directory_iterator(output),
                         std::filesystem::directory_iterator{}) == 3);

    // A complete existing packet and arbitrary foreign output are preserved.
    const auto original_manifest = read(published.receipt_path);
    assert(!spider::publish_exe_window_packet(raw, k_sha, output, acquire, &source).ok());
    assert(read(published.receipt_path) == original_manifest);
    const auto occupied = root / "occupied";
    assert(std::filesystem::create_directory(occupied));
    { std::ofstream file(occupied / "sentinel"); file << "foreign"; }
    assert(!spider::publish_exe_window_packet(raw, k_sha, occupied, acquire, &source).ok());
    assert(read(occupied / "sentinel") == "foreign");

    // Plan/authority errors never call the acquisition node or create output.
    const auto rejected = root / "rejected";
    const auto calls = source.calls;
    assert(!spider::publish_exe_window_packet("{}", k_sha, rejected, acquire, &source).ok());
    const auto wrong_sha = spider::publish_exe_window_packet(
        raw, std::string(64U, 'b'), rejected, acquire, &source);
    assert(wrong_sha.execution_error == spider::ExeWindowPacketError::expected_sha_mismatch);
    assert(source.calls == calls);
    assert(!std::filesystem::exists(rejected));

    source.fail = true;
    assert(!spider::publish_exe_window_packet(raw, k_sha, rejected, acquire, &source).ok());
    assert(!std::filesystem::exists(rejected));
    source.fail = false;
    source.invalid = true;
    const auto invalid = spider::publish_exe_window_packet(raw, k_sha, rejected, acquire, &source);
    assert(invalid.execution_error == spider::ExeWindowPacketError::invalid_receipt);
    assert(!std::filesystem::exists(rejected));
    source.invalid = false;
    source.throws = true;
    try {
        (void)spider::publish_exe_window_packet(raw, k_sha, rejected, acquire, &source);
        assert(false);
    } catch (const std::runtime_error&) {
        assert(!std::filesystem::exists(rejected));
    }
    source.throws = false;

    // The legacy plan accepts 'packet' as a child ID. It collides with the
    // final receipt: fail without replacement and roll back the owned output,
    // including the plan and child already written before that final failure.
    const auto collision = root / "collision";
    const auto failed_write = spider::publish_exe_window_packet(
        plan("packet"), k_sha, collision, acquire, &source);
    assert(failed_write.error == spider::ExeWindowPacketPublicationError::write_failed);
    assert(!std::filesystem::exists(collision));
    assert(read(occupied / "sentinel") == "foreign");

    const auto blocker = root / "file";
    { std::ofstream file(blocker); file << "keep"; }
    assert(!spider::publish_exe_window_packet(raw, k_sha, blocker / "packet", acquire, &source).ok());
    assert(read(blocker) == "keep");
    assert(!spider::publish_exe_window_packet(raw, k_sha, {}, acquire, &source).ok());
}
