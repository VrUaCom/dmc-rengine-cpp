#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The localization message tables beside the game (`m01_s00_msg_eng.txt`,
// `_frn`, `_chn`, ...): a cutscene's Message clips (`No 100`) show their
// message as a subtitle. A table is text:
//   # Msg 00100
//       "<PS 124 421><IT 0><SZ 22>"
//       "Sorry, not open for business yet.<WT 155>"
//       "<FO 0>"
//   # End
// ... up to `# AllEnd`. The quoted strings of a message run together; the
// tags (the table's own header explains them) are
//   <PS x y>  where the text starts on the 640 x 480 screen   (0 0)
//   <IT n>    frames between characters                       (1)
//   <FC r g b> / <BC r g b>  front / back colour, 128 = 1.0   (128 / 0)
//   <SZ n>    font size in screen pixels                      (20)
//   <WT n>    wait n frames before what follows
//   <BR>      new line;  <CL>  clear: what follows is a new page
//   <JP n>    go on with message n;  <NE>  stay after the end
//   <FI n> / <FO n>  fade the whole message in / out over n frames
//   <AL a>    the whole message's alpha (128 = 1.0)
//   <WI>      wait for the player (not waited here);  <MCN> <MCR>  (unused)
// The text is UTF-8 in the HD files (Chinese with a BOM); other bytes are
// read as Windows-1252. The comments (`;`) are Shift-JIS and skipped.
// First written for the DMC Native Reader (cutscene subtitles, v92).
namespace dmc::rengine::formats::messages {

struct Page final {
    // Lines with the frame (of the message) each code point appears at.
    struct Line final {
        std::string text;
        std::vector<float> appear;  // one per code point
        std::vector<std::size_t> offset;  // byte offset of each code point
    };
    std::vector<Line> lines;
    float start{};
    float end{};
    float x{};
    float y{};
    float size{20.0F};
    std::array<std::uint8_t, 3> front{128U, 128U, 128U};
    std::array<std::uint8_t, 3> back{0U, 0U, 0U};
};

struct Message final {
    int number{};
    std::vector<Page> pages;
    float length{};      // frames until its last wait ends
    float fade_in{};
    float fade_out{};
    float alpha{1.0F};
    bool stays{};        // <NE>
};

struct Table final {
    std::string language;  // "eng" from m01_s00_msg_eng.txt; empty when the name says none
    std::map<int, Message> messages;
};

[[nodiscard]] bool looks_like_table(std::span<const std::uint8_t> bytes) noexcept;
[[nodiscard]] std::optional<Table> parse_table(std::string_view name, std::span<const std::uint8_t> bytes);

// What a message shows `frame` frames after it starts: its page's lines as
// far as they have appeared, where, how big and how opaque; none before its
// first character, between pages and after its end (unless it stays).
struct Shown final {
    std::vector<std::string> lines;
    float x{};      // the page's PS position on the 640 x 480 screen
    float y{};
    float size{};   // font size in screen pixels of that screen
    std::array<std::uint8_t, 3> front{};
    std::array<std::uint8_t, 3> back{};
    float alpha{1.0F};
};
// The screen the positions are on: 640 x 480 (an inference: the two-line
// English subtitles start at y 421 with size 22, past a 448-line screen).
inline constexpr float kScreenWidth = 640.0F;
inline constexpr float kScreenHeight = 480.0F;
[[nodiscard]] std::optional<Shown> shown_at(const Message& message, float frame);

// How the shell draws it on a picture of any size: the lines centred at
// `centre_x` (the x of every table centres its line for the game's font, so
// the screen's centre), the first line's top at `top`, the font `size`
// high, all as fractions of the picture; colours on 0..255 (the table's
// 128 is full), the back colour the text's outline.
struct Layout final {
    float centre_x{0.5F};
    float top{};
    float size{};
    std::array<std::uint8_t, 3> front{};
    std::array<std::uint8_t, 3> back{};
    float alpha{1.0F};
};
[[nodiscard]] Layout layout(const Shown& shown) noexcept;

} // namespace dmc::rengine::formats::messages
