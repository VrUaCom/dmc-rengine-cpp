#include "dmc_rengine/formats/demo.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace dmc::rengine::formats::demo {
namespace {

constexpr std::size_t kCamHeader = 0x60U;
constexpr float kPi = 3.14159265358979323846F;

[[nodiscard]] std::uint32_t u32(std::span<const std::uint8_t> b, std::size_t at) noexcept {
    std::uint32_t v = 0U;
    std::memcpy(&v, b.data() + at, 4U);
    return v;
}

[[nodiscard]] std::uint16_t u16(std::span<const std::uint8_t> b, std::size_t at) noexcept {
    std::uint16_t v = 0U;
    std::memcpy(&v, b.data() + at, 2U);
    return v;
}

[[nodiscard]] float f32(std::span<const std::uint8_t> b, std::size_t at) noexcept {
    float v = 0.0F;
    std::memcpy(&v, b.data() + at, 4U);
    return v;
}

[[nodiscard]] bool container(std::span<const std::uint8_t> b) noexcept {
    return b.size() >= 8U && (std::memcmp(b.data(), "PAC\0", 4U) == 0 || std::memcmp(b.data(), "PNST", 4U) == 0);
}

// Slots of a PAC / PNST: (offset, size) inside `b`. A slot ends where the
// next slot that starts later begins (or the container ends); a zero offset
// is an empty slot.
[[nodiscard]] std::optional<std::vector<std::pair<std::size_t, std::size_t>>> slots(std::span<const std::uint8_t> b) {
    if (!container(b)) return std::nullopt;
    const std::uint32_t count = u32(b, 4U);
    if (count > 0x10000U || 8U + std::size_t{count} * 4U > b.size()) return std::nullopt;
    std::vector<std::uint32_t> offsets(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        offsets[i] = u32(b, 8U + std::size_t{i} * 4U);
        if (offsets[i] > b.size()) return std::nullopt;
    }
    std::vector<std::pair<std::size_t, std::size_t>> out;
    out.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        if (offsets[i] == 0U) {
            out.emplace_back(0U, 0U);
            continue;
        }
        std::size_t end = b.size();
        for (std::uint32_t j = i + 1U; j < count; ++j) {
            if (offsets[j] > offsets[i]) {
                end = offsets[j];
                break;
            }
        }
        out.emplace_back(offsets[i], end - offsets[i]);
    }
    return out;
}

[[nodiscard]] std::string_view text_of(std::span<const std::uint8_t> b) noexcept {
    std::string_view text{reinterpret_cast<const char*>(b.data()), b.size()};
    const auto nul = text.find('\0');
    return nul == std::string_view::npos ? text : text.substr(0, nul);
}

[[nodiscard]] std::string_view trim(std::string_view s) noexcept {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}

template <typename F>
void each_line(std::string_view text, F&& f) {
    while (!text.empty()) {
        const auto nl = text.find('\n');
        f(trim(text.substr(0, nl)));
        if (nl == std::string_view::npos) break;
        text.remove_prefix(nl + 1U);
    }
}

[[nodiscard]] std::vector<std::string> words(std::string_view line) {
    std::vector<std::string> out;
    std::size_t i = 0;
    while (i < line.size()) {
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
        const std::size_t start = i;
        while (i < line.size() && line[i] != ' ' && line[i] != '\t') ++i;
        if (i > start) out.emplace_back(line.substr(start, i - start));
    }
    return out;
}

[[nodiscard]] float to_float(const std::string& s) noexcept {
    char* end = nullptr;
    const float v = std::strtof(s.c_str(), &end);
    return end != s.c_str() && std::isfinite(v) ? v : 0.0F;
}

[[nodiscard]] bool is_track_number(const std::string& s) noexcept {
    return !s.empty() && s.size() <= 3U && std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; });
}

// The names of a name list, in order; commented lines go to `skipped`.
[[nodiscard]] std::vector<std::string> name_list(std::string_view text, std::vector<std::string>* skipped) {
    std::vector<std::string> out;
    each_line(text, [&](std::string_view line) {
        if (line.empty() || line == "# End") return;
        if (line.front() == ';') {
            if (skipped != nullptr) skipped->emplace_back(line.substr(1));
            return;
        }
        out.emplace_back(line);
    });
    return out;
}

[[nodiscard]] float hermite(const CamKey& a, const CamKey& b, float frame) noexcept {
    const float dt = b.frame - a.frame;
    if (dt <= 0.0F) return b.value;
    const float s = (frame - a.frame) / dt;
    const float s2 = s * s, s3 = s2 * s;
    return (2.0F * s3 - 3.0F * s2 + 1.0F) * a.value + (s3 - 2.0F * s2 + s) * dt * a.out_slope +
           (-2.0F * s3 + 3.0F * s2) * b.value + (s3 - s2) * dt * b.in_slope;
}

[[nodiscard]] float sample(const CamTrack& track, float frame) noexcept {
    const auto& keys = track.keys;
    if (keys.empty()) return 0.0F;
    if (frame <= keys.front().frame) return keys.front().value;
    if (frame >= keys.back().frame) return keys.back().value;
    const auto next = std::upper_bound(keys.begin(), keys.end(), frame,
                                       [](float f, const CamKey& k) { return f < k.frame; });
    const auto& b = *next;
    const auto& a = *(next - 1);
    if (track.hermite) return hermite(a, b, frame);
    const float dt = b.frame - a.frame;
    return dt <= 0.0F ? b.value : a.value + (b.value - a.value) * (frame - a.frame) / dt;
}

// Group rotation in degrees, x then y then z (the authoring tool's default
// order; every retail camera has it at zero).
[[nodiscard]] Vec3 rotate(Vec3 p, float rx, float ry, float rz) noexcept {
    const auto turn = [](float& a, float& b, float degrees) {
        const float r = degrees * kPi / 180.0F, c = std::cos(r), s = std::sin(r);
        const float na = a * c - b * s, nb = a * s + b * c;
        a = na;
        b = nb;
    };
    turn(p.y, p.z, rx);
    turn(p.z, p.x, ry);
    turn(p.x, p.y, rz);
    return p;
}

}  // namespace

// ---- .cam ----------------------------------------------------------------

bool looks_like_cam(std::span<const std::uint8_t> bytes) noexcept {
    return bytes.size() >= kCamHeader + 4U && u32(bytes, 0U) == kCamHeader &&
           std::memcmp(bytes.data() + 4U, "CAM\0", 4U) == 0;
}

std::optional<CameraAnim> parse_cam(std::span<const std::uint8_t> bytes) {
    if (!looks_like_cam(bytes)) return std::nullopt;
    CameraAnim anim;
    anim.mask = u32(bytes, 0x08U);
    anim.start = f32(bytes, 0x10U);
    anim.end = f32(bytes, 0x14U);
    for (std::size_t i = 0; i < kCamChannels; ++i) anim.base[i] = f32(bytes, 0x20U + i * 4U);
    const std::uint32_t count = u32(bytes, kCamHeader);
    std::size_t at = kCamHeader + 4U;
    std::size_t channel = 0;
    // Key frames count from the range start when it is negative (they are
    // stored unsigned).
    const float origin = anim.start < 0.0F ? anim.start : 0.0F;
    for (std::uint32_t t = 0; t < count; ++t) {
        while (channel < kCamChannels && ((anim.mask >> channel) & 1U) == 0U) ++channel;
        if (channel >= kCamChannels || at + 8U > bytes.size()) return std::nullopt;
        const std::size_t size = u16(bytes, at);
        const std::size_t keys = u16(bytes, at + 2U);
        const std::uint8_t kind = bytes[at + 4U];
        const std::size_t stride = kind == 1U ? 16U : 8U;
        if (size < 8U + keys * stride || at + size > bytes.size()) return std::nullopt;
        CamTrack track;
        track.channel = static_cast<CamChannel>(channel);
        track.hermite = kind == 1U;
        track.keys.reserve(keys);
        for (std::size_t k = 0; k < keys; ++k) {
            const std::size_t key = at + 8U + k * stride;
            CamKey out;
            out.frame = static_cast<float>(u32(bytes, key) & 0x7FFFU) + origin;
            out.value = f32(bytes, key + 4U);
            if (track.hermite) {
                out.in_slope = f32(bytes, key + 8U);
                out.out_slope = f32(bytes, key + 12U);
            }
            track.keys.push_back(out);
        }
        anim.tracks.push_back(std::move(track));
        at += size;
        ++channel;
    }
    return anim;
}

CameraPose evaluate(const CameraAnim& anim, float frame) noexcept {
    auto values = anim.base;
    for (const auto& track : anim.tracks) {
        if (!track.keys.empty()) values[static_cast<std::size_t>(track.channel)] = sample(track, frame);
    }
    const auto v = [&](CamChannel c) { return values[static_cast<std::size_t>(c)]; };
    const Vec3 group{v(CamChannel::GroupX), v(CamChannel::GroupY), v(CamChannel::GroupZ)};
    const auto place = [&](Vec3 p) {
        p = rotate(p, v(CamChannel::GroupRotX), v(CamChannel::GroupRotY), v(CamChannel::GroupRotZ));
        return Vec3{p.x + group.x, p.y + group.y, p.z + group.z};
    };
    CameraPose pose;
    pose.eye = place({v(CamChannel::EyeX), v(CamChannel::EyeY), v(CamChannel::EyeZ)});
    pose.target = place({v(CamChannel::TargetX), v(CamChannel::TargetY), v(CamChannel::TargetZ)});
    const float fov = v(CamChannel::Fov);
    pose.fov_degrees = std::isfinite(fov) && fov > 1.0F && fov < 170.0F ? fov : 45.0F;
    pose.roll_degrees = std::isfinite(v(CamChannel::Roll)) ? v(CamChannel::Roll) : 0.0F;
    return pose;
}

// ---- .hid ----------------------------------------------------------------

bool HideAnim::Object::shown_at(float frame) const noexcept {
    bool state = shown;
    for (const auto& key : keys) {
        if (key.frame > frame) break;
        state = key.shown;
    }
    return state;
}

std::optional<HideAnim> parse_hid(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < 0x20U || std::memcmp(bytes.data() + 4U, "HID\0", 4U) != 0) return std::nullopt;
    const std::size_t header = u32(bytes, 0U);
    const std::size_t count = bytes[0x1CU];
    if (header < 0x1DU + count || header + 4U > bytes.size()) return std::nullopt;
    HideAnim anim;
    anim.start = f32(bytes, 0x0CU);
    anim.end = f32(bytes, 0x10U);
    std::vector<std::uint32_t> flagged;
    for (std::size_t i = 0; i < count; ++i) {
        if (bytes[0x1DU + i] != 0U) flagged.push_back(static_cast<std::uint32_t>(i));
    }
    const std::uint32_t tracks = u32(bytes, header);
    std::size_t at = header + 4U;
    for (std::uint32_t t = 0; t < tracks && t < flagged.size(); ++t) {
        if (at + 16U > bytes.size()) return std::nullopt;
        const std::size_t size = u16(bytes, at);
        if (size < 16U || at + size > bytes.size()) return std::nullopt;
        const std::size_t keys = u16(bytes, at + 2U);
        const float low = f32(bytes, at + 8U);
        const float high = f32(bytes, at + 12U);
        HideAnim::Object object{.object = flagged[t], .shown = true, .keys = {}};
        for (std::size_t k = 0; k < keys && 16U + k * 4U + 4U <= size; ++k) {
            const std::size_t o = at + 16U + k * 4U;
            const auto frame = static_cast<float>(static_cast<unsigned>(bytes[o]) << 8U | bytes[o + 1U]);
            const auto quantized = static_cast<unsigned>(bytes[o + 2U]) << 8U | bytes[o + 3U];
            const float value = low + (high - low) * static_cast<float>(quantized) / 65535.0F;
            object.keys.push_back({frame, std::isfinite(value) && value < 0.0F});
        }
        std::stable_sort(object.keys.begin(), object.keys.end(),
                         [](const HideAnim::Key& a, const HideAnim::Key& b) { return a.frame < b.frame; });
        // A track without keys keeps its low float as its state.
        object.shown = object.keys.empty() ? std::isfinite(low) && low < 0.0F : object.keys.front().shown;
        anim.objects.push_back(std::move(object));
        at += size;
    }
    return anim;
}

// ---- clips ---------------------------------------------------------------

std::string Clip::value(std::string_view key) const {
    for (const auto& [name, values] : fields) {
        if (name == key) return values.empty() ? std::string{} : values.front();
    }
    return {};
}

bool looks_like_script(std::string_view text) noexcept {
    text = text.substr(0, text.find('\0'));
    return !text.empty() && text.front() == ';' && text.find("# Load") != std::string_view::npos &&
           text.find("# Set") != std::string_view::npos;
}

namespace {

// A file name of a name list: printable, no blanks, with an extension.
[[nodiscard]] bool is_file_name(std::string_view line) noexcept {
    return line.size() <= 96U && line.find('.') != std::string_view::npos &&
           std::all_of(line.begin(), line.end(), [](char c) { return c > ' ' && c < 127; });
}

// An effect id of the bank's manifest: a capital letter, blanks, digits ("G 105").
[[nodiscard]] bool is_effect_id(std::string_view line) noexcept {
    std::size_t i = 1U;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    return line.size() >= 3U && line[0] >= 'A' && line[0] <= 'Z' && i > 1U && i < line.size() &&
           std::all_of(line.begin() + static_cast<std::ptrdiff_t>(i), line.end(),
                       [](char c) { return c >= '0' && c <= '9'; });
}

// A list of the archive (name_list's lines up to "# End", ";" lines
// commented out) whose every entry passes `entry`.
template <typename Entry>
[[nodiscard]] bool looks_like_list(std::string_view text, Entry&& entry) noexcept {
    text = text.substr(0, text.find('\0'));
    if (text.empty() || text.size() > 64U * 1024U) return false;
    bool ended = false;
    std::size_t entries = 0U;
    bool fine = true;
    each_line(text, [&](std::string_view line) {
        if (!fine || ended || line.empty() || line.front() == ';') return;
        if (line == "# End") {
            ended = true;
            return;
        }
        if (entry(line)) ++entries;
        else fine = false;
    });
    return fine && ended && entries > 0U;
}

}  // namespace

bool looks_like_name_list(std::string_view text) noexcept { return looks_like_list(text, is_file_name); }

bool looks_like_effect_list(std::string_view text) noexcept { return looks_like_list(text, is_effect_id); }

std::vector<std::string> effect_ids(std::string_view text) {
    if (!looks_like_effect_list(text)) return {};
    return name_list(text.substr(0, text.find('\0')), nullptr);
}

std::vector<std::string> list_names(std::string_view text) {
    if (!looks_like_name_list(text)) return {};
    return name_list(text.substr(0, text.find('\0')), nullptr);
}

bool looks_like_clips(std::string_view text) noexcept {
    const auto clip = text.find("# Clip");
    return clip != std::string_view::npos && text.find("SetFrame", clip) != std::string_view::npos &&
           text.find("ClipScale") != std::string_view::npos;
}

std::vector<Clip> parse_clips(std::string_view text) {
    std::vector<Clip> out;
    bool open = false;
    each_line(text, [&](std::string_view line) {
        if (line.empty() || line.front() == ';') return;
        if (line == "# Clip") {
            out.emplace_back();
            open = true;
            return;
        }
        if (line == "# End") {
            open = false;
            return;
        }
        if (!open) return;
        auto parts = words(line);
        if (parts.empty()) return;
        auto key = std::move(parts.front());
        parts.erase(parts.begin());
        auto& clip = out.back();
        const float first = parts.empty() ? 0.0F : to_float(parts.front());
        if (key == "SetFrame") clip.set_frame = first;
        else if (key == "SkipFrame") clip.skip_frame = first;
        else if (key == "CutFrame") clip.cut_frame = first;
        else if (key == "ClipScale") clip.clip_scale = first > 0.0F ? first : 1.0F;
        else if (key == "Life") clip.life = std::max(0.0F, first);
        else clip.fields.emplace_back(std::move(key), std::move(parts));
    });
    return out;
}

// ---- the archive ----------------------------------------------------------

const Member* Document::member(std::string_view file) const noexcept {
    for (const auto& m : members) {
        if (m.name == file) return &m;
    }
    return nullptr;
}

const CameraAnim* Document::camera(std::string_view file) const noexcept {
    for (const auto& [name, anim] : cameras) {
        if (name == file) return &anim;
    }
    return nullptr;
}

const HideAnim* Document::hide(std::string_view file) const noexcept {
    for (const auto& [name, anim] : hides) {
        if (name == file) return &anim;
    }
    return nullptr;
}

const ClipList* Document::list(std::string_view kind, int track) const noexcept {
    for (const auto& l : lists) {
        if (l.kind == kind && l.track == track) return &l;
    }
    return nullptr;
}

std::vector<Track> Document::tracks() const {
    std::vector<Track> out;
    for (const auto& l : lists) {
        if (l.track < 0 || l.clips.empty() || (l.kind != "Model" && l.kind != "Motion" && l.kind != "Hide")) continue;
        auto it = std::find_if(out.begin(), out.end(), [&](const Track& t) { return t.index == l.track; });
        if (it == out.end()) {
            out.push_back(Track{l.track});
            it = out.end() - 1;
        }
        if (l.kind == "Model") it->model = &l;
        else if (l.kind == "Motion") it->motion = &l;
        else it->hide = &l;
    }
    std::sort(out.begin(), out.end(), [](const Track& a, const Track& b) { return a.index < b.index; });
    return out;
}

bool looks_like_demo(std::span<const std::uint8_t> bytes) noexcept {
    try {
        const auto top = slots(bytes);
        if (!top || top->size() < 3U || (*top)[0].second == 0U) return false;
        const auto script = text_of(bytes.subspan((*top)[0].first, (*top)[0].second));
        return looks_like_script(script) && script.find("Set.txt") != std::string_view::npos;
    } catch (...) {
        return false;
    }
}

std::optional<Document> parse_demo(std::span<const std::uint8_t> bytes) {
    if (!looks_like_demo(bytes)) return std::nullopt;
    const auto top = slots(bytes);
    const auto slice = [&](std::size_t i) { return bytes.subspan((*top)[i].first, (*top)[i].second); };
    Document doc;

    // The Set section of the script: one list per line.
    bool in_set = false;
    each_line(text_of(slice(0)), [&](std::string_view line) {
        if (line.empty() || line.front() == ';') return;
        if (line.front() == '#') {
            in_set = line == "# Set";
            return;
        }
        if (!in_set) return;
        auto parts = words(line);
        if (parts.size() < 2U) return;
        ClipList list;
        list.kind = parts.front();
        if (parts.size() >= 3U && is_track_number(parts[1])) list.track = std::atoi(parts[1].c_str());
        list.file = parts.back();
        doc.lists.push_back(std::move(list));
    });
    for (const auto& list : doc.lists) {
        const auto cut = list.file.find('_', list.file.find('_') + 1U);
        if (cut != std::string::npos) {
            doc.name = list.file.substr(0, cut);
            break;
        }
    }

    // Slot 1 names the clip texts of slot 2.
    const auto texts = name_list(text_of(slice(1)), nullptr);
    if (const auto inner = slots(slice(2))) {
        const auto base = (*top)[2].first;
        for (std::size_t i = 0; i < texts.size() && i < inner->size(); ++i) {
            const auto [offset, size] = (*inner)[i];
            const auto body = text_of(bytes.subspan(base + offset, size));
            for (auto& list : doc.lists) {
                if (list.file == texts[i]) list.clips = parse_clips(body);
            }
        }
    }

    // Name list / container pairs.
    for (std::size_t i = 3; i + 1U < top->size(); i += 2U) {
        const auto names = name_list(text_of(slice(i)), &doc.skipped);
        const auto box = slice(i + 1U);
        const auto inner = slots(box);
        if (!inner) continue;
        const auto base = (*top)[i + 1U].first;
        // The effect bank: its files are named by effect id ("G 48", "V 287").
        const bool effects = !names.empty() && std::all_of(names.begin(), names.end(), [](const std::string& n) {
            return is_effect_id(n);
        });
        if (effects) {
            doc.effect_bank_offset = base;
            doc.effect_bank_size = box.size();
            doc.effect_manifest_offset = (*top)[i].first;
            doc.effect_manifest_size = (*top)[i].second;
        }
        for (std::size_t j = 0; j < names.size() && j < inner->size(); ++j) {
            const auto [offset, size] = (*inner)[j];
            if (size == 0U) continue;
            doc.members.push_back(Member{names[j], base + offset, size, i});
        }
    }
    for (const auto& member : doc.members) {
        const auto body = bytes.subspan(member.offset, member.size);
        if (auto anim = parse_cam(body)) {
            doc.cameras.emplace_back(member.name, std::move(*anim));
        } else if (auto hide = parse_hid(body)) {
            doc.hides.emplace_back(member.name, std::move(*hide));
        }
    }

    // The cutscene ends with its picture: the last Camera or Fade clip.
    // Clips past it (pad vibration, subtitles, an effect) belong to the
    // movie that follows (m01_s00: 5949 of 12455 frames). Lights and screen
    // effects are the set's surroundings and run on regardless (m01_v00
    // lights one for 48900 frames past its last shot at 5729), so neither
    // end counts them.
    const auto last_end = [&](bool picture_only) {
        float end = 0.0F;
        for (const auto& list : doc.lists) {
            if (list.kind == "Light" || list.kind == "ScrEfc") continue;
            if (picture_only && list.kind != "Camera" && list.kind != "Fade") continue;
            for (const auto& clip : list.clips) {
                if (clip.life >= 100000.0F) continue;  // models stand for 1000000 frames
                end = std::max(end, clip.end_frame());
            }
        }
        return end;
    };
    doc.timeline_end_frame = last_end(false);
    doc.end_frame = last_end(true);
    if (doc.end_frame <= 0.0F) doc.end_frame = doc.timeline_end_frame;
    return doc;
}

std::optional<CameraShot> camera_at(const Document& doc, float frame) {
    const auto* list = doc.list("Camera");
    if (list == nullptr || list->clips.empty()) return std::nullopt;
    int chosen = -1;
    for (std::size_t i = 0; i < list->clips.size(); ++i) {
        const auto& clip = list->clips[i];
        if (!clip.covers(frame)) continue;
        if (chosen < 0 || clip.set_frame >= list->clips[static_cast<std::size_t>(chosen)].set_frame) {
            chosen = static_cast<int>(i);
        }
    }
    bool held = false;
    if (chosen < 0) {
        // Between clips: the last one to end before the frame; before the
        // first clip: the first.
        float best = -1.0e30F;
        for (std::size_t i = 0; i < list->clips.size(); ++i) {
            const auto& clip = list->clips[i];
            if (clip.end_frame() <= frame && clip.end_frame() > best) {
                best = clip.end_frame();
                chosen = static_cast<int>(i);
            }
        }
        if (chosen < 0) {
            float first = 1.0e30F;
            for (std::size_t i = 0; i < list->clips.size(); ++i) {
                if (list->clips[i].set_frame < first) {
                    first = list->clips[i].set_frame;
                    chosen = static_cast<int>(i);
                }
            }
        }
        held = true;
    }
    const auto& clip = list->clips[static_cast<std::size_t>(chosen)];
    CameraShot shot;
    shot.clip = chosen;
    shot.camera = clip.value("Camera");
    const auto* anim = doc.camera(shot.camera);
    if (anim == nullptr) return std::nullopt;
    const float local = held ? (frame < clip.set_frame ? 0.0F : clip.life) : clip.local_frame(frame);
    shot.local_frame = anim->start + std::clamp(local, 0.0F, clip.life);
    shot.pose = evaluate(*anim, shot.local_frame);
    return shot;
}

} // namespace dmc::rengine::formats::demo
