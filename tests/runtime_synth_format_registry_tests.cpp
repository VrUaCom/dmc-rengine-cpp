#include "dmc_rengine/integration/format_registry.hpp"
#include "dmc_rengine/integration/native_reader_registry.hpp"
#include "dmc_rengine/profiles/dmc3/container_parsers.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>

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
    namespace integration = dmc::rengine::integration;

    const integration::FormatIntegrationRegistry registry;
    const integration::NativeReaderModuleRegistry readers;
    const auto containers =
        dmc::rengine::profiles::dmc3::make_container_parser_registry();

    assert(readers.size() == 8U);
    for (const std::string_view parser_id : {
             "formats.dds-dmc3-reader",
             "formats.ptx-dmc3-reader",
             "formats.hits-record-scanner",
             "formats.dca-record-scanner",
             "formats.lig2-record-scanner",
             "formats.stage-txt-lexer",
             "formats.scm-structural-v1",
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
    assert(has_limitation(*scm, "not a promoted SCM writer"));
    assert(has_limitation(*scm, "Header +0x14"));

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
