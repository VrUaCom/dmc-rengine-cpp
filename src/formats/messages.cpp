#include "dmc_rengine/formats/messages.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace dmc::rengine::formats::messages {
namespace {

constexpr std::string_view kBom{"\xEF\xBB\xBF"};

// Whether the bytes are well-formed UTF-8.
[[nodiscard]] bool valid_utf8(std::string_view s) noexcept {
    for (std::size_t i = 0; i < s.size();) {
        const auto c = static_cast<unsigned char>(s[i]);
        std::size_t n = c < 0x80U ? 1U : (c >> 5U) == 6U ? 2U : (c >> 4U) == 14U ? 3U : (c >> 3U) == 30U ? 4U : 0U;
        if (n == 0U || i + n > s.size()) return false;
        for (std::size_t k = 1; k < n; ++k) {
            if ((static_cast<unsigned char>(s[i + k]) & 0xC0U) != 0x80U) return false;
        }
        i += n;
    }
    return true;
}

void append_utf8(std::string* out, std::uint32_t cp) {
    if (cp < 0x80U) {
        out->push_back(static_cast<char>(cp));
    } else if (cp < 0x800U) {
        out->push_back(static_cast<char>(0xC0U | (cp >> 6U)));
        out->push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    } else {
        out->push_back(static_cast<char>(0xE0U | (cp >> 12U)));
        out->push_back(static_cast<char>(0x80U | ((cp >> 6U) & 0x3FU)));
        out->push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    }
}

// Windows-1252 to UTF-8 (0x80..0x9F as the code page has them).
[[nodiscard]] std::string from_1252(std::string_view s) {
    static constexpr std::uint16_t kHigh[32] = {
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
        0x2039, 0x0152, 0x008D, 0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
        0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};
    std::string out;
    for (const char ch : s) {
        const auto c = static_cast<unsigned char>(ch);
        append_utf8(&out, c >= 0x80U && c < 0xA0U ? kHigh[c - 0x80U] : c);
    }
    return out;
}

// The length of the UTF-8 code point starting at s[i].
[[nodiscard]] std::size_t point_length(std::string_view s, std::size_t i) noexcept {
    const auto c = static_cast<unsigned char>(s[i]);
    const std::size_t n = c < 0x80U ? 1U : (c >> 5U) == 6U ? 2U : (c >> 4U) == 14U ? 3U : (c >> 3U) == 30U ? 4U : 1U;
    return std::min(n, s.size() - i);
}

[[nodiscard]] std::vector<float> numbers(std::string_view text) {
    std::vector<float> out;
    std::string token;
    const auto flush = [&] {
        if (!token.empty()) out.push_back(std::strtof(token.c_str(), nullptr));
        token.clear();
    };
    for (const char c : text) {
        if (c == ' ' || c == '\t') {
            flush();
        } else {
            token.push_back(c);
        }
    }
    flush();
    return out;
}

[[nodiscard]] std::array<std::uint8_t, 3> colour(const std::vector<float>& v) {
    std::array<std::uint8_t, 3> out{};
    for (std::size_t k = 0; k < 3U && k < v.size(); ++k) {
        out[k] = static_cast<std::uint8_t>(std::clamp(v[k], 0.0F, 255.0F));
    }
    return out;
}

struct Built final {
    Message message;
    std::vector<std::pair<int, float>> jumps;  // <JP n> at a frame
};

// The pages of one message's text (its quoted strings run together).
[[nodiscard]] Built build(int number, std::string_view text) {
    Built built;
    auto& m = built.message;
    m.number = number;
    float t = 0.0F;
    float interval = 1.0F;
    Page state;  // the attributes in force
    Page page = state;
    bool any = false;  // the page has a character
    const auto close = [&] {
        page.end = t;
        if (any) m.pages.push_back(page);
        page = state;
        page.lines.clear();
        page.start = t;
        any = false;
    };
    for (std::size_t i = 0; i < text.size();) {
        if (text[i] == '<') {
            const auto end = text.find('>', i);
            if (end == std::string_view::npos) break;
            const auto tag = text.substr(i + 1U, end - i - 1U);
            i = end + 1U;
            const auto space = tag.find(' ');
            const auto name = tag.substr(0, space);
            const auto values = space == std::string_view::npos ? std::vector<float>{} : numbers(tag.substr(space + 1U));
            const float first = values.empty() ? 0.0F : values.front();
            if (name == "WT") {
                t += std::max(first, 0.0F);
            } else if (name == "BR") {
                if (page.lines.empty()) page.lines.emplace_back();
                page.lines.emplace_back();
            } else if (name == "CL") {
                close();
            } else if (name == "IT") {
                interval = std::max(first, 0.0F);
            } else if (name == "PS" && values.size() >= 2U) {
                state.x = values[0];
                state.y = values[1];
            } else if (name == "SZ" && !values.empty()) {
                state.size = first;
            } else if (name == "FC") {
                state.front = colour(values);
            } else if (name == "BC") {
                state.back = colour(values);
            } else if (name == "FI") {
                m.fade_in = std::max(first, 0.0F);
            } else if (name == "FO") {
                m.fade_out = std::max(first, 0.0F);
            } else if (name == "AL") {
                m.alpha = std::clamp(first / 128.0F, 0.0F, 1.0F);
            } else if (name == "NE") {
                m.stays = true;
            } else if (name == "JP") {
                built.jumps.emplace_back(static_cast<int>(first), t);
            }
            // A page takes the attributes in force until its first character.
            if (!any) {
                page.x = state.x;
                page.y = state.y;
                page.size = state.size;
                page.front = state.front;
                page.back = state.back;
            }
            continue;
        }
        const auto n = point_length(text, i);
        if (page.lines.empty()) page.lines.emplace_back();
        auto& line = page.lines.back();
        line.offset.push_back(line.text.size());
        line.text.append(text.substr(i, n));
        line.appear.push_back(t);
        t += interval;
        any = true;
        i += n;
    }
    close();
    m.length = t;
    return built;
}

}  // namespace

bool looks_like_table(std::span<const std::uint8_t> bytes) noexcept {
    const std::string_view text{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    return text.find("# Msg ") != std::string_view::npos && text.find("# End") != std::string_view::npos;
}

std::optional<Table> parse_table(std::string_view name, std::span<const std::uint8_t> bytes) {
    if (!looks_like_table(bytes)) return std::nullopt;
    std::string_view text{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    if (text.starts_with(kBom)) text.remove_prefix(kBom.size());
    Table table;
    if (const auto at = name.rfind("_msg_"); at != std::string_view::npos) {
        auto language = name.substr(at + 5U);
        language = language.substr(0, language.find('.'));
        table.language = std::string{language};
    }
    std::map<int, Built> built;
    int current = -1;
    std::string body;
    std::size_t start = 0;
    while (start <= text.size()) {
        auto end = text.find('\n', start);
        if (end == std::string_view::npos) end = text.size();
        auto line = text.substr(start, end - start);
        start = end + 1U;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) line.remove_suffix(1);
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.remove_prefix(1);
        if (line.starts_with("# AllEnd")) break;
        if (line.starts_with("# Msg ")) {
            current = std::atoi(std::string{line.substr(6U)}.c_str());
            body.clear();
            continue;
        }
        if (line.starts_with("# End")) {
            if (current >= 0) {
                const std::string utf8 = valid_utf8(body) ? body : from_1252(body);
                built[current] = build(current, utf8);
            }
            current = -1;
            continue;
        }
        if (current < 0 || line.empty() || line.front() == ';') continue;
        // The quoted strings of the line (a ';' outside them starts a comment).
        bool quoted = false;
        for (const char c : line) {
            if (c == '"') {
                quoted = !quoted;
            } else if (quoted) {
                body.push_back(c);
            } else if (c == ';') {
                break;
            }
        }
        if (start > text.size()) break;
    }
    // <JP n>: the message goes on with message n's pages from that frame.
    for (auto& [number, b] : built) {
        auto& m = b.message;
        int guard = 0;
        for (std::size_t j = 0; j < b.jumps.size() && guard < 8; ++j, ++guard) {
            const auto next = built.find(b.jumps[j].first);
            if (next == built.end() || next->first == number) continue;
            const float at = b.jumps[j].second;
            for (auto page : next->second.message.pages) {
                page.start += at;
                page.end += at;
                for (auto& line : page.lines) {
                    for (auto& a : line.appear) a += at;
                }
                m.pages.push_back(std::move(page));
            }
            m.length = std::max(m.length, at + next->second.message.length);
            // The message it goes on with ends it: its <NE> holds the whole.
            m.stays = m.stays || next->second.message.stays;
        }
        table.messages[number] = m;
    }
    if (table.messages.empty()) return std::nullopt;
    return table;
}

std::optional<Shown> shown_at(const Message& message, float frame) {
    if (!(frame >= 0.0F) || message.pages.empty()) return std::nullopt;
    float alpha = message.alpha;
    if (message.fade_in > 0.0F) alpha *= std::clamp(frame / message.fade_in, 0.0F, 1.0F);
    const Page* page = nullptr;
    if (frame >= message.length) {
        // After the end: held when it stays, else fading out.
        if (!message.stays) {
            if (!(message.fade_out > 0.0F) || frame >= message.length + message.fade_out) return std::nullopt;
            alpha *= 1.0F - (frame - message.length) / message.fade_out;
        }
        page = &message.pages.back();
        if (page->end < message.length) return std::nullopt;  // a gap closed the message
    } else {
        for (const auto& p : message.pages) {
            if (frame >= p.start && frame < p.end) page = &p;
        }
    }
    if (page == nullptr) return std::nullopt;
    Shown out;
    out.x = page->x;
    out.y = page->y;
    out.size = page->size;
    out.front = page->front;
    out.back = page->back;
    out.alpha = std::clamp(alpha, 0.0F, 1.0F);
    bool any = false;
    for (const auto& line : page->lines) {
        std::size_t visible = 0;
        while (visible < line.appear.size() && line.appear[visible] <= frame) ++visible;
        out.lines.push_back(visible < line.offset.size() ? line.text.substr(0, line.offset[visible]) : line.text);
        any = any || visible > 0U;
    }
    while (!out.lines.empty() && out.lines.back().empty()) out.lines.pop_back();
    if (!any || !(out.alpha > 0.0F)) return std::nullopt;
    return out;
}

Layout layout(const Shown& shown) noexcept {
    const auto full = [](const std::array<std::uint8_t, 3>& c) {
        std::array<std::uint8_t, 3> out{};
        for (std::size_t k = 0; k < 3U; ++k) out[k] = static_cast<std::uint8_t>(std::min(255, c[k] * 255 / 128));
        return out;
    };
    Layout out;
    out.top = shown.y / kScreenHeight;
    out.size = shown.size / kScreenHeight;
    out.front = full(shown.front);
    out.back = full(shown.back);
    out.alpha = shown.alpha;
    return out;
}

} // namespace dmc::rengine::formats::messages
