#include "dmc_rengine/evidence/record.hpp"
#include "dmc_rengine/formats/dca.hpp"
#include "dmc_rengine/formats/lig2.hpp"
#include "dmc_rengine/formats/mod.hpp"
#include "dmc_rengine/integration/project_workspace.hpp"
#include "dmc_rengine/integration/resource_analyzer.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] dmc::rengine::gdspaces::ResourceRef resource(
    std::string path,
    std::string format,
    std::uint64_t offset,
    std::uint64_t size) {
    return dmc::rengine::gdspaces::ResourceRef{
        .id = dmc::rengine::gdspaces::ResourceId{
            .source_id = "structural-analyzer-test",
            .logical_path = std::move(path),
            .container_chain = "NBZ[0]/PAC[0]",
            .offset = offset,
            .size = size,
        },
        .display_name = "resource",
        .format = std::move(format),
        .profile = "dmc3-hd",
        .synthetic_name = false,
        .container = false,
    };
}

[[nodiscard]] std::vector<std::byte> dca_bytes() {
    std::vector<std::byte> bytes(
        dmc::rengine::formats::dca::header_size +
            dmc::rengine::formats::dca::record_size,
        std::byte{0});
    bytes[0] = std::byte{'D'};
    bytes[1] = std::byte{'C'};
    bytes[2] = std::byte{'A'};
    bytes[3] = std::byte{0};
    return bytes;
}

[[nodiscard]] std::vector<std::byte> lig2_bytes() {
    return std::vector<std::byte>(
        dmc::rengine::formats::lig2::expected_dmc3_file_size,
        std::byte{0});
}

[[nodiscard]] std::vector<std::byte> text_bytes(std::string_view value) {
    std::vector<std::byte> bytes;
    bytes.reserve(value.size());
    for (const auto character : value) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

void put_u8(std::vector<std::byte>& bytes, std::size_t offset, std::uint8_t value) {
    bytes[offset] = static_cast<std::byte>(value);
}

void put_u16(std::vector<std::byte>& bytes, std::size_t offset, std::uint16_t value) {
    put_u8(bytes, offset + 0U, static_cast<std::uint8_t>(value & 0xFFU));
    put_u8(bytes, offset + 1U, static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

void put_u32(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU));
    }
}

void put_u64(std::vector<std::byte>& bytes, std::size_t offset, std::uint64_t value) {
    for (std::size_t index = 0U; index < 8U; ++index) {
        put_u8(bytes, offset + index,
               static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU));
    }
}

void put_f32(std::vector<std::byte>& bytes, std::size_t offset, float value) {
    put_u32(bytes, offset, std::bit_cast<std::uint32_t>(value));
}

[[nodiscard]] std::vector<std::byte> mod_bytes() {
    std::vector<std::byte> bytes(0x240U, std::byte{0});
    bytes[0] = std::byte{'M'};
    bytes[1] = std::byte{'O'};
    bytes[2] = std::byte{'D'};
    bytes[3] = std::byte{' '};
    put_f32(bytes, 0x04U, 1.01F);
    put_u8(bytes, 0x10U, 1U);
    put_u8(bytes, 0x11U, 1U);
    put_u64(bytes, 0x20U, 0x200U);

    put_u8(bytes, 0x40U, 1U);
    put_u16(bytes, 0x42U, 1U);
    put_u64(bytes, 0x48U, 0x80U);

    put_u16(bytes, 0x80U, 1U);
    put_u64(bytes, 0x90U, 0xD0U);
    put_u64(bytes, 0x98U, 0xE0U);
    put_u64(bytes, 0xA0U, 0xF0U);
    put_u64(bytes, 0xA8U, 0x100U);
    put_u64(bytes, 0xB0U, 0x110U);
    put_u64(bytes, 0xC0U, 0xA0U);

    put_f32(bytes, 0xD0U, 1.0F);
    put_f32(bytes, 0xD4U, 2.0F);
    put_f32(bytes, 0xD8U, 3.0F);
    put_f32(bytes, 0xE4U, 1.0F);
    put_u16(bytes, 0xF0U, 4096U);
    put_u16(bytes, 0xF2U, 2048U);
    put_u16(bytes, 0x110U, 0x001FU);

    put_u32(bytes, 0x200U, 0x10U);
    put_u32(bytes, 0x204U, 0x20U);
    put_u32(bytes, 0x208U, 0x30U);
    put_u8(bytes, 0x210U, 0xFFU);
    put_u8(bytes, 0x220U, 0U);
    put_u8(bytes, 0x230U, 0U);
    return bytes;
}

} // namespace

int main() {
    using dmc::rengine::evidence::Confidence;
    using dmc::rengine::evidence::EvidenceRecord;
    using dmc::rengine::gdspaces::ResourcePayload;
    using dmc::rengine::gdspaces::StageResourceCategory;
    using dmc::rengine::integration::IntegrationMaturity;
    using dmc::rengine::integration::ProjectWorkspace;
    using dmc::rengine::integration::ResourceAnalyzer;
    using dmc::rengine::integration::WorkspaceContext;
    using dmc::rengine::integration::WorkspaceEventType;

    ProjectWorkspace project;
    assert(project.add_evidence(EvidenceRecord{
        .id = "ev-stage-set-test",
        .claim_id = "claim-dmc3-stageset-token-classifier",
        .title = "StageSet classifier test evidence",
        .summary = "Synthetic claim used to verify format-evidence linkage.",
        .confidence = Confidence::confirmed,
        .locations = {},
        .tags = {"synthetic-test"},
        .supersedes = {},
    }));

    const WorkspaceContext stage_context{
        .stage_context = true,
        .menu_context = false,
        .evidence_context = true,
    };

    const auto dca_data = dca_bytes();
    const auto dca = resource(
        "room/st001cfg_008.ukn", "dca", 0x1000U, dca_data.size());
    assert(project.create_session(ResourcePayload{
        .resource = dca,
        .bytes = dca_data,
        .diagnostics = {},
    }, stage_context));
    const auto dca_report = ResourceAnalyzer::analyze(project, dca.id);
    assert(dca_report.ok());
    assert(dca_report.parser_id == "formats.dca-record-scanner");
    assert(dca_report.binary_document_attached);
    const auto* dca_session = project.find_session(dca.id);
    assert(dca_session != nullptr);
    assert(dca_session->format() != nullptr);
    assert(dca_session->format()->maturity == IntegrationMaturity::structural);
    assert(dca_session->format()->stage_category == StageResourceCategory::unknown);
    assert(dca_session->binary_document() != nullptr);
    assert(dca_session->events().by_type(WorkspaceEventType::parser_completed).size() == 1U);

    const auto lig_data = lig2_bytes();
    const auto lig = resource(
        "room/st001cfg_001.lig", "lig", 0x2000U, lig_data.size());
    assert(project.create_session(ResourcePayload{
        .resource = lig,
        .bytes = lig_data,
        .diagnostics = {},
    }, stage_context));
    const auto lig_report = ResourceAnalyzer::analyze(project, lig.id);
    assert(lig_report.ok());
    assert(lig_report.parser_id == "formats.lig2-record-scanner");
    assert(lig_report.binary_document_attached);
    const auto* lig_session = project.find_session(lig.id);
    assert(lig_session != nullptr);
    assert(lig_session->format() != nullptr);
    assert(lig_session->format()->stage_category == StageResourceCategory::lighting);
    assert(lig_session->binary_document() != nullptr);
    assert(lig_session->events().by_type(WorkspaceEventType::parser_completed).size() == 1U);

    const auto txt_data = text_bytes("#SET STAY\nDOOR BoxIn NextRoom\n");
    const auto txt = resource(
        "room/st001cfg_004.txt", "txt", 0x3000U, txt_data.size());
    assert(project.create_session(ResourcePayload{
        .resource = txt,
        .bytes = txt_data,
        .diagnostics = {},
    }, stage_context));
    const auto txt_report = ResourceAnalyzer::analyze(project, txt.id);
    assert(txt_report.ok());
    assert(txt_report.parser_id == "formats.stage-txt-lexer");
    assert(txt_report.binary_document_attached);
    const auto* txt_session = project.find_session(txt.id);
    assert(txt_session != nullptr);
    assert(txt_session->format() != nullptr);
    assert(txt_session->format()->stage_category == StageResourceCategory::scripts);
    assert(txt_session->binary_document() != nullptr);
    assert(txt_session->events().by_type(WorkspaceEventType::parser_completed).size() == 1U);
    assert(std::find(
        txt_session->evidence_record_ids().begin(),
        txt_session->evidence_record_ids().end(),
        "ev-stage-set-test") != txt_session->evidence_record_ids().end());

    const auto mod_data = mod_bytes();
    const auto mod = resource(
        "model/pl000.mod", "mod", 0x3800U, mod_data.size());
    assert(project.create_session(ResourcePayload{
        .resource = mod,
        .bytes = mod_data,
        .diagnostics = {},
    }, stage_context));
    const auto mod_report = ResourceAnalyzer::analyze(project, mod.id);
    assert(mod_report.ok());
    assert(mod_report.parser_available);
    assert(mod_report.recognized);
    assert(mod_report.parser_id == "formats.mod-structural-v1");
    assert(!mod_report.binary_document_attached);
    const auto* mod_session = project.find_session(mod.id);
    assert(mod_session != nullptr);
    assert(mod_session->format() != nullptr);
    assert(mod_session->format()->stage_category == StageResourceCategory::models);
    assert(mod_session->parser_validation() != nullptr);
    assert(mod_session->parser_validation()->parser_id == "formats.mod-structural-v1");
    assert(mod_session->events().by_type(WorkspaceEventType::parser_completed).size() == 1U);

    const std::vector<std::byte> invalid_txt{
        std::byte{'#'}, std::byte{'S'}, std::byte{'E'}, std::byte{'T'},
        std::byte{0}, std::byte{'S'}, std::byte{'T'}, std::byte{'A'}, std::byte{'Y'},
    };
    const auto invalid = resource(
        "room/st001cfg_invalid.txt", "txt", 0x4000U, invalid_txt.size());
    assert(project.create_session(ResourcePayload{
        .resource = invalid,
        .bytes = invalid_txt,
        .diagnostics = {},
    }, stage_context));
    const auto invalid_report = ResourceAnalyzer::analyze(project, invalid.id);
    assert(!invalid_report.ok());
    assert(invalid_report.parser_available);
    assert(!invalid_report.recognized);
    assert(!invalid_report.diagnostics.empty());
    const auto* invalid_session = project.find_session(invalid.id);
    assert(invalid_session != nullptr);
    assert(invalid_session->binary_document() == nullptr);
    assert(invalid_session->events().by_type(WorkspaceEventType::parser_completed).size() == 1U);

    return 0;
}
