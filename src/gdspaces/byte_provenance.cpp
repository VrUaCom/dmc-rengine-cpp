#include "dmc_rengine/gdspaces/byte_provenance.hpp"

namespace dmc::rengine::gdspaces {

bool ByteProvenance::valid() const noexcept {
    if (authority_id.empty()) {
        return false;
    }

    switch (kind) {
    case ByteOriginKind::direct_source_span:
        return transform == ByteTransform::none &&
            stored_size == materialized_size;
    case ByteOriginKind::transformed_source_span:
        return transform != ByteTransform::none;
    case ByteOriginKind::materialized_parent_span:
        return transform == ByteTransform::none &&
            stored_size == materialized_size;
    }
    return false;
}

bool ByteProvenance::direct_byte_mapping() const noexcept {
    return valid() && kind == ByteOriginKind::direct_source_span &&
        transform == ByteTransform::none;
}

} // namespace dmc::rengine::gdspaces
