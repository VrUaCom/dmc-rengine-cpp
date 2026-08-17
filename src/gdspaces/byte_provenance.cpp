#include "dmc_rengine/gdspaces/byte_provenance.hpp"

namespace dmc::rengine::gdspaces {
namespace {

[[nodiscard]] bool direct_transform(ByteTransform transform) noexcept {
    return transform == ByteTransform::none ||
        transform == ByteTransform::zip_stored;
}

} // namespace

bool ByteProvenance::valid() const noexcept {
    if (authority_id.empty()) {
        return false;
    }

    switch (kind) {
    case ByteOriginKind::direct_source_span:
        return direct_transform(transform) &&
            stored_size == materialized_size;
    case ByteOriginKind::transformed_source_span:
        return transform == ByteTransform::zip_deflate ||
            transform == ByteTransform::unknown;
    case ByteOriginKind::materialized_parent_span:
        return transform == ByteTransform::none &&
            stored_size == materialized_size;
    }
    return false;
}

bool ByteProvenance::direct_byte_mapping() const noexcept {
    return valid() && kind == ByteOriginKind::direct_source_span &&
        direct_transform(transform);
}

} // namespace dmc::rengine::gdspaces
