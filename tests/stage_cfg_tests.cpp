// POS, EVE, CAM, ITM, STE, EST and SEF read from synthetic files built here in their layouts:
// every field lands where it was written, malformed files are refused, and
// the structure view lists them.

#include "dmc_rengine/formats/stage_cfg.hpp"
#include "dmc_rengine/integration/resource_structure.hpp"

#include <cassert>
#include <cstring>
#include <utility>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace cfg = dmc::rengine::formats::stage_cfg;

namespace {

struct Writer final {
    std::vector<std::byte> bytes;

    void tag(const char* text, std::uint16_t version, std::uint16_t count) {
        put(text, 4U);
        u16(version);
        u16(count);
        zeros(8U);
    }
    void put(const void* data, std::size_t size) {
        const auto* begin = static_cast<const std::byte*>(data);
        bytes.insert(bytes.end(), begin, begin + size);
    }
    void u16(std::uint16_t value) { put(&value, 2U); }
    void u32(std::uint32_t value) { put(&value, 4U); }
    void f32(float value) { put(&value, 4U); }
    void zeros(std::size_t count) { bytes.insert(bytes.end(), count, std::byte{0}); }
    void corners(float y, float x0, float z0) {
        for (int corner = 0; corner < 4; ++corner) {
            f32(x0 + (corner == 1 || corner == 2 ? 300.0F : 0.0F));
            f32(y);
            f32(z0 + (corner >= 2 ? 200.0F : 0.0F));
            f32(1.0F);
        }
    }
};

[[nodiscard]] std::vector<std::byte> pos_file() {
    Writer w;
    w.tag("POS", 2U, 2U);
    for (int index = 0; index < 4; ++index) {
        if (index < 2) {
            w.u32(0U);
            w.f32(100.0F + index);
            w.f32(5.0F);
            w.f32(-300.0F);
            w.f32(index == 0 ? 180.0F : 90.0F);
            w.zeros(28U);
        } else {
            w.zeros(48U);
        }
    }
    return w.bytes;
}

[[nodiscard]] std::vector<std::byte> eve_file() {
    Writer w;
    w.tag("EVE", 0x100U, 2U);
    for (int index = 0; index < 2; ++index) {
        w.u32(static_cast<std::uint32_t>(index));
        w.u32(3U + index);
        w.u32(index == 0 ? 10001U : (1000U << 16U));
        w.zeros(0x14U);
        w.corners(-50.0F, 1000.0F * index, 2000.0F);
        w.f32(500.0F);
        w.zeros(0x1CU);
    }
    return w.bytes;
}

[[nodiscard]] std::vector<std::byte> cam_file() {
    Writer w;
    w.tag("CAM", 1U, 1U);
    w.u32(256U);
    w.zeros(12U);
    // Two paths of three points, a coefficient per point, then one area.
    for (int path = 0; path < 2; ++path) {
        for (int point = 0; point < 3; ++point) {
            w.f32(2400.0F + point);
            w.f32(110.0F + path);
            w.f32(4500.0F - 100.0F * point);
        }
        w.zeros(12U); // ends the run; 36 + 12 = 48 keeps rows aligned
    }
    for (int point = 0; point < 3; ++point) w.f32(0.96F);
    w.zeros(4U);
    w.corners(-127.5F, 2000.0F, 3000.0F);
    w.f32(2000.0F);
    w.zeros(12U);
    return w.bytes;
}

[[nodiscard]] std::vector<std::byte> itm_file(std::uint16_t count) {
    Writer w;
    w.tag("ITM", 1U, count);
    for (std::uint16_t index = 0U; index < count; ++index) {
        w.u32(0x30U + index);
        w.f32(10.0F * index);
        w.f32(0.0F);
        w.f32(-20.0F);
        w.f32(1.5F);
    }
    while (w.bytes.size() % 16U != 0U) w.zeros(1U);
    return w.bytes;
}

[[nodiscard]] std::vector<std::byte> ste_file() {
    Writer w;
    w.tag("STE", 1U, 2U);
    for (int index = 0; index < 2; ++index) {
        w.u16(2U);
        w.u16(static_cast<std::uint16_t>(10 + index));
        for (const float value : {2500.0F, 0.0F, 1000.0F, 0.0F, 180.0F * index, 0.0F}) w.f32(value);
        for (int axis = 0; axis < 3; ++axis) w.f32(1.0F + 2.0F * index);
    }
    return w.bytes;
}

// Two rows: one program for every mode, then two programs by mode.
[[nodiscard]] std::vector<std::byte> est_file() {
    Writer w;
    w.put("EST", 4U);
    w.u16(1U);
    w.u16(2U);
    w.u32(0U); // table offset, patched below
    w.zeros(4U);
    const auto program = [&w](std::uint32_t kind) {
        const auto start = static_cast<std::uint32_t>(w.bytes.size());
        w.u32(0x0202U);
        w.u32(kind);
        w.u32(5U);
        w.u32(0x0303U);
        for (const std::int32_t value : {2900, 20, -2800}) w.u32(static_cast<std::uint32_t>(value));
        w.u32(0x0105U);
        w.u32(static_cast<std::uint32_t>(-90));
        w.u32(0U);
        return start;
    };
    const auto a = program(0U);
    const auto b = program(8U);
    const auto c = program(16U);
    const auto table = static_cast<std::uint32_t>(w.bytes.size());
    for (int mode = 0; mode < 5; ++mode) w.u32(a);
    for (int mode = 0; mode < 5; ++mode) w.u32(mode < 3 ? b : c);
    std::memcpy(w.bytes.data() + 8, &table, 4U);
    return w.bytes;
}

[[nodiscard]] std::vector<std::byte> sef_file() {
    Writer w;
    w.put("SEF", 4U);
    w.u16(2U);
    w.u16(1U);
    w.u32(3U);
    w.u32(0x18U);
    w.u32(7U);
    w.u32(0x30U);
    w.zeros(0x30U - 0x18U);
    w.zeros(20U);
    return w.bytes;
}

// CAM zone records, the game's cameras and the camera at a player's place,
// and the SEF fog section.
void test_cam_shots_and_fog() {
    Writer w;
    const auto pad_to = [&](std::size_t size) { w.zeros(size - w.bytes.size()); };
    const auto corner = [&](float x, float y, float z) { w.f32(x); w.f32(y); w.f32(z); w.f32(1.0F); };
    // One 0x400 record of two zones (count in the byte at +0x14), then a path.
    w.tag("CAM", 1U, 2U);
    auto at = w.bytes.size();
    w.u32(0x400U); w.zeros(0x10U); w.u32(2U); pad_to(at + 0x40U);
    for (int z = 0; z < 2; ++z) {
        const float o = 20.0F * static_cast<float>(z);
        corner(o, 5, 0); corner(o + 10, 5, 0); corner(o + 10, 5, 10); corner(o, 5, 10);
        w.f32(30.0F); w.zeros(12U);
    }
    for (int p = 0; p < 3; ++p) { w.f32(100.0F + static_cast<float>(p)); w.f32(7.5F); w.f32(200.0F + static_cast<float>(p)); }
    pad_to((w.bytes.size() + 15U) & ~std::size_t{15U});
    const auto read = cfg::read_cam(w.bytes);
    assert(read.ok());
    assert(read.document.records.size() == 1U && read.document.records[0].zone_count == 2U);
    assert(read.document.areas.size() == 2U && read.document.areas[1].record == 0);
    assert(read.document.paths.size() == 1U && read.document.paths[0].points.size() == 3U);
    assert(cfg::read_cam_shots(w.bytes).empty());  // the second record is not a camera record

    // A fixed camera following the player over one zone; a rail camera over a
    // far zone and a wide one round both.
    Writer s;
    const auto spad = [&](std::size_t size) { s.zeros(size - s.bytes.size()); };
    const auto zone = [&](float x0, float z0, float x1, float z1) {
        for (const auto& [x, z] : {std::pair{x0, z0}, std::pair{x0, z1}, std::pair{x1, z1}, std::pair{x1, z0}}) {
            s.f32(x); s.f32(-10.0F); s.f32(z); s.f32(1.0F);
        }
        s.f32(500.0F); s.zeros(12U);
    };
    s.tag("CAM", 1U, 2U);
    at = s.bytes.size();
    s.u32(0x000U); s.zeros(0x10U); s.u32(0x00780101U);  // count 1, then st002's 01 78
    spad(at + 0x40U); s.u32(0x200U);
    spad(at + 0x50U); s.f32(0.0F); s.f32(105.0F); s.f32(0.0F);
    spad(at + 0x90U); s.f32(500.0F); s.f32(300.0F); s.f32(500.0F);
    spad(at + 0xA0U); s.f32(1.05F);
    spad(at + 0xB0U);
    zone(0, 0, 1000, 1000);
    at = s.bytes.size();
    s.u32(0x100U); s.zeros(0x10U); s.u32(2U);
    spad(at + 0x80U);
    for (const float x : {2000.0F, 3000.0F}) { s.f32(x); s.f32(100.0F); s.f32(0.0F); }
    spad(at + 0x200U);
    for (const float x : {2000.0F, 3000.0F}) { s.f32(x); s.f32(150.0F); s.f32(500.0F); }
    spad(at + 0x380U); s.f32(1.0F); s.f32(1.2F);
    spad(at + 0x480U); s.u32(2U);
    spad(at + 0x490U);
    zone(1500, -500, 3500, 1500);
    zone(-1000, -1000, 5000, 2000);
    const auto shots = cfg::read_cam_shots(s.bytes);
    assert(shots.size() == 2U && shots[0].type == cfg::cam_fixed && shots[0].follows_player && shots[1].eye.size() == 2U);
    const auto fixed = cfg::cam_view_at(shots, {200.0F, 0.0F, 300.0F});
    assert(fixed && fixed->shot == 0 && fixed->eye.x == 500.0F && fixed->target.y == 105.0F &&
           fixed->target.x == 200.0F && std::fabs(fixed->fov_radians - 1.05F) < 1.0e-6F);
    const auto rail = cfg::cam_view_at(shots, {2500.0F, 0.0F, 800.0F});
    assert(rail && rail->shot == 1 && std::fabs(rail->eye.x - 2500.0F) < 1.0e-2F && rail->eye.z == 0.0F &&
           std::fabs(rail->target.z - 500.0F) < 1.0e-3F && std::fabs(rail->fov_radians - 1.1F) < 1.0e-5F);
    assert(cfg::cam_view_at(shots, {4500.0F, 0.0F, 0.0F})->shot == 1);
    assert(!cfg::cam_view_at(shots, {9000.0F, 0.0F, 0.0F}));
    assert(!cfg::cam_view_at(shots, {200.0F, 600.0F, 300.0F}));
    auto torn = s.bytes;
    torn.resize(torn.size() - 0x50U);
    assert(cfg::read_cam_shots(torn).empty());

    // SEF: one fog section (value 2), distances far first as st001 writes them.
    Writer f;
    f.put("SEF", 4U); f.u16(1U); f.u16(0U); f.u32(2U); f.u32(0x10U);
    f.f32(2100.0F); f.f32(1680.0F);
    for (const std::uint8_t b : {std::uint8_t{0x60}, std::uint8_t{0x70}, std::uint8_t{0x80}, std::uint8_t{0x40}}) f.put(&b, 1U);
    f.zeros(4U);
    const auto sef = cfg::read_sef(f.bytes);
    assert(sef.ok());
    const auto fog = cfg::sef_fog(f.bytes, sef.document);
    assert(fog && fog->start == 1680.0F && fog->end == 2100.0F && fog->colour[2] == 0x80U && fog->alpha == 0.5F);

    static_assert(cfg::ste_effect_kind(0U) == 'P' && cfg::ste_effect_kind(3U) == 'V' && !cfg::ste_effect_kind(4U));
}

} // namespace

int main() {
    test_cam_shots_and_fog();
    {
        const auto result = cfg::read_pos(pos_file());
        assert(result.ok());
        assert(result.document.header.version == 2U && result.document.capacity == 4U);
        assert(result.document.records.size() == 2U);
        assert(result.document.records[0].heading_degrees == 180.0F);
        assert(result.document.records[1].position.x == 101.0F && result.document.records[1].tail_zero);

        auto broken = pos_file();
        broken.pop_back();
        assert(!cfg::read_pos(broken).ok());
        auto overcounted = pos_file();
        overcounted[6] = std::byte{9};
        assert(!cfg::read_pos(overcounted).ok());
    }
    {
        const auto result = cfg::read_eve(eve_file());
        assert(result.ok());
        assert(result.document.records.size() == 2U);
        const auto& first = result.document.records[0];
        assert(first.kind == 3U && first.argument == 10001U && first.extent == 500.0F);
        assert(first.area.homogeneous && first.area.level && first.rest_zero);
        assert(result.document.records[1].argument >> 16U == 1000U);

        auto short_file = eve_file();
        short_file.resize(short_file.size() - 128U);
        assert(!cfg::read_eve(short_file).ok());
        auto wrong_tag = eve_file();
        wrong_tag[0] = std::byte{'X'};
        assert(!cfg::read_eve(wrong_tag).recognized);
    }
    {
        const auto result = cfg::read_cam(cam_file());
        assert(result.ok());
        assert(result.document.header.count == 1U);
        assert(result.document.paths.size() == 2U);
        assert(result.document.paths[0].points.size() == 3U && result.document.paths[1].points[0].y == 111.0F);
        assert(result.document.coefficient_runs.size() == 1U && result.document.coefficient_runs[0].count == 3U);
        assert(result.document.areas.size() == 1U && result.document.areas[0].extent == 2000.0F);
        assert(result.document.unexplained_bytes == 4U); // the 256 after the header
    }
    {
        const auto empty = cfg::read_itm(itm_file(0U));
        assert(empty.ok() && empty.document.records.empty());
        const auto result = cfg::read_itm(itm_file(3U));
        assert(result.ok() && result.document.records.size() == 3U);
        assert(result.document.records[2].item_id == 0x32U && result.document.records[2].position.x == 20.0F);
        assert(result.document.records[0].rotation_y == 1.5F);
        auto short_file = itm_file(3U);
        short_file.resize(0x10U + 2U * 0x14U);
        assert(!cfg::read_itm(short_file).ok());
    }
    {
        const auto result = cfg::read_ste(ste_file());
        assert(result.ok() && result.document.records.size() == 2U);
        const auto& second = result.document.records[1];
        assert(second.kind == 2U && second.number == 11U);
        assert(second.rotation_degrees.y == 180.0F && second.scale.z == 3.0F && second.position.z == 1000.0F);
        auto short_file = ste_file();
        short_file.pop_back();
        assert(!cfg::read_ste(short_file).ok());
    }
    {
        const auto result = cfg::read_est(est_file());
        assert(result.ok());
        const auto& doc = result.document;
        assert(doc.programs.size() == 3U && doc.unexplained_bytes == 0U);
        assert(doc.rows.size() == 2U);
        assert(doc.rows[0][0] == 0 && doc.rows[0][4] == 0);
        assert(doc.rows[1][2] == 1 && doc.rows[1][3] == 2);
        const auto& first = doc.programs[0];
        assert(first.terminated && first.commands.size() == 3U);
        assert(first.commands[1].code == 3U && first.commands[1].arguments.size() == 3U);
        assert(first.commands[1].arguments[2] == -2800 && first.commands[2].arguments[0] == -90);

        // A table past the end, and a word that cannot be a command.
        auto bad_table = est_file();
        bad_table[9] = std::byte{0x7F};
        assert(!cfg::read_est(bad_table).ok());
        auto bad_word = est_file();
        bad_word[0x10 + 2] = std::byte{1};
        assert(!cfg::read_est(bad_word).ok());
        // Bytes no program reads are counted.
        auto stray = est_file();
        stray.insert(stray.begin() + 0x10, {std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0}});
        stray[0x10] = std::byte{0x55};
        for (std::size_t at = 8U; at < 12U; ++at) stray[at] = std::byte{0};
        const auto table = static_cast<std::uint32_t>(stray.size() - 40U);
        std::memcpy(stray.data() + 8, &table, 4U);
        for (std::size_t entry = 0U; entry < 10U; ++entry) {
            std::uint32_t offset = 0U;
            std::memcpy(&offset, stray.data() + table + entry * 4U, 4U);
            offset += 4U;
            std::memcpy(stray.data() + table + entry * 4U, &offset, 4U);
        }
        const auto strayed = cfg::read_est(stray);
        assert(strayed.ok() && strayed.document.unexplained_bytes == 1U);
    }
    {
        const auto result = cfg::read_sef(sef_file());
        assert(result.ok() && result.document.sections.size() == 2U);
        assert(result.document.sections[0].offset == 0x18U && result.document.sections[0].size == 0x18U);
        assert(result.document.sections[1].value == 7U && result.document.sections[1].size == 20U);
        auto backwards = sef_file();
        backwards[0x14] = std::byte{0x10};
        assert(!cfg::read_sef(backwards).ok());
    }
    for (const auto& [format, bytes] : {std::pair{"pos", pos_file()}, std::pair{"eve", eve_file()},
                                        std::pair{"cam", cam_file()}, std::pair{"itm", itm_file(2U)},
                                        std::pair{"ste", ste_file()}, std::pair{"est", est_file()},
                                        std::pair{"sef", sef_file()}}) {
        std::string detail;
        const auto view = dmc::rengine::integration::read_structure(format, bytes, "st000cfg", detail);
        assert(view.has_value() && !view->sections.empty() && !view->summary.empty());
        assert(dmc::rengine::integration::has_structure(format));
    }
    // An effect record opened on its own: the view answers whatever the bytes.
    {
        std::string detail;
        const std::vector<std::byte> record(96U, std::byte{0});
        const auto view = dmc::rengine::integration::read_structure("fx-e", record, "E 7", detail);
        assert(view.has_value() && view->format == "fx-e" && !view->summary.empty());
        assert(!dmc::rengine::integration::read_structure("fx-", record, "", detail).has_value());
    }
    std::cout << "stage_cfg_tests: POS, EVE, CAM, ITM, STE, EST and SEF read and refuse as their layouts say\n";
    return 0;
}
