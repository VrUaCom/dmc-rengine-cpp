#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/content_tag_census_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/tm2_contract.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

// What the runtime does with a four-byte tag, settled for the whole set.
//
// This project reads ten of them and has been printing every one as though it
// identified the record the way the game identifies it. A sweep of every byte
// comparison chain and every ASCII immediate in the image says the runtime
// compares five, plus two container magics, and does nothing whatever with the
// rest. A tag we read is a fact about the file, not about the game.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Census = dmc3::ContentTagCensusContract;
using Walk = dmc3::RelativeSlotWalkContract;
using Resource = dmc3::ResourceTypeContract;

[[nodiscard]] std::vector<std::byte> payload(std::string_view head) {
    std::vector<std::byte> bytes(0x40U, std::byte{0});
    for (std::size_t index = 0U; index < head.size(); ++index) {
        bytes[index] = static_cast<std::byte>(head[index]);
    }
    return bytes;
}

void the_runtime_compares_five_content_tags() {
    assert(Census::runtime_compares("MOD"));
    assert(Census::runtime_compares("EFM"));
    assert(Census::runtime_compares("SCM"));
    assert(Census::runtime_compares("SHW"));
    assert(Census::runtime_compares("MRP"));
    // And the two container magics, which are compared by the walks.
    assert(Census::runtime_compares("PAC"));
    assert(Census::runtime_compares("PNST"));
}

// The census is only worth anything if it says no to the tags this project
// reads. Every one of these appears in the corpus at offset zero and is
// compared nowhere in the image.
void the_tags_we_read_are_authoring_conventions() {
    for (const auto tag : Census::tags_read_but_never_compared) {
        assert(!Census::runtime_compares(tag));
    }
    // The one that comes closest is stored as an object's type field and still
    // never compared.
    assert(!Census::runtime_compares(Census::stored_but_not_compared));
}

// The dispatcher runs a handler for four of the five; the probe knows a fifth
// that no handler dispatches and no corpus file carries.
void the_two_sites_agree_where_they_overlap() {
    std::size_t dispatched = 0U;
    std::size_t probed = 0U;
    for (const auto& entry : Census::compared_tags) {
        if (entry.dispatcher_compare_va != 0U) {
            dispatched += 1U;
        }
        if (entry.probe_compare_va != 0U) {
            probed += 1U;
        }
        // Every compared tag is compared by at least one of the two sites.
        assert(entry.dispatcher_compare_va != 0U || entry.probe_compare_va != 0U);
    }
    assert(dispatched == 4U);
    assert(probed == Census::compared_tags.size());

    // The four the dispatcher handles are exactly the four the walk contract
    // records handlers for. Two contracts describing one routine have to
    // agree, or one of them is describing a function it did not read.
    assert(Walk::dispatched_payload_tags.size() == dispatched);
    for (const auto tag : Walk::dispatched_payload_tags) {
        bool matched = false;
        for (const auto& entry : Census::compared_tags) {
            if (entry.tag == tag) {
                assert(entry.dispatcher_compare_va != 0U);
                matched = true;
            }
        }
        assert(matched);
    }
}

void the_tm2_reader_matches_four_bytes() {
    using Tm2 = dmc3::Tm2Contract;
    // The whole dword including the NUL, unlike PAC's three.
    static_assert(Tm2::magic_bytes == 4U);
    static_assert(Tm2::magic_bytes > Walk::pac_magic_bytes);
    assert(Tm2::magic_matches(Tm2::magic_dword));
    assert(!Tm2::magic_matches(0x78324D54U)); // "TM2x"

    const auto tm2 = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{payload("TM2")}, false);
    assert(tm2.format == "tm2");
    assert(tm2.magic_confirmed);
    assert(tm2.byte_derived);
    // A texture is not a container, whatever else it is.
    assert(!tm2.container);

    // And the fourth byte matters: `TM2x` is not a TM2.
    const auto near_miss = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{payload("TM2x")}, false);
    assert(near_miss.format != "tm2");
}

void the_dds_reader_accepts_more_than_the_corpus_shows() {
    using Dds = dmc3::DdsPixelFormatContract;
    static_assert(Dds::mappings.size() == 10U);
    static_assert(Dds::aliases.size() == 4U);

    // The DXGI codes the chain stores.
    assert(Dds::format_for("DXT1") == 71U);
    assert(Dds::format_for("DXT5") == 77U);
    assert(Dds::format_for("BC5S") == 84U);
    assert(Dds::format_for("YUY2") == 107U);

    // Aliases resolve to another entry's code rather than carrying their own,
    // which is what the chain does with a jump into an existing store.
    assert(Dds::format_for("DXT2") == Dds::format_for("DXT3"));
    assert(Dds::format_for("DXT4") == Dds::format_for("DXT5"));
    assert(Dds::format_for("BC4U") == Dds::format_for("ATI1"));
    assert(Dds::format_for("BC5U") == Dds::format_for("ATI2"));

    // Nothing outside the table gets a code.
    assert(Dds::format_for("XXXX") == 0U);
    assert(Dds::format_for("") == 0U);

    // Every mapping is distinct, or an alias was recorded as a format.
    for (std::size_t left = 0U; left < Dds::mappings.size(); ++left) {
        for (std::size_t right = left + 1U; right < Dds::mappings.size(); ++right) {
            assert(Dds::mappings[left].fourcc != Dds::mappings[right].fourcc);
            assert(
                Dds::mappings[left].format_code !=
                Dds::mappings[right].format_code);
        }
    }
}

// The probe's fifth tag is a type the first registry already carried a code
// for, so the two contracts have to name the same thing.
static_assert(Resource::TypeCode::mrp == static_cast<Resource::TypeCode>(3));
static_assert(Census::compared_tags[4].tag == "MRP");
static_assert(
    Census::canonical_target_sha256 == Walk::canonical_target_sha256);
static_assert(
    dmc3::Tm2Contract::canonical_target_sha256 ==
    Walk::canonical_target_sha256);
static_assert(
    dmc3::DdsPixelFormatContract::canonical_target_sha256 ==
    Walk::canonical_target_sha256);
// The probe is not the dispatcher; conflating them would make the census a
// census of one site.
static_assert(Census::content_probe_va != Census::dispatcher_va);
static_assert(Census::content_probe_va == Resource::content_type_probe_va);
static_assert(Census::dispatcher_va == Walk::pnst_walk_va);

// The FourCC table earns its place by being used. A texture the game reads and
// this parser does not must say which of the two it is, or a gap in this
// project reads as a property of the format.
void a_refusal_names_whose_gap_it_is() {
    using Dds = dmc3::DdsPixelFormatContract;
    // A format the runtime's chain accepts but this parser has no corpus
    // descriptor mapping for.
    assert(Dds::format_for("BC5S") != 0U);
    assert(Dds::format_for("YUY2") != 0U);
    // A FourCC nothing accepts.
    assert(Dds::format_for("XXXX") == 0U);
    // The two the parser does map are, of course, in the runtime's chain too.
    assert(Dds::format_for("DXT1") != 0U);
    assert(Dds::format_for("DXT5") != 0U);
    // An alias of a mapped format resolves rather than reading as unknown,
    // which is what keeps DXT4 from being reported as a format nothing reads.
    assert(Dds::format_for("DXT4") == Dds::format_for("DXT5"));
}

} // namespace

int main() {
    a_refusal_names_whose_gap_it_is();
    the_runtime_compares_five_content_tags();
    the_tags_we_read_are_authoring_conventions();
    the_two_sites_agree_where_they_overlap();
    the_tm2_reader_matches_four_bytes();
    the_dds_reader_accepts_more_than_the_corpus_shows();
    return 0;
}
