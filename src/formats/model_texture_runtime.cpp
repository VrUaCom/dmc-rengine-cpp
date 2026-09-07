#include "dmc_rengine/formats/model_texture_runtime.hpp"

namespace dmc::rengine::formats::model_family {
namespace {

constexpr auto tex0 = decode_legacy_gs_tex0(0xB2C88AD59D354123ULL);
static_assert(tex0.tbp0 == 0x0123U);
static_assert(tex0.tbw == 0x15U);
static_assert(tex0.psm == 0x13U);
static_assert(tex0.tw == 0x07U);
static_assert(tex0.th == 0x06U);
static_assert(tex0.tcc);
static_assert(tex0.tfx == 0x02U);
static_assert(tex0.cbp == 0x0456U);
static_assert(tex0.cpsm == 0x09U);
static_assert(tex0.csm);
static_assert(tex0.csa == 0x12U);
static_assert(tex0.cld == 0x05U);

constexpr auto miptbp1 = decode_legacy_gs_miptbp1(0x0D03338C22248111ULL);
static_assert(miptbp1.mip1.tbp == 0x0111U);
static_assert(miptbp1.mip1.tbw == 0x12U);
static_assert(miptbp1.mip2.tbp == 0x0222U);
static_assert(miptbp1.mip2.tbw == 0x23U);
static_assert(miptbp1.mip3.tbp == 0x0333U);
static_assert(miptbp1.mip3.tbw == 0x34U);

static_assert(RuntimeTextureDescriptorAbi::mip_source_pointer_fields[0] == 0x00U);
static_assert(RuntimeTextureDescriptorAbi::mip_source_pointer_fields[1] == 0x08U);
static_assert(RuntimeTextureDescriptorAbi::mip_source_pointer_fields[2] == 0x10U);
static_assert(RuntimeTextureDescriptorAbi::mip_source_pointer_fields[3] == 0x18U);

} // namespace
} // namespace dmc::rengine::formats::model_family
