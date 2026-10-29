# Cutscenes (demo PAC)

> Ported 2026-10-29 from the DMC Native Reader (`NR-Luna-v73`, `docs/CUTSCENES.md`, versions v78–v120). The format readers are in Rengine as `formats/demo.hpp` (archive, script, name lists, clip texts, `.cam`, `.hid`, the camera timeline; `tests/demo_tests.cpp`) and `formats/messages.hpp` (the localization message tables of the subtitles; `tests/messages_tests.cpp`). Playback (scene assembly, motion, cloth, sound, the movie part, screen effects) stays in the Reader; paths starting with `app/` are the Reader's.

The Reader opens a cutscene archive (`demo\...\mNN_xNN.pac`, `CSceneDemo`)
as its scene: the stage, the characters and their weapons play their
motions over the timeline, seen through the cutscene camera. Everything is
read from the bytes; only ids, offsets and layouts are recorded here.

## Archive layout (reversed from the retail cutscenes)

A PAC:

| slot | holds |
| --- | --- |
| 0 | the script: `# Load` and `# Set` lists of `Kind [track] file.txt` lines |
| 1 | the names of the clip texts in slot 2 (CRLF lines, `# End`) |
| 2 | a PAC of clip texts, one per `Set` line that has data |
| 3, 5, 7, ... | a name list: the files of the next slot, in order; a line starting with `;` is commented out and has no member |
| 4, 6, 8, ... | the container (PAC or PNST): models and textures (`.scm .mod .efm .ptx .shw .clt`), motions (`.mot`), hide animations (`.hid`), cameras (`.cam`), effect / sound ids (`G 48`, `E 257`, ...) |

`*_movie.pac` carries only the PadVibe and Message clips of a cutscene that
plays as a movie.

m07_c00_movie.pac (15 120 bytes) is one:

- Slot 0: the full script, the same as a real-time cutscene's (WorkRate,
  Camera, Model / Motion / Hide 00 to 31, Effect, EffectI 00 and 01, Light,
  PadVibe, Message, ScrEfc, Fade, Program, Sound). It is Shift-JIS; its
  comments read "# = the start of a section", "; = a comment", "load" and
  "clip".
- Slot 1: the name list of slot 2, `m07_c00_PadVibeSet.txt` and
  `m07_c00_MessageSet.txt`.
- Slot 2: a PAC of those two texts. The script names 108 Set files (and 108
  Load files): WorkRate, Camera, Model / Motion / Hide 00 to 31, Effect,
  EffectI 00 and 01, Light, PadVibe, Message, ScrEfc, Fade, Program,
  Sound. Only these two exist: the movie brings the picture and the sound
  itself. The script is the real-time m07_c00's: a real-time archive holds
  every Set file its script names (m03_s00: 105 of 105, m01_s00: 116 of
  116), a movie track only these two (m01_s00_movie: 2 of 116).
- PadVibe: 29 clips from frame 330 to 12122. Per clip: `LMotorSt` and
  `LMotorEd` (the large motor's strength at the start and the end, 0-255),
  `SMotorSt` and `SMotorEd` (the small motor, off or on), `Life` (frames).
- Message: 13 clips, `No` 1800 to 1813 (1810 is not used), frames 1920 to
  9900. The text is in the language's `*_msg_<lang>.txt`.
- The clips are not in frame order (5691 comes before 5675, 7311 before
  7012, and 9112, 1108 and 1466 come last). The Reader takes every clip by
  its own frame, so the order does not matter.
- The timeline is 12142 frames (12122 + 20), 3 min 22 s at 60 frames a
  second: the length of m07_c00's movie.

m07_b00.pac (7 527 088 bytes, 13 slots) is a real-time cutscene of the same
mission, not m07_c00's:

- Its script names 75 Set files (and 75 Load files), all with the m07_b00_
  prefix, and slot 2 holds all 75. The kinds are m07_c00's, but the tracks
  go from 00 to 20 (m07_c00's script has 00 to 31): 33 fewer files.
- Tracks: 00 the set `st500_3.scm` (a wall hidden by `st500_3_wall.hid`);
  01-06 `pl100_00`..`pl100_05`, 07 `wp100`, 08 `wp101`; 09-14
  `pl101_00`..`pl101_05`, 15 `wp103`, 16 `wp105`; 17 `ss500_3_00.scm`;
  18-20 empty.
- Clips: Camera 12 (12 `.cam` shots, `cm_c001`..`cm_c013`), Light 47,
  ScrEfc 19, Message 10, PadVibe 2, Fade 1, Sound 1, Effect 0.
- Every file its Set texts name (116: models, textures, motions, cloth,
  hide animations, cameras) is in its name lists under the very same name,
  case included (`PL101_0x.ptx`), and every name in the lists is used.
- It is not m07_c00's scene: its subtitles are 1700-1709 (m07_c00's
  1800-1813) and it lasts 4168 frames, 69 s (m07_c00: 12142). m07_c00's
  models are named only in its own Model Set texts, which its movie track
  does not carry.
- Slots 3-12 are name list and container pairs: the models and textures
  (PNST), 72 motions and cloth (`.mot`, `.clt`), 2 hide animations, the 12
  cameras, and the effect manifest (`G 105`, `G 3`, `V 7`, `P 228`, ...)
  with its records (PNST).

In an archive's list, slot 0 shows as SCRIPT, a name list as NAMES and an
effect manifest as FX-IDS (all .txt; `demo::looks_like_script`,
`demo::looks_like_name_list`, `demo::looks_like_effect_list`), and the PAC or
PNST after a name list shows the names it holds. An empty clip text
(`# End` alone, an unused track's Set file) shows as EMPTY.

Opening such a container names its files with the list
(`ChildResource::member_names`): "#1 · m07_b00_CameraSet.txt · CLIPS",
"#0 · st500_3.scm · SCM", "#0 · cm_c001.cam · DEMO-CAM", "#3 · G 105 · BIN";
a file name is also what the file saves as.

A cutscene opens as its scene. `⋮ → Browse .PAC files…` opens the archive's
files as a gallery (`open_files_view`, companion 29), as a stage's and an
assembled PAC's do.

The cutscene's picture ends with its last Camera or Fade clip (m01_s00:
frame 5949 of 12455). Pad vibration, subtitles and one effect clip run on
to 12455: they belong to the movie that follows the real-time part. Lights
and screen effects are the set's surroundings and run on regardless
(m01_v00 lights one for 48900 frames past its last shot at 5729), so they
count for neither end. The Reader plays the picture from frame 0 to its end
and stops there.

A movie's own track (`m01_v01_movie.pac`: only PadVibe and Message clips,
7100..7105, no scene) opens as the timeline its movie plays on, from 0.

### Effects

The name list of the last pair holds effect ids (`G 48`, `V 287`, `E 257`,
`P 119`, `T 19`, `A 32`, `M 62`) and its container the records: a stage
FXBANK is a PNST of exactly these two (manifest, records; an M record takes
two slots), so the Reader puts them together and reads the cutscene's bank
like a stage's. Effect and EffectI clips (no Life) carry `Id`, `No`, `Pos`,
`Rot` (radians), `Sca`, `WaitTimer`, `IsChain`, `SpFlag`, `SpTrackNo`,
`SpBoneNo`. An effect starts at SetFrame, ages in its own frames (ClipScale
applied, WaitTimer frames of waiting), stops at CutFrame when it has one
and at the end of its own length otherwise. With `SpFlag` set it stands on
joint `SpBoneNo` of the model whose Set line is `SpTrackNo` (the script's
Set lines counted from 0: WorkRate 0, Camera 1, Model 00 2, Motion 00 3,
Hide 00 4, Model 01 5, ...; m01_s00 puts dust on Dante, line 5, and a muzzle
flash on Ebony & Ivory, line 14). The G records in m01_s00's bank are zero
filled (the game's resident effects presumably serve them), so those draw
nothing.

### Sound

Sound clips hold an ADX stream number (`No`) and no file: the streams sit
outside the archive, and the HD release stores them as .ogg (the EXE renames
the requested .adx, 0x140031D80). m01_s00 has no Sound clip. The user gives
the cutscene its .ogg from `⋮ → 🔊 Cutscene sound`: the n-th file plays from
the n-th Sound clip's frame (from frame 0 when there is none). The core
decodes it (stb_vorbis, `third_party/stb_vorbis`), mixes the files and hands
the shell the mix from the position of any timeline frame
(`audio_position`, `audio_read`); the shell only streams it to its audio
output while the timeline plays, restarts it on resume and stops it on
pause. A lone .ogg opens as its own resource (Info, waveform, ▶).

### The movie part

`m01_s00`'s PadVibe, Message and one effect clip run on past its picture
(5950) to 12455: the cutscene ends as a movie. The HD release stores the
game's SFD movies as `.wmv` (the EXE swaps the extension before its media
loader): ASF with Windows Media video (WMV1/2/3, VC-1) and audio (WMA). The
user gives the cutscene its movie from `⋮ → 🎞 Cutscene movie`: it plays from
the picture's end, the timeline grows to the movie's end, the picture is the
movie's frame (letterboxed) and its sound joins the mix (every sound is
resampled to the first one's rate). A lone `.wmv` opens as a timeline of
its own (`Movie <name>` in the motion strip; Info: size, length, codecs, the
first frame). `video.h` decodes it with a minimal FFmpeg
(`third_party/ffmpeg/README.md`) from memory.

### Fades

`Fade` clips (`FadeType I` / `O`, `DistRGBA r g b a`, alpha 128 opaque as
PS2 alphas are): I fades the picture in from the colour over the clip's
life, O out to it, holding it past its clip only when it ends the picture
(m03_s00 fades out over 10 frames at 6295 as a cut through black and plays
on to its last fade at 8067). m01_v00 fades in over its first 60 frames and
out over its last 60; the Reader lays the fade over the rendered picture
(`fade_at`, `apply_fade`).

### Clip texts

`# Clip` blocks of `Key<TAB>values` closed by `# End`. Every clip has
`SetFrame` (where it starts on the timeline), `SkipFrame`, `CutFrame`,
`ClipScale` (its time scale) and `Life` (its length at scale 1). A clip
covers `SetFrame .. SetFrame + Life * ClipScale` and plays its own time
`(frame - SetFrame) / ClipScale` (checked: `cm011.cam` at scale 3.3 ends
exactly where the next camera clip starts). Frames are game frames, 60 a
second.

Kinds: `Camera` (`Camera file.cam`), `Model NN` (`Model`, `Texture`,
`Shadow`, `Cloth`, `Pos`, `Rot`, `CnsPos track joint`), `Motion NN`
(`Motion file.mot`), `Hide NN` (`HideKind A|H|D`, `Hide file.hid`),
`Effect`, `EffectI NN`, `Light`, `PadVibe`, `Message` (`No`), `ScrEfc`,
`Program`, `Sound`, `WorkRate`.

### `.cam`

| at | holds |
| --- | --- |
| +0x00 | u32 0x60 (header size) |
| +0x04 | `CAM\0` |
| +0x08 | u32 mask of animated channels |
| +0x10 | f32 frame range start, end, start, end |
| +0x20 | 14 f32 base values: group translate xyz, group rotate xyz, eye xyz, target xyz, field of view (degrees), roll (degrees) |
| +0x60 | u32 track count, then one track per set mask bit, in channel order |

A track: u16 byte size, u16 key count, u8 kind (the bytes after it are
noise). Kind 1 keys are 16 bytes `(u32 frame | 0x8000, value, in slope,
out slope)`, cubic Hermite with slopes per frame; kind 0 keys are 8 bytes
`(u32 frame, value)`, linear. Key frames are the range's frames; when the
range starts below zero (`cm_c010.cam`, -70) they count from its start.
The eye and target are moved by the group translate (rotation: x, y, z
order, zero in every retail camera). The stage configuration's CAM member
is unrelated (its tag is at +0).

### `.hid`

| at | holds |
| --- | --- |
| +0x00 | u32 header size |
| +0x04 | `HID\0` |
| +0x0C | f32 frame range start, end, start, end |
| +0x1C | u8 object count N, then N bytes: 1 when the object has a track |
| header | u32 track count, then one track per flagged object, in object order |

A track: u16 byte size, u16 key count, u32 kind (7), f32 low, f32 high,
then 4 bytes a key: a big-endian u16 frame of the range and a big-endian
u16 value quantized between low and high (value = low + (high - low) *
u16 / 65535: -1, 0 or +1 in every file). The object shows while the value
is below zero (DispOn: low -1, keys 0 -> -1; DispOff: low 0 -> 0), and a key
holds until the next one: m03_b01's Cerberus body (`em125_02_0054.hid`,
-1 then +1 at 1249) disappears as it shatters; m02_c01's `st501_3_c002.hid`
swaps stage pieces at 56, 76, 89 and 275 (range -2 .. 2: 0x3fff is -1,
0xbfff is +1). The keys run on the range from the clip's start.
`DispOn.hid` / `DispOff.hid` hold 100 tracks: objects 0..99 shown /
hidden. Several stage files show one object and hide another (a piece
swapped for its broken version). The object index is the model's object
(SCM: object k hangs from scene node k + 1).

## Playback

`demo_session.h` builds the scene (`open_session` of a `DEMO` archive):

* SCM tracks are stage pieces: they go to the room pass in their own
  coordinates (`stage_room::assemble_room` over the opened pieces, each with
  its own PTX, so every piece's vertices and triangles are known).
* Stage pieces play their Motion clips too. An SCM scene node has the MOD
  joint record (translation, rotation xyz), so the MOT binds to the nodes;
  its channels are turned into node matrices the SCM way (its rotation order
  is not the MOD joints': `MotionClip::evaluate_channels`), composed down the
  node tree, and every object moves rigidly with its node (SCM meshes are
  local to their node). A piece whose rebuilt rest pose does not match its
  own stays still. The room's `revision` changes whenever it moves or hides,
  so the GPU rebuilds its cached buffers.
* MOD / EFM tracks are composed (`compose_mod_sessions`, source
  coordinates) and get their own `Texture`.
* MOT keys: compression 2 and 3 as the vendored reader decodes them, and
  compression 0 / 1 (float keys, 0x1402E9650 cases 0 and 1: an 8-byte
  track prefix, keys of 8 bytes `u16 time | mode bit, u16, f32 value` or 16
  with f32 incoming / outgoing slopes) read by the Reader itself: every
  pl101 MOT of m01_v00 is compression 0. A MOT whose domain is shorter than
  its model drives the leading joints (m01_v00's wp105 MOTs: 2 masks in a
  header with room for 9, for the 10-node Yamato).
* Each model track plays its Motion clips: the clip that started last among
  the covering ones, held at its end between clips. A cutscene MOT keeps
  the frame range it was cut from (f32 start at +0x08, end at +0x0C,
  repeated at +0x10 / +0x14) and its keys sit on those frames
  (`pl100_00_0004`: 650..1060), so a clip plays its MOT at
  `start + local time`; a range that starts below zero (`pl100_00_c010`,
  -70..565) has its keys count from its start, so it plays at `local
  time`. `CutFrame` trims a clip's end, it does not move its start. Weapon
  MOTs often hold two equal keys: the weapon lies still (on the desk)
  while the body plays. Body MOTs of the lighter cutscene body variants
  have longer channel domains than the model; they drive its leading joints
  (`load_cutscene_motion`, as 0x140310A80 walks the model's joints).
* `CnsPos track joint`: a model authored in the joint's own frame (its root
  rests at 0: hands `pl100_03` / `pl100_04` in m01_c00, the amulet
  `wp140_00`, em105's parts) rides the joint as it is (vertex x joint
  world); one authored at the host's rest pose (hair and heads, whose root
  rests where the joint does; m02_c01's `em108_08`, whose root rests at 0
  200.5 39.4, off its joint 3) follows the joint's move from its rest world
  (vertex x inverse joint rest world x joint world). Their MOTs leave the
  root alone (m01_c00's hand MOTs mask channels 0 and 1 out).
* `Pos x y z` / `Rot x y z` (degrees) on the Model clip: the model's own
  space in its joint's frame, built as the player's weapon records are
  (m03_c00: Lady's wp110..wp113 on her joints 14, 16, 19).
* `RefMotion n`: the model takes track n's node worlds (its skeleton is a
  copy) and stands where that model does (m03_b00: Cerberus' em125_07..14
  on em125_00..03).
* `ClothSim1D file.clt` with `ClothSim1DCnsPos0 track joint` and
  `ClothSim1DCnsPos1 track joint`: a chain hung between two joints (a
  model's or a stage piece's): m03_b00's Cerberus chains (em125_05) from
  its body to the stage, m03_b01's wp115_01. The .clt is its own dialect
  (`ClothSim1D segments length`, Gravity, SpringForce, Damping, MaxSpeed,
  FloorLevel, `Cut a b`); the chain's points are the model's nodes from
  node 1 on, the ends ride the joints, the others fall and keep their rest
  distances (`model_cloth.h`). When the far anchor is a stage the archive
  comments out (st006.scm), that end holds where the rest pose put it.
* `Crush` (m03_b01: Cerberus shatters from 1240 for 270 frames, hidden at
  1509; options DELAY, GRAVITY, SPIN, BLINK, REDUCE): how the game cuts its
  pieces (`Crush 0 QUARTER`) is not recovered; every crushed object flies
  off as one piece, falls, turns, shrinks and blinks out.
* Limb parts without CnsPos (m01_s00's hands, `plNNN_03` / `plNNN_04` only;
  em125's parts share such offsets without being limbs): a model whose node 1 is a
  joint of a model of the same character (`pl100_03` and `pl100_10`, both
  `pl100`), its node 2 at that joint's child offset (the wrist, 26.86 along
  x: body joint 12 -> 13; `pl100_04`: 8 -> 9), rides the joint: vertex x
  inverse node 1 world x joint world. Their MOTs put node 1 exactly on the
  forearm while a clip plays; between clips the hand follows the arm instead
  of staying behind.
* Cloth: the Model clip names a track's own `.clt` (`Cloth
  pl100_05_m03_b01.clt`), a Motion clip a later one (`Cloth pl100_01.clt`; with none
  named, a clip's `ClothMode 1` runs the model's own, `pl101_01.clt` for
  `pl101_01.mod`, as m01_v00 does) and its chain runs the solver of
  `cloth_chain.h` in the world, one step a game frame (`model_cloth.h`):
  Dante's and Vergil's hair and coats, the amulet, Yamato's cord, em105's
  sleeves, em108's and em134's parts. `ClothMode 0` (it holds until a clip
  says otherwise; on before) leaves the bones to the MOT (m01_c00's coat,
  carried over the shoulder, is baked). `ResetClt` /
  `ResetClt2` restart it at the pose and settle it `ResetCltLevel` frames
  (20); a scrub or a jump does the same.
* Visibility, per object of every track (models and stage pieces), on two
  levels: the model (H hides it, D and A show it) and its objects (a clip's
  `.hid` sets the states of the objects it lists, the others keep theirs).
  Every Hide clip set up to the frame applies in order. m03_c00 hides Lady
  (H at 938) and brings her back with only her bandage hidden (A
  `pl102_00_houtai.hid` at 1164). The meshes draw the shown objects'
  triangles only.
* The camera: the covering Camera clip that started last; between clips the
  last one that ended holds its final pose. The view stands at the eye,
  looks at the target, and the field of view becomes the viewer's zoom
  (`tan(0.5) / tan(fov / 2)`; the viewer's focal length spans the shorter
  image side).

The timeline is the session's one motion (`motion_player.h`): its library
entry `Cutscene mNN_xNN` binds it, `apply_motion_frame` seeks it, so the
viewer's play / pause, tap-to-pause and scrub drive the cutscene. Its loop
starts at its end: it plays once, start to end, and holds the last frame.

### The camera

While the cuts drive the view, the core puts the shot's camera into every
render (`prepare_view`). The first camera gesture (turn, pinch, two- or
three-finger gestures, or ✈) takes it (`demoTakeCamera`): the free camera
stands at the shot's eye and continues from its direction and field of
view, so the user can look around and zoom in a pause or while it plays,
and fly through the scene. `⋮ → 🎬 Cutscene camera` gives the view back to
the cuts.

### The camera inside the scene

Shots stand over a shoulder or inside a model (`cm036`: the eye in Dante's
head). The model pass clips its triangles at the near plane in camera space
(the room pass and the GPU pass already did), so geometry at and behind the
eye is cut, never projected across the picture. A camera inside the scene
keeps its near plane at 0.2% of the framing radius (an orbit view keeps 5%):
a cutscene frames its whole set, and 5% of m01_s00's was 56 units, a third
of Dante.

A taken camera flies from the shot's eye: pinch zooms, the two-finger dolly
moves it along the view and the two-finger pan across it, scaled to the
shot's target distance.

### Subtitles (v92)

A cutscene's `Message` clips (`No 100`, life 0) show messages of the
localization tables beside the game, `<cutscene>_msg_<lang>.txt`
(`m01_s00_msg_eng.txt`, `_frn`, `_chn`, ...). The ⋮ menu's **Subtitles**
(the cutscene file picker) loads one (`messages.h`, `demo::attach_subtitles`).
A table holds `# Msg 00100` blocks of quoted strings with tags (the table's
own Japanese header explains them): `<PS x y>` position, `<SZ n>` font size,
`<IT n>` frames a character, `<FC>` / `<BC>` colours (128 = 1.0), `<WT n>`
wait, `<BR>` new line, `<CL>` new page, `<JP n>` go on with message n,
`<NE>` stay, `<FI>` / `<FO>` fade, `<AL>` alpha. The time a message shows is
its own: its waits from the clip's frame (m01_s00's 100: 155 frames from
1485); the Chinese table splits long lines into pages with `<CL>` and a
`<WT 30>` gap. Text is UTF-8 (Chinese with a BOM), other bytes Windows-1252;
the comments are Shift-JIS and skipped. Positions are on a 640 x 480 screen
(two-line English subtitles start at y 421, size 22); the core hands the
shell the lines, their top and size as fractions of the picture, the colours
and alpha (`messages::layout`, JNI `demoSubtitle` / `demoSubtitleStyle`), and
Android draws them centred with an outline in the back colour (the x of every
table centres its line for the game's font). Messages 140 / 141 / 180 / 181
of m01_s00 belong to its movie part and show once the .wmv is attached.

### Screen effects, pad vibration (v91)

* `ScrEfc` clips `Id GFG` are a depth fog: `StartZ` / `EndZ` camera
  distances, `MinColor` / `MaxColor` (u32, bytes R G B A) and `MinAlpha` /
  `MaxAlpha` (0x80 = 1.0) at them, linear between and held beyond
  (`demo::fog`; the renderer blends the drawn surfaces, effects stay over
  it; the GPU pass does it in its mesh shader from v96, the software pass
  from its depth buffer). `Type` 0 / 1 is not told apart.
* `PadVibe` clips pulse the phone (`demo::vibration`, JNI `demoVibration`):
  the large motor eased from `LMotorSt` to `LMotorEd` over the clip, the
  small motor (`SMotorSt` / `SMotorEd`, 0 or 1) at least 96 of 255.

The other screen effects (`DOF`, `FCS` focus, `CCT` colour, `BGA` / `BGO`,
`ATF`) and the `Light` clips (point lights: `ValidType`, `IsMinus`, `Type`,
`SrcPos` / `SrcForce` / `SrcRange` / `SrcRgb` to `Dst...`) are read but not
drawn: their meaning is not settled from the data alone.

## Not yet

Lights and the screen effects above other than the fog, and the
stream-number-to-file table
of the EXE (so the right .ogg and .wmv would be found by themselves). A
cutscene that
does not carry its stage (`m01_c00` comments out `st000.scm`: the game uses
the shop it is in) shows it when that stage is picked as the room.

Tests: `demo_test.cpp`, `video_test.cpp` (synthetic bytes).
