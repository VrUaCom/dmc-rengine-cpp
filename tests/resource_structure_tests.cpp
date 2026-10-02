// integration::read_structure: what each recovered reader takes out of a
// payload, as rows a GDS client lists. Every format the view names is read
// from a synthetic fixture here; the classifier is checked for the two
// nameless character tables it now types by their bytes.
#include "dmc_rengine/gdspaces/classifier.hpp"
#include "dmc_rengine/integration/resource_structure.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using dmc::rengine::integration::read_structure;
using dmc::rengine::integration::StructureView;

void put_u32(std::vector<std::byte>& b, std::size_t o, std::uint32_t v) { std::memcpy(b.data() + o, &v, 4U); }
void put_f32(std::vector<std::byte>& b, std::size_t o, float v) { std::memcpy(b.data() + o, &v, 4U); }

[[nodiscard]] std::vector<std::byte> bytes_of(std::string_view text) {
    std::vector<std::byte> out(text.size());
    std::memcpy(out.data(), text.data(), text.size());
    return out;
}

[[nodiscard]] std::vector<std::byte> bytes_of(const std::vector<std::uint8_t>& raw) {
    std::vector<std::byte> out(raw.size());
    std::memcpy(out.data(), raw.data(), raw.size());
    return out;
}

[[nodiscard]] const std::string& row(const StructureView& view, std::size_t section, std::string_view label) {
    for (const auto& r : view.sections.at(section).rows) {
        if (r.label == label) return r.value;
    }
    assert(false && "row not found");
    static const std::string none;
    return none;
}

[[nodiscard]] std::vector<std::byte> pnst(const std::vector<std::vector<std::byte>>& slots) {
    std::size_t at = 8U + slots.size() * 4U;
    std::vector<std::size_t> offsets;
    for (const auto& s : slots) {
        offsets.push_back(at);
        at += s.size();
    }
    std::vector<std::byte> out(at, std::byte{0});
    std::memcpy(out.data(), "PNST", 4U);
    put_u32(out, 4U, static_cast<std::uint32_t>(slots.size()));
    for (std::size_t i = 0U; i < slots.size(); ++i) {
        put_u32(out, 8U + i * 4U, static_cast<std::uint32_t>(offsets[i]));
        std::memcpy(out.data() + offsets[i], slots[i].data(), slots[i].size());
    }
    return out;
}

constexpr std::string_view k_clt =
    ";pl000_02.clt\n\nClothNum\t1\n\nClothNo     0\nClothId     0\n"
    "Gravity     0.500000  0.000000  0.000000\nSpringForce 0.020000\n"
    "MaxSpeed    50.000000\nStiffness   0.000000\n"
    "Wind        0.000000  0.000000  0.000000\nWindLocal   1\nWindParent  0\n"
    "WindType    1\nBone      1    Y\nBone      2    NZ\nEnd\n$\n";

constexpr std::string_view k_tsc =
    "\r\n.TSC\t\r\n\t# RELATIVE\t\r\n\t\t<Start\r\n\t\t\tScrlNo\t\t0\r\n"
    "\t\t\tScrlType\t1\r\n\t\t\tTexNo\t\t3\r\n\t\t\tDirUV\t\tstay,   up\r\n"
    "\t\t\tTimeUV\t\t0, \t90\r\n\t\tEnd>\r\n\t\t<Finish>\r\n$\t\r\n";

[[nodiscard]] std::vector<std::byte> evt_bytes() {
    std::vector<std::byte> out(0x60U, std::byte{0});
    std::memcpy(out.data(), "EVT\0", 4U);
    put_u32(out, 0x04U, 0x00010001U);
    std::size_t cursor = 0x20U;
    put_u32(out, cursor, 0x00000102U);
    put_u32(out, cursor + 4U, 0x37U);
    cursor += 8U;
    put_u32(out, cursor, 0x00000203U);
    put_u32(out, cursor + 4U, 0x11U);
    put_u32(out, cursor + 8U, 0x22U);
    cursor += 12U;
    put_u32(out, cursor, 0x00000001U);
    cursor += 4U;
    put_u32(out, cursor, 0x00000020U);
    put_u32(out, 0x08U, static_cast<std::uint32_t>(cursor));
    return out;
}

[[nodiscard]] std::vector<std::byte> hits_bytes() {
    std::vector<std::byte> b(0x88U, std::byte{0});
    std::memcpy(b.data(), "HITS", 4U);
    put_u32(b, 0x04U, 0x88U);
    put_f32(b, 0x14U, 100.0F); put_f32(b, 0x18U, 100.0F); put_f32(b, 0x1CU, 100.0F);
    put_f32(b, 0x20U, 100.0F); put_f32(b, 0x24U, 100.0F); put_f32(b, 0x28U, 100.0F);
    put_u32(b, 0x2CU, 1U); put_u32(b, 0x30U, 1U); put_u32(b, 0x34U, 1U);
    put_u32(b, 0x38U, 1U); put_u32(b, 0x3CU, 0x3CU); put_u32(b, 0x40U, 0x48U);
    put_u32(b, 0x44U, 0x40U);
    put_u32(b, 0x48U, 0U);
    put_u32(b, 0x4CU, 0xFFFFFFFFU);
    put_u32(b, 0x50U, 0x12345678U);
    put_f32(b, 0x60U, 10.0F); put_f32(b, 0x74U, 10.0F);
    put_f32(b, 0x7CU, 1.0F);
    return b;
}

// Sphere and capsule only: what em000 slot 40 holds, which the classifier
// already types as "so-volume". `with_box` adds a player-style box record.
[[nodiscard]] std::vector<std::byte> shape_table(bool with_box = false) {
    std::vector<std::byte> s((with_box ? 3U : 2U) * 80U, std::byte{0});
    s[0] = std::byte{2};  // sphere: centre (0, 50, 0), radius 40
    put_f32(s, 0x14U, 50.0F); put_f32(s, 0x1CU, 1.0F); put_f32(s, 0x20U, 40.0F);
    s[0x50] = std::byte{4};  // capsule a (0, 500, 0) b (0, 0, 0) r 120
    put_f32(s, 0x50U + 0x14U, 500.0F); put_f32(s, 0x50U + 0x1CU, 1.0F); put_f32(s, 0x50U + 0x2CU, 1.0F);
    put_f32(s, 0x50U + 0x30U, 120.0F);
    if (with_box) {
        s[0xA0] = std::byte{3};  // box: centre (41, 40, 5) rot (13, 11, 0) half size (58, 54, 5)
        put_f32(s, 0xA0U + 0x10U, 41.0F); put_f32(s, 0xA0U + 0x14U, 40.0F); put_f32(s, 0xA0U + 0x18U, 5.0F);
        put_f32(s, 0xA0U + 0x1CU, 13.0F); put_f32(s, 0xA0U + 0x20U, 11.0F);
        put_f32(s, 0xA0U + 0x28U, 58.0F); put_f32(s, 0xA0U + 0x2CU, 54.0F); put_f32(s, 0xA0U + 0x30U, 5.0F);
    }
    return s;
}

// Enemy layout (bind mode 1): table A lists banks, table B maps actions to
// MOT ids (group * 100 + slot).
[[nodiscard]] std::vector<std::byte> enemy_script() {
    return bytes_of(std::vector<std::uint8_t>{
        0x06, 0x00, 0x1E, 0x00, 0xFF, 0xFF,
        0x04, 0x00, 0xFF, 0xFF,
        0x06, 0x00, 0x0C, 0x00, 0xFF, 0xFF,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00,
        0x01, 0x00,
        0x04, 0x00, 0xFF, 0xFF,
        0x06, 0x00, 0x0E, 0x00, 0xFF, 0xFF,
        0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0C, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00,
    });
}

[[nodiscard]] std::vector<std::byte> effect_bank() {
    // Two A records (sprite animations) in the inner PNST, named by the
    // `<kind> <id>` manifest in slot 0.
    std::vector<std::byte> a(16U, std::byte{0});
    a[0] = std::byte{1};   // version
    a[1] = std::byte{2};   // texture id
    a[2] = std::byte{4};   // ticks per frame
    a[3] = std::byte{0};   // one frame
    a[4] = std::byte{1};   // loops
    a[6] = std::byte{8}; a[8] = std::byte{16}; a[10] = std::byte{32}; a[12] = std::byte{64};
    auto b = a;
    b[4] = std::byte{0};
    return pnst({bytes_of("A 5\nA 6\n#\n"), pnst({a, b})});
}

[[nodiscard]] std::vector<std::byte> pac_with(std::uint32_t slots) {
    std::vector<std::byte> out(8U + slots * 4U, std::byte{0});
    std::memcpy(out.data(), "PAC\0", 4U);
    put_u32(out, 4U, slots);
    return out;
}

void text_formats() {
    std::string detail;
    const auto clt = read_structure("clt", bytes_of(k_clt), "pl000_02.clt", detail);
    assert(clt && clt->summary == "1 cloth chain(s), 2 bone(s).");
    assert(row(*clt, 0U, "Gravity") == "0.5, 0, 0");
    assert(row(*clt, 0U, "Bones (node axis)") == "1 Y → 2 NZ");

    const auto tsc = read_structure("tsc", bytes_of(k_tsc), "", detail);
    assert(tsc && tsc->sections.size() == 1U);
    assert(tsc->sections[0].title == "Scroll 0 · ScrlType 1");
    assert(row(*tsc, 0U, "Texture") == "3");
    assert(row(*tsc, 0U, "Direction U, V") == "stay, up");

    const auto evt = read_structure("evt", evt_bytes(), "", detail);
    assert(evt && evt->sections.size() == 2U && evt->sections[1].rows.size() == 4U);
    assert(evt->summary.starts_with("4 command(s) in 1 stream(s)"));

    assert(!read_structure("clt", bytes_of("not a cloth"), "", detail));
    assert(!detail.empty());
}

void stage_and_effects() {
    std::string detail;
    const auto hits = read_structure("hits", hits_bytes(), "st001.hits", detail);
    assert(hits && hits->sections.size() == 2U);
    assert(row(*hits, 0U, "Cells") == "1 × 1 × 1");
    assert(row(*hits, 0U, "Triangle-plane records") == "1");
    assert(hits->sections[1].rows.size() == 1U);
    assert(hits->sections[1].rows[0].label == "Kind 0  flags 0x12345678");

    const auto bank = read_structure("pnst", effect_bank(), "", detail);
    assert(bank && bank->summary == "2 effect record(s): 2 A.");
    assert(bank->sections.size() == 3U);
    assert(row(*bank, 0U, "Inner record slots") == "2");
    assert(row(*bank, 0U, "Manifest ends with '#'") == "yes");
    assert(bank->sections[1].title == "A 5 · slot 0");
    assert(row(*bank, 1U, "Texture id") == "2");
    assert(row(*bank, 1U, "Frames") == "1 × 4 tick(s), loops from frame 0");
    assert(row(*bank, 1U, "First frame (x, y, w, h)") == "8, 16, 32, 64");
    assert(row(*bank, 2U, "Frames") == "1 × 4 tick(s), once");

    // An ordinary PNST is a container, not a bank: declined with a reason.
    assert(!read_structure("pnst", pnst({evt_bytes()}), "", detail));
    assert(detail.starts_with("Not an effect bank"));
}

void character_tables() {
    std::string detail;
    const auto shapes = read_structure("collision-shapes", shape_table(true), "", detail);
    assert(shapes && shapes->summary == "3 shape record(s): 1 sphere, 1 box, 1 capsule.");
    assert(shapes->sections[2].title == "Shape 2 · box");
    assert(row(*shapes, 2U, "Half size") == "58, 54, 5");
    const auto volume = read_structure("so-volume", shape_table(), "", detail);
    assert(volume && volume->format == "so-volume" && volume->sections.size() == 2U);
    assert(shapes->sections[0].title == "Shape 0 · sphere");
    assert(row(*shapes, 0U, "Radius") == "40");
    assert(row(*shapes, 1U, "End A") == "0, 500, 0");

    const auto script = read_structure("motion-script", enemy_script(), "", detail);
    assert(script && script->sections.size() == 1U);
    assert(script->summary == "1 bank(s), 2 action(s) (enemy layout); 2 motion id(s) in table B.");
    assert(row(*script, 0U, "Action 1").ends_with("MOT 1:1"));

    const auto pl000 = read_structure("pac", pac_with(16U), "obj/PL000.PAC", detail);
    assert(pl000 && pl000->sections[0].title == "Slot roles of pl000.pac (16 slots)");
    assert(row(*pl000, 0U, "Slot 12") == "MOD: coat, root on body joint 3");
    assert(row(*pl000, 0U, "Slot 13") == "CLT: coat chain");

    const auto sword = read_structure("pac", pac_with(4U), "plwp_sword.pac", detail);
    assert(sword && row(*sword, 0U, "Class") == "CPlWpSword");
    assert(row(*sword, 0U, "Attach joint") == "3");

    const auto em028 = read_structure("pac", pac_with(13U), "em028.pac", detail);
    assert(em028 && row(*em028, 0U, "Slot 4") == "part on body joints: 0→3, 1→4, 2→5");

    const auto em000 = read_structure("pac", pac_with(42U), "em000.pac", detail);
    assert(em000 && row(*em000, 0U, "Slot 41") == "effect bank (mode 2)");

    assert(!read_structure("pac", pac_with(3U), "st001.pac", detail));
    assert(detail.find("st001") != std::string::npos);
}

void stage_layout_and_enemy_events() {
    std::string detail;
    constexpr std::string_view game =
        "# GAME\n# SET 0 CONFIG\n  cam_init 1.5, 2, -3 ; camera\n"
        "# SET 1 MODEL\n  model 4\n  pos 100, 0, -50\n  rot 0, 90, 0\n  uv 0, 1, 0.5, -0.25\n"
        "  eff V 98\n  epos 1, 2, 3\n"
        "# SET 3 BREAK\n  model 3\n  bmodel 4\n  beff V 104\n  remain on\n# GAME_END\n";
    const auto layout = read_structure("txt", bytes_of(game), "st001.txt", detail);
    assert(layout && layout->summary == "3 placed object(s): 1 BREAK, 1 CONFIG, 1 MODEL; initial camera set.");
    assert(layout->sections.front().title == "CONFIG");
    assert(row(*layout, 0U, "cam_init") == "1.5, 2, -3");
    assert(layout->sections[2].title == "SET 1 · MODEL");
    assert(row(*layout, 2U, "UV scroll (part, texture, U, V)") == "0, 1, 0.5, -0.25");
    assert(row(*layout, 2U, "Effect") == "V 98 at 1, 2, 3");
    assert(row(*layout, 3U, "Broken model") == "4");
    assert(row(*layout, 3U, "Break effect (once)") == "V 104 at 0, 0, 0");
    assert(row(*layout, 3U, "Remain") == "stays (on)");
    assert(!read_structure("txt", bytes_of("plain notes\n"), "", detail));
    assert(detail.find("# GAME") != std::string::npos);

    // em000: slot roles, then the event handler's cases, the death schedule
    // and the frame-gated attack events.
    const auto em000 = read_structure("pac", pac_with(42U), "em000.pac", detail);
    assert(em000 && em000->sections.size() == 4U);
    assert(em000->sections[1].title.starts_with("Effect events (handler 0x1401C3130"));
    assert(row(*em000, 2U, "Tick 0") == "code 0x69, 0xC8");
    // em006 has no slot contract, only its events; em009 has neither.
    const auto em006 = read_structure("pac", pac_with(8U), "em006.pac", detail);
    assert(em006 && em006->sections.size() == 1U);
    assert(em006->summary == "Effect events of em006 recovered from the executable.");
    assert(!read_structure("pac", pac_with(8U), "em009.pac", detail));
}

void nameless_slots_are_typed_by_bytes() {
    using dmc::rengine::gdspaces::ResourceClassifier;
    assert(ResourceClassifier::classify("", shape_table()).format == "so-volume");
    assert(ResourceClassifier::classify("", shape_table(true)).format == "collision-shapes");
    assert(ResourceClassifier::classify("slot.bin", shape_table(true)).format == "collision-shapes");
    assert(ResourceClassifier::classify("", enemy_script()).format == "motion-script");
    // A name keeps its identity: only nameless slots are probed.
    assert(ResourceClassifier::classify("table.dat", shape_table(true)).format == "dat");

    assert(dmc::rengine::integration::has_structure("motion-script"));
    assert(!dmc::rengine::integration::has_structure("mod"));
    assert(dmc::rengine::integration::has_structure("txt"));
    std::string detail;
    assert(!read_structure("mod", evt_bytes(), "", detail));
    assert(detail.find("No structure reader") != std::string::npos);
}

}  // namespace

int main() {
    text_formats();
    stage_and_effects();
    character_tables();
    stage_layout_and_enemy_events();
    nameless_slots_are_typed_by_bytes();
    return 0;
}
