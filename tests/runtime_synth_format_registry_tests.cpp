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
    assert(has_limitation(*pac, "typed verified runtime-synth"));
    assert(has_limitation(*pac, "semantic child-to-slot"));

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

    const auto* nbz = registry.find("NBZ");
    assert(nbz != nullptr);
    assert(nbz->allows_writer_mode("store-overlay-nbz"));
    assert(!nbz->allows_writer_mode("runtime-synth-relative-slot"));

    const auto* afs_namespace = registry.find("AFS-NAMESPACE");
    assert(afs_namespace != nullptr);
    assert(afs_namespace->valid());
    assert(afs_namespace->parser_id.empty());
    assert(!afs_namespace->allows_working_copy());
    assert(has_limitation(*afs_namespace, "logical DMC3-HD namespace"));

    const auto* afs_binary = registry.find("AFS-BINARY-CANDIDATE");
    assert(afs_binary != nullptr);
    assert(afs_binary->valid());
    assert(!afs_binary->allows_guarded_export());
    assert(afs_binary->writer_modes.empty());
    assert(has_limitation(*afs_binary, "raw binary AFS artifact"));

    const auto* pack_binary = registry.find("PACK-BINARY-CANDIDATE");
    assert(pack_binary != nullptr);
    assert(pack_binary->valid());
    assert(!pack_binary->allows_guarded_export());
    assert(pack_binary->writer_modes.empty());
    assert(has_limitation(*pack_binary, "Web DMC Rengine product"));

    return 0;
}
