#pragma once

/// Minimal indenting JSON writer shared by the executable analysis reports.
///
/// Internal to `src/exe`. Members are emitted in call order, so the caller
/// controls the layout and the output stays byte-stable across runs, which is
/// what lets a report be committed as an evidence record.

#include <array>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>

namespace dmc::rengine::exe::detail {

[[nodiscard]] inline std::string escape_json(std::string_view value) {
    std::ostringstream output;
    constexpr char hex[] = "0123456789abcdef";

    for (const unsigned char character : value) {
        switch (character) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20U) {
                output << "\\u00" << hex[(character >> 4U) & 0x0FU] << hex[character & 0x0FU];
            } else {
                output << static_cast<char>(character);
            }
            break;
        }
    }
    return output.str();
}

[[nodiscard]] inline std::string hex_u64(std::uint64_t value) {
    std::array<char, 32> buffer{};
    const int written = std::snprintf(buffer.data(), buffer.size(), "0x%llx",
                                      static_cast<unsigned long long>(value));
    if (written <= 0) {
        return "0x0";
    }
    return std::string{buffer.data(), static_cast<std::size_t>(written)};
}

/// Minimal indenting JSON writer.
///
/// Members are emitted in call order, so the caller controls the layout and
/// the output stays byte-stable across runs.
class JsonWriter final {
public:
    explicit JsonWriter(std::ostringstream& output) : output_(output) {}

    void begin_object() {
        separate();
        output_ << '{';
        ++depth_;
        fresh_ = true;
    }

    void end_object() {
        --depth_;
        if (!fresh_) {
            newline();
        }
        output_ << '}';
        fresh_ = false;
    }

    void begin_array() {
        separate();
        output_ << '[';
        ++depth_;
        fresh_ = true;
    }

    void end_array() {
        --depth_;
        if (!fresh_) {
            newline();
        }
        output_ << ']';
        fresh_ = false;
    }

    void key(std::string_view name) {
        separate();
        output_ << '"' << escape_json(name) << "\": ";
        suppress_separator_ = true;
    }

    void string(std::string_view value) {
        separate();
        output_ << '"' << escape_json(value) << '"';
        fresh_ = false;
    }

    void number(std::uint64_t value) {
        separate();
        output_ << value;
        fresh_ = false;
    }

    void number(std::int64_t value) {
        separate();
        output_ << value;
        fresh_ = false;
    }

    void boolean(bool value) {
        separate();
        output_ << (value ? "true" : "false");
        fresh_ = false;
    }

    void member(std::string_view name, std::string_view value) {
        key(name);
        string(value);
    }

    /// Without this overload a string literal converts to `bool` before it
    /// converts to `string_view`, and every literal member silently becomes
    /// `true`.
    void member(std::string_view name, const char* value) {
        key(name);
        string(value == nullptr ? std::string_view{} : std::string_view{value});
    }

    void member(std::string_view name, std::uint64_t value) {
        key(name);
        number(value);
    }

    void member(std::string_view name, bool value) {
        key(name);
        boolean(value);
    }

    void hex_member(std::string_view name, std::uint64_t value) {
        key(name);
        string(hex_u64(value));
    }

private:
    void newline() {
        output_ << '\n';
        for (int level = 0; level < depth_; ++level) {
            output_ << "  ";
        }
    }

    void separate() {
        if (suppress_separator_) {
            suppress_separator_ = false;
            return;
        }
        if (at_start_) {
            at_start_ = false;
            fresh_ = false;
            return;
        }
        if (!fresh_) {
            output_ << ',';
        }
        newline();
        fresh_ = false;
    }

    std::ostringstream& output_;
    int depth_{};
    bool fresh_{true};
    bool at_start_{true};
    bool suppress_separator_{false};
};

} // namespace dmc::rengine::exe::detail
