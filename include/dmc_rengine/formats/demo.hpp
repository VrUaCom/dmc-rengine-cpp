#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// Cutscene archives (demo\...\mNN_xNN.pac, CSceneDemo), read from their
// bytes. Reversed from the retail cutscene archives:
//
// - The archive is a PAC. Slot 0 is the script: "# Load" and "# Set" lists
//   of `Kind [track] file.txt` lines. Slot 1 names the members of slot 2 (a
//   PAC of clip texts, one per Set line that has data). Then pairs follow: a
//   name list (CRLF, ";" comments a line out, "# End" closes it) and the
//   container (PAC or PNST) holding the listed files in order: the models and
//   textures (.scm .mod .efm .ptx .shw .clt), the motions (.mot), the hide
//   animations (.hid), the cameras (.cam) and the effect / sound ids.
// - A clip text is "# Clip" blocks of `Key<TAB>values` up to "# End". Every
//   clip has SetFrame (where it starts on the cutscene timeline), SkipFrame,
//   CutFrame, ClipScale (its time scale) and Life (its length at scale 1); it
//   covers SetFrame .. SetFrame + Life * ClipScale and plays its own time
//   (frame - SetFrame) / ClipScale. Frames are game frames, 60 a second.
// - A .cam ("CAM\0" at +4, header 0x60): +0x08 the mask of animated
//   channels, +0x10 the frame range (start, end, start, end), +0x20 fourteen
//   base values (group translate xyz, group rotate xyz, eye xyz, target xyz,
//   field of view in degrees, roll in degrees), +0x60 the track count and the
//   tracks of the animated channels in channel order: u16 byte size, u16 key
//   count, u8 kind; kind 1 keys are (u32 frame | 0x8000, value, in slope, out
//   slope) cubic Hermite, kind 0 keys (u32 frame, value) linear. Key frames
//   are the range's frames; with a negative range start they count from it.
namespace dmc::rengine::formats::demo {

// Evidence and the full archive layout: docs/research/dmc3-cutscenes-2026-10-24.md.
// First written for the DMC Native Reader (its demo module, v78-v92).
struct Vec3 final {
    float x{};
    float y{};
    float z{};
};

// ---- .cam ----------------------------------------------------------------

enum class CamChannel : std::uint8_t {
    GroupX, GroupY, GroupZ, GroupRotX, GroupRotY, GroupRotZ,
    EyeX, EyeY, EyeZ, TargetX, TargetY, TargetZ, Fov, Roll,
};
inline constexpr std::size_t kCamChannels = 14;

struct CamKey final {
    float frame{};
    float value{};
    float in_slope{};
    float out_slope{};
};

struct CamTrack final {
    CamChannel channel{};
    bool hermite{};
    std::vector<CamKey> keys;
};

struct CameraAnim final {
    float start{};
    float end{};
    std::array<float, kCamChannels> base{};
    std::uint32_t mask{};
    std::vector<CamTrack> tracks;
};

struct CameraPose final {
    Vec3 eye{};
    Vec3 target{};
    float fov_degrees{45.0F};
    float roll_degrees{};
};

[[nodiscard]] bool looks_like_cam(std::span<const std::uint8_t> bytes) noexcept;
[[nodiscard]] std::optional<CameraAnim> parse_cam(std::span<const std::uint8_t> bytes);
// The camera at a frame of its own range (held before the first and after
// the last key of each channel).
[[nodiscard]] CameraPose evaluate(const CameraAnim& anim, float frame) noexcept;

// ---- .hid ----------------------------------------------------------------
//
// Hide animation of a model's objects: u32 header size, "HID\0", u32, the
// frame range (start, end, start, end), u8 object count N and N bytes (1:
// the object has a track), padded to the header size; then u32 track count
// and the tracks of the flagged objects in object order: u16 byte size, u16
// key count, u32 kind (7), f32 state (-1 shown, 0 hidden), f32 1.0, then 4
// bytes a key (big-endian u16 frame from the range, u16 flag). DispOn.hid
// shows and DispOff.hid hides objects 0..99.

// A track's keys hold a value quantized between the track's two floats
// (+8 low, +12 high: value = low + (high - low) * u16 / 65535; -1 / 0 / +1
// in every file seen); the object shows while it is below zero (DispOn:
// -1, DispOff: 0; m03_b01's Cerberus body turns +1 at 1249, as it
// shatters). A key holds until the next (st501_3_bg's repeated keys).
struct HideAnim final {
    float start{};
    float end{};
    struct Key final {
        float frame{};  // in the range (start .. end)
        bool shown{true};
    };
    struct Object final {
        std::uint32_t object{};
        bool shown{true};        // at the first key
        std::vector<Key> keys;   // in frame order
        // The state at a frame of the range (the first key's before it).
        [[nodiscard]] bool shown_at(float frame) const noexcept;
    };
    std::vector<Object> objects;
};

[[nodiscard]] std::optional<HideAnim> parse_hid(std::span<const std::uint8_t> bytes);

// ---- clips ---------------------------------------------------------------

struct Clip final {
    float set_frame{};
    float skip_frame{};
    float cut_frame{};
    float clip_scale{1.0F};
    float life{};
    // Every other key in file order with its values (Camera, Model, Motion,
    // Hide, HideKind, Pos, Rot, CnsPos, Texture, Shadow, No, ...).
    std::vector<std::pair<std::string, std::vector<std::string>>> fields;

    [[nodiscard]] float end_frame() const noexcept { return set_frame + life * clip_scale; }
    [[nodiscard]] bool covers(float frame) const noexcept {
        return frame >= set_frame && frame < end_frame();
    }
    // The clip's own time at a timeline frame.
    [[nodiscard]] float local_frame(float frame) const noexcept {
        return clip_scale > 0.0F ? (frame - set_frame) / clip_scale : 0.0F;
    }
    // First value of a key ("" when absent).
    [[nodiscard]] std::string value(std::string_view key) const;
};

[[nodiscard]] bool looks_like_clips(std::string_view text) noexcept;

// The archive's own texts, met as slots: the script of slot 0 (";"
// comments, "# Load" names what to load, "# Set" the clip texts of each
// kind and track) and a name list ("m01_s00_PadVibeSet.txt" lines up to
// "# End", the files of the container after it, in slot order).
[[nodiscard]] bool looks_like_script(std::string_view text) noexcept;
[[nodiscard]] bool looks_like_name_list(std::string_view text) noexcept;
[[nodiscard]] std::vector<std::string> list_names(std::string_view text);
// The effect bank's manifest: "G 105", "P 228", ... (a kind letter and an
// effect id a line) up to "# End".
[[nodiscard]] bool looks_like_effect_list(std::string_view text) noexcept;
[[nodiscard]] std::vector<std::string> effect_ids(std::string_view text);
[[nodiscard]] std::vector<Clip> parse_clips(std::string_view text);

// ---- the archive ----------------------------------------------------------

// One Set line of the script: what it drives and which track (-1 none).
struct ClipList final {
    std::string kind;   // WorkRate, Camera, Model, Motion, Hide, Effect, EffectI, Light, ...
    int track{-1};
    std::string file;   // m01_s00_Model00Set.txt
    std::vector<Clip> clips;
};

// A file of the archive: its name and where its bytes are.
struct Member final {
    std::string name;
    std::size_t offset{};
    std::size_t size{};
    std::size_t group{};  // which name list / container pair it came from
};

struct Track final {
    int index{};
    const ClipList* model{};
    const ClipList* motion{};
    const ClipList* hide{};
};

struct Document final {
    std::string name;  // m01_s00, from the clip file names
    std::vector<ClipList> lists;
    std::vector<Member> members;
    // Lines of the loaded name lists that were commented out (";st000.scm").
    std::vector<std::string> skipped;
    // Every .cam of the archive, decoded once.
    std::vector<std::pair<std::string, CameraAnim>> cameras;
    // Every .hid, decoded once.
    std::vector<std::pair<std::string, HideAnim>> hides;
    // Where the cutscene's picture ends (its last Camera, Light or screen
    // effect clip), and where its last clip of any kind does.
    float end_frame{};
    float timeline_end_frame{};
    // The effect bank (the container whose files are named by effect id),
    // inside the archive; size 0 when there is none.
    std::size_t effect_bank_offset{};
    std::size_t effect_bank_size{};
    // Its name list: the bank's manifest ("G 48", ..., "M 62": an M record
    // takes two slots, the model and its companion).
    std::size_t effect_manifest_offset{};
    std::size_t effect_manifest_size{};

    [[nodiscard]] const Member* member(std::string_view file) const noexcept;
    [[nodiscard]] const CameraAnim* camera(std::string_view file) const noexcept;
    [[nodiscard]] const HideAnim* hide(std::string_view file) const noexcept;
    [[nodiscard]] const ClipList* list(std::string_view kind, int track = -1) const noexcept;
    // Tracks with any Model, Motion or Hide clip, by index.
    [[nodiscard]] std::vector<Track> tracks() const;
};

// The script of slot 0 opens with a ";" comment and holds "# Load" and
// "# Set"; slot 1 names the clip texts of slot 2.
[[nodiscard]] bool looks_like_demo(std::span<const std::uint8_t> bytes) noexcept;
[[nodiscard]] std::optional<Document> parse_demo(std::span<const std::uint8_t> bytes);

// The cutscene camera at a timeline frame: the covering Camera clip that
// started last; between clips the last one that ended holds its final pose.
struct CameraShot final {
    CameraPose pose;
    std::string camera;  // the .cam file
    int clip{-1};
    float local_frame{};
};
[[nodiscard]] std::optional<CameraShot> camera_at(const Document& doc, float frame);

} // namespace dmc::rengine::formats::demo
