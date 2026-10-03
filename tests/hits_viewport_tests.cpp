#include "dmc_rengine/formats/scm_layout.hpp"
#include "dmc_rengine/hits/viewport.hpp"
#include "hits_test_fixture.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>

using namespace dmc::rengine;
namespace {
template <class T> void put(std::vector<std::byte>& bytes, std::uint64_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}
std::vector<std::byte> scm_fixture() {
    using namespace formats::scm;
    ObjectShape shape;
    shape.mesh_vertex_counts = {3};
    const auto layout = build_serialized_layout(std::span<const ObjectShape>(&shape, 1), 1);
    std::vector<std::byte> b(static_cast<std::size_t>(layout.file_size));
    std::memcpy(b.data(), "SCM ", 4);
    put(b, 4, 1.01F);
    b[0x10] = b[0x11] = std::byte{1};
    b[0x12] = std::byte{2};
    put(b, 0x14, std::uint32_t{300100});
    put(b, 0x20, layout.scene.block_offset);
    const auto& o = layout.objects[0];
    const auto& m = o.meshes[0];
    b[o.record_offset] = std::byte{1};
    b[o.record_offset + 1] = std::byte{0x80};
    put(b, o.record_offset + 2, std::uint16_t{3});
    put(b, o.record_offset + 8, o.mesh_table_offset);
    put(b, o.record_offset + 0x3C, 2.0F);
    put(b, m.record_offset, std::uint16_t{3});
    put(b, m.record_offset + 0x10, m.positions_offset);
    put(b, m.record_offset + 0x18, m.normals_offset);
    put(b, m.record_offset + 0x20, m.uv_offset);
    put(b, m.record_offset + 0x38, m.color_flags_offset);
    put(b, m.record_offset + 0x40, m.index_workspace_offset - m.record_offset);
    put(b, m.index_workspace_offset, std::uint16_t{0x1212});
    put(b, m.positions_offset + 12, 1.0F);
    put(b, m.positions_offset + 24 + 8, 1.0F);
    for (std::uint64_t i = 0; i < 3; ++i) {
        put(b, m.normals_offset + i * 12 + 4, 1.0F);
        for (std::uint64_t j = 0; j < 3; ++j)
            b[m.color_flags_offset + i * 4 + j] = std::byte{255};
    }
    const auto& s = layout.scene;
    put(b, s.block_offset, s.parent_rel);
    put(b, s.block_offset + 4, s.order_rel);
    put(b, s.block_offset + 8, s.object_binding_rel);
    put(b, s.block_offset + 12, s.transform_rel);
    b[s.block_offset + s.parent_rel] = std::byte{0xFF};
    put(b, s.block_offset + s.transform_rel, 15.0F);
    put(b, s.block_offset + s.transform_rel + 4, 2.0F);
    put(b, s.block_offset + s.transform_rel + 8, 20.0F);
    put(b, s.block_offset + s.transform_rel + 12, std::sqrt(629.0F));
    return b;
}
} // namespace
int main() {
    hits::viewport::Controller c;
    assert(!c.save().ok());
    const auto source = tests::hits_fixture::make_minimal_hits();
    assert(c.open_hits(source));
    assert(c.save().bytes == source); // untouched source must round-trip exactly
    auto frame = c.frame(800, 600);
    assert(frame.size() == 1);
    const auto& p = frame[0].points;
    const float x = (p[0].x + p[1].x + p[2].x) / 3, y = (p[0].y + p[1].y + p[2].y) / 3;
    assert(c.pick(x, y, 800, 600));
    assert(c.selection().size() == 1);
    assert(c.paint(hits::editor::CollisionPreset::red_raw_18060001));
    auto saved = c.save();
    assert(saved.ok());
    auto parsed = formats::hits::RecordScanner::scan(saved.bytes);
    assert(parsed.ok() && parsed.triangles[0].flags == 0x18060001);
    assert(c.undo());
    assert(c.save().bytes == source);
    assert(c.redo());
    assert(c.dirty());
    assert(!c.open_hits({}));
    assert(c.dirty()); // failed load is transactional
    assert(!c.open_scm({}));
    assert(c.select_connected());
    assert(!c.translate({std::numeric_limits<float>::infinity(), 0, 0}));
    assert(c.translate({10, 0, 0}));
    assert(c.save().ok()); // fit-grid fallback
    assert(!c.pick(-1000, -1000, 800, 600));
    assert(c.selection().empty());
    assert(c.frame(0, 600).empty());
    c.zoom(std::numeric_limits<float>::quiet_NaN());
    c.orbit(0.3F, 0.2F);
    for (auto t : c.frame(800, 600))
        for (auto q : t.points)
            assert(std::isfinite(q.x) && std::isfinite(q.y));
    assert(c.boundary({0, 0, 0}, {2, 3, 4}, hits::editor::CollisionPreset::blue_raw_00000001));
    assert(c.session()->editor().surfaces().size() == 9);
    assert(c.save().ok());
    // Overlapping projected geometry must select the closest triangle at the
    // cursor even when insertion order is reversed.
    auto session = hits::standalone::Session::open(source);
    assert(session);
    const auto back = session->editor().add_surface(9, {0, 0, 2}, {1, 0, 2}, {0, 1, 2});
    const auto front = session->editor().add_surface(10, {0, 0, -2}, {1, 0, -2}, {0, 1, -2});
    assert(back && front);
    auto layered = session->save();
    assert(layered.ok());
    assert(c.open_hits(layered.bytes));
    c.orbit(-0.8F, -0.7F);
    c.fit();
    auto packets = c.frame(800, 600);
    assert(packets.size() == 3);
    auto front_packet = std::find_if(packets.begin(), packets.end(),
                                     [](const auto& t) { return t.color == 0x40CB78; });
    assert(front_packet != packets.end());
    auto points = front_packet->points;
    assert(c.pick((points[0].x + points[1].x + points[2].x) / 3,
                  (points[0].y + points[1].y + points[2].y) / 3, 800, 600));
    auto inspector = c.session()->editor().inspect_surface(c.selection()[0]);
    assert(inspector && inspector->raw_flags == 10);
    // Real binary parser -> transformed overlay -> object picking -> authoring
    // -> canonical HITS reparse, including rejection of a replacement SCM.
    assert(c.open_hits(source));
    assert(c.open_scm(scm_fixture()));
    c.orbit(0.5F, 0.5F); // Keep the horizontal SCM triangle visible to picking.
    const auto overlay = c.frame(800, 600);
    auto object = std::find_if(overlay.begin(), overlay.end(),
                               [](const auto& t) { return t.scm_object.has_value(); });
    assert(object != overlay.end());
    const auto q = object->points;
    assert(c.pick((q[0].x + q[1].x + q[2].x) / 3, (q[0].y + q[1].y + q[2].y) / 3, 800, 600, true));
    assert(!c.open_scm({})); // Selected overlay remains valid after failure.
    assert(c.import_selected_object(hits::editor::CollisionPreset::orange_raw_00000009));
    assert(c.selection().size() == 1);
    assert(c.session()->editor().surfaces().size() == 2);
    auto imported_bytes = c.save();
    assert(imported_bytes.ok());
    auto imported_scan = formats::hits::RecordScanner::scan(imported_bytes.bytes);
    assert(imported_scan.ok());
    assert(imported_scan.triangles[1].flags == 9);
    const auto& imported = c.session()->editor().surfaces()[1];
    assert(imported.point_a.x >= 15 && imported.point_a.z >= 20 && imported.point_a.y == 2);
}
