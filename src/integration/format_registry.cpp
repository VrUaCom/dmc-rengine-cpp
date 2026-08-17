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
    if (maturity != IntegrationMaturity::recognized && parser_id.empty()) {
        return false;
    }
    if (write_policy == ResourceWritePolicy::guarded_export &&
        maturity != IntegrationMaturity::exportable) {
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
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "Real PAC layout is not yet migrated into the clean repository.",
                "Synthetic slot containers must not be represented as PAC compatibility.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "pnst",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {
                "PNST text-index and binary linkage parser is pending evidence migration.",
            },
        },
        FormatIntegrationDescriptor{
            .format = "afs",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {"AFS source implementation is pending."},
        },
        FormatIntegrationDescriptor{
            .format = "nbz",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = std::nullopt,
            .evidence_claim_ids = {},
            .limitations = {"NBZ volume source implementation is pending."},
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
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::models,
            .evidence_claim_ids = {},
            .limitations = {"SCM object/node/buffer parser migration is pending."},
        },
        FormatIntegrationDescriptor{
            .format = "mod",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::models,
            .evidence_claim_ids = {},
            .limitations = {"MOD model parser migration is pending."},
        },
        FormatIntegrationDescriptor{
            .format = "dds",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {"DDS is recognized, but texture editing/export is not yet integrated."},
        },
        FormatIntegrationDescriptor{
            .format = "ptx",
            .parser_id = {},
            .maturity = IntegrationMaturity::recognized,
            .write_policy = ResourceWritePolicy::read_only,
            .binary_adapter = false,
            .stage_category = gdspaces::StageResourceCategory::textures,
            .evidence_claim_ids = {},
            .limitations = {"Phase 16 PTX/runtime findings remain to be migrated."},
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
