#include "custom_build_fixture.hpp"

#include "dmc_rengine/core/sha256.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"
#include "dmc_rengine/source/custom_build_reopen.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace {

void write_u16(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint16_t value) {
    bytes[offset] = static_cast<std::byte>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
}

void write_u32(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

void write_u64(
    std::vector<std::byte>& bytes,
    std::size_t offset,
    std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::byte>(
            (value >> static_cast<unsigned>(index * 8U)) & 0xFFU);
    }
}

[[nodiscard]] std::vector<std::byte> custom_pe_bytes() {
    std::vector<std::byte> bytes(0x4000U, std::byte{0});
    bytes[0] = std::byte{'M'};
    bytes[1] = std::byte{'Z'};
    write_u32(bytes, 0x3CU, 0x80U);
    write_u32(bytes, 0x80U, 0x00004550U);
    write_u16(bytes, 0x84U, 0x8664U);
    write_u16(bytes, 0x86U, 8U);
    write_u16(bytes, 0x94U, 0x00F0U);

    constexpr std::size_t optional = 0x98U;
    write_u16(bytes, optional, 0x020BU);
    write_u32(bytes, optional + 16U, 0x1000U);
    write_u64(bytes, optional + 24U, 0x140000000ULL);
    write_u32(bytes, optional + 56U, 0x9000U);
    write_u32(bytes, optional + 60U, 0x0400U);
    write_u16(bytes, optional + 68U, 2U);

    constexpr std::size_t section_table = 0x188U;
    for (std::size_t index = 0U; index < 8U; ++index) {
        const auto section = section_table + index * 40U;
        bytes[section] = std::byte{'.'};
        bytes[section + 1U] = static_cast<std::byte>('s');
        bytes[section + 2U] = static_cast<std::byte>('0' + index);
        const auto virtual_address = static_cast<std::uint32_t>(
            0x1000U + index * 0x1000U);
        const auto raw_offset = index == 0U
            ? 0x0400U
            : static_cast<std::uint32_t>(0x0C00U + (index - 1U) * 0x400U);
        const auto raw_size = index == 0U ? 0x0800U : 0x0400U;
        write_u32(bytes, section + 8U, raw_size);
        write_u32(bytes, section + 12U, virtual_address);
        write_u32(bytes, section + 16U, raw_size);
        write_u32(bytes, section + 20U, raw_offset);
        write_u32(
            bytes,
            section + 36U,
            index == 0U ? 0x60000020U : 0x40000040U);
    }
    return bytes;
}

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::uint64_t size,
    std::string source_id) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = std::move(source_id),
            .logical_path = "custom-dmc3.exe",
            .container_chain = {},
            .offset = 0U,
            .size = size,
        },
        .display_name = "custom-dmc3.exe",
        .format = "pe",
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] bool has_diagnostic(
    const dmc::rengine::source::CustomBuildReopenContext& context,
    std::string_view code) {
    return std::any_of(
        context.diagnostics.begin(), context.diagnostics.end(),
        [code](const auto& diagnostic) {
            return diagnostic.code == code;
        });
}

void add_executable_session(
    dmc::rengine::integration::ProjectWorkspace& workspace,
    const dmc::rengine::gdspaces::ResourceRef& executable,
    const std::vector<std::byte>& bytes) {
    assert(workspace.create_session(dmc::rengine::gdspaces::ResourcePayload{
        .resource = executable,
        .bytes = bytes,
        .diagnostics = {},
    }));
    assert(dmc::rengine::integration::ResourceAnalyzer::analyze(
        workspace,
        executable.id).ok());
}

} // namespace

int main() {
    using namespace dmc::rengine;
    using namespace tests::custom_build_fixture;
    using core::Sha256;
    using integration::ProjectWorkspace;
    using source::reopen_custom_build;

    const auto bytes = custom_pe_bytes();
    const auto sha256 = Sha256::compute(
        std::span<const std::byte>{bytes}).hex();
    const auto executable = resource(bytes.size(), "custom-build-reopen");

    ProjectWorkspace workspace;
    prepare_workspace(workspace);
    add_executable_session(workspace, executable, bytes);
    auto build = build_record();
    build.identity.executable_sha256 = sha256;
    assert(build.valid());
    assert(workspace.register_custom_build_record(build));

    const auto reopened = reopen_custom_build(workspace, executable.id);
    assert(reopened.ok());
    assert(reopened.build != nullptr);
    assert(reopened.integration_project != nullptr);
    assert(reopened.build->identity.executable_sha256 == sha256);
    assert(reopened.integration_project->id() ==
        "integration:custom-build-fixture");
    assert(reopened.mappings.size() == 1U);
    const auto* mapping = reopened.mapping_for_rva(0x1220U);
    assert(mapping != nullptr);
    assert(mapping->source_path ==
        "recovered/gameplay/item/inventory_policy.cpp");
    assert(mapping->source_line_begin == 10U);
    assert(mapping->source_line_end == 42U);

    ProjectWorkspace unknown_workspace;
    prepare_workspace(unknown_workspace);
    const auto unknown_resource = resource(
        bytes.size(), "custom-build-reopen-unknown");
    add_executable_session(unknown_workspace, unknown_resource, bytes);
    const auto unknown = reopen_custom_build(
        unknown_workspace,
        unknown_resource.id);
    assert(!unknown.ok());
    assert(has_diagnostic(
        unknown,
        "custom-build-reopen.build-record-missing"));

    ProjectWorkspace metadata_workspace;
    prepare_workspace(metadata_workspace);
    const auto metadata_resource = resource(
        bytes.size(), "custom-build-reopen-metadata");
    add_executable_session(metadata_workspace, metadata_resource, bytes);
    auto metadata_build = build_record(
        "dmc3-custom-build-metadata-mismatch",
        '4');
    metadata_build.identity.executable_sha256 = sha256;
    metadata_build.pe_validation.section_count = 7U;
    assert(metadata_build.valid());
    assert(metadata_workspace.register_custom_build_record(metadata_build));
    const auto metadata_mismatch = reopen_custom_build(
        metadata_workspace,
        metadata_resource.id);
    assert(!metadata_mismatch.ok());
    assert(has_diagnostic(
        metadata_mismatch,
        "custom-build-reopen.pe-metadata-mismatch"));

    ProjectWorkspace size_workspace;
    prepare_workspace(size_workspace);
    const auto size_resource = resource(
        bytes.size(), "custom-build-reopen-size");
    add_executable_session(size_workspace, size_resource, bytes);
    auto size_build = build_record(
        "dmc3-custom-build-size-mismatch",
        '5');
    size_build.identity.executable_sha256 = sha256;
    size_build.executable_size += 1U;
    assert(size_build.valid());
    assert(size_workspace.register_custom_build_record(size_build));
    const auto size_mismatch = reopen_custom_build(
        size_workspace,
        size_resource.id);
    assert(!size_mismatch.ok());
    assert(has_diagnostic(
        size_mismatch,
        "custom-build-reopen.file-size-mismatch"));

    const auto invalid_resource = gdspaces::ResourceId{};
    const auto missing_context = reopen_custom_build(
        workspace,
        invalid_resource);
    assert(!missing_context.ok());
    assert(has_diagnostic(
        missing_context,
        "custom-build-reopen.executable-context-missing"));

    return 0;
}
