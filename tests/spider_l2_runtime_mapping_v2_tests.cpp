#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/spider/l2_runtime_mapping_v2.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::uint64_t k_image_base = 0x140000000ULL;
constexpr std::uint64_t k_module_base = 0x7FF600000000ULL;
constexpr std::uint64_t k_creation = 133800000000000000ULL;
constexpr std::uint64_t k_window_size = 0x40U;
constexpr std::uint32_t k_text_rva = 0x1000U;
constexpr std::uint32_t k_raw_pointer = 0x200U;
constexpr std::uint32_t k_raw_size = 0x330000U;
constexpr std::string_view k_protected_sha =
    "81c7e61983564113b5105e931d9f185accc14e44ae147d27f720c2d50935c7d6";

constexpr std::array<std::uint64_t, 7> k_anchors{
    0x0002FCA0U,
    0x00326D20U,
    0x00326DA0U,
    0x00327430U,
    0x00327800U,
    0x00328160U,
    0x00328290U,
};

void write_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>((value >> (index * 8U)) & 0xFFU);
    }
}

void write_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>((value >> (index * 8U)) & 0xFFU);
    }
}

std::string hex_u64(std::uint64_t value) {
    constexpr char digits[] = "0123456789ABCDEF";
    char buffer[16]{};
    std::size_t count = 0U;
    do {
        buffer[count++] = digits[value & 0xFU];
        value >>= 4U;
    } while (value != 0U);
    std::string result{"0x"};
    while (count != 0U) {
        result.push_back(buffer[--count]);
    }
    return result;
}

struct CanonicalFixture final {
    std::vector<std::byte> bytes;
    std::array<std::string, k_anchors.size()> hashes;
    dmc::rengine::spider::L2RuntimeMappingV2Authority authority;
};

CanonicalFixture make_canonical_fixture() {
    CanonicalFixture fixture;
    fixture.bytes.resize(static_cast<std::size_t>(k_raw_pointer) + k_raw_size, std::byte{0});

    fixture.bytes[0] = std::byte{'M'};
    fixture.bytes[1] = std::byte{'Z'};
    write_u32(fixture.bytes, 0x3CU, 0x80U);

    constexpr std::size_t pe = 0x80U;
    fixture.bytes[pe + 0U] = std::byte{'P'};
    fixture.bytes[pe + 1U] = std::byte{'E'};
    write_u16(fixture.bytes, pe + 4U, 0x8664U);
    write_u16(fixture.bytes, pe + 6U, 1U);
    write_u16(fixture.bytes, pe + 20U, 0xF0U);

    constexpr std::size_t optional = pe + 24U;
    write_u16(fixture.bytes, optional + 0U, 0x20BU);
    write_u32(fixture.bytes, optional + 16U, k_text_rva);
    write_u64(fixture.bytes, optional + 24U, k_image_base);
    write_u32(fixture.bytes, optional + 56U, 0x340000U);
    write_u32(fixture.bytes, optional + 60U, k_raw_pointer);

    constexpr std::size_t section = optional + 0xF0U;
    fixture.bytes[section + 0U] = std::byte{'.'};
    fixture.bytes[section + 1U] = std::byte{'t'};
    fixture.bytes[section + 2U] = std::byte{'e'};
    fixture.bytes[section + 3U] = std::byte{'x'};
    fixture.bytes[section + 4U] = std::byte{'t'};
    write_u32(fixture.bytes, section + 8U, k_raw_size);
    write_u32(fixture.bytes, section + 12U, k_text_rva);
    write_u32(fixture.bytes, section + 16U, k_raw_size);
    write_u32(fixture.bytes, section + 20U, k_raw_pointer);

    for (std::size_t index = 0; index < k_anchors.size(); ++index) {
        const auto rva = k_anchors[index];
        const auto file_offset = static_cast<std::size_t>(k_raw_pointer + (rva - k_text_rva));
        const auto value = static_cast<std::byte>(index + 1U);
        for (std::size_t offset = 0; offset < k_window_size; ++offset) {
            fixture.bytes[file_offset + offset] = value;
        }
        fixture.hashes[index] = dmc::rengine::core::Sha256::compute(
            std::span<const std::byte>{fixture.bytes}.subspan(file_offset, k_window_size)).hex();
    }

    fixture.authority.canonical_analysis_size = fixture.bytes.size();
    fixture.authority.canonical_analysis_sha256 =
        dmc::rengine::core::Sha256::compute(fixture.bytes).hex();
    fixture.authority.preferred_image_base = k_image_base;
    return fixture;
}

std::string make_receipt(
    std::uint64_t rva,
    std::string_view window_sha,
    std::uint64_t creation = k_creation,
    std::string_view schema = "dmc-rengine.exe-process-window.v2",
    bool include_expectation = false,
    std::string_view expected_window = {}) {
    std::string json =
        "{\n"
        "  \"schema\": \"" + std::string(schema) + "\",\n"
        "  \"artifact_sha256\": \"" + std::string(k_protected_sha) + "\",\n"
        "  \"artifact_size\": 6567320,\n"
        "  \"image_path\": \"C:/Users/LocalUser/Games/DMC3/dmc3.exe\",\n"
        "  \"preferred_image_base\": \"0x140000000\",\n"
        "  \"pid\": 4242,\n"
        "  \"process_creation_filetime\": " + std::to_string(creation) + ",\n"
        "  \"module_base\": \"" + hex_u64(k_module_base) + "\",\n"
        "  \"rva\": \"" + hex_u64(rva) + "\",\n"
        "  \"runtime_va\": \"" + hex_u64(k_module_base + rva) + "\",\n"
        "  \"size\": 64,\n"
        "  \"section\": \".text\",\n"
        "  \"window_sha256\": \"" + std::string(window_sha) + "\"";
    if (include_expectation) {
        const auto diagnostic = expected_window.empty() ? window_sha : expected_window;
        json +=
            ",\n  \"expected_window_artifact_sha256\": \"PLACEHOLDER_CANONICAL_SHA\",\n"
            "  \"expected_window_sha256\": \"" + std::string(diagnostic) + "\",\n"
            "  \"matches_expected_window\": true";
    }
    json += "\n}\n";
    return json;
}

void replace_once(std::string& text, std::string_view from, std::string_view to) {
    const auto position = text.find(from);
    assert(position != std::string::npos);
    text.replace(position, from.size(), to);
}

struct ReceiptSet final {
    std::vector<std::string> storage;
    std::vector<std::string_view> views;
};

ReceiptSet make_valid_receipts(const CanonicalFixture& fixture) {
    ReceiptSet result;
    result.storage.reserve(k_anchors.size());
    for (std::size_t index = 0; index < k_anchors.size(); ++index) {
        result.storage.push_back(make_receipt(k_anchors[index], fixture.hashes[index]));
    }
    result.views.reserve(result.storage.size());
    for (const auto& receipt : result.storage) {
        result.views.emplace_back(receipt);
    }
    return result;
}

bool has_error(
    const dmc::rengine::spider::L2RuntimeMappingBuildResultV2& result,
    std::string_view fragment) {
    for (const auto& error : result.errors) {
        if (error.find(fragment) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    namespace spider = dmc::rengine::spider;

    const auto fixture = make_canonical_fixture();
    auto valid_receipts = make_valid_receipts(fixture);
    const auto valid = spider::build_l2_runtime_mapping_v2(
        valid_receipts.views,
        fixture.bytes,
        fixture.authority);
    assert(valid.ok());
    assert(valid.packet->anchors.size() == k_anchors.size());
    assert(valid.packet->pid == 4242U);
    assert(valid.packet->process_creation_filetime == k_creation);
    assert(valid.packet->image_name == "dmc3.exe");
    assert(valid.packet->canonical_analysis_artifact_sha256 ==
        fixture.authority.canonical_analysis_sha256);
    for (const auto& anchor : valid.packet->anchors) {
        assert(anchor.window_sha256 == anchor.canonical_window_sha256);
    }
    const auto encoded = spider::l2_runtime_mapping_v2_to_json(*valid.packet);
    assert(encoded.find("bounded_process_instance_match") != std::string::npos);
    assert(encoded.find("derived-directly-from-exact-canonical-exe-by-validator") !=
        std::string::npos);
    assert(encoded.find("image_path") == std::string::npos);
    assert(encoded.find("LocalUser") == std::string::npos);

    auto legacy = make_valid_receipts(fixture);
    legacy.storage[3] = make_receipt(
        k_anchors[3],
        fixture.hashes[3],
        k_creation,
        "dmc-rengine.exe-process-window.v1");
    legacy.views.clear();
    for (const auto& receipt : legacy.storage) {
        legacy.views.emplace_back(receipt);
    }
    const auto legacy_result = spider::build_l2_runtime_mapping_v2(
        legacy.views, fixture.bytes, fixture.authority);
    assert(!legacy_result.ok());
    assert(has_error(legacy_result, "legacy v1 is not promotion authority"));

    auto wrong_creation = make_valid_receipts(fixture);
    wrong_creation.storage[3] = make_receipt(
        k_anchors[3], fixture.hashes[3], k_creation + 1U);
    wrong_creation.views.clear();
    for (const auto& receipt : wrong_creation.storage) {
        wrong_creation.views.emplace_back(receipt);
    }
    const auto wrong_creation_result = spider::build_l2_runtime_mapping_v2(
        wrong_creation.views, fixture.bytes, fixture.authority);
    assert(!wrong_creation_result.ok());
    assert(has_error(wrong_creation_result, "one exact process instance/module session"));

    auto forged = make_valid_receipts(fixture);
    const std::string forged_hash(64U, 'f');
    forged.storage[3] = make_receipt(
        k_anchors[3], forged_hash, k_creation,
        "dmc-rengine.exe-process-window.v2", true, forged_hash);
    replace_once(
        forged.storage[3],
        "PLACEHOLDER_CANONICAL_SHA",
        fixture.authority.canonical_analysis_sha256);
    forged.views.clear();
    for (const auto& receipt : forged.storage) {
        forged.views.emplace_back(receipt);
    }
    const auto forged_result = spider::build_l2_runtime_mapping_v2(
        forged.views, fixture.bytes, fixture.authority);
    assert(!forged_result.ok());
    assert(has_error(forged_result, "independently derived canonical window"));

    auto raw = make_valid_receipts(fixture);
    replace_once(raw.storage[3], "\n}\n", ",\n  \"bytes_hex\": \"00\"\n}\n");
    raw.views.clear();
    for (const auto& receipt : raw.storage) {
        raw.views.emplace_back(receipt);
    }
    const auto raw_result = spider::build_l2_runtime_mapping_v2(
        raw.views, fixture.bytes, fixture.authority);
    assert(!raw_result.ok());
    assert(has_error(raw_result, "metadata-only receipts"));

    auto tampered = fixture.bytes;
    tampered.back() ^= std::byte{0x01};
    const auto tampered_result = spider::build_l2_runtime_mapping_v2(
        valid_receipts.views, tampered, fixture.authority);
    assert(!tampered_result.ok());
    assert(has_error(tampered_result, "canonical EXE SHA-256 mismatch"));

    return 0;
}
