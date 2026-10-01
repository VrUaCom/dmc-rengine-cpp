#include "dmc_rengine/profiles/dmc3/motion_script.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <cctype>
#include <span>
#include <utility>

namespace dmc::rengine::profiles::dmc3::motion {
namespace {

// Instruction lengths of the fixed-size opcodes (interpreter 0x140058FE0,
// jump table 0x140059240 / 0x140059204).
[[nodiscard]] std::size_t opcode_length(std::uint8_t op) noexcept {
    switch (op) {
    case 1U: return 8U;               // play MOT (0x140059950)
    case 3U: return 6U;               // channel 0 bytes -> +0x91..+0x95
    case 4U: return 6U;               // channel 1 bytes -> +0x96..+0x9A
    case 5U: return 2U;               // clear channels
    case 6U: return 2U;               // 0x140059650
    case 7U: return 2U;
    case 8U: return 2U;
    case 32U: return 4U;              // 0x140059820
    case 33U: return 2U;
    case 34U: return 6U;              // 0x1400596A0
    case 35U: return 2U;
    case 36U: return 4U;              // 0x1400598C0
    default: break;
    }
    if (op >= 16U && op <= 31U) return static_cast<std::size_t>(op - 15U) * 2U + 4U;
    return 0U;
}

}  // namespace

std::optional<MotionScriptFile> MotionScriptFile::parse(std::span<const std::uint8_t> bytes) {
    const auto u16 = [&bytes](std::size_t o) -> std::optional<std::size_t> {
        if (o + 2U > bytes.size()) return std::nullopt;
        return static_cast<std::size_t>(bytes[o]) | (static_cast<std::size_t>(bytes[o + 1U]) << 8U);
    };
    // 0x1400594B0: A = file + u16[0] (scripts), B = file + u16[2] (motion
    // resources). Mode 0 nests once: banks = A + u16[A], resources = B + u16[B].
    const auto table = u16(0U);
    if (!table || *table < 6U || *table >= bytes.size()) return std::nullopt;
    const auto resources = u16(2U);
    // Mode detection: the enemy form (mode 1) has a script (opcode 1) right
    // behind bank 0's first entry; the player form has another offset list.
    bool nested = true;
    if (const auto first = u16(*table)) {
        const std::size_t sub = *table + *first;
        if (const auto entry = u16(sub); entry && *entry != 0xFFFFU && sub + *entry < bytes.size() &&
                                         bytes[sub + *entry] == 1U) {
            nested = false;
        }
    }
    std::size_t base = *table;
    if (nested) {
        const auto first = u16(*table);
        if (!first) return std::nullopt;
        base = *table + *first;
    }
    MotionScriptFile out;
    out.nested_ = nested;
    for (std::size_t o = base; out.banks_.size() < 64U; o += 2U) {
        const auto entry = u16(o);
        if (!entry) return std::nullopt;
        if (*entry == 0xFFFFU) break;
        if (base + *entry >= bytes.size()) return std::nullopt;
        out.banks_.push_back(base + *entry);
    }
    if (out.banks_.empty()) return std::nullopt;
    if (resources && *resources != 0U && *resources < bytes.size()) {
        std::size_t r = *resources;
        if (nested) {
            const auto first = u16(r);
            r = first && *first != 0xFFFFU ? r + *first : 0U;
        }
        out.resources_ = r < bytes.size() ? r : 0U;
    }
    out.bytes_.assign(bytes.begin(), bytes.end());
    out.table_ = *table;
    return out;
}

bool MotionScriptFile::looks_like(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < 16U || (bytes[0] & 1U) != 0U || bytes[1] != 0U || bytes[0] > 64U) {
        return false;
    }
    const auto file = parse(bytes);
    if (!file) return false;
    std::size_t scripts = 0U;
    for (std::size_t bank = 0U; bank < file->bank_count(); ++bank) {
        const auto count = file->script_count(bank);
        if (count == 0U) continue;
        const auto first = file->summarize(bank, 0U);
        if (!first || first->opcodes[1] == 0U) return false;
        scripts += count;
    }
    return scripts != 0U;
}

std::size_t MotionScriptFile::script_count(std::size_t bank) const noexcept {
    if (bank >= banks_.size()) return 0U;
    std::size_t count = 0U;
    for (std::size_t o = banks_[bank]; o + 2U <= bytes_.size() && count < 1024U; o += 2U) {
        const auto entry = static_cast<std::size_t>(bytes_[o]) |
                           (static_cast<std::size_t>(bytes_[o + 1U]) << 8U);
        if (entry == 0xFFFFU) return count;
        ++count;
    }
    return 0U;  // no terminator: not a bank list
}

std::optional<ScriptSummary> MotionScriptFile::summarize(std::size_t bank,
                                                        std::size_t index) const {
    if (bank >= banks_.size() || index >= script_count(bank)) return std::nullopt;
    const auto& s = bytes_;
    const auto u16 = [&s](std::size_t o) -> std::size_t {
        return o + 2U <= s.size()
            ? static_cast<std::size_t>(s[o]) | (static_cast<std::size_t>(s[o + 1U]) << 8U)
            : 0xFFFFFFFFU;
    };
    const std::size_t sub = banks_[bank];
    std::size_t p = sub + u16(sub + index * 2U);
    if (p >= s.size()) return std::nullopt;
    ScriptSummary out;
    float after = -1.0F;
    bool started = false;
    for (int step = 0; step < 4096 && p < s.size(); ++step) {
        const std::uint8_t op = s[p];
        if (op < out.opcodes.size()) ++out.opcodes[op];
        ++out.instructions;
        if (op == 0U) {
            const auto frame = u16(p + 2U);
            if (frame == 0xFFFFFFFFU || frame == 0x7FFFU) break;
            ++out.waits;
            out.last_frame = static_cast<std::uint16_t>(std::max<std::size_t>(out.last_frame, frame));
            after = static_cast<float>(frame);
            p += 6U;
            continue;
        }
        if (op == 1U) {
            if (started) {
                out.hands_over = true;
                break;
            }
            started = true;
            if (p + 6U <= s.size()) {
                out.play_bank = s[p + 4U];
                out.play_index = s[p + 5U];
            }
        }
        if (op == 2U) {
            out.loops = true;
            break;
        }
        if (op == 3U && p + 6U <= s.size()) {
            out.signals.push_back({
                after,
                {s[p + 1U], s[p + 2U], s[p + 3U], s[p + 4U], s[p + 5U]},
            });
            const auto state = static_cast<std::uint8_t>(s[p + 2U] & 0x3FU);
            if (state != 0U) out.states.push_back({after, state});
        }
        const auto length = opcode_length(op);
        if (length == 0U) break;
        p += length;
    }
    return out;
}

std::vector<WeaponStateKey> MotionScriptFile::weapon_states(std::size_t bank,
                                                           std::size_t action) const {
    auto summary = summarize(bank, action);
    return summary ? std::move(summary->states) : std::vector<WeaponStateKey>{};
}

std::vector<ScriptSignalKey> MotionScriptFile::signals(std::size_t bank,
                                                       std::size_t action) const {
    auto summary = summarize(bank, action);
    return summary ? std::move(summary->signals) : std::vector<ScriptSignalKey>{};
}

std::vector<MotionResource> MotionScriptFile::resources(std::size_t bank,
                                                        std::size_t action) const {
    std::vector<MotionResource> out;
    if (resources_ == 0U) return out;
    const auto& s = bytes_;
    const auto u16 = [&s](std::size_t o) -> std::size_t {
        return o + 2U <= s.size()
            ? static_cast<std::size_t>(s[o]) | (static_cast<std::size_t>(s[o + 1U]) << 8U)
            : 0xFFFFFFFFU;
    };
    // 0x14005A360: r10 = B + u16[B + 2 bank]; record = r10 + u16[r10 + 2 action].
    const auto bank_entry = u16(resources_ + bank * 2U);
    if (bank_entry == 0xFFFFFFFFU || bank_entry == 0xFFFFU) return out;
    // Bank lists end at 0xFFFF; an action past the end has no record.
    for (std::size_t b = 0U; b <= bank; ++b) {
        const auto e = u16(resources_ + b * 2U);
        if (e == 0xFFFFFFFFU || e == 0xFFFFU) return out;
    }
    const std::size_t sub = resources_ + bank_entry;
    for (std::size_t a = 0U; a <= action; ++a) {
        const auto e = u16(sub + a * 2U);
        if (e == 0xFFFFFFFFU || e == 0xFFFFU) return out;
    }
    const auto entry = u16(sub + action * 2U);
    const std::size_t record = sub + entry;
    if (record >= s.size()) return out;
    const std::size_t count = s[record];
    for (std::size_t k = 0U; k < count && k < 16U; ++k) {
        const std::size_t o = record + 2U + k * 6U;
        if (o + 6U > s.size()) break;
        out.push_back({s[o], s[o + 1U], s[o + 2U], s[o + 3U],
                       static_cast<std::uint16_t>(s[o + 4U] | (s[o + 5U] << 8U))});
    }
    return out;
}

std::vector<ScriptAction> MotionScriptFile::actions_for(std::uint16_t id) const {
    std::vector<ScriptAction> out;
    for (std::size_t bank = 0U; bank < banks_.size(); ++bank) {
        const auto count = script_count(bank);
        for (std::size_t action = 0U; action < count; ++action) {
            for (const auto& r : resources(bank, action)) {
                if (r.id == id) {
                    out.push_back({bank, action});
                    break;
                }
            }
        }
    }
    return out;
}

std::vector<WeaponStateKey> MotionScriptFile::weapon_states_for_motion(std::size_t group,
                                                                       std::size_t slot) const {
    if (resources_ != 0U && group < 656U && slot < 100U) {
        const auto actions = actions_for(static_cast<std::uint16_t>(group * 100U + slot));
        const ScriptAction* pick = nullptr;
        for (const auto& a : actions) {
            if (a.bank != group) continue;
            if (a.action == slot) {
                pick = &a;
                break;
            }
            if (pick == nullptr) pick = &a;
        }
        if (pick != nullptr) return weapon_states(pick->bank, pick->action);
    }
    return weapon_states(group, slot);
}

std::vector<std::uint16_t> MotionScriptFile::resource_ids() const {
    std::vector<std::uint16_t> ids;
    for (std::size_t bank = 0U; bank < banks_.size(); ++bank) {
        const auto count = script_count(bank);
        for (std::size_t action = 0U; action < count; ++action) {
            for (const auto& r : resources(bank, action)) ids.push_back(r.id);
        }
    }
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return ids;
}

namespace {

[[nodiscard]] std::string lower_stem(std::string_view name) {
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string_view::npos) name.remove_prefix(slash + 1U);
    std::string out;
    for (const char c : name) {
        if (c == '.') break;
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

struct ExeGroups final {
    std::string_view stem;
    std::array<int, 7> slots;  // group -> top-level PAC slot; -1 = not closed
};

// Motion PAC arrays read from class init/runtime evidence. A single physical
// pack may serve more than one script resource group (CEm034 slot6 serves
// groups 4 and 6), so EXE-confirmed bindings are intentionally reusable.
constexpr std::array<ExeGroups, 3> kExeGroups{{
    {"em028", {2, 3, -1, -1, -1, -1, -1}},
    {"em000", {35, -1, -1, -1, -1, -1, -1}},
    {"em034", {-1, 3, 4, 5, 6, -1, 6}},
}};

}  // namespace

std::vector<MotionGroupBinding> bind_motion_groups(const MotionScriptFile& script,
                                                   std::span<const MotionPack> packs,
                                                   std::string_view archive_name) {
    std::vector<MotionGroupBinding> out;
    for (const auto id : script.resource_ids()) {
        const std::uint16_t group = id / 100U;
        if (out.empty() || out.back().group != group) out.push_back({group, {}, std::nullopt, false});
        out.back().slots.push_back(id % 100U);
    }
    const auto stem = lower_stem(archive_name);
    const ExeGroups* exe = nullptr;
    for (const auto& e : kExeGroups) {
        if (e.stem == stem) exe = &e;
    }
    std::vector<bool> used(packs.size(), false);
    const auto covers = [](const MotionPack& pack, const std::vector<std::uint32_t>& need) {
        for (const auto s : need) {
            if (std::find(pack.slots.begin(), pack.slots.end(), s) == pack.slots.end()) return false;
        }
        return true;
    };
    for (auto& binding : out) {
        if (exe != nullptr && binding.group < exe->slots.size() && exe->slots[binding.group] >= 0) {
            const auto slot = static_cast<std::uint32_t>(exe->slots[binding.group]);
            for (std::size_t p = 0U; p < packs.size(); ++p) {
                if (packs[p].archive_slot == slot) {
                    binding.archive_slot = slot;
                    binding.exe_confirmed = true;
                    used[p] = true;
                }
            }
        }
    }
    // A pack serves one group; a group no unused pack covers stays unbound
    // (its PAC comes from elsewhere, e.g. a shared archive).
    for (auto& binding : out) {
        if (binding.archive_slot) continue;
        for (std::size_t p = 0U; p < packs.size(); ++p) {
            if (used[p] || !covers(packs[p], binding.slots)) continue;
            binding.archive_slot = packs[p].archive_slot;
            used[p] = true;
            break;
        }
    }
    return out;
}

std::uint8_t weapon_state_at(const std::vector<WeaponStateKey>& keys, float frame) noexcept {
    std::uint8_t state = 0U;
    for (const auto& key : keys) {
        if (frame > key.after_frame) state = key.state;
    }
    return state;
}

std::optional<std::size_t> player_motion_bank(std::string_view name) noexcept {
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string_view::npos) name.remove_prefix(slash + 1U);
    constexpr std::string_view prefix = "pl000_00_";
    if (name.size() <= prefix.size() + 4U) return std::nullopt;
    for (std::size_t i = 0U; i < prefix.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(name[i])) != prefix[i]) return std::nullopt;
    }
    std::size_t value = 0U;
    std::size_t i = prefix.size();
    for (; i < name.size() && std::isdigit(static_cast<unsigned char>(name[i])); ++i) {
        value = value * 10U + static_cast<std::size_t>(name[i] - '0');
    }
    if (i == prefix.size() || name.substr(i).size() != 4U) return std::nullopt;
    const auto ext = name.substr(i);
    if (std::tolower(static_cast<unsigned char>(ext[1])) != 'p' ||
        std::tolower(static_cast<unsigned char>(ext[2])) != 'a' ||
        std::tolower(static_cast<unsigned char>(ext[3])) != 'c' || ext[0] != '.') {
        return std::nullopt;
    }
    return value;
}

}  // namespace dmc::rengine::profiles::dmc3::motion
