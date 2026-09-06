#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/native_reader_registry.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

bool has_limitation(
    const dmc::rengine::integration::FormatIntegrationDescriptor& descriptor,
    std::string_view needle) {
    return std::any_of(
        descriptor.limitations.begin(), descriptor.limitations.end(),
        [needle](const std::string& value) {
            return value.find(needle) != std::string::npos;
        });
}

} // namespace

int main() {
    namespace gdspaces = dmc::rengine::gdspaces;
    namespace integration = dmc::rengine::integration;

    const integration::FormatIntegrationRegistry registry;
    const integration::NativeReaderModuleRegistry readers;
    const auto containers =
        dmc::rengine::profiles::dmc3::make_container_parser_registry();

    assert(readers.size() == 10U);
    for (const std::string_view parser_id : {
             "formats.dds-dmc3-reader",
             "formats.ptx-dmc3-reader",
             "formats.hits-record-scanner",
             "formats.dca-record-scanner",
             "formats.lig2-record-scanner",
             "formats.stage-txt-lexer",
             "formats.scm-structural-v1",
             "formats.mod-structural-v1",
             "formats.shw-structural-v1",
             "exe.pe-reader"}) {
        const auto* module = readers.find(parser_id);
        assert(module != nullptr);
        assert(module->valid());
    }

    const auto* pac = registry.find("PAC");
    assert(pac != nullptr);
    assert(pac->valid());
    assert(pac->write_policy == integration::ResourceWritePolicy::working_copy_only);
    assert(pac->parser_validation_required);
    assert(pac->allows_writer_mode("layout-preserving-packed"));
    assert(pac->allows_writer_mode("runtime-synth-relative-slot"));
    assert(!pac->allows_writer_mode("generic-writer-receipt"));
    assert(has_limitation(*pac, "independently intrinsic"));
    assert(has_limitation(*pac, "typed verified runtime-synth"));
    assert(has_limitation(*pac, "semantic child-to-slot"));
    assert(readers.find(pac->parser_id) == nullptr);
    assert(containers.find_by_id(pac->parser_id) != nullptr);

    const auto* pnst = registry.find("PNST");
    assert(pnst != nullptr);
    assert(pnst->valid());
    assert(pnst->write_policy == integration::ResourceWritePolicy::working_copy_only);
    assert(pnst->parser_validation_required);
    assert(pnst->allows_writer_mode("layout-preserving-packed"));
    assert(pnst->allows_writer_mode("runtime-synth-relative-slot"));
    assert(!pnst->allows_writer_mode("generic-writer-receipt"));
    assert(has_limitation(*pnst, "independently intrinsic"));
    assert(has_limitation(*pnst, "typed verified runtime-synth"));
    assert(has_limitation(*pnst, "semantic child-to-slot"));
    assert(readers.find(pnst->parser_id) == nullptr);
    assert(containers.find_by_id(pnst->parser_id) != nullptr);

    const auto* nbz = registry.find("NBZ");
    assert(nbz != nullptr);
    assert(nbz->allows_writer_mode("store-overlay-nbz"));
    assert(!nbz->allows_writer_mode("runtime-synth-relative-slot"));
    assert(nbz->source_adapter_id == "gdspaces.nbz-zip-source-v1");

    // DMC3 `.afs/` is a logical namespace boundary, not automatic binary AFS
    // expansion authority. Exact binary-looking inputs remain explicit read-only
    // acquisition candidates until a profile-specific backend is evidenced.
    const auto* afs_namespace = registry.find("AFS-NAMESPACE");
    assert(afs_namespace != nullptr);
    assert(afs_namespace->valid());
    assert(afs_namespace->maturity == integration::IntegrationMaturity::recognized);
    assert(afs_namespace->write_policy == integration::ResourceWritePolicy::read_only);
    assert(afs_namespace->parser_id.empty());
    assert(afs_namespace->source_adapter_id.empty());

    const auto* afs_candidate = registry.find("AFS-BINARY-CANDIDATE");
    assert(afs_candidate != nullptr);
    assert(afs_candidate->valid());
    assert(afs_candidate->maturity == integration::IntegrationMaturity::recognized);
    assert(afs_candidate->write_policy == integration::ResourceWritePolicy::read_only);
    assert(afs_candidate->parser_id.empty());
    assert(afs_candidate->source_adapter_id.empty());

    const auto* pack_candidate = registry.find("PACK-BINARY-CANDIDATE");
    assert(pack_candidate != nullptr);
    assert(pack_candidate->valid());
    assert(pack_candidate->maturity == integration::IntegrationMaturity::recognized);
    assert(pack_candidate->write_policy == integration::ResourceWritePolicy::read_only);
    assert(pack_candidate->parser_id.empty());
    assert(pack_candidate->source_adapter_id.empty());

    const auto namespace_classification = gdspaces::ResourceClassifier::classify(
        "DMC3/GData.afs/");
    assert(namespace_classification.format == "afs-namespace");
    assert(!namespace_classification.container);

    const auto x360_namespace = gdspaces::ResourceClassifier::classify(
        "DMC3/GDataX360.afs");
    assert(x360_namespace.format == "afs-namespace");
    assert(!x360_namespace.container);

    const auto generic_afs = gdspaces::ResourceClassifier::classify(
        "mods/custom.afs");
    assert(generic_afs.format == "afs-binary-candidate");
    assert(!generic_afs.container);

    const std::vector<std::byte> afs_magic{
        std::byte{'A'}, std::byte{'F'}, std::byte{'S'}, std::byte{0}};
    const auto afs_magic_classification = gdspaces::ResourceClassifier::classify(
        "unknown/blob.bin", std::span<const std::byte>{afs_magic});
    assert(afs_magic_classification.format == "afs-binary-candidate");
    assert(afs_magic_classification.magic_confirmed);
    assert(!afs_magic_classification.container);

    const std::vector<std::byte> pack_magic{
        std::byte{'P'}, std::byte{'A'}, std::byte{'C'}, std::byte{'K'}};
    const auto pack_magic_classification = gdspaces::ResourceClassifier::classify(
        "unknown/blob.bin", std::span<const std::byte>{pack_magic});
    assert(pack_magic_classification.format == "pack-binary-candidate");
    assert(pack_magic_classification.magic_confirmed);
    assert(!pack_magic_classification.container);

    assert(!gdspaces::ResourceClassifier::is_container_format("afs"));
    assert(!gdspaces::ResourceClassifier::is_container_format("afs-namespace"));
    assert(!gdspaces::ResourceClassifier::is_container_format("afs-binary-candidate"));
    assert(!gdspaces::ResourceClassifier::is_container_format("pack-binary-candidate"));
    assert(gdspaces::ResourceClassifier::is_container_format("nbz"));
    assert(gdspaces::ResourceClassifier::is_container_format("pac"));
    assert(gdspaces::ResourceClassifier::is_container_format("pnst"));

    const auto* scm = registry.find("SCM");
    assert(scm != nullptr);
    assert(scm->valid());
    assert(scm->maturity == integration::IntegrationMaturity::structural);
    assert(scm->parser_id == "formats.scm-structural-v1");
    assert(scm->parser_validation_required);
    assert(scm->write_policy == integration::ResourceWritePolicy::read_only);
    assert(readers.find(scm->parser_id) != nullptr);
    assert(!scm->allows_working_copy());
    assert(!scm->allows_guarded_export());
    assert(scm->writer_modes.empty());
    assert(has_limitation(*scm, "0x50 mesh records"));
    assert(has_limitation(*scm, "read-only"));

    const auto* mod = registry.find("MOD");
    assert(mod != nullptr);
    assert(mod->valid());
    assert(mod->maturity == integration::IntegrationMaturity::structural);
    assert(mod->parser_id == "formats.mod-structural-v1");
    assert(mod->parser_validation_required);
    assert(mod->write_policy == integration::ResourceWritePolicy::read_only);
    assert(readers.find(mod->parser_id) != nullptr);
    assert(!mod->allows_working_copy());
    assert(!mod->allows_guarded_export());
    assert(mod->writer_modes.empty());
    assert(has_limitation(*mod, "three serialized influences"));
    assert(has_limitation(*mod, "not authorized"));

    const auto* shw = registry.find("SHW");
    assert(shw != nullptr);
    assert(shw->valid());
    assert(shw->maturity == integration::IntegrationMaturity::structural);
    assert(shw->parser_id == "formats.shw-structural-v1");
    assert(shw->parser_validation_required);
    assert(shw->write_policy == integration::ResourceWritePolicy::read_only);
    assert(readers.find(shw->parser_id) != nullptr);
    assert(!shw->allows_working_copy());
    assert(!shw->allows_guarded_export());
    assert(shw->writer_modes.empty());
    assert(has_limitation(*shw, "self-contained shadow-hull"));
    assert(has_limitation(*shw, "matrix palette"));
    assert(has_limitation(*shw, "read-only"));

    const auto* dds = registry.find("DDS");
    assert(dds != nullptr);
    assert(dds->valid());
    assert(dds->maturity == integration::IntegrationMaturity::structural);
    assert(dds->parser_id == "formats.dds-dmc3-reader");
    assert(dds->binary_adapter);
    assert(readers.find(dds->parser_id) != nullptr);

    const auto* ptx = registry.find("PTX");
    assert(ptx != nullptr);
    assert(ptx->valid());
    assert(ptx->maturity == integration::IntegrationMaturity::structural);
    assert(ptx->parser_id == "formats.ptx-dmc3-reader");
    assert(ptx->binary_adapter);
    assert(readers.find(ptx->parser_id) != nullptr);

    return 0;
}