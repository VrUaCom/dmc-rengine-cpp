#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace dmc::rengine::profiles::dmc3 {

// How an animation resource gets its name.
//
// The animation registry types a resource by name and never by bytes, and a
// relative-slot container stores no names. That left an open question all the
// way through this project's naming work: where does the name come from?
//
// It comes from a **text script**. The registrar's key is built as
// `"%s/%s"` from a group and a name; the group is built as `"demo/%s"`, and
// the name is a token read out of a keyword-driven cutscene script. So a
// motion is `demo/<demo name>/<file>` and the file half is a line in a script
// somebody authored.
//
// That is the whole answer to "the names are invented": for animation they are
// not invented and not stored in the container either — they are written in a
// script the container never sees.
struct DemoScriptContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    // The token reader every one of these sites drives.
    static constexpr std::uint64_t next_token_va = 0x140322AB0ULL;
    // A line beginning with this is skipped.
    static constexpr std::string_view comment_prefix = "#";
    static constexpr std::uint64_t comment_literal_va = 0x1404DBDD8ULL;
    // The keyword that closes a block.
    static constexpr std::string_view block_terminator = "End";
    static constexpr std::uint64_t block_terminator_va = 0x140506848ULL;

    // The group the registrar is handed, and the two other path shapes the
    // demo system formats.
    static constexpr std::string_view group_format = "demo/%s";
    static constexpr std::uint64_t group_format_va = 0x140506BC8ULL;
    static constexpr std::string_view rooted_group_format = "/demo/%s";
    static constexpr std::string_view rooted_pair_format = "/demo/%s/%s";

    // The three commands that register a resource into the animation registry,
    // with the site that does it. Each matches one animation type code.
    struct RegisteringCommand final {
        std::string_view keyword;
        std::uint64_t keyword_literal_va;
        std::uint64_t register_call_va;
        std::int32_t animation_type_code;
    };

    static constexpr std::array<RegisteringCommand, 3> registering_commands{
        RegisteringCommand{"Motion", 0x140506978ULL, 0x1402D5854ULL, 0},
        RegisteringCommand{"Camera", 0x1405069A4ULL, 0x1402D50C4ULL, 2},
        RegisteringCommand{"Hide", 0x140506980ULL, 0x1402D5414ULL, 3},
    };

    // The script's whole command vocabulary, in pool order. Recorded because a
    // vocabulary is the closest thing to a grammar available without a single
    // script file to read, and because three of these are already known to
    // name resources — the rest name something too.
    static constexpr std::array<std::string_view, 22> commands{
        "Load", "Set", "Offset", "Model", "Motion", "Hide", "Facial",
        "Shape", "WorkRate", "Camera", "Effect", "EffectI", "Light",
        "PadVibe", "Fade", "Quake", "Program", "Message", "Sound",
        "ScrEfc", "Clip", "SetFrame",
    };

    // Argument keywords that follow a command.
    static constexpr std::array<std::string_view, 15> arguments{
        "SkipFrame", "CutFrame", "ClipScale", "ChangeType", "Life",
        "SrcValue", "DistValue", "DistRGBA", "No", "Arg", "Msg",
        "LMotorSt", "LMotorEd", "SMotorSt", "SMotorEd",
    };

    // No script file exists in any supplied corpus, so the grammar joining
    // these tokens is not recovered — only the vocabulary and the three sites
    // that consume it.
    static constexpr bool script_corpus_available = false;

    [[nodiscard]] static constexpr bool is_command(
        std::string_view token) noexcept {
        for (const auto command : commands) {
            if (command == token) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] static constexpr bool registers_a_resource(
        std::string_view token) noexcept {
        for (const auto& entry : registering_commands) {
            if (entry.keyword == token) {
                return true;
            }
        }
        return false;
    }
};

// `.c1d`, the third type of the first resource registry, identified.
//
// The registry carried a code for it and nothing here had ever looked at one.
// Its parser compares the file's first token for equality against
// `ClothSim1D` at `0x1402C8D2D` and bails when it differs, so the format is a
// text file whose first token names it — the only self-identifying text format
// found so far in this game.
//
// The keyword pool sitting immediately after that literal is its vocabulary:
// cloth simulation parameters. That is an identification, not a parser. No
// `.c1d` file exists in any supplied corpus, so the grammar is unrecovered and
// deliberately unguessed.
struct ClothSim1dContract final {
    static constexpr std::string_view canonical_target_sha256 =
        "e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082";

    static constexpr std::string_view extension = "c1d";
    static constexpr std::string_view first_token = "ClothSim1D";
    static constexpr std::uint64_t first_token_literal_va = 0x1405067F8ULL;
    static constexpr std::uint64_t first_token_compare_va = 0x1402C8D2DULL;
    static constexpr bool first_token_identifies_the_file = true;

    static constexpr std::array<std::string_view, 19> keywords{
        "Gravity", "SpringForce", "Damping", "MaxSpeed", "FloorLevel",
        "Cut", "End", "ClothNo", "Wind", "WindLocal", "WindParent",
        "Stiffness", "WindType", "LimitLength", "Bone", "NX", "NY", "NZ",
        "ClothNum",
    };

    static constexpr bool corpus_available = false;

    [[nodiscard]] static constexpr bool is_keyword(
        std::string_view token) noexcept {
        for (const auto keyword : keywords) {
            if (keyword == token) {
                return true;
            }
        }
        return false;
    }
};

} // namespace dmc::rengine::profiles::dmc3
