// Motion script, CLT cloth, TSC UV scroll, character collision tables and
// stage HITS queries carried over from Native Reader with their regression
// blocks (docs/research/dmc3-native-reader-code-facts-2026-10-01.md and the
// notes each header cites).
#include "dmc_rengine/profiles/dmc3/cloth_chain.hpp"
#include "dmc_rengine/profiles/dmc3/collision_shapes.hpp"
#include "dmc_rengine/profiles/dmc3/environment_collision.hpp"
#include "dmc_rengine/profiles/dmc3/motion_script.hpp"
#include "dmc_rengine/profiles/dmc3/uv_scroll.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <vector>

namespace {

namespace motion = dmc::rengine::profiles::dmc3::motion;

bool near(float a, float b) { return std::fabs(a - b) < 1.0e-3F; }

void put_u32(std::vector<std::uint8_t>& b, std::size_t o, std::uint32_t v) { std::memcpy(b.data() + o, &v, 4U); }
void put_f32(std::vector<std::uint8_t>& b, std::size_t o, float v) { std::memcpy(b.data() + o, &v, 4U); }

std::vector<std::uint8_t> make_hits() {
    // One valid HITS cell and one triangle-plane record.  The fixture checks
    // the environment-collision boundary without assigning surface semantics
    // to raw flags.
    std::vector<std::uint8_t> bytes(0x88U, 0U);
    bytes[0] = 'H'; bytes[1] = 'I'; bytes[2] = 'T'; bytes[3] = 'S';
    put_u32(bytes, 0x04U, 0x88U);
    put_f32(bytes, 0x08U, 0.0F);   put_f32(bytes, 0x0CU, 0.0F);   put_f32(bytes, 0x10U, 0.0F);
    put_f32(bytes, 0x14U, 100.0F); put_f32(bytes, 0x18U, 100.0F); put_f32(bytes, 0x1CU, 100.0F);
    put_f32(bytes, 0x20U, 100.0F); put_f32(bytes, 0x24U, 100.0F); put_f32(bytes, 0x28U, 100.0F);
    put_u32(bytes, 0x2CU, 1U); put_u32(bytes, 0x30U, 1U); put_u32(bytes, 0x34U, 1U);
    put_u32(bytes, 0x38U, 1U); put_u32(bytes, 0x3CU, 0x3CU); put_u32(bytes, 0x40U, 0x48U);
    put_u32(bytes, 0x44U, 0x40U);  // cell list at file offset 0x48
    put_u32(bytes, 0x48U, 0U);      // triangle byte offset 0
    put_u32(bytes, 0x4CU, 0xFFFFFFFFU);
    put_u32(bytes, 0x50U, 0x12345678U);
    put_f32(bytes, 0x54U, 0.0F);  put_f32(bytes, 0x58U, 0.0F);  put_f32(bytes, 0x5CU, 0.0F);
    put_f32(bytes, 0x60U, 10.0F); put_f32(bytes, 0x64U, 0.0F);  put_f32(bytes, 0x68U, 0.0F);
    put_f32(bytes, 0x6CU, 0.0F);  put_f32(bytes, 0x70U, 0.0F);  put_f32(bytes, 0x74U, 10.0F);
    put_f32(bytes, 0x78U, 0.0F);  put_f32(bytes, 0x7CU, 1.0F);  put_f32(bytes, 0x80U, 0.0F);
    put_f32(bytes, 0x84U, 0.0F);
    return bytes;
}


}  // namespace

int main() {
    // CLT text (";pl000_02.clt" layout) and one chain solver node (0x1402C9450).
    {
        constexpr std::string_view clt =
            ";test.clt\n\nClothNum\t1\n\nClothNo     0\nClothId     0\n"
            "Gravity     0.500000  0.000000  0.000000\nSpringForce 0.020000\n"
            "MaxSpeed    50.000000\nStiffness   0.000000\n"
            "Wind        0.000000  0.000000  0.000000\nWindLocal   1\nWindParent  0\n"
            "WindType    1\nBone      1    Y\nBone      2    NZ\nEnd\n$\n";
        const auto blocks = motion::parse_clt(clt);
        assert(blocks.size() == 1U);
        const auto& params = blocks.front();
        assert(near(params.gravity[0], 0.5F) && near(params.stiffness, 0.0F));
        assert(near(params.spring_force, 0.02F) && params.wind_local && params.limit_length);
        assert(near(params.damping, 0.99F) && near(params.floor_level, -1000000.0F));
        assert(params.bones.size() == 2U && params.bones[0].node == 1U &&
               params.bones[0].axis == 1U && params.bones[1].axis == 5U);
        assert(motion::parse_clt("ClothNo 0\n").empty());

        motion::ClothState state;
        state.params = params;
        state.sim.assign(2U, {});
        state.velocity.assign(2U, {});
        state.axis_by_node = {-1, 1};
        std::array<float, 16> parent{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        auto target = parent;
        target[13] = -10.0F;  // rest: 10 below the parent, +Y points at it
        state.sim[1] = target;
        std::array<float, 16> world{};
        for (int frame = 0; frame < 300; ++frame) {
            world = motion::step_cloth_node(state, 1U, target, parent, parent, 10.0F, 1.0F);
        }
        const float length = std::sqrt(world[12] * world[12] + world[13] * world[13] +
                                       world[14] * world[14]);
        assert(near(length, 10.0F));        // LimitLength keeps the bone length
        assert(world[12] > 5.0F);           // sideways gravity swung it out
        const float up = (0.0F - world[12]) * world[4] + (0.0F - world[13]) * world[5] -
                         world[14] * world[6];
        assert(near(up, 10.0F));            // Y axis still points at the parent
        // Capsule push-out (0x1402D0630): a node inside a capsule lands on its
        // surface and loses its x/z velocity.
        {
            motion::ClothState hit_state = state;
            hit_state.axis_by_node[1] = 1;
            hit_state.sim[1] = target;
            hit_state.velocity[1] = {1.0F, 0.0F, 1.0F};
            const std::array<motion::WorldCapsule, 1> capsule{{
                {{3.0F, -20.0F, 0.0F}, {3.0F, 0.0F, 0.0F}, 5.0F}}};
            const auto pushed = motion::step_cloth_node(hit_state, 1U, target, parent, parent,
                                                        10.0F, 1.0F, capsule);
            const float dx = pushed[12] - 3.0F;
            const float dz = pushed[14];
            assert(std::sqrt(dx * dx + dz * dz) > 4.0F);   // outside the core
            assert(std::fabs(hit_state.velocity[1][2]) < 1.0F);
            assert(motion::kPlayerCoatCapsules.size() == 6U &&
                   motion::kPlayerCoatCapsules[0].host_joint == 3U &&
                   near(motion::kPlayerCoatCapsules[1].radius, 18.0F));
        }
        state.axis_by_node[1] = -1;
        const auto kept = motion::step_cloth_node(state, 1U, target, parent, parent, 10.0F, 1.0F);
        assert(near(kept[13], -10.0F));     // not simulated: rest target unchanged

        // ClothNum 2 (pl001_02.clt layout): every ClothNo is its own chain
        // with its own parameters; only the first chain collides.
        constexpr std::string_view two =
            ";two.clt\n\nClothNum\t2\n\nClothNo     0\nGravity 0 -0.02 0\nBone 1 Y\nEnd\n\n"
            "ClothNo     1\nGravity 0 -0.01 0\nStiffness 0.5\nBone 2 Y\nEnd\n$\n";
        const auto pair = motion::parse_clt(two);
        assert(pair.size() == 2U && pair[1].bones.size() == 1U && pair[1].bones[0].node == 2U);
        motion::ClothState chains;
        chains.params = pair[0];
        chains.blocks = pair;
        chains.block_by_node = {0U, 0U, 1U};
        assert(near(chains.params_for(1U).gravity[1], -0.02F));
        assert(near(chains.params_for(2U).stiffness, 0.5F));
        assert(near(chains.params_for(9U).gravity[1], -0.02F));  // unlisted: block 0
        chains.sim.assign(3U, target);
        chains.velocity.assign(3U, {});
        chains.axis_by_node = {-1, 1, 1};
        const std::array<motion::WorldCapsule, 1> core{{
            {{0.0F, -20.0F, 0.0F}, {0.0F, 0.0F, 0.0F}, 3.0F}}};
        const auto first = motion::step_cloth_node(chains, 1U, target, parent, parent, 10.0F, 1.0F, core);
        const auto second = motion::step_cloth_node(chains, 2U, target, parent, parent, 10.0F, 1.0F, core);
        assert(std::fabs(first[12]) + std::fabs(first[14]) > 2.0F);   // pushed out
        assert(std::fabs(second[12]) + std::fabs(second[14]) < 0.1F); // block 1: no capsule
    }

    // TSC (em028_013 layout): two scrolls, linear v and eased u with drift.
    {
        constexpr std::string_view tsc =
            "\r\n.TSC\t\r\n\t# RELATIVE\t\r\n\t\t<Start\r\n\t\t\tScrlNo\t\t0\r\n"
            "\t\t\tScrlType\t1\r\n\t\t\tTexNo\t\t3\r\n\t\t\tDirUV\t\tstay,   up\r\n"
            "\t\t\tTimeUV\t\t0, \t90\r\n\t\tEnd>\r\n\t\t<Start\r\n\t\t\tScrlNo\t\t1\r\n"
            "\t\t\tScrlType\t3\r\n\t\t\tTexNo\t\t2\r\n\t\t\tDirUV\t\tleft, stay\r\n"
            "\t\t\tTimeUV\t\t400,      0\r\n\t\t\tMinimumUV\t0.0001,\t0.005\r\n\t\tEnd>\r\n"
            "\t\t<Finish>\r\n$\t\r\n\t# ABSOLUTE\r\n\t\t<Start ScrlNo 7 ScrlType 1 End>\r\n";
        assert(motion::looks_like_tsc(tsc));
        const auto records = motion::parse_tsc(tsc);
        assert(records.size() == 2U);  // '$' ends the text: ABSOLUTE is never read
        assert(records[0].number == 0U && records[0].type == 1U && records[0].texture == 3);
        assert(records[0].direction[0] == 0 && records[0].direction[1] == 1);
        assert(near(records[0].time[1], 90.0F) && !records[0].has_minimum);
        assert(records[1].type == 3U && records[1].direction[0] == 1 && records[1].has_minimum);
        assert(near(records[1].time[0], 400.0F) && near(records[1].minimum[1], 0.005F));
        // Linear: one full texture per 90 frames, quantized to 1/4096.
        const auto linear = motion::scroll_offset(records[0], 45.0F);
        assert(linear && near((*linear)[0], 0.0F) && near((*linear)[1], 0.5F));
        // Eased: (cos((1 - p) pi) + 1) / 2 plus the MinimumUV drift; v stays.
        const auto eased = motion::scroll_offset(records[1], 200.0F);
        const float expected = std::floor(
            ((std::cos(0.5F * 3.14159265F) + 1.0F) * 0.5F + 0.02F) * 4096.0F) / 4096.0F;
        assert(eased && std::fabs((*eased)[0] - expected) < 0.0005F && near((*eased)[1], 0.0F));
        // Type 4 (0x14030B820): linear, DirUV reverses every TurnTimeUV steps.
        motion::ScrollRecord ping;
        ping.type = 4U;
        ping.direction = {1, 0};
        ping.time = {10.0F, 1.0F};
        ping.turn_time = {5.0F, 5.0F};
        ping.has_turn_time = true;
        assert(near((*motion::scroll_offset(ping, 5.0F))[0], 0.5F));
        assert(std::fabs((*motion::scroll_offset(ping, 7.0F))[0] - 0.3F) < 0.001F);
        assert((*motion::scroll_offset(ping, 10.0F))[0] < 0.001F ||
               (*motion::scroll_offset(ping, 10.0F))[0] > 0.999F);
        // Type 10 (0x14030BB50): (1 - (facing + 1) / 2) * RateUV * dir.
        motion::ScrollRecord facing;
        facing.type = 10U;
        facing.direction = {1, 0};
        facing.rate = {0.5F, 0.0F};
        auto facing_state = motion::start_scroll(facing);
        motion::step_scroll(facing, facing_state, 0.0F);
        assert(facing_state.output[0] == 1024 && facing_state.output[1] == 0);
        assert(!motion::scroll_offset(motion::ScrollRecord{.type = 7U}, 3.0F).has_value());
        assert(motion::object_scroll_number(0x01020000U) == 0);
        assert(motion::object_scroll_number(0x02000000U) == 1);
        assert(motion::object_scroll_number(0x00020000U) == -1);
        assert(!motion::looks_like_tsc(";pl000_02.clt\nClothNo 0\n"));
    }

    // Motion script (pl000.pac slot 5): header -> bank list -> MOT scripts;
    // opcode 3 byte 2 low 6 bits = weapon state, opcode 0 = wait for frame.
    {
        const std::vector<std::uint8_t> file{
            0x06, 0x00, 0x00, 0x00, 0x00, 0x00,  // +0: table at 6
            0x04, 0x00,                          // table[0] -> bank list at 6 + 4
            0x00, 0x00,
            0x04, 0x00, 0xFF, 0xFF,              // bank 0 at 10 + 4
            0x04, 0x00, 0xFF, 0xFF,              // bank 0: MOT 0 script at 14 + 4
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7F,  // play MOT
            0x03, 0x80, 0x02, 0x00, 0x00, 0x00,  // state 2 (right hand)
            0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,  // wait frame 10
            0x03, 0x80, 0x83, 0x00, 0x00, 0x00,  // state 3 (flag bits dropped)
            0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00,  // until the end
        };
        const auto script = motion::MotionScriptFile::parse(file);
        assert(script && script->bank_count() == 1U);
        const auto keys = script->weapon_states(0U, 0U);
        assert(keys.size() == 2U && keys[0].state == 2U && keys[1].state == 3U);
        assert(motion::weapon_state_at(keys, 0.0F) == 2U);
        assert(motion::weapon_state_at(keys, 10.0F) == 2U);   // runs once past frame 10
        assert(motion::weapon_state_at(keys, 11.0F) == 3U);
        assert(script->script_count(0U) == 1U);
        const auto summary = script->summarize(0U, 0U);
        assert(summary && summary->waits == 1U && summary->last_frame == 10U &&
               summary->opcodes[3] == 2U && !summary->loops);
        assert(!script->summarize(0U, 1U).has_value());
        assert(motion::MotionScriptFile::looks_like(file));
        auto broken = file;
        broken[18] = 0x03;  // first script no longer starts with "play MOT"
        assert(!motion::MotionScriptFile::looks_like(broken));
        assert(script->nested() && !script->has_resources());

        // Enemy form (bind mode 1): A lists banks directly, B maps actions
        // to MOT ids (group * 100 + slot) with a loop flag.
        const std::vector<std::uint8_t> enemy{
            0x06, 0x00, 0x1E, 0x00, 0xFF, 0xFF,  // A = 6, B = 30, C unused
            0x04, 0x00, 0xFF, 0xFF,              // A: bank 0 at 6 + 4
            0x06, 0x00, 0x0C, 0x00, 0xFF, 0xFF,  // bank 0: actions at 10 + 6, 10 + 12
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00,  // action 0: play (bank 0, action 0)
            0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00,  // ... until the end
            0x01, 0x00,                          // action 1 (truncated play)
            0x04, 0x00, 0xFF, 0xFF,              // B: bank 0 at 30 + 4
            0x06, 0x00, 0x0E, 0x00, 0xFF, 0xFF,  // B bank 0: records at 34 + 6, 34 + 14
            0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0C, 0x00,  // 1 record: obj 0 loop, MOT 12
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00,  // 1 record: obj 0 once, MOT 101
        };
        const auto enemy_script = motion::MotionScriptFile::parse(enemy);
        assert(enemy_script && !enemy_script->nested() && enemy_script->bank_count() == 1U);
        assert(enemy_script->script_count(0U) == 2U && enemy_script->has_resources());
        const auto played = enemy_script->resources(0U, 0U);
        assert(played.size() == 1U && played[0].id == 12U && played[0].loop == 1U &&
               played[0].group() == 0U && played[0].slot() == 12U);
        assert(enemy_script->resources(0U, 1U).front().id == 101U);
        assert(enemy_script->resources(0U, 2U).empty());      // past the 0xFFFF end
        assert(enemy_script->actions_for(101U).size() == 1U &&
               enemy_script->actions_for(101U)[0].action == 1U);
        assert((enemy_script->resource_ids() == std::vector<std::uint16_t>{12U, 101U}));
        const std::vector<motion::MotionPack> packs{{2U, {0U, 5U}}, {3U, {12U, 13U}}, {4U, {1U}}};
        const auto groups = motion::bind_motion_groups(*enemy_script, packs, "em999.pac");
        assert(groups.size() == 2U && groups[0].group == 0U && groups[0].archive_slot == 3U &&
               !groups[0].exe_confirmed && groups[1].group == 1U && groups[1].archive_slot == 4U);
        const std::vector<motion::MotionPack> em028_packs{{2U, {0U, 12U}}, {3U, {20U}}};
        const auto em028_groups = motion::bind_motion_groups(*enemy_script, em028_packs, "st/EM028.PAC");
        assert(em028_groups[0].archive_slot == 2U && em028_groups[0].exe_confirmed);
        assert(em028_groups[1].archive_slot == 3U && em028_groups[1].exe_confirmed);  // class array
        assert(motion::MotionScriptFile::looks_like(enemy));
    }
    // Collision tables (ICollisionHandle, 0x14005C260): 80-byte shape
    // records (2 sphere, 3 box, 4 capsule) and 4-byte attack entries.
    {
        namespace collision = dmc::rengine::profiles::dmc3::collision;
        std::vector<std::uint8_t> shapes(3U * collision::kShapeRecordSize, 0U);
        const auto put_f = [&shapes](std::size_t o, float v) { std::memcpy(shapes.data() + o, &v, 4U); };
        shapes[0] = 2U;                      // sphere: centre (0, 50, 0), radius 40
        put_f(0x14U, 50.0F); put_f(0x1CU, 1.0F); put_f(0x20U, 40.0F);
        shapes[0x50] = 4U;                   // capsule a (0,500,0) b (0,0,0) r 120
        put_f(0x50U + 0x14U, 500.0F); put_f(0x50U + 0x1CU, 1.0F); put_f(0x50U + 0x2CU, 1.0F);
        put_f(0x50U + 0x30U, 120.0F);
        shapes[0xA0] = 3U;                   // box: centre (41,40,5) rot (13,11,0) size (58,54,5)
        put_f(0xA0U + 0x10U, 41.0F); put_f(0xA0U + 0x14U, 40.0F); put_f(0xA0U + 0x18U, 5.0F);
        put_f(0xA0U + 0x1CU, 13.0F); put_f(0xA0U + 0x20U, 11.0F);
        put_f(0xA0U + 0x28U, 58.0F); put_f(0xA0U + 0x2CU, 54.0F); put_f(0xA0U + 0x30U, 5.0F);
        assert(collision::looks_like_shape_table(shapes));
        const auto parsed = collision::parse_shapes(shapes);
        assert(parsed.size() == 3U && near(parsed[0].a[1], 50.0F) && near(parsed[0].radius, 40.0F));
        assert(near(parsed[1].a[1], 500.0F) && near(parsed[1].b[1], 0.0F) && near(parsed[1].radius, 120.0F));
        assert(near(parsed[2].b[0], 13.0F) && near(parsed[2].size[1], 54.0F));
        auto bad = shapes;
        bad[3] = 1U;                         // padding must stay zero
        assert(!collision::looks_like_shape_table(bad));
        const std::vector<std::uint8_t> index{6, 0, 0, 0, 2, 3, 1, 0, 0, 0, 0, 0, 1, 9, 2, 0};
        assert(collision::looks_like_attack_index(index, 3U));
        assert(!collision::looks_like_attack_index(index, 2U));   // shape 2 missing
        const auto attacks = collision::parse_attack_index(index);
        assert(attacks.size() == 4U && attacks[1].bone == 3U && attacks[1].shape == 1U &&
               attacks[2].mask == 0U && attacks[3].bone == 9U);
    }
    // Stage collision queries (0x14005E880 segment test and the Reader's
    // character proxy) on a floor y = 0 and a wall x = 100 facing -x.
    {
        namespace ec = dmc::rengine::profiles::dmc3::environment_collision;
        using dmc::rengine::profiles::dmc3::Vec3;
        ec::Source source;
        source.bounds_min = {-500.0F, -10.0F, -500.0F};
        source.bounds_max = {500.0F, 500.0F, 500.0F};
        // Floor (normal +y, plane y = 0), two triangles.
        source.triangles.push_back({0U, {-500.0F, 0.0F, -500.0F}, {-500.0F, 0.0F, 500.0F},
                                    {500.0F, 0.0F, 500.0F}, {0.0F, 1.0F, 0.0F}, 0.0F});
        source.triangles.push_back({0U, {-500.0F, 0.0F, -500.0F}, {500.0F, 0.0F, 500.0F},
                                    {500.0F, 0.0F, -500.0F}, {0.0F, 1.0F, 0.0F}, 0.0F});
        // Wall x = 100 (normal -x: n.p + d = 0 with d = 100), category 0x0002.
        source.triangles.push_back({0x00020000U, {100.0F, 0.0F, -500.0F}, {100.0F, 500.0F, -500.0F},
                                    {100.0F, 0.0F, 500.0F}, {-1.0F, 0.0F, 0.0F}, 100.0F});
        source.triangles.push_back({0x00020000U, {100.0F, 500.0F, -500.0F}, {100.0F, 500.0F, 500.0F},
                                    {100.0F, 0.0F, 500.0F}, {-1.0F, 0.0F, 0.0F}, 100.0F});

        // Kinds: one per distinct flags value, ascending; floors/walls counted.
        {
            const auto kinds = ec::kinds(source);
            assert(kinds.size() == 2U && kinds[0].flags == 0U && kinds[0].count == 2U &&
                   kinds[0].floors == 2U && kinds[1].flags == 0x00020000U && kinds[1].walls == 2U);
            const auto line_kinds = ec::debug_line_kinds(source, kinds);
            assert(line_kinds.size() == 12U && line_kinds[0] == 0U && line_kinds[6] == 1U);
        }

        // Nearest hit wins: a diagonal shot meets the wall before the floor.
        const auto wall = ec::segment_hit(source, {0.0F, 100.0F, 0.0F}, {200.0F, -100.0F, 0.0F});
        assert(wall && wall->triangle >= 2U);
        assert(std::fabs(wall->point.x - 100.0F) < 1.0e-3F && std::fabs(wall->point.y) < 1.0e-3F + 1.0F);
        assert(std::fabs(wall->fraction - 0.5F) < 1.0e-4F);
        // A category mask skips the wall records.
        const auto masked = ec::segment_hit(source, {0.0F, 100.0F, 0.0F}, {200.0F, 50.0F, 0.0F}, 0x0002U);
        assert(!masked);
        const auto floor = ec::segment_hit(source, {0.0F, 50.0F, 0.0F}, {0.0F, -50.0F, 0.0F});
        assert(floor && std::fabs(floor->point.y) < 1.0e-4F && floor->normal.y == 1.0F);
        assert(!ec::segment_hit(source, {0.0F, 50.0F, 0.0F}, {50.0F, 60.0F, 0.0F}));

        // The sphere stops `radius` short of the wall and slides along it.
        const auto slid = ec::slide_sphere(source, {0.0F, 90.0F, 0.0F}, {300.0F, 90.0F, 40.0F}, 50.0F);
        assert(std::fabs(slid.x - 50.0F) < 0.5F && std::fabs(slid.z - 40.0F) < 0.5F);
        const auto free_move = ec::slide_sphere(source, {0.0F, 90.0F, 0.0F}, {-200.0F, 90.0F, 0.0F}, 50.0F);
        assert(std::fabs(free_move.x + 200.0F) < 1.0e-3F);
        const auto height = ec::floor_below(source, {10.0F, 90.0F, 10.0F}, 1000.0F);
        assert(height && std::fabs(*height) < 1.0e-4F);
        assert(!ec::floor_below(source, {10.0F, -5.0F, 10.0F}, 1000.0F));

    }

    {
        const auto hits = make_hits();
        const auto collision = dmc::rengine::profiles::dmc3::environment_collision::parse(
            "slot_0003.hits", 3U, std::span<const std::uint8_t>{hits.data(), hits.size()});
        assert(collision && collision->resource_slot == 3U && collision->triangles.size() == 1U);
        assert(collision->cell_reference_count == 1U);
        assert(dmc::rengine::profiles::dmc3::environment_collision::debug_lines(*collision).size() == 6U);
    }
    std::printf("dmc3 reader ports ok\n");
    return 0;
}
