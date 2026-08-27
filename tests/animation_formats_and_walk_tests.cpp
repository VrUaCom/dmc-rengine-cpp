#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/gdspaces/resource_ref.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/relative_slot_walk_contract.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

// The six animation kinds the image knows, and what the container walk does
// and does not do with them.
//
// The question that prompted this was "where are the animation files". The
// answer turns out to be structural: the container walk is not the path to
// them at all. It dispatches four payload tags and recurses into `PNST`, and
// it does not descend into a nested `PAC` — which is exactly where the one
// motion in the corpus lives. Animation is reached by name through a second
// registry, and a container stores no names.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Animation = dmc3::AnimationTypeContract;
using Walk = dmc3::RelativeSlotWalkContract;
using Code = Animation::TypeCode;

// The smallest byte image the motion parser accepts: a header, one track of
// two keys, and a chain that lands exactly on the terminator.
[[nodiscard]] std::vector<std::byte> one_track_motion() {
    constexpr std::size_t track_bytes = 32U + 8U * 2U;
    std::vector<std::byte> bytes(0x54U + track_bytes + 4U, std::byte{0});
    const auto put_u32 = [&bytes](std::size_t at, std::uint32_t value) {
        for (std::size_t index = 0U; index < 4U; ++index) {
            bytes[at + index] =
                static_cast<std::byte>((value >> (8U * index)) & 0xFFU);
        }
    };
    const auto put_u16 = [&bytes](std::size_t at, std::uint16_t value) {
        bytes[at] = static_cast<std::byte>(value & 0xFFU);
        bytes[at + 1U] = static_cast<std::byte>((value >> 8U) & 0xFFU);
    };
    put_u32(0x00U, 0x50U);
    bytes[0x04] = static_cast<std::byte>('M');
    bytes[0x05] = static_cast<std::byte>('O');
    bytes[0x06] = static_cast<std::byte>('T');
    put_u32(0x0CU, 0x42C80000U); // 100.0f
    put_u32(0x50U, 1U);
    put_u16(0x54U, static_cast<std::uint16_t>(track_bytes));
    put_u16(0x56U, 2U);
    put_u32(0x58U, 3U);
    put_u16(0x54U + 0x20U, static_cast<std::uint16_t>(-32768));
    put_u16(0x54U + 0x28U, static_cast<std::uint16_t>(-32668));
    return bytes;
}

void the_image_knows_six_animation_kinds() {
    assert(Animation::type_for_name("pl000.mot") == Code::motion);
    assert(Animation::type_for_name("pl000.mcv") == Code::curve);
    assert(Animation::type_for_name("pl000.cam") == Code::camera);
    assert(Animation::type_for_name("pl000.hid") == Code::hide);
    assert(Animation::type_for_name("pl000.clt") == Code::palette);
    assert(Animation::type_for_name("pl000.tsc") == Code::tsc);
    // Upper case is a separate literal, not a folded comparison.
    assert(Animation::type_for_name("PL000.MOT") == Code::motion);
    assert(Animation::type_for_name("PL000.TSC") == Code::tsc);
}

// The numbered motions. The game reads `pl000.mot1` through `pl000.mot6`, and
// carries no numbered literal for any of them: the comparison is `strstr`, so
// `.mot` is found inside `.mot1`. A tail match — the obvious reading — refuses
// exactly these, which is how this was wrong before.
void a_numbered_motion_is_a_motion() {
    for (const auto* name : {"pl000.mot1", "pl000.mot2", "pl000.mot3",
                             "pl000.mot4", "pl000.mot5", "pl000.mot6"}) {
        assert(Animation::type_for_name(name) == Code::motion);
    }
    // The same rule for the other kinds, since it is one comparison.
    assert(Animation::type_for_name("pl000.cam2") == Code::camera);
    assert(Animation::type_for_name("pl000.hid3") == Code::hide);
    // And the extension need not be at the end at all, because `strstr` does
    // not care where it is.
    assert(Animation::type_for_name("a.mot/b") == Code::motion);
}

// The flag and the implementation must agree. The contract said the match was
// a substring from the day the registry was recovered, and a tail matcher was
// written anyway; nothing failed because nothing tied the two together.
void the_flag_and_the_matcher_agree() {
    static_assert(Animation::extension_matched_as_substring);
    // A tail matcher cannot pass this; a substring matcher cannot fail it.
    assert(Animation::type_for_name("pl000.mot1") == Code::motion);
    assert(Animation::type_for_name("pl000.mot") == Code::motion);
    static_assert(Animation::match_function == "strstr");

    // Chain order decides, not position in the name. `.mot` is compared before
    // `.cam`, so a name carrying both is a motion wherever each one sits.
    static_assert(Animation::first_match_wins_by_chain_order);
    assert(Animation::type_for_name("x.cam.mot") == Code::motion);
    assert(Animation::type_for_name("x.mot.cam") == Code::motion);
}

// Case is enumerated in pairs, so a variant the chain does not carry is not a
// match. Folding case would accept names the game refuses.
void an_uncompared_case_variant_is_not_a_match() {
    assert(Animation::type_for_name("pl000.Mot") == Code::unregistered);
    // `.Clt` exists as a literal in the first registry's block and this
    // classifier never compares it.
    assert(Animation::type_for_name("pl000.Clt") == Code::unregistered);
    assert(Animation::type_for_name("pl000.pac") == Code::unregistered);
    // No dot, so no substring either — the literals all carry one.
    assert(Animation::type_for_name("mot") == Code::unregistered);
    assert(Animation::type_for_name("motion") == Code::unregistered);
}

// One of the six can be found without a name; five cannot.
void only_the_motion_can_be_found_without_a_name() {
    assert(Animation::structure_is_recovered(Code::motion));
    assert(!Animation::structure_is_recovered(Code::curve));
    assert(!Animation::structure_is_recovered(Code::camera));
    assert(!Animation::structure_is_recovered(Code::hide));
    assert(!Animation::structure_is_recovered(Code::palette));
    assert(!Animation::structure_is_recovered(Code::tsc));
}

void the_classifier_reports_the_registry_verdict() {
    // The classifier has to ask the path, not the format it derived from it.
    // `pl000.mot1` has the extension `mot1`, which is not one of the six.
    const auto numbered =
        gdspaces::ResourceClassifier::classify("GData.afs/pl000.mot1");
    assert(numbered.format == "mot1");
    assert(numbered.animation_type == static_cast<std::int32_t>(Code::motion));
    assert(numbered.animation_structure_recovered);

    // A synthesized name is not a name, so the bytes answer instead. This is
    // the one case the registry itself could not type: a container slot has no
    // name for `strstr` to search, and only the structural probe can say what
    // it holds.
    const auto motion = one_track_motion();
    const auto nameless = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{motion}, false);
    assert(nameless.format == "mot");
    assert(nameless.byte_derived);
    assert(nameless.animation_type == static_cast<std::int32_t>(Code::motion));
    assert(nameless.animation_structure_recovered);

    // And with neither a real name nor bytes there is no verdict, which is the
    // honest answer rather than a default.
    const auto nothing =
        gdspaces::ResourceClassifier::classify("slot_0000.mot", {}, false);
    assert(nothing.animation_type == -1);

    const auto curve = gdspaces::ResourceClassifier::classify("GData.afs/pl000.mcv");
    assert(curve.format == "mcv");
    assert(curve.animation_type == static_cast<std::int32_t>(Code::curve));
    // Named, not read — and the classification says which.
    assert(!curve.animation_structure_recovered);
    assert(!curve.byte_derived);
    // An animation is never a container, whatever its name suggests.
    assert(!curve.container);

    const auto other = gdspaces::ResourceClassifier::classify("GData.afs/st001.pac");
    assert(other.animation_type == -1);
    assert(!other.animation_structure_recovered);
}

// The walk facts that answer "why is animation missing from an unpacked
// folder". None of these are about animation formats; all of them are about
// what the walk reaches.
// The verdict has to survive the trip from the classifier to the thing the
// tree actually holds, or the app asks a ref that always says no.
void the_verdict_reaches_the_resource_ref() {
    gdspaces::ResourceRef ref;
    assert(ref.animation_type == -1);
    assert(!ref.animation_structure_recovered);

    const auto classified =
        gdspaces::ResourceClassifier::classify("GData.afs/pl000.hid");
    ref.animation_type = classified.animation_type;
    ref.animation_structure_recovered = classified.animation_structure_recovered;
    assert(ref.animation_type == static_cast<std::int32_t>(Code::hide));
    assert(!ref.animation_structure_recovered);
}

void the_walk_is_not_the_path_to_animation() {
    // The dispatcher handles four payload tags and recurses into PNST.
    static_assert(Walk::dispatched_payload_tags.size() == 4U);
    static_assert(Walk::dispatched_payload_tags[0] == "MOD");
    static_assert(Walk::dispatched_payload_tags[3] == "SHW");
    static_assert(
        Walk::dispatched_payload_handlers.size() ==
        Walk::dispatched_payload_tags.size());
    // None of the four is an animation kind.
    for (const auto tag : Walk::dispatched_payload_tags) {
        std::string lowered;
        for (const auto value : tag) {
            lowered.push_back(
                static_cast<char>(value - 'A' + 'a'));
        }
        assert(!Animation::is_animation_format(lowered));
    }

    // And it does not descend into a nested PAC, which is where the corpus's
    // one motion sits.
    static_assert(!Walk::dispatcher_walks_nested_pac);
}

// Two tags are recognized and skipped, not one.
static_assert(Walk::recognized_not_walked == "EFW");
static_assert(Walk::second_recognized_not_walked == "EFE");
static_assert(
    Walk::recognized_not_walked != Walk::second_recognized_not_walked);
static_assert(
    Walk::recognized_not_walked.size() ==
    Walk::second_recognized_not_walked.size());

// The negative that makes an unpacked extent a product decision.
static_assert(!Walk::walk_computes_child_size);
static_assert(Walk::slot_count_reread_each_iteration);

// The pool finalizer's state transition, and the fields it reads.
static_assert(Walk::pool_slot_state_walked == 2);
static_assert(Walk::pool_slot_state_after_walk == 3);
static_assert(Walk::pool_slot_state_after_walk != Walk::pool_slot_state_walked);
static_assert(Walk::pool_slot_state_offset < Walk::pool_slot_finalizer_offset);
static_assert(Walk::pool_slot_finalizer_offset < Walk::pool_slot_payload_offset);
static_assert(Walk::pool_slot_payload_offset < Walk::pool_slot_stride);
static_assert(Walk::pool_slot_payload_may_be_a_bare_record);

// The animation registry's literals: ten of its own, twelve compared.
static_assert(Animation::own_literal_count == 10U);
static_assert(Animation::compared_literal_count == 12U);
static_assert(
    Animation::compared_literal_count == Animation::extension_types.size());
static_assert(
    Animation::own_literal_bytes == Animation::own_literal_count * 8U);
// The two it borrows sit outside its own block.
static_assert(
    Animation::shared_lowercase_literal_va <
    Animation::extension_literal_table_va);
static_assert(
    Animation::shared_uppercase_literal_va <
    Animation::extension_literal_table_va);
static_assert(Animation::comparison_order_is_significant);
static_assert(Animation::registry_key_arity == 2U);

// Both contracts describe the same image.
static_assert(
    Animation::canonical_target_sha256 == Walk::canonical_target_sha256);

} // namespace

int main() {
    the_image_knows_six_animation_kinds();
    a_numbered_motion_is_a_motion();
    the_flag_and_the_matcher_agree();
    an_uncompared_case_variant_is_not_a_match();
    only_the_motion_can_be_found_without_a_name();
    the_classifier_reports_the_registry_verdict();
    the_verdict_reaches_the_resource_ref();
    the_walk_is_not_the_path_to_animation();
    return 0;
}
