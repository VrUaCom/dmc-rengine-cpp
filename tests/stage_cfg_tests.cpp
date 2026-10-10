// POS, EVE and CAM read from synthetic files built here in their layouts:
// every field lands where it was written, malformed files are refused, and
// the structure view lists them.

#include "dmc_rengine/formats/stage_cfg.hpp"
#include "dmc_rengine/integration/resource_structure.hpp"

#include <cassert>
#include <cstring>
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

} // namespace

int main() {
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
    for (const auto& [format, bytes] : {std::pair{"pos", pos_file()}, std::pair{"eve", eve_file()},
                                        std::pair{"cam", cam_file()}}) {
        std::string detail;
        const auto view = dmc::rengine::integration::read_structure(format, bytes, "st000cfg", detail);
        assert(view.has_value() && !view->sections.empty() && !view->summary.empty());
        assert(dmc::rengine::integration::has_structure(format));
    }
    std::cout << "stage_cfg_tests: POS, EVE and CAM read and refuse as their layouts say\n";
    return 0;
}
