#include "dmc_rengine/integration/format_registry.hpp"

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

void expect_recognized_read_only(
    const dmc::rengine::integration::FormatIntegrationRegistry& registry,
    std::string_view name) {
    namespace integration = dmc::rengine::integration;
    const auto* descriptor = registry.find(name);
    assert(descriptor != nullptr);
    assert(descriptor->valid());
    assert(descriptor->maturity == integration::IntegrationMaturity::recognized);
    assert(descriptor->write_policy == integration::ResourceWritePolicy::read_only);
    assert(descriptor->parser_id.empty());
}

} // namespace

int main() {
    namespace integration = dmc::rengine::integration;

    const integration::FormatIntegrationRegistry registry;

    const auto* pac = registry.find("PAC");
    assert(pac != nullptr);
    assert(pac->valid());
    assert(pac->write_policy == integration::ResourceWritePolicy::working_copy_only);
    assert(pac->parser_validation_required);
    assert(pac->allows_writer_mode("layout-preserving-packed"));
    assert(pac->allows_writer_mode("runtime-synth-relative-slot"));
    assert(!pac->allows_writer_mode("generic-writer-receipt"));
    assert(has_limitation(*pac, "independently intrinsic"));
    assert(has_limitation(*pac, "semantic child-to-slot"));

    const auto* pnst = registry.find("PNST");
    assert(pnst != nullptr);
    assert(pnst->valid());
    assert(pnst->write_policy == integration::ResourceWritePolicy::working_copy_only);
    assert(pnst->parser_validation_required);
    assert(pnst->allows_writer_mode("layout-preserving-packed"));
    assert(pnst->allows_writer_mode("runtime-synth-relative-slot"));
    assert(!pnst->allows_writer_mode("generic-writer-receipt"));
    assert(has_limitation(*pnst, "typed runtime-synth composition"));
    assert(has_limitation(*pnst, "semantic child-to-slot"));

    const auto* nbz = registry.find("NBZ");
    assert(nbz != nullptr);
    assert(nbz->allows_writer_mode("store-overlay-nbz"));
    assert(!nbz->allows_writer_mode("runtime-synth-relative-slot"));

    // Canonical EXE census propagation: identities are registered without
    // pretending unresolved families already have structural parsers.
    for (const std::string_view family : {
             "efm", "mrp", "mcv", "shw", "efe", "efw", "c1d",
             "clt", "mot", "hid", "tsc", "tm2", "vagp"}) {
        expect_recognized_read_only(registry, family);
    }

    const auto* hits = registry.find("hits");
    assert(hits != nullptr);
    assert(has_limitation(*hits, "DATA_CONFIRMED"));
    assert(has_limitation(*hits, "does not establish HITS as an EXE runtime tag"));
    assert(has_limitation(*hits, "HITS$ is rejected"));

    const auto* dds = registry.find("dds");
    assert(dds != nullptr);
    assert(has_limitation(*dds, "0x140049A8E"));
    assert(has_limitation(*dds, "0x14004AD9D"));

    const auto* tm2 = registry.find("tm2");
    assert(tm2 != nullptr);
    assert(has_limitation(*tm2, "TM2\\0"));
    assert(has_limitation(*tm2, "0x1403365BA"));
    assert(has_limitation(*tm2, "TIM2"));

    const auto* vagp = registry.find("vagp");
    assert(vagp != nullptr);
    assert(has_limitation(*vagp, "0x140032970"));

    const auto* lig2 = registry.find("lig2");
    assert(lig2 != nullptr);
    assert(has_limitation(*lig2, "0x14023ECC9"));

    const auto* mcv = registry.find("mcv");
    assert(mcv != nullptr);
    assert(has_limitation(*mcv, "0x50000000"));

    const auto* efe = registry.find("efe");
    assert(efe != nullptr);
    assert(has_limitation(*efe, "0x1401B9FA0"));

    return 0;
}
