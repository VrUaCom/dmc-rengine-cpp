#include "dmc_rengine/integration/format_registry.hpp"

#include <algorithm>
#include <cassert>
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

    const auto* pac = registry.find("PAC");
    assert(pac != nullptr);
    assert(pac->valid());
    assert(pac->write_policy == integration::ResourceWritePolicy::working_copy_only);
    assert(pac->parser_validation_required);
    assert(pac->allows_writer_mode("layout-preserving-packed"));
    assert(pac->allows_writer_mode("runtime-synth-relative-slot"));
    assert(!pac->allows_writer_mode("generic-writer-receipt"));
    assert(has_limitation(*pac, "independently intrinsic"));
    assert(has_limitation(*pac, "typed verified"));

    const auto* pnst = registry.find("PNST");
    assert(pnst != nullptr);
    assert(pnst->valid());
    assert(pnst->write_policy == integration::ResourceWritePolicy::working_copy_only);
    assert(pnst->parser_validation_required);
    assert(pnst->allows_writer_mode("layout-preserving-packed"));
    assert(pnst->allows_writer_mode("runtime-synth-relative-slot"));
    assert(!pnst->allows_writer_mode("generic-writer-receipt"));
    assert(has_limitation(*pnst, "independently intrinsic"));
    assert(has_limitation(*pnst, "typed verified"));

    const auto* nbz = registry.find("NBZ");
    assert(nbz != nullptr);
    assert(nbz->allows_writer_mode("store-overlay-nbz"));
    assert(!nbz->allows_writer_mode("runtime-synth-relative-slot"));

    return 0;
}
