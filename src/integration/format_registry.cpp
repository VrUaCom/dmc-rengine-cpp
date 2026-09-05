#include "dmc_rengine/integration/format_registry.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace dmc::rengine::integration {
namespace {

[[nodiscard]] std::string normalized_format(std::string_view format) {
    std::string result(format);
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return result;
}

[[nodiscard]] FormatIntegrationDescriptor recognized_read_only(
    std::string format,
    std::vector<std::string> limitations,
    std::optional<gdspaces::StageResourceCategory> stage_category = std::nullopt) {
    return FormatIntegrationDescriptor{
        .format = std::move(format),
        .parser_id = {},
        .maturity = IntegrationMaturity::recognized,
        .write_policy = ResourceWritePolicy::read_only,
        .binary_adapter = false,
        .stage_category = stage_category,
        .evidence_claim_ids = {},
        .limitations = std::move(limitations),
    };
}

} // namespace

bool FormatIntegrationDescriptor::valid() const noexcept {
    if (format.empty()) {
        return false;
    }
    if (maturity != IntegrationMaturity::recognized &&
        parser_id.empty() && source_adapter_id.empty()) {
        return false;
    }
    if (parser_validation_required && parser_id.empty()) {
        return false;
    }
    if (write_policy == ResourceWritePolicy::guarded_export &&
        (maturity != IntegrationMaturity::exportable || writer_modes.empty())) {
        return false;
    }
    return true;
}

bool FormatIntegrationDescriptor::allows_working_copy() const noexcept {
    return write_policy == ResourceWritePolicy::working_copy_only ||
           write_policy == ResourceWritePolicy::guarded_export;
}

bool FormatIntegrationDescriptor::allows_guarded_export() const noexcept {
    return write_policy == ResourceWritePolicy::guarded_export;
}

bool FormatIntegrationDescriptor::allows_writer_mode(
    std::string_view mode) const noexcept {
    if (mode.empty()) {
        return false;
    }
    return std::find(writer_modes.begin(), writer_modes.end(), mode) !=
        writer_modes.end();
}

FormatIntegrationRegistry::FormatIntegrationRegistry() {
    formats_ = {
        FormatIntegrationDescriptor{
            .format = "pe",
            .parser_id = "exe.pe-reader",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {
                "claim-dmc3-pe-identity",
                "claim-dmc3-image-base",
                "claim-dmc3-entry-point",
            },
            .limitations = {
                "No full decompilation model.",
                "No direct executable writer.",
                "Patches must use guarded patch plans.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "pac",
            .parser_id = "dmc3-pac-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::working_copy_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Structural parser preserves sparse/alias slot topology only; slot semantics remain schema-specific.",
                "Bounded authoring supports same-size layout-preserving packed output, runtime-synth size-changing output from independently intrinsic standalone child bytes, and typed verified runtime-synth complete-image child results for nested size-changing composition.",
                "Generic/self-declared writer receipts do not prove intrinsic extent; semantic child-to-slot linkage and broad real intrinsic-byte providers remain evidence-gated.",
                "WorkingCopy requires successful canonical parser validation for the exact immutable workspace source.",
            },
            .source_adapter_id = {},
            .writer_modes = {"layout-preserving-packed", "runtime-synth-relative-slot"},
            .parser_validation_required = true,
        },
        FormatIntegrationDescriptor{
            .format = "pnst",
            .parser_id = "dmc3-pnst-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::working_copy_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "PNST shares the relative-slot physical envelope with PAC but not a global semantic slot schema.",
                "Bounded authoring supports same-size layout-preserving packed output and typed runtime-synth composition.",
                "Semantic child-to-slot linkage and broad real intrinsic-byte providers remain evidence-gated.",
                "WorkingCopy requires successful canonical parser validation for the exact immutable workspace source.",
            },
            .source_adapter_id = {},
            .writer_modes = {"layout-preserving-packed", "runtime-synth-relative-slot"},
            .parser_validation_required = true,
        },
        FormatIntegrationDescriptor{
            .format = "afs",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "DMC3 HD .afs/ strings are confirmed logical namespace prefixes; a dedicated binary AFS backend is not evidenced on the canonical path.",
                "Do not add a binary AFS parser/source without supported raw artifact or direct parser/backend evidence.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "nbz",
            .parser_id = {},
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "NbzZipSource is the canonical product source/materializer; NBZ is not represented by a fake in-memory parser authority.",
                "Generated STORE-only next-volume overlays are product authoring, not Capcom packer equivalence or lossless retail-volume repack.",
            },
            .source_adapter_id = "gdspaces.nbz-zip-source-v1",
            .writer_modes = {"store-overlay-nbz"},
            .parser_validation_required = false,
        },
        FormatIntegrationDescriptor{
            .format = "hits",
            .parser_id = "formats.hits-record-scanner",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::working_copy_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::collision,
            .evidence_claim_ids = {},
            .limitations = {
                "Canonical data magic is the four-byte HITS prefix; HITS$ is rejected.",
                "HITS identity is DATA_CONFIRMED from corpus/parser evidence; the bounded canonical EXE type-ID census does not establish HITS as an EXE runtime tag.",
                "Parsing is header-driven through bounds, grid dimensions, spatial lists and triangle/plane records.",
                "0x18060001 is an observed raw flag value, not a universal record marker.",
                "The writer is a DMC Rengine working-copy writer; Capcom builder equivalence remains unproven.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "itm",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::working_copy_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Historical Item Editor behavior is not yet fully migrated into this C++ repository.",
                "No generic container reintegration writer is available for all ITM contexts.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "txt",
            .parser_id = "formats.stage-txt-lexer",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::working_copy_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::scripts,
            .evidence_claim_ids = {"claim-dmc3-stageset-token-classifier"},
            .limitations = {
                "The lexer recognizes safe lexical structure and confirmed StageSet/door tokens only.",
                "Full script semantics and a validated writer remain unavailable.",
            },
        },
        recognized_read_only(
            "scm",
            {
                "EXE-confirmed three-byte registry identity, container handler and four-byte family-mask identity.",
                "SCM object/node/buffer parser migration is pending.",
                "Three-byte runtime identity must not be treated as proof that arbitrary SCMx bytes match the canonical mesh layout.",
            },
            gdspaces::StageResourceCategory::models),
        recognized_read_only(
            "mod",
            {
                "EXE-confirmed three-byte registry identity, container handler and four-byte family-mask identity.",
                "MOD model parser migration is pending.",
                "Three-byte runtime identity must not be treated as proof that arbitrary MODx bytes match the canonical mesh layout.",
            },
            gdspaces::StageResourceCategory::models),
        recognized_read_only(
            "efm",
            {
                "EXE-confirmed three-byte registry identity, dedicated handler 0x1402F7A90 and four-byte family-mask identity.",
                "Mesh-bearing effect-model purpose is confirmed, but a real retail EFM payload is still required for exact stream-to-shader binding.",
                "Do not reuse the MOD decoder by family similarity alone.",
            },
            gdspaces::StageResourceCategory::models),
        recognized_read_only(
            "mrp",
            {
                "EXE-confirmed type code 3 in the three-byte registry and 0x40000000 in the four-byte family-mask classifier.",
                "No generic container handler, model-factory branch or standalone mesh ownership is proven.",
            }),
        recognized_read_only(
            "mcv",
            {
                "EXE-confirmed exact MCV<space> family-mask identity 0x50000000 and .mcv class 1 in the motion/control dispatcher.",
                "Exact field semantics and relation to MOT/model consumers remain open; no mesh decoder is authorized.",
            }),
        recognized_read_only(
            "shw",
            {
                "EXE-confirmed registry identity and dedicated handler 0x1403204C0.",
                "SHW is a shadow geometry/topology companion with external spatial data, not a proven self-contained textured mesh.",
            }),
        recognized_read_only(
            "efe",
            {
                "EXE-confirmed three-byte sentinel in container dispatcher 0x1401B9FA0.",
                "No normal handler or exact semantic schema is recovered; recognition must remain fail-closed.",
            }),
        recognized_read_only(
            "efw",
            {
                "EXE-confirmed three-byte sentinel in container dispatcher 0x1401B9FA0.",
                "No normal handler or exact semantic schema is recovered; recognition must remain fail-closed.",
            }),
        FormatIntegrationDescriptor{
            .format = "dds",
            .parser_id = "formats.dds-dmc3-reader",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "DDS identity is EXE-confirmed by direct first-DWORD checks at 0x140049A8E and 0x14004AD9D.",
                "Native structural reading is bounded to the corpus-confirmed DMC3 HD DXT1/DXT5 profile and complete mip chains.",
                "Texture texel decoding and editing/export are not yet integrated into the modular reader.",
                "DXT/ATI/BC and related FourCC values are DDS subformats, not standalone DMC resource families.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "ptx",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "PTX identity is EXE-confirmed by the primary extension classifier.",
                "Bundle schema/editor migration remains incomplete.",
            },
        },
        recognized_read_only(
            "tm2",
            {
                "Direct canonical EXE content check is exact TM2\\0 at 0x1403365BA.",
                "TIM2 is retained only as an alias/historical tooling label; it is not canonical direct EXE magic authority.",
                "Exact schema and HD conversion/ownership chain remain open.",
            },
            gdspaces::StageResourceCategory::textures),
        recognized_read_only(
            "c1d",
            {
                "EXE-confirmed extension identity in the primary registrar classifier.",
                "One-dimensional cloth purpose is high-confidence; exact binary schema remains open.",
            }),
        recognized_read_only(
            "clt",
            {
                "EXE-confirmed extension identity in both primary and motion/control classification paths.",
                "Cloth/deformation schema remains open.",
            }),
        recognized_read_only(
            "mot",
            {
                "Base MOT is EXE-confirmed as class 0 in the motion/control extension dispatcher.",
                "MOT2..MOT6 are corpus variants; exact variant ABI and bindings remain open.",
            }),
        FormatIntegrationDescriptor{
            .format = "cam",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::cameras,
            .evidence_claim_ids = {},
            .limitations = {
                "CAM identity is EXE-confirmed as class 2 in the motion/control extension dispatcher.",
                "Historical CAM Editor/full schema is not yet migrated.",
            },
        },
        recognized_read_only(
            "hid",
            {
                "Base HID is EXE-confirmed as class 3 in the motion/control extension dispatcher.",
                "HID2/HID3 remain corpus variants and the track schema is open.",
            }),
        recognized_read_only(
            "tsc",
            {
                "TSC is EXE-confirmed as class 5 in the motion/control extension dispatcher.",
                "Exact semantic role and binary schema remain research-required.",
            }),
        FormatIntegrationDescriptor{
            .format = "dca",
            .parser_id = "formats.dca-record-scanner",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::unknown,
            .evidence_claim_ids = {},
            .limitations = {
                "Only DCA\\0 magic, 0x10-byte header and 0x410-byte record boundaries are confirmed.",
                "Header and record field semantics remain raw and no writer is available.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "lig",
            .parser_id = "formats.lig2-record-scanner",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::lighting,
            .evidence_claim_ids = {},
            .limitations = {
                "Only the corpus-backed 0x20-byte header and 0x30-byte record envelope are structural facts.",
                "LIG and LIG2 identity must not be collapsed solely from record size.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "lig2",
            .parser_id = "formats.lig2-record-scanner",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::lighting,
            .evidence_claim_ids = {},
            .limitations = {
                "EXE constructor 0x14023ECB0 writes FourCC LIG2 to object +0x08 at 0x14023ECC9.",
                "The 0x20-byte header and 0x30-byte records are corpus-backed structural facts; exact object-tag-to-disk relation and fields remain open.",
            },
        },
        recognized_read_only(
            "vagp",
            {
                "EXE-confirmed direct first-DWORD VAGp payload check at 0x140032970.",
                "Exact DMC3 bank/container ownership and complete VAG schema remain outside current product support.",
            }),
        FormatIntegrationDescriptor{
            .format = "sltc",
            .parser_id = "formats.synthetic-slot-container",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Synthetic test format only; never claim compatibility with game containers.",
            },
        },
    };
}

const FormatIntegrationDescriptor* FormatIntegrationRegistry::find(
    std::string_view format) const noexcept {
    const auto normalized = normalized_format(format);
    const auto iterator = std::find_if(
        formats_.begin(), formats_.end(),
        [&normalized](const FormatIntegrationDescriptor& descriptor) {
            return descriptor.format == normalized;
        });
    return iterator == formats_.end() ? nullptr : &*iterator;
}

const std::vector<FormatIntegrationDescriptor>&
FormatIntegrationRegistry::formats() const noexcept {
    return formats_;
}

std::vector<const FormatIntegrationDescriptor*>
FormatIntegrationRegistry::by_maturity(IntegrationMaturity maturity) const {
    std::vector<const FormatIntegrationDescriptor*> result;
    for (const auto& descriptor : formats_) {
        if (descriptor.maturity == maturity) {
            result.push_back(&descriptor);
        }
    }
    return result;
}

} // namespace dmc::rengine::integration