// Localization message tables (formats/messages.hpp): tags, pages, waits,
// jumps, typing, colours and the subtitle layout.
#include "dmc_rengine/formats/messages.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <span>
#include <string>

namespace messages = dmc::rengine::formats::messages;

namespace {
bool near(float a, float b, float eps = 1.0e-3F) { return std::fabs(a - b) <= eps; }
} // namespace

int main() {
        // Comments in Shift-JIS, a Windows-1252 e acute, two lines, a page
        // cleared after 20 frames and a 10-frame gap, a jump, typed text.
        const std::string table_text =
            "# NotRegistered\r\n; \x83\x81\x83\x62\r\n"
            "# Msg 00007\r\n\t\"<PS 100 421><IT 0><SZ 24>\"\r\n\t\"D\xe9sol\xe9,<BR>second line<WT 20><CL>\"\r\n"
            "\t\"<WT 10><PS 200 430>page two<WT 30>\"\r\n\t\"<JP 8><FO 0>\"\r\n# End\r\n"
            "# Msg 00008\r\n\t\"<IT 2><FC 128 64 0>abc<WT 5><NE>\"\r\n# End\r\n# AllEnd\r\n";
        const std::span<const std::uint8_t> table_bytes{reinterpret_cast<const std::uint8_t*>(table_text.data()),
                                                        table_text.size()};
        assert(messages::looks_like_table(table_bytes));
        const auto table = messages::parse_table("m99_t00_msg_eng.txt", table_bytes);
        assert(table && table->language == "eng" && table->messages.size() == 2U);
        const auto& m7 = table->messages.at(7);
        auto shown = messages::shown_at(m7, 5.0F);
        assert(shown && shown->lines.size() == 2U && shown->lines[0] == "D\xC3\xA9sol\xC3\xA9," &&
               shown->lines[1] == "second line" && shown->x == 100.0F && shown->size == 24.0F);
        assert(!messages::shown_at(m7, 25.0F));                                   // the gap
        shown = messages::shown_at(m7, 31.0F);
        assert(shown && shown->lines.size() == 1U && shown->lines[0] == "page two" && shown->x == 200.0F);
        // Message 8 follows at 60: typed two frames a letter, then held (NE).
        shown = messages::shown_at(m7, 62.5F);
        assert(shown && shown->lines[0] == "ab" && shown->front[1] == 64U);
        assert(messages::shown_at(m7, 1000.0F) && messages::shown_at(m7, 1000.0F)->lines[0] == "abc");
        assert(!messages::shown_at(table->messages.at(8), -1.0F));
        const auto look = messages::layout(*messages::shown_at(m7, 5.0F));
        assert(near(look.centre_x, 0.5F) && near(look.top, 421.0F / 480.0F) && look.front[0] == 255U && look.back[0] == 0U);
        assert(messages::layout(*shown).front[1] == 127U);
        assert(!messages::parse_table("x.txt", std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>("CAM"), 3U}));

    return 0;
}
