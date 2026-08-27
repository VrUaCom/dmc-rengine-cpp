#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/profiles/dmc3/animation_type_contract.hpp"
#include "dmc_rengine/profiles/dmc3/demo_script_contract.hpp"
#include "dmc_rengine/profiles/dmc3/resource_type_contract.hpp"

#include <cassert>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

// Where an animation resource's name comes from.
//
// The registry types by name and never by bytes; a container stores no names.
// The gap is filled by a text script: the group is `demo/<name>` and the file
// half is a token on a script line. So for animation the names are neither
// invented by this tool nor stored in the container — they are written in a
// script the container never sees.

namespace {

namespace gdspaces = dmc::rengine::gdspaces;
namespace dmc3 = dmc::rengine::profiles::dmc3;
using Demo = dmc3::DemoScriptContract;
using Cloth = dmc3::ClothSim1dContract;
using Animation = dmc3::AnimationTypeContract;

[[nodiscard]] std::vector<std::byte> text_of(std::string_view content) {
    std::vector<std::byte> bytes;
    for (const auto value : content) {
        bytes.push_back(static_cast<std::byte>(value));
    }
    return bytes;
}

void three_script_commands_register_a_resource() {
    assert(Demo::registers_a_resource("Motion"));
    assert(Demo::registers_a_resource("Camera"));
    assert(Demo::registers_a_resource("Hide"));
    // The other commands are commands and do not reach the registry.
    assert(Demo::is_command("Light"));
    assert(!Demo::registers_a_resource("Light"));
    assert(!Demo::registers_a_resource("Sound"));
    // An argument keyword is not a command.
    assert(!Demo::is_command("SkipFrame"));
    assert(!Demo::is_command("nonsense"));
}

// Each registering command must line up with the animation type its site
// stores, or the mapping is a coincidence rather than a reading.
void each_registering_command_matches_its_animation_type() {
    for (const auto& entry : Demo::registering_commands) {
        assert(Demo::is_command(entry.keyword));
        assert(entry.keyword_literal_va != 0U);
        assert(entry.register_call_va != 0U);
    }
    using Code = Animation::TypeCode;
    assert(Demo::registering_commands[0].animation_type_code ==
           static_cast<std::int32_t>(Code::motion));
    assert(Demo::registering_commands[1].animation_type_code ==
           static_cast<std::int32_t>(Code::camera));
    assert(Demo::registering_commands[2].animation_type_code ==
           static_cast<std::int32_t>(Code::hide));
    // Three distinct sites, three distinct codes.
    assert(Demo::registering_commands[0].register_call_va !=
           Demo::registering_commands[1].register_call_va);
    assert(Demo::registering_commands[1].register_call_va !=
           Demo::registering_commands[2].register_call_va);
}

// The key the registrar builds is a pair, and the group half is itself
// formatted. Both halves are recorded so a reader cannot quietly assume a
// resource is identified by a bare filename.
void the_key_is_a_group_and_a_name() {
    static_assert(Animation::registry_key_arity == 2U);
    static_assert(Animation::registry_key_format == "%s/%s");
    static_assert(Demo::group_format == "demo/%s");
    // The rooted variants are the same path with a leading separator, so a
    // reader that strips one must not treat them as different namespaces.
    static_assert(Demo::rooted_group_format.substr(1U) == Demo::group_format);
    static_assert(Demo::rooted_pair_format.starts_with(Demo::rooted_group_format));
}

// `.c1d` was a type code with nothing behind it. It is a text format that
// names itself.
void cloth_sim_identifies_itself_by_its_first_token() {
    static_assert(Cloth::first_token == "ClothSim1D");
    static_assert(Cloth::first_token_identifies_the_file);
    // The registry already carried the extension; the two must agree.
    static_assert(
        dmc3::ResourceTypeContract::type_for_name(".c1d") ==
        dmc3::ResourceTypeContract::TypeCode::c1d);

    const auto payload = text_of("ClothSim1D\r\nClothNum 2\r\nEnd\r\n");
    const auto classified = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin", std::span<const std::byte>{payload}, false);
    assert(classified.format == "c1d");
    assert(classified.magic_confirmed);
    assert(classified.byte_derived);
    assert(!classified.container);

    // A text file that is not one must not be claimed.
    const auto other = gdspaces::ResourceClassifier::classify(
        "slot_0000.bin",
        std::span<const std::byte>{text_of("# GAME\r\nEnd\r\n")}, false);
    assert(other.format != "c1d");

    assert(Cloth::is_keyword("Stiffness"));
    assert(Cloth::is_keyword("ClothNum"));
    assert(!Cloth::is_keyword("Motion"));
}

// What is deliberately not claimed. No script and no cloth file exists in any
// supplied corpus, so both vocabularies are identifications and neither is a
// grammar. A later edit that flips these has to say why.
static_assert(!Demo::script_corpus_available);
static_assert(!Cloth::corpus_available);
static_assert(Demo::commands.size() == 22U);
static_assert(Demo::arguments.size() == 15U);
static_assert(Cloth::keywords.size() == 19U);
// The two vocabularies are disjoint apart from the shared block terminator,
// which both parsers use to close a block.
static_assert(Cloth::is_keyword(Demo::block_terminator));
static_assert(!Demo::is_command(Demo::block_terminator));
static_assert(
    Demo::canonical_target_sha256 == Cloth::canonical_target_sha256);
static_assert(
    Demo::canonical_target_sha256 == Animation::canonical_target_sha256);

} // namespace

int main() {
    three_script_commands_register_a_resource();
    each_registering_command_matches_its_animation_type();
    the_key_is_a_group_and_a_name();
    cloth_sim_identifies_itself_by_its_first_token();
    return 0;
}
