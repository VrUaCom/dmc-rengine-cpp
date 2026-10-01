#include "dmc_rengine/profiles/dmc3/fx/effect_bank.hpp"

#include <bit>
#include <cmath>
#include <cstring>

namespace dmc::rengine::profiles::dmc3::fx::effect_bank {
namespace {

[[nodiscard]] std::uint16_t u16(std::span<const std::uint8_t> b, std::size_t o) noexcept {
    if (o + 2U > b.size()) return 0U;
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(b[o]) |
        (static_cast<std::uint16_t>(b[o + 1U]) << 8U));
}

[[nodiscard]] std::uint32_t u32(std::span<const std::uint8_t> b, std::size_t o) noexcept {
    if (o + 4U > b.size()) return 0U;
    return static_cast<std::uint32_t>(b[o]) | (static_cast<std::uint32_t>(b[o + 1U]) << 8U) |
           (static_cast<std::uint32_t>(b[o + 2U]) << 16U) | (static_cast<std::uint32_t>(b[o + 3U]) << 24U);
}

[[nodiscard]] std::int32_t i32(std::span<const std::uint8_t> b, std::size_t o) noexcept {
    return static_cast<std::int32_t>(u32(b, o));
}

[[nodiscard]] std::int16_t i16(std::span<const std::uint8_t> b, std::size_t o) noexcept {
    return static_cast<std::int16_t>(u16(b, o));
}

[[nodiscard]] float f32(std::span<const std::uint8_t> b, std::size_t o) noexcept {
    const auto bits = u32(b, o);
    float value = 0.0F;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

[[nodiscard]] bool add_relative(std::size_t base, std::int32_t relative,
                                std::size_t size, std::size_t* out) noexcept {
    if (out == nullptr) return false;
    const auto absolute = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(relative);
    if (absolute < 0 || static_cast<std::uint64_t>(absolute) >= size) return false;
    *out = static_cast<std::size_t>(absolute);
    return true;
}

// Relative-slot container (PNST): magic, u32 count, u32 offsets from the
// container start; a slot ends at the next larger offset.
struct Slots final {
    std::vector<std::span<const std::uint8_t>> slots;
};

[[nodiscard]] std::optional<Slots> read_pnst(std::span<const std::uint8_t> b) {
    if (b.size() < 12U || std::memcmp(b.data(), "PNST", 4U) != 0) return std::nullopt;
    const auto count = u32(b, 4U);
    if (count == 0U || count > 8192U || 8U + static_cast<std::size_t>(count) * 4U > b.size()) {
        return std::nullopt;
    }
    std::vector<std::uint32_t> offsets(count);
    for (std::uint32_t i = 0U; i < count; ++i) offsets[i] = u32(b, 8U + i * 4U);
    Slots out;
    out.slots.resize(count);
    for (std::uint32_t i = 0U; i < count; ++i) {
        const auto start = offsets[i];
        if (start == 0U) continue;
        if (start >= b.size()) return std::nullopt;
        std::size_t end = b.size();
        for (const auto o : offsets) {
            if (o > start && o < end) end = o;
        }
        out.slots[i] = b.subspan(start, end - start);
    }
    return out;
}

// Tokenizer 0x140322AB0: separators NUL, tab, LF, CR, space, ','; ';' skips
// to the end of the line; '$' ends the text.
class Tokens final {
public:
    explicit Tokens(std::string_view text) : text_(text) {}

    [[nodiscard]] std::optional<std::string_view> next() {
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if (c == '$') {
                pos_ = text_.size();
                return std::nullopt;
            }
            if (c == ';') {
                while (pos_ < text_.size() && text_[pos_] != '\n') ++pos_;
                continue;
            }
            if (separator(c)) {
                ++pos_;
                continue;
            }
            break;
        }
        if (pos_ >= text_.size()) return std::nullopt;
        const auto begin = pos_;
        while (pos_ < text_.size() && !separator(text_[pos_]) && text_[pos_] != '$' && text_[pos_] != ';') ++pos_;
        return text_.substr(begin, pos_ - begin);
    }

private:
    [[nodiscard]] static bool separator(char c) noexcept {
        return c == '\0' || c == '\t' || c == '\n' || c == '\r' || c == ' ' || c == ',';
    }
    std::string_view text_;
    std::size_t pos_{};
};

[[nodiscard]] std::optional<std::uint32_t> decimal(std::string_view s) noexcept {
    if (s.empty() || s.size() > 9U) return std::nullopt;
    std::uint32_t v = 0U;
    for (const char c : s) {
        if (c < '0' || c > '9') return std::nullopt;
        v = v * 10U + static_cast<std::uint32_t>(c - '0');
    }
    return v;
}

}  // namespace

std::uint64_t registrar(char kind) noexcept {
    switch (kind) {
    case 'A': return 0x140322990ULL;
    case 'C': return 0x1402D3BE0ULL;
    case 'E': return 0x1402E87A0ULL;
    case 'G': return 0x1402ECBD0ULL;
    case 'M': return 0x1402E35D0ULL;
    case 'P': return 0x140314B80ULL;
    case 'T': return 0x140322F20ULL;
    case 'V': return 0x140325030ULL;
    default: return 0ULL;
    }
}

std::string_view kind_name(char kind) noexcept {
    // Keep the UI neutral: letters are the canonical manifest/runtime kinds.
    // Do not turn nearby class names or tool-side interpretations into file
    // format names. Only T/M carry a byte-backed payload description here.
    switch (kind) {
    case 'T': return "T record (descriptor + DDS payload)";
    case 'M': return "M record (MOD / EFM payload + companion)";
    case 'V': return "V record (P/E/G/V composite dispatch)";
    case 'G': return "G record";
    case 'E': return "E record";
    case 'P': return "P record";
    case 'A': return "A record";
    case 'C': return "C record";
    default: return "unregistered manifest kind";
    }
}

bool looks_like_bank(std::span<const std::uint8_t> bytes) noexcept {
    try {
        const auto outer = read_pnst(bytes);
        if (!outer || outer->slots.size() < 2U || outer->slots[0].empty() || outer->slots[1].size() < 12U ||
            std::memcmp(outer->slots[1].data(), "PNST", 4U) != 0) {
            return false;
        }
        const std::string_view text{reinterpret_cast<const char*>(outer->slots[0].data()), outer->slots[0].size()};
        Tokens tokens{text};
        const auto kind = tokens.next();
        const auto id = tokens.next();
        const auto parsed_id = id ? decimal(*id) : std::optional<std::uint32_t>{};
        return kind && kind->size() == 1U && (*kind)[0] >= 'A' && (*kind)[0] <= 'Z' &&
               parsed_id.has_value() && *parsed_id <= 0xFFFFU;
    } catch (...) {
        return false;
    }
}

std::optional<Bank> parse_bank(std::span<const std::uint8_t> bytes) {
    if (!looks_like_bank(bytes)) return std::nullopt;
    const auto outer = read_pnst(bytes);
    const auto inner = read_pnst(outer->slots[1]);
    if (!inner) return std::nullopt;
    Bank bank;
    bank.record_slots = inner->slots.size();
    bank.manifest_bytes = outer->slots[0].size();
    const std::string_view text{reinterpret_cast<const char*>(outer->slots[0].data()), outer->slots[0].size()};
    Tokens tokens{text};
    std::uint32_t slot = 0U;
    while (bank.records.size() < 4096U) {
        const auto token = tokens.next();
        if (!token) break;
        if ((*token)[0] == '#') {
            bank.terminated = true;
            break;
        }
        const auto id_token = tokens.next();
        const auto parsed_id = id_token ? decimal(*id_token) : std::optional<std::uint32_t>{};
        if (id_token && (!parsed_id.has_value() || *parsed_id > 0xFFFFU)) {
            return std::nullopt;
        }
        Record r;
        r.kind = (*token)[0];
        if (!parsed_id.has_value()) return std::nullopt;
        r.id = static_cast<std::uint16_t>(*parsed_id);
        r.slot = slot;
        if (slot >= inner->slots.size()) return std::nullopt;
        r.bytes = inner->slots[slot];
        ++slot;
        if (r.kind == 'M') {
            if (slot >= inner->slots.size()) return std::nullopt;
            r.companion = inner->slots[slot];
            ++slot;
        }
        bank.records.push_back(r);
    }
    for (; slot < inner->slots.size(); ++slot) {
        // Empty capacity slots are legal. A populated unconsumed slot would
        // contradict the EXE physical-slot walk and must not be re-labeled.
        if (!inner->slots[slot].empty()) return std::nullopt;
    }
    return bank;
}

std::optional<SpriteAnimation> sprite_animation(const Record& record) {
    const auto& b = record.bytes;
    if (record.kind != 'A' || b.size() < 16U || b[0] != 1U) return std::nullopt;
    SpriteAnimation out;
    out.texture = b[1];
    out.frame_time = b[2];
    out.loop = b[4] != 0U;
    out.loop_frame = b[5];
    const std::size_t count = static_cast<std::size_t>(b[3]) + 1U;
    for (std::size_t i = 0U; i < count; ++i) {
        const std::size_t o = 6U + i * 10U;
        if (o + 8U > b.size()) break;
        const auto u16 = [&b](std::size_t at) {
            return static_cast<std::uint16_t>(b[at] | (b[at + 1U] << 8U));
        };
        out.frames.push_back({u16(o), u16(o + 2U), u16(o + 4U), u16(o + 6U)});
    }
    return out;
}

std::optional<EffectDescriptor> effect_descriptor(const Record& record) {
    const auto& b = record.bytes;
    if (record.kind != 'E' || b.size() < 0x14U) return std::nullopt;
    EffectDescriptor out;
    out.mode = b[0x01U];
    out.texture = u16(b, 0x04U);
    out.animation_gate = b[0x06U];
    out.animation = u16(b, 0x08U);
    out.rectangle = {
        u16(b, 0x0CU), u16(b, 0x0EU),
        u16(b, 0x10U), u16(b, 0x12U)};
    // CEffect 0x1402E4190 loads +0x80 (i32 -> float) into effect+0x8B0;
    // the state-1 update 0x1402E47F0 subtracts dt unless +0x84 is set.
    if (b.size() >= 0x85U) {
        out.lifetime_ticks = i32(b, 0x80U);
        out.lifetime_known = true;
        out.held_by_parent = b[0x84U] != 0U;
    }
    if (b.size() >= 0x1F6U) {
        const auto mean = [&b](std::size_t o) {
            return 0.5F * (f32(b, o) + f32(b, o + 4U));
        };
        const auto size_mode = b[0x2CU];
        if (size_mode == 0U) {
            out.size = {f32(b, 0x3CU), f32(b, 0x44U), f32(b, 0x4CU)};
            out.pivot = {f32(b, 0x30U), f32(b, 0x34U), f32(b, 0x38U)};
        } else {
            if (size_mode == 1U) {
                out.size = {mean(0x3CU), mean(0x44U), mean(0x4CU)};
            } else {
                const float uniform = mean(0x3CU);
                out.size = {uniform, uniform, uniform};
            }
            // 0x1402E44FD: B = A * 0.5 (a zero extent keeps a zero pivot).
            for (std::size_t i = 0U; i < 3U; ++i) out.pivot[i] = 0.5F * out.size[i];
        }
        out.scale = {f32(b, 0xA8U), f32(b, 0xACU), f32(b, 0xB0U)};
        for (std::size_t i = 0U; i < 3U; ++i) {
            const std::size_t o = 0x150U + 12U * i;
            out.rotation_degrees[i] = b[o] == 1U ? mean(o + 4U) : 0.0F;
        }
        out.orientation = b[0x1F5U];
        bool finite = true;
        for (std::size_t i = 0U; i < 3U; ++i) {
            finite = finite && std::isfinite(out.size[i]) && std::isfinite(out.pivot[i]) &&
                     std::isfinite(out.scale[i]) && std::isfinite(out.rotation_degrees[i]);
        }
        out.geometry_known = finite;
    }
    return out;
}

std::optional<CompositeRecord> composite_record(const Record& record) {
    const auto& b = record.bytes;
    if (record.kind != 'V' || b.size() < 4U) return std::nullopt;
    const auto count = i16(b, 0U);
    if (count < 0 || count > 64) return std::nullopt;
    constexpr std::size_t kEntrySize = 0x2CU;
    const auto total = 4U + static_cast<std::size_t>(count) * kEntrySize;
    if (total > b.size()) return std::nullopt;

    CompositeRecord out;
    out.entries.reserve(static_cast<std::size_t>(count));
    for (std::size_t index = 0U; index < static_cast<std::size_t>(count); ++index) {
        const auto base = 4U + index * kEntrySize;
        CompositeEntry entry;
        entry.dispatch_kind = b[base + 0x00U];
        if (entry.dispatch_kind > 3U) return std::nullopt;
        entry.id = u16(b, base + 0x02U);
        entry.activation_offset = i16(b, base + 0x04U);
        for (std::size_t component = 0U; component < 3U; ++component) {
            entry.translation[component] = f32(b, base + 0x08U + component * 4U);
            entry.rotation_degrees[component] = f32(b, base + 0x14U + component * 4U);
            entry.scale[component] = f32(b, base + 0x20U + component * 4U);
            if (!std::isfinite(entry.translation[component]) ||
                !std::isfinite(entry.rotation_degrees[component]) ||
                !std::isfinite(entry.scale[component])) {
                return std::nullopt;
            }
        }
        out.entries.push_back(entry);
    }
    return out;
}

std::span<const std::uint8_t> texture_dds(const Record& record) noexcept {
    const auto& b = record.bytes;
    if (record.kind != 'T' || b.size() < kTextureDescriptorSize + 128U) return {};
    if (std::memcmp(b.data() + kTextureDescriptorSize, "DDS ", 4U) != 0) return {};
    return b.subspan(kTextureDescriptorSize);
}

std::optional<VRuntimeView> v_runtime_view(const Record& record) {
    if (record.kind != 'V' || record.bytes.size() < 0x30U) return std::nullopt;
    const auto b = record.bytes;
    const auto signed_count = static_cast<std::int16_t>(u16(b, 0U));
    if (signed_count < 0 || signed_count > 64) return std::nullopt;

    VRuntimeView out;
    out.count = signed_count;
    out.entries.reserve(static_cast<std::size_t>(signed_count));
    for (std::int16_t index = 0; index < signed_count; ++index) {
        const std::size_t base = static_cast<std::size_t>(index) * 0x2CU;
        if (base + 0x30U > b.size()) return std::nullopt;

        VEntry entry;
        entry.dispatch = b[base + 0x04U];
        entry.id = u16(b, base + 0x06U);
        entry.translation = {
            f32(b, base + 0x0CU),
            f32(b, base + 0x10U),
            f32(b, base + 0x14U),
        };
        entry.rotation_degrees = {
            f32(b, base + 0x18U),
            f32(b, base + 0x1CU),
            f32(b, base + 0x20U),
        };
        entry.scale = {
            f32(b, base + 0x24U),
            f32(b, base + 0x28U),
            f32(b, base + 0x2CU),
        };
        out.entries.push_back(entry);
    }
    return out;
}

std::optional<ERuntimeView> e_runtime_view(const Record& record) {
    if (record.kind != 'E' || record.bytes.size() < 0x14U) return std::nullopt;
    const auto b = record.bytes;

    ERuntimeView out;
    out.mode = b[0x01U];
    out.texture_id = u16(b, 0x04U);
    out.uses_animation = b[0x06U] == 1U && u16(b, 0x08U) != 0xFFFFU;
    out.animation_id = u16(b, 0x08U);
    out.rectangle = {
        u16(b, 0x0CU),
        u16(b, 0x0EU),
        u16(b, 0x10U),
        u16(b, 0x12U),
    };
    return out;
}

std::optional<GRuntimeView> g_runtime_view(const Record& record) {
    if (record.kind != 'G' || record.bytes.size() < 0x5AU) return std::nullopt;
    const auto b = record.bytes;

    GRuntimeView out;
    out.mode = b[0x01U];
    out.c_id = u16(b, 0x02U);
    out.value_18 = i32(b, 0x18U);
    out.value_20 = i32(b, 0x20U);
    out.mode_30 = b[0x30U];
    out.value_38 = f32(b, 0x38U);
    out.value_3c = f32(b, 0x3CU);
    out.steps_40 = u16(b, 0x40U);
    out.value_50 = f32(b, 0x50U);
    out.value_54 = f32(b, 0x54U);
    out.value_59 = b[0x59U];
    return out;
}

std::optional<PRuntimeView> p_runtime_view(const Record& record) {
    if (record.kind != 'P' || record.bytes.size() < 0x20U) return std::nullopt;
    const auto b = record.bytes;
    const auto version = u32(b, 0U);
    if (version != 2U) return std::nullopt;

    constexpr std::size_t kRelativeBase = 0x10U;
    std::size_t root = 0U;
    std::size_t list = 0U;
    if (!add_relative(kRelativeBase, i32(b, 0x10U), b.size(), &root) ||
        !add_relative(kRelativeBase, i32(b, 0x14U), b.size(), &list) ||
        root + 0xCAU > b.size()) {
        return std::nullopt;
    }

    const auto count = u16(b, root + 0xC0U);
    // 0x140312DC0 stages the relative target table from raw+0x18 in a
    // bounded 0xD0-byte local block, leaving at most 50 entries.
    if (count > 50U || 0x18U + static_cast<std::size_t>(count) * 4U > b.size()) {
        return std::nullopt;
    }

    PRuntimeView out;
    out.version = version;
    out.subtype = b[root + 0x01U];
    out.root_offset = static_cast<std::uint32_t>(root);
    out.list_offset = static_cast<std::uint32_t>(list);
    out.target_offsets.reserve(count);
    for (std::uint16_t index = 0U; index < count; ++index) {
        std::size_t target = 0U;
        if (!add_relative(kRelativeBase, i32(b, 0x18U + static_cast<std::size_t>(index) * 4U),
                          b.size(), &target)) {
            return std::nullopt;
        }
        out.target_offsets.push_back(static_cast<std::uint32_t>(target));
    }
    return out;
}

}  // namespace dmc::rengine::profiles::dmc3::fx::effect_bank
