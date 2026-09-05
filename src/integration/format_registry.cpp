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
                "Generic/self-declared writer receipts do not prove intrinsic extent; typed verified runtime-synth composition is available, but semantic child-to-slot linkage and broad real intrinsic-byte providers remain evidence-gated.",
                "WorkingCopy requires successful canonical parser validation for the exact immutable workspace source.",
            },
            .source_adapter_id = {},
            .writer_modes = {
                "layout-preserving-packed",
                "runtime-synth-relative-slot",
            },
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
                "Bounded authoring supports same-size layout-preserving packed output, runtime-synth size-changing output from independently intrinsic standalone child bytes, and typed verified runtime-synth complete-image child results for nested size-changing composition.",
                "Generic/self-declared writer receipts do not prove intrinsic extent; typed verified runtime-synth composition is available, but semantic child-to-slot linkage and broad real intrinsic-byte providers remain evidence-gated.",
                "WorkingCopy requires successful canonical parser validation for the exact immutable workspace source.",
            },
            .source_adapter_id = {},
            .writer_modes = {
                "layout-preserving-packed",
                "runtime-synth-relative-slot",
            },
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
                "Do not add a binary AFS parser/source without a supported raw artifact or direct parser/backend evidence.",
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
                "Generated STORE-only next-volume overlays are supported as product authoring, not Capcom packer equivalence or lossless retail-volume repack.",
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
                "Canonical magic is the four-byte HITS prefix; HITS$ is rejected as an obsolete scanner assumption.",
                "Parsing is header-driven through bounds, cell sizes, grid dimensions, spatial lists, and 0x38 raw-flags + triangle-plane records.",
                "0x18060001 is an observed raw flag value, not a universal record marker; unknown flag semantics remain evidence-gated.",
                "A deterministic DMC Rengine working-copy writer exists, but Capcom offline-builder equivalence and production container reintegration/export remain unproven.",
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
                "Historical Item Editor behavior is not yet migrated into this C++ repository.",
                "No container reintegration writer is available.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "txt",
            .parser_id = "formats.stage-txt-lexer",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::working_copy_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::scripts,
            .evidence_claim_ids = {
                "claim-dmc3-stageset-token-classifier",
            },
            .limitations = {
                "The lexer recognizes safe lexical structure and confirmed StageSet/door tokens only.",
                "Full script semantics and a validated writer remain unavailable.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "scm",
            .parser_id = "formats.scm-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::models,
            .evidence_claim_ids = {},
            .limitations = {
                "The Model Family SCM adapter materializes the 0x40 header/object shell, 0x50 mesh records, position/normal/fixed-point UV streams, scene hierarchy and XYZ-radian transforms.",
                "SCM-specific texture-slot binding, alpha control, legacy GS sampler state and topology/control bytes remain in the SCM adapter rather than the shared ABI.",
                "SCM authoring remains read-only in Native Reader; experimental writer work does not imply production or Capcom-builder equivalence.",
            },
            .source_adapter_id = {},
            .writer_modes = {},
            .parser_validation_required = true,
        },
        FormatIntegrationDescriptor{
            .format = "mod",
            .parser_id = "formats.mod-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::models,
            .evidence_claim_ids = {},
            .limitations = {
                "The Model Family MOD adapter validates the recovered 0x40 outer / 0x50 inner mesh grammar and materializes position, normal, fixed-point UV, BLENDINDICES-correlated and packed skin/control streams.",
                "The recovered skin contract supports up to three serialized influences with 5-bit weights normalized by /31; raw source/control bytes remain preservation authority for unknown variants.",
                "MOD writing/editing is not authorized until broader revision coverage, bind/inverse-bind ownership and original-game mutation acceptance are closed.",
            },
            .source_adapter_id = {},
            .writer_modes = {},
            .parser_validation_required = true,
        },
        FormatIntegrationDescriptor{
            .format = "dds",
            .parser_id = "formats.dds-dmc3-reader",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "Native reading validates the corpus-confirmed DMC3 HD DDS header profile, complete mip chain and DXT1/DXT5 payload extent.",
                "Texture texel editing/export remains outside the Native Reader boundary.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "ptx",
            .parser_id = "formats.ptx-dmc3-reader",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "Native reading validates the evidence-backed 0x800 bundle header, 0x70 descriptors, sector spans, DXT1/DXT5 DDS children and alignment padding.",
                "Validated DDS children are materialized through the canonical TextureSlotExpander with stable byte provenance.",
                "Packed-reflow authoring remains a separate profile capability and is not implicitly promoted by the reader.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "cam",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::cameras,
            .evidence_claim_ids = {},
            .limitations = {"Historical CAM Editor is not yet migrated."},
        },
        FormatIntegrationDescriptor{
            .format = "dca",
            .parser_id = "formats.dca-record-scanner",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::unknown,
            .evidence_claim_ids = {},
            .limitations = {
                "Only DCA\\0 magic, 0x10-byte header, and 0x410-byte record boundaries are confirmed.",
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
                "Only the 0x20-byte header and 0x30-byte record boundaries are structural facts.",
                "The confirmed DMC3 corpus uses 48 records; field semantics and writing remain unavailable.",
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
                "Only the 0x20-byte header and 0x30-byte record boundaries are structural facts.",
                "The confirmed DMC3 corpus uses 48 records; field semantics and writing remain unavailable.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "efm",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::effects,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the recovered runtime type contract (profiles::dmc3::ResourceTypeContract); no structural parser exists yet.",
                "Recognized at registry probe 0x1402DB1F0, container dispatch 0x1401B9FA0 and family mask 0x1402FD650.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "shw",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::models,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the recovered runtime type contract (profiles::dmc3::ResourceTypeContract); no structural parser exists yet.",
                "Recognized at registry probe 0x1402DB1F0, container dispatch 0x1401B9FA0 and family mask 0x1402FD650.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "mrp",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the recovered runtime type contract (profiles::dmc3::ResourceTypeContract); no structural parser exists yet.",
                "Recognized at registry probe 0x1402DB1F0 and family mask 0x1402FD650; the registry census records no handler VA.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "mcv",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::animations,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the recovered runtime type contract (profiles::dmc3::ResourceTypeContract); no structural parser exists yet.",
                "Recognized only by the four-byte family-mask probe 0x1402FD650, not by the three-byte registry probe.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "clt",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the recovered runtime type contract (profiles::dmc3::ResourceTypeContract); no structural parser exists yet.",
                "Selected by registrar extension match (strstr against the 0x140507070 literal table), not by a content tag.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "c1d",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the recovered runtime type contract (profiles::dmc3::ResourceTypeContract); no structural parser exists yet.",
                "Selected by registrar extension match (strstr against the 0x140507070 literal table), not by a content tag.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "tm2",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "Observed in the retail dmc3-0.nbz central-directory surface bound by archive SHA-256 2c2302ce...fd13df; see docs/reverse/dmc3-nbz-archive-key-census-2026-09-03.md.",
                "Recognized by path extension only; no magic probe and no structural parser are evidence-backed yet.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "fon",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Observed in the retail dmc3-0.nbz central-directory surface bound by archive SHA-256 2c2302ce...fd13df; see docs/reverse/dmc3-nbz-archive-key-census-2026-09-03.md.",
                "Recognized by path extension only; font structure is unreversed.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "bin",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::events,
            .evidence_claim_ids = {},
            .limitations = {
                "Observed in the retail dmc3-0.nbz central-directory surface bound by archive SHA-256 2c2302ce...fd13df; see docs/reverse/dmc3-nbz-archive-key-census-2026-09-03.md.",
                "Recognized by path extension only; covers unrelated payloads such as EventTbl and SpuMap and must not be treated as one schema.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "bd",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::sounds,
            .evidence_claim_ids = {},
            .limitations = {
                "Observed in the retail dmc3-0.nbz central-directory surface bound by archive SHA-256 2c2302ce...fd13df; see docs/reverse/dmc3-nbz-archive-key-census-2026-09-03.md.",
                "Recognized by path extension only; sound-bank body structure is unreversed.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "phd",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::sounds,
            .evidence_claim_ids = {},
            .limitations = {
                "Observed in the retail dmc3-0.nbz central-directory surface bound by archive SHA-256 2c2302ce...fd13df; see docs/reverse/dmc3-nbz-archive-key-census-2026-09-03.md.",
                "Recognized by path extension only; sound-bank header structure is unreversed.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "tsb",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::sounds,
            .evidence_claim_ids = {},
            .limitations = {
                "Observed in the retail dmc3-0.nbz central-directory surface bound by archive SHA-256 2c2302ce...fd13df; see docs/reverse/dmc3-nbz-archive-key-census-2026-09-03.md.",
                "Recognized by path extension only; structure is unreversed.",
            },
        },
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