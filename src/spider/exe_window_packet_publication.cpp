#include "dmc_rengine/spider/exe_window_packet_publication.hpp"

#include "dmc_rengine/core/no_replace_publication.hpp"
#include "dmc_rengine/core/sha256.hpp"

#include <locale>
#include <span>
#include <sstream>
#include <system_error>
#include <vector>

namespace dmc::rengine::spider {
namespace {

// Packet serialization owns only the packet schema. Child receipt semantics
// and encoding remain in exe::byte_window_receipt_to_json().
std::string json_string(std::string_view text) {
    constexpr char digits[] = "0123456789abcdef";
    std::string result{"\""};
    for (const unsigned char ch : text) {
        if (ch == '"' || ch == '\\') {
            result += '\\';
            result += static_cast<char>(ch);
        } else if (ch < 0x20U) {
            result += "\\u00";
            result += digits[ch >> 4U];
            result += digits[ch & 0x0FU];
        } else {
            result += static_cast<char>(ch);
        }
    }
    result += '"';
    return result;
}

std::string hex(std::uint64_t value) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "0x" << std::hex << value;
    return out.str();
}

std::string sha256(std::string_view text) {
    return core::Sha256::compute(
        std::as_bytes(std::span<const char>{text.data(), text.size()})).hex();
}

struct OwnedOutput final {
    std::filesystem::path path;
    bool published{};

    ~OwnedOutput() {
        if (!published) {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    }
};

core::NoReplacePublicationResult write_new(
    const std::filesystem::path& path,
    std::string_view text) {
    return core::publish_bytes_no_replace(
        path, std::as_bytes(std::span<const char>{text.data(), text.size()}));
}

std::string packet_json(
    const ExeWindowPacketProgram& program,
    const ExeWindowPacketExecution& execution,
    const std::vector<std::string>& child_hashes) {
    const auto& packet = program.packet;
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{\n  \"schema\": " << json_string(k_exe_window_packet_receipt_schema)
        << ",\n  \"status\": \"acquired\",\n  \"plan_id\": " << json_string(packet.id)
        << ",\n  \"plan_schema\": " << json_string(k_exe_window_packet_plan_schema)
        << ",\n  \"plan_receipt\": \"packet.plan.json\",\n  \"plan_sha256\": "
        << json_string(program.summary.plan_sha256)
        << ",\n  \"artifact_sha256\": " << json_string(packet.artifact_sha256)
        << ",\n  \"artifact_size\": " << packet.artifact_size
        << ",\n  \"authority_role\": " << json_string(packet.authority_role)
        << ",\n  \"raw_bytes_included\": false,\n  \"semantic_claim\": false,\n  \"windows\": [\n";
    for (std::size_t i = 0; i < packet.windows.size(); ++i) {
        const auto& request = packet.windows[i];
        const auto& receipt = execution.receipts[i];
        if (i != 0U) {
            out << ",\n";
        }
        out << "    {\n      \"id\": " << json_string(request.id)
            << ",\n      \"mode\": "
            << json_string(request.mode == ExeWindowMode::probe ? "probe" : "known-body")
            << ",\n      \"va\": " << json_string(hex(receipt.va))
            << ",\n      \"rva\": " << json_string(hex(receipt.rva))
            << ",\n      \"file_offset\": " << json_string(hex(receipt.file_offset))
            << ",\n      \"size\": " << receipt.size
            << ",\n      \"section\": " << json_string(receipt.section_name)
            << ",\n      \"issues\": [";
        for (std::size_t j = 0; j < request.issues.size(); ++j) {
            if (j != 0U) {
                out << ", ";
            }
            out << request.issues[j];
        }
        out << "],\n      \"purpose\": " << json_string(request.purpose)
            << ",\n      \"receipt\": " << json_string(request.id + ".receipt.json")
            << ",\n      \"receipt_schema\": \"dmc-rengine.exe-byte-window.v1\""
            << ",\n      \"receipt_sha256\": " << json_string(child_hashes[i])
            << ",\n      \"window_sha256\": " << json_string(receipt.window_sha256)
            << "\n    }";
    }
    out << "\n  ]\n}\n";
    return out.str();
}

} // namespace

std::string exe_window_packet_summary_to_json(
    const ExeWindowPacketSummary& summary) {
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{\n  \"schema\": " << json_string(k_exe_window_packet_validation_schema)
        << ",\n  \"status\": \"valid\",\n  \"plan_id\": " << json_string(summary.plan_id)
        << ",\n  \"plan_sha256\": " << json_string(summary.plan_sha256)
        << ",\n  \"artifact_sha256\": " << json_string(summary.artifact_sha256)
        << ",\n  \"artifact_size\": " << summary.artifact_size
        << ",\n  \"authority_role\": " << json_string(summary.authority_role)
        << ",\n  \"window_count\": " << summary.window_count
        << ",\n  \"probe_count\": " << summary.probe_count
        << ",\n  \"known_body_count\": " << summary.known_body_count
        << ",\n  \"semantic_claim\": false\n}\n";
    return out.str();
}

ExeWindowPacketPublication publish_exe_window_packet(
    std::string_view plan_json,
    std::string_view expected_artifact_sha256,
    const std::filesystem::path& output_directory,
    ExeWindowAcquireFn acquire,
    void* context) {
    ExeWindowPacketPublication result;
    const auto compiled = compile_exe_window_packet(plan_json);
    if (!compiled.ok()) {
        result.error = ExeWindowPacketPublicationError::invalid_plan;
        for (const auto& error : compiled.errors) {
            result.message += error + '\n';
        }
        return result;
    }

    // Finish authority/receipt validation before creating any output files.
    const auto& program = *compiled.program;
    const auto execution = execute_exe_window_packet(
        program, expected_artifact_sha256, acquire, context);
    if (!execution.ok()) {
        result.error = ExeWindowPacketPublicationError::execution_failed;
        result.execution_error = execution.error;
        result.failed_window = execution.failed_window;
        result.message = execution.message;
        return result;
    }

    std::error_code error;
    const auto output = std::filesystem::absolute(output_directory, error);
    if (output_directory.empty() || error) {
        result.error = ExeWindowPacketPublicationError::output_unavailable;
        result.message = "output directory must be a non-empty usable path";
        return result;
    }
    std::filesystem::create_directories(output.parent_path(), error);
    if (error || !std::filesystem::create_directory(output, error) || error) {
        result.error = ExeWindowPacketPublicationError::output_unavailable;
        result.message = "cannot reserve output directory; existing outputs are never replaced";
        return result;
    }
    OwnedOutput owned{output};
    const auto write = [&](const std::filesystem::path& path, std::string_view text) {
        const auto written = write_new(path, text);
        if (!written.ok()) {
            result.error = ExeWindowPacketPublicationError::write_failed;
            result.message = written.detail;
            return false;
        }
        return true;
    };

    if (!write(output / "packet.plan.json", plan_json)) {
        return result;
    }
    std::vector<std::string> child_hashes;
    child_hashes.reserve(execution.receipts.size());
    for (std::size_t i = 0; i < execution.receipts.size(); ++i) {
        const auto encoded = exe::byte_window_receipt_to_json(execution.receipts[i]);
        if (encoded.empty()) {
            result.error = ExeWindowPacketPublicationError::write_failed;
            result.message = "canonical child receipt serialization failed";
            return result;
        }
        if (!write(output / (program.packet.windows[i].id + ".receipt.json"), encoded)) {
            return result;
        }
        child_hashes.push_back(sha256(encoded));
    }
    const auto receipt_path = output / "packet.receipt.json";
    if (!write(receipt_path, packet_json(program, execution, child_hashes))) {
        return result;
    }
    result.receipt_path = receipt_path;
    owned.published = true;
    return result;
}

} // namespace dmc::rengine::spider
