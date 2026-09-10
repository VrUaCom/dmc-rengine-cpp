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
                "The descriptor auxiliary pair is bounded by mode 0-2 and by mode and value being zero or non-zero together. It is not bound to the compression: the retail em000 texture bundle opens with a DXT1 descriptor whose mode is 2, and requiring DXT5 there refused the whole pack.",
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
            .parser_id = "formats.shw-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::models,
            .evidence_claim_ids = {},
            .limitations = {
                "The evidence-backed reader materializes self-contained shadow-hull geometry: 0x20 header, 0x40 hull records, triangle topology, exact adjacency, float4 positions and per-vertex transform selectors.",
                "The selector stream is EXE-confirmed as indexing 0x40-byte transform matrices; ownership and construction of the selected matrix palette remain open.",
                "Grammar is hash-bound to one real DMC3 SHW payload plus canonical-EXE corroboration; broader revision coverage remains required and authoring stays read-only.",
            },
            .source_adapter_id = {},
            .writer_modes = {},
            .parser_validation_required = true,
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
            .format = "so-graph",
            .parser_id = "formats.so-graph-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Structural reader only: a type-6 indexed block whose offset table closes exactly on its first entry, and a boundary word pointing at a type-8 companion block with the same property.",
                "Entry payload semantics are unrecovered; the reader reports block geometry, not what the graph connects.",
                "Bound by the complete em000 extraction (source archive SHA-256 3061301250...66d07b), where it accepts 1 payload of 306.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "so-volume",
            .parser_id = "formats.so-volume-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Structural reader only: 0x50 records, each carrying a known kind (2 sphere, 4 segment), twelve reserved zero bytes, and a first vector whose w is 1 — a position in homogeneous coordinates.",
                "Kind 2 places a centre in vector0 and a radius in vector1.x; kind 4 places two points in vector0/vector1 and a radius in vector2.x. Both readings come from a single 23-record corpus payload and are not confirmed against a second.",
                "Bound by the complete em000 extraction (source archive SHA-256 3061301250...66d07b), where it accepts 1 payload of 306; any one of the three record invariants cuts a size-only match from 28 to that one.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "so-link",
            .parser_id = "formats.so-link-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Structural reader only: a leading word and one four-byte record per volume record, each with a reserved zero fourth byte.",
                "The weakest of the three SO gates. Neither invariant is sufficient alone against the bound corpus — the leading word leaves one effect record, the reserved byte leaves ten effect-M companions — and only one link payload exists to generalize from.",
                "The record's third byte is the node a volume hangs off, not the record's own ordinal: it repeats for volumes sharing a node and is zero for those bound to the root. analysis::so binds it to the MOD transform domain.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "wrapped-dds",
            .parser_id = "formats.ptx-dmc3-reader",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {
                "A texture slot's second framing: one 0x70 descriptor and the DDS it describes, with no bundle header in front. Read by the same TextureSlotFramingParser as a PTX bundle, and reported as its own format because it is not one.",
                "Carries no magic; identity is the descriptor's declared dimensions, row bytes and reciprocals agreeing with the DDS behind them.",
                "Bound by the complete em000 extraction (source archive SHA-256 3061301250...66d07b), where it accepts 8 payloads of 306.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "mot",
            .parser_id = "formats.mot-structural-v1",
            .maturity = IntegrationMaturity::structural,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = true,
            .stage_category = gdspaces::StageResourceCategory::animations,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the second recovered type registry (profiles::dmc3::AnimationTypeContract at 0x1402E01A0), which types a motion by name alone and never probes its bytes.",
                "The `MOT` tag at +4 is compared nowhere in the image, so identity is structural: the track chain must close on the payload's 16-byte alignment and every track's declared size must equal its key count's.",
                "Two track kinds are read (kind 3: 0x20 header / 8-byte keys; kind 2: 0x10 header / 4-byte keys); an unknown kind is refused rather than guessed.",
                "Key channel semantics are unrecovered: the reader reports stamps, the 15-bit stamp's flag bit, and geometry, not what any channel drives.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "hid",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the second recovered type registry (profiles::dmc3::AnimationTypeContract at 0x1402E01A0); no structural parser exists yet.",
                "Typed by registrar extension match only; that registry has no content-tag fallback, so an unnamed payload is not typed at all.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "tsc",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Declared by the second recovered type registry (profiles::dmc3::AnimationTypeContract at 0x1402E01A0); no structural parser exists yet.",
                "Serialized as text and identified by the `.TSC` tag line its own payload opens with (profiles::dmc3::TextResourceDialects), which is how a nameless slot is typed; the runtime itself types it by name only.",
                "Record semantics are unrecovered; the dialect probe establishes identity, not content.",
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
                "Declared by both recovered type registries — code 5 in ResourceTypeContract and code 4 in AnimationTypeContract — so a stored type code is only meaningful together with the registry that issued it.",
                "Selected by registrar extension match (strstr against the 0x140507070 literal table), not by a content tag.",
                "Serialized as text; a nameless slot is typed by the `;<name>.clt` comment the payload opens with (profiles::dmc3::TextResourceDialects), which also carries the original filename. In the em000 corpus that name disagrees with the enclosing container for seven of eight payloads, so the container must not be used to name it.",
                "Cloth/deformation record semantics are unrecovered; no structural parser exists yet.",
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