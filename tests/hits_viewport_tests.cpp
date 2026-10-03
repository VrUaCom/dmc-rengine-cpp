#include "dmc_rengine/hits/viewport.hpp"
#include "hits_test_fixture.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

using namespace dmc::rengine;
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
}
