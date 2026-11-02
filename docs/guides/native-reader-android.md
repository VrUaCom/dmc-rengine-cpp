# DMC Native Reader for Android — complete guide through V142

DMC Native Reader for Android opens and inspects resources from **Devil May Cry 3: Special Edition in Devil May Cry HD Collection**. The NR-Luna-v73 development branch combines model and texture viewing, assembled stages and characters, motion playback, effects, collision inspection, cutscenes and a virtual photo/video camera. It remains a reader and visual inspection tool: saving a picture, UV map or report does not edit or repack the original game resources.

## 1. Version, evidence and downloads

**This guide describes NR-Luna-v73 through V142 at source SHA `7f8caeacac9b168de3954ee15fa42beb01d61f15`, reviewed on 2026-11-02.** V142 is the branch's development revision, not Android versionCode 142. The branch name retains v73 while later changes are numbered V74–V142.

The source documentation still mentions historical review head `41358b89fb08ff9ec59284a267299bbfd52e51b7`. It is not the current branch HEAD. Some documents retain an older update date even though their bodies describe later revisions; this guide pins the code instead of relying on that date.

| Evidence label | What it means here |
| --- | --- |
| Source implementation | The pinned branch contains or documents the feature. It is not a promise that every file works. |
| Component evidence | A named parser, renderer, emulator comparison or desktop test is recorded in the repository. This does not certify the entire Android application. |
| Historical device pass | A physical Android test of an earlier, specifically identified APK. It does not accept every change through V142. |
| Current APK acceptance | A build, its exact SHA and artifact identity, automated results and physical device checks must match. Full V142 acceptance was not established in this guide update. |

The [Downloads catalog](https://vruacom.github.io/dmc-rengine-cpp/downloads/) currently selects the public **code-72 / 1.0.45 APK from revision 7d1188e**, a **debug/device-test artifact**. It is older than the V142 source described here. The earlier README names **v68 / 1.0.41** as a promoted Android baseline. **Do not expect the selected older APK to contain the entire V142 guide.** No new V142 APK, tag or download is published by this documentation change.

The [capability matrix](https://vruacom.github.io/dmc-rengine-cpp/capabilities/) remains package/version-specific. A public **Windows x64 v1.0.0 technical preview** and an **unsigned developer artifact** for iOS have their own limits; neither receives Android branch parity from this guide. The existing MOD/SCM task walkthroughs explicitly target Windows Preview.

## 2. Start with a resource

- Confirm that your files belong to DMC3 in HD Collection. DMC3's 2006 PC release and PS2 edition are separate targets; similar extensions do not establish compatibility.
- Choose the Android build you actually have, record its package version and source revision where available, and compare them with section 1.
- Open a copied resource using **↑ Open** when nothing is loaded. With a resource open, use **⋮ → Open / replace resource**. The earlier Android device pass also exercised opening from an external file manager.
- Read **Info** and its File Health indicator before interpreting the preview. Keep the original archive, member and physical-slot identity alongside extracted files.
- For a stage, open `stNNN.pac` to inspect the assembled scene; for a character archive, use the assembled resource and its available motions, parts and companions.
- If the file is still inside NBZ, follow the separate [NBZ → PAC/PNST → leaf-resource workflow](https://vruacom.github.io/dmc-rengine-cpp/guides/extract-nbz/). Native Reader is not advertised here as an NBZ unpacker or repacker.

A useful first session is: open a resource, inspect Info, examine its model or texture, toggle a relevant overlay, then try an available motion. Add a base room or stage companions when you need their context. Enter Camera mode after arranging the scene.

## 3. Resource families and depth of support

The earlier `main` registry grouped **16 native module entries**. The pinned V142 registry has **18 initial entries**, then appends stage-configuration, family and cutscene modules. Those are registry entries and groups, not a flat count of fully supported formats. The two texture entries, container children and format-specific adapters make the counting units different.

| Resource | What the Android development branch provides | Boundary |
| --- | --- | --- |
| MOD | Model geometry, objects, hierarchy, texture relationships, skin/skeleton-facing data and viewing | Inspection does not create a geometry editor. |
| SCM | Scene/object structure, transforms, hierarchy and visual inspection | Structural reading is distinct from game-ready scene authoring. |
| DDS | Texture decoding and image preview | Support is bounded by the decoder, not every possible DDS variant. |
| PTX | Texture banks, child-image navigation and model companion texture attachment | Texture slots and resource identity still matter. |
| `.tm2` names | The validated DDS path for recognized DMC wrapped-DDS content | Not a universal PlayStation TIM2 implementation. |
| PAC | Container browsing, child resources, character/stage assembly where recognized | Opening an archive does not imply writing it back. |
| PNST | Nested resource groups, model/resource relationships and supported assembly | Preserve physical resource relationships. |
| MOT | Motion channels, skeleton transforms and supported playback | Raw MOT and scripted actions are different modes. |
| SHW | Shadow resource inspection | A parsed shadow resource does not prove every original shadow draw path. |
| EFM | Supported effect-model geometry and viewing | Not all effect behavior is geometry. |
| TSC / CLT | Bounded structural inspection; supported scene/cloth consumers where implemented | Structural CLT opening is not full cloth-runtime equivalence. |
| EventTbl | Event-table inspection | Reading entries is not complete gameplay execution. |
| Motion Scripts | Action/motion relationships and supported scripted playback | AI and player/world-dependent behavior remain bounded. |
| Collision Shape / Index | Collision shapes, indexed relationships and hitbox inspection | Distinct from the stage's HITS surfaces. |
| FXBANK | V/E/G/P banks and supported effect graphs | Each family and nested path has its own implementation depth. |
| `.hits` | Renderable collision surfaces, record edges, kinds and reports | Reader collision proxies are not the original entire gameplay physics. |
| Stage configuration | Known LIG2, POS, CAM, STE, SEF and related sections | Unknown fields stay unknown; some projections remain experimental. |
| `.cam` / `.hid` | Cutscene camera animation / object show-hide animation | Depends on the associated cutscene resources. |
| `.ogg` | Audio inspection, waveform and playback; cutscene sound attachment | Stage sound loading is a separate unfinished workflow. |
| `.wmv` | Movie frames, codec/length information and timeline playback | Decoder support is bounded; not a general-purpose video editor. |
| Cutscene scripts | Commands, clips, timing and resource relationships | Some entries are structural or require externally supplied companions. |

The branch also registers a font module. This guide's format table preserves the audit's practical families without turning registry presence into equal-depth compatibility or device acceptance.

Sources: [pinned module registry](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/module_registry.cpp), [native modules](https://github.com/VrUaCom/DMC-Native-Reader/tree/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules), [cutscene format research](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/CUTSCENES.md).

## 4. Viewer controls and information

| Control | What to do / what it shows |
| --- | --- |
| ↑ Open | Select a resource while the session is empty. |
| ⋮ | Open session-specific actions, replace the resource, browse available archive files or access relevant attachments. |
| W | Toggle wireframe. A stage shows its meshes; behind a model, the room's mesh lines can be dimmed. |
| Bones | Inspect skeleton/joint hierarchy, including placed merged models where supported. |
| ▦ | Show the room/stage HITS collision overlay. Long-press for its structural report. |
| ◎ | Show hitboxes/attack shapes. Long-press for shape types and attack target-layer groupings. |
| ℹ Info | Inspect resource identity, structure, children, available metadata and health. |
| 📷 beside ⋮ | Enter or leave Camera mode; it is not merely a screenshot button. |
| Drone picture | Enable the free camera's two sticks: left moves, right looks. |
| ⇄ Next spot | The action in ⋮ steps through base-room POS spawn points in V142, or floor spots when no POS are available. |
| Motion strip | Choose and play a supported motion or timeline; available entries depend on the resource. |

Archive and cutscene internals remain accessible through **⋮ → Browse .PAC files…**. V118–V120 expose recognized script/name lists as **SCRIPT** and **NAMES**, effect manifests as **FX-IDS**, and empty clip texts as **EMPTY**, rather than presenting everything as anonymous BIN data. Associated PAC/PNST name lists supply meaningful child names when recognized.

### File Health and the V138 Info panel

The small colored dot belongs to **Info**; it replaced the older orange title-bar triangle. Info's bottom actions are **✕ close**, **⇩ export/download the displayed information**, and **⚠ health report**. In the report view, **ℹ** returns to the normal information.

| State | Meaning | Practical response |
| --- | --- | --- |
| Red | Severe detected problems: empty/all-zero data, truncated PAC slots, or a diagnostics log, ZIP, picture or web page under a game-resource name | Stop treating the file as a valid source resource; compare it with an untouched copy. |
| Orange | Naming/extension mismatches, copy suffixes or spaces, a `.pac` without PAC content, no accepting reader, or incompletely specified structure | Read the report to distinguish a harmless filename issue from unsupported content. |
| No warning | The implemented checks found no issue | This is not a guarantee of full semantic correctness or original-game compatibility. |

The repository reports **30 sample game files** passing those checks and an overwritten `st001cfg.pac` receiving red status. That is a recorded sample result, not a new test of every V142 file or a certified validator for the original game.

Source: [Android controls and File Health UI](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/java/com/dmcrengine/nativeviewer/MainActivity.java).

## 5. Stages, Stage Ops and five base rooms

A **stage opened as its own scene** and a **base room around a model** are separate workspaces. **⋮ → Stage Ops** belongs to the opened stage. **Settings → Base room** supplies the surroundings for a model, enemy or character. Their effect banks and configuration are kept separately, so adding companions to one should not silently change the other.

| Stage Ops row | Resource and purpose |
| --- | --- |
| Room | `stNNN.pac`: assembled room/stage geometry and supported layout objects. In an opened stage, this row can select another resource. |
| Effects | `stNNN_effect.pac`: the stage's effect bank. |
| Config | `stNNNcfg.pac`: known lighting, fog, spawn points, camera zones and placed effects. Long-press for details. |
| Sounds | A stage-sound placeholder; this stage-sound reading workflow is not finished. |
| ✕ | Remove an attached resource where the row provides the button. The opened stage itself and Sounds do not have this removal action. |
| Long press | Read the resource's fuller information instead of relying on the compact row. |

V133 adds **five independent base-room slots**, selected by squares **1–5**. Each retains its room, effect bank, configuration and spawn point; the square identifies its stage. A new room clears unrelated old-stage companions while retaining companions that belong to the new stage. A base-room configuration supplies lighting, fog and placed effects; stage-marker presentation is scoped to a stage opened as its own scene.

**Settings → Game folder** selects a game directory once. Choosing `stNNN.pac` can then locate `stNNN_effect.pac` and `stNNNcfg.pac`. An effect bank or configuration from a different stage appears in orange as **another stage's**; use its ✕ action to detach it.

### Place a character and choose a camera

- In **Settings → Base room**, load the room and its matching configuration, then choose a **Spawn point**.
- V140 places the model at that POS and selects the matching game CAM zone when one contains the position, with model framing derived from that camera. A room spot or a position with no matching CAM zone retains the usual view.
- V141 orders floor spots by nearest neighbor so the sequence behaves more like a walk through the room; the first spot stays first.
- V142 makes **⇄ Next spot** prefer the base configuration's POS list, preserving the choice with that room. Without such POS it uses floor spots.

Scenes assemble recognized SCM/MOD geometry, PNST objects at layout positions and stage textures. Supported views include HITS surfaces, recognized intact/broken layout states, configuration fog and STE-placed effects. Configuration information exposes LIG2 lighting, light types/flags/colors/positions/reach, ambient terms and masks; raw words remain available rather than receiving fabricated meanings.

Stage collision is active when the room is drawn in the documented branch workflow. Characters use a **Reader sphere proxy of radius 50**, follow floors and stop at walls; supported projectile paths can stop, explode or bounce at HITS. These are bounded Reader projections of researched behavior, not complete original gameplay physics. Collision reports group records by kind/color, floor/wall/ceiling, type/category bits and known skip tests. Hitbox reports instead group shape types and attack target layers.

Sources: [stage configuration](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/stage_config.cpp), [stage markers](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/stage_markers.cpp), [V133–V142 status](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/STATUS.md).

## 6. Characters, motions and FXBANK effects

Supported assemblies combine MOD parts, texture-slot relationships, skeletons, costumes, cloth-facing resources and attached objects. A raw MOT clip is different from a **MotionScript / Script Play** action: the latter can involve resource bindings, effect profiles and scripted events. Some actions require player, AI, collision or arena state that a reader session does not reproduce.

In V141, motion details indicate whether a motion **scales bones**, using **16 sampled frames at load time**. This is a sampled indicator, not an exhaustive proof that no unsampled frame contains scaling.

| FXBANK family | What is implemented or researched |
| --- | --- |
| P — particles | Particle geometry and supported gravity, friction, color tracks and blending. |
| E — sprites | Supported sprite geometry, animation, color, blending and lifetime. |
| G — generators | Child effects, generation events, delays, scale and motion in supported graphs. |
| V — visual effects | Supported visual roots and their nested child graphs, attachment and local clocks. |

**⋮ → Effects (play any)**, added in V122, lists supported V/G/E/P entries from the loaded file, stage or cutscene banks. The selected effect loops at the model's feet or in the picture center for free/cutscene cameras. Rows identify banks and, where known, game classes that spawn the effect. **⋮ → Effect bank**, expanded in V123, attaches additional banks such as `static_effect.pac` or `stNNN_effect.pac`.

The **875** figure is a catalog of **effect-spawn call sites in `dmc3.exe`**, not 875 distinct finished visual effects. Repository component evidence compares **109 real P records over 12 ticks** and **74 real G records** with emulator-driven results; the **G → G** path is explicitly not emulator-checked. Those bounded results do not certify every nested effect graph or the entire APK.

For Lady, the historical device report identified misplaced/near-vertical effects. The branch records targeted Shl02/V423 attachment, basis, motion and lifetime corrections. Gameplay-dependent steering, proximity/collision context and some player-dependent paths remain distinct limits; a targeted source fix is not a new device acceptance pass.

Sources: [particles](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/particle_sprt.cpp), [generators](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/generator.cpp), [effect banks](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/effect_bank.cpp), [875 spawn-site research](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/research/dmc3-effect-spawn-sites-v122.md).

## 7. Cutscenes, sound and movies

A cutscene session coordinates several resources on one timeline. Open its archive, inspect its script/children and attach external companions through the session menu where needed. Container opening does not imply that every sound or movie was embedded in that container.

| Component | Reader behavior |
| --- | --- |
| Script | Commands, Set/clip relationships and timing. |
| Camera | `.cam` animation, including documented linear and cubic Hermite interpolation. |
| Model / Motion | Character/object models and MOT poses on the timeline. |
| Hide | `.hid` object visibility changes. |
| Effect | Supported effects at clip times, including bone attachments where resolved. |
| Sound | OGG streams attached separately through **⋮ → Cutscene sound**. |
| Movie | WMV attached through **⋮ → Cutscene movie**, with frames and audio joining the timeline where supported. |
| Fade | Fade-in/out transitions, color and clip timing. |
| Message | Structural message entries and supported subtitle attachment/presentation; not a claim that all script commands are gameplay-complete. |

OGG decoding uses **stb_vorbis**. A standalone OGG opens with Info, waveform and playback; attached cutscene streams follow clip timing, mix and pause/resume with the timeline. WMV decoding uses the repository's included **minimal FFmpeg components**; a standalone WMV provides movie information, frame/timeline viewing and supported audio. A cutscene's separately supplied movie can extend its timeline beyond the rendered scene's picture end.

Camera mode can film a cutscene **through its own cuts**. The viewfinder shows **CUTS** while its camera drives the view; the chosen lens/exposure/style still apply. A finger takes control at the current shot's eye, and **⋮ → Cutscene camera** returns control to the cutscene. Gyroscope input or starting recording should not by itself replace the cutscene camera in the documented V113 path.

Sources: [cutscene session](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/demo_session.cpp), [cutscene research and examples](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/CUTSCENES.md).

## 8. Enter Camera mode

Use **📷 beside ⋮** to enter Camera mode and the same control or **↩** to return. The lower motion/tool rows become the camera bar. Its dials are **Lens, Aperture, Mode, ISO, Shutter, EV, WB, Frame, Focus, CPL, Style and File**. Tap a dial to open its settings above the view; long numeric lists use sliders. The controls remain square tiles, and hints explain short option names.

The bar includes settings, play/pause, photo shutter, video start/stop, reset view, back and the relevant sensor/drone controls. The **⋮** menu also provides **Take a picture**, **Record a video / Stop recording** and **Camera settings**. These are actions in the reader, not simulated screenshots on this website.

The virtual camera stands at its own eye and turns in place rather than orbiting a target. Drone adds sticks to that free camera. Returning to the ordinary scene restores orbit behavior while preserving the framing. **Camera mode is not restored at startup:** the app and newly opened resources begin in the standard view. Individual chosen camera options are kept separately.

On Android 10+, photos save under **Pictures/DMC Native Reader** and videos under **Movies/DMC Native Reader**; older Android versions use the application's folders. Video recording is **silent**, even when the cutscene being viewed has sound. Leaving the app stops recording.

## 9. Photo formats, size and capture

Bold values below indicate documented defaults where shown. The settings describe a virtual rendered camera, not the phone's physical camera sensor.

| Setting | Choices / behavior |
| --- | --- |
| File | **PNG**, JPEG, WebP lossless, RAW DNG, DNG + JPEG. |
| JPEG quality | 60, 75, 85, 90, **95**, 100. |
| RAW data | **Linear RGB** or Bayer RGGB. |
| Long-side size | **As the view**, 1280, 1920, 2560, 3840, 5120, 6144, 7680 px (the 8K label). |
| Megapixel mode | By size, or 2, 5, 8, 12, 16, 20, 24, 33, 40, 48, 64, 100, 124, 150, 200 MP in the chosen frame shape; maximum side 24576 px. |
| Self-timer | **Off**, 2, 5, 10 seconds. |
| Burst | **1**, 3, 5, 10 shots, documented as a quarter-second apart. |
| Subtitles | **Show** or hide in the saved picture. |

Photos exceeding a single-render size use tiles; the documentation describes 100 MP and above as the high-resolution tiled path. A **124 MP** example requires approximately **1.5 GB of memory**, with a desktop example around four seconds. Those are source-document examples, not Android device measurements or a guarantee that 200 MP succeeds on every device. Pictures render/save on a worker, sequentially, and memory limits can prevent capture.

DNG contains a small developed gallery preview and 16-bit RAW data with exposure/lens/sensor treatment; white balance and camera settings are recorded in metadata, while style/color curves/sharpening remain editor-side choices. Linear RGB and Bayer RGGB are different RAW representations; Bayer requires demosaicing. The repository reports both opening in **LibRaw 0.22**. **DNG + JPEG** saves both from one render. Nikon NEF is not a supported substitute generated by this camera.

## 10. Video recording

| Setting | Choices / behavior |
| --- | --- |
| Size | 720p, **1080p**, 1440p, 2160p; documented short-side sizes with the long side capped at 4096. |
| Frame rate | 24, 25, **30**, 50, 60 FPS. |
| Codec | **H.264** or H.265. |
| Bit rate | **Auto** or 4–100 Mbit/s. Auto uses the documented per-pixel codec estimates. |
| Keyframe interval | 0.5, **1**, 2 or 5 seconds. |
| Time limit | **No limit**, 15/30 seconds, 1/5/10 minutes. |
| Motion blur | **Off** or shutter-based blending of preceding frames. |
| Subtitles | **Show** or hide in the video. |
| Sound | **Not recorded**. |

The app checks whether the phone's encoder supports the requested codec/size before recording. That check does not guarantee sustained 4K/60 FPS, thermal stability or compatibility on every phone. Start with 1080p/30 and a short clip to inspect the result before increasing settings.

Photo and video share framing, lens, aperture, exposure, filter and look, while their file/resolution/codec choices remain separate. Their shutter and record controls remain distinct.

## 11. Lens, exposure, focus and image style

| Control | Choices / practical meaning |
| --- | --- |
| Lens | As-view pinch zoom, or 14, 18, 24, 28, 35, 50, 85, 105, 135, 200, 300, 400 mm full-frame equivalents. |
| Aperture | f/0.5, f/0.95, f/1.2, f/1.4, f/2, f/2.8, **f/4**, f/5.6, f/8, f/11, f/16, f/22. |
| Exposure mode | **Auto** preserves rendered brightness except EV; Manual lets ISO, shutter and aperture determine exposure. |
| ISO | **Auto**, then 25–102400 in third-stop choices, followed by 204800, 409600, 819200, 1638400, 3276800, 4000000. |
| Shutter | 30 seconds to 1/2000 in documented third-stop choices; **1/60** is the baseline. |
| EV | −3 to +3, default **0**. |
| White balance | **As rendered**, or the documented presets 2000, 3200, 4000, 5500, 5800, 6500, 7500, 10000 K; not a promise of every intermediate Kelvin value. |
| Depth of field | **Off** / On; depends on rendered depth. |
| Focus | **Auto** at the frame center, 0.5, 1, 2, 3, 5, 10, 20 metres, infinity; touch focus and focus lock are also available. |
| Bokeh | **Round** or 5–9 blades; **soft** / bright highlights. |
| CPL | Circular polarizer **Off** / On, weak / **medium** / strong, with ring angle control. |
| Vignetting | **Off**, aperture-derived or strong. |
| Distortion | **Off** or focal-length-derived barrel/pincushion distortion. |
| Color fringing | **Off**, weak or strong chromatic aberration. |
| Highlights | **Clip** or soft roll-off. |
| Noise | Off, **ISO-based** sensor noise, or film grain. |
| Holding | **Tripod**, Hand-held, Stabilised. |
| Style | **Standard**, Vivid, Neutral, Monochrome, Sepia, Film, Cinema. |
| Contrast / saturation | −2 to +2, default **0**. |
| Sharpness | **Off**, low, medium, high. |

In **Auto**, changing ISO, aperture or shutter switches to **Manual** (V115); Auto ISO becomes 100 so the starting image stays comparable. Choose **Mode → Auto** to return. Manual's reference is **ISO 100, 1/60, f/4**: increasing ISO, lengthening exposure or opening the aperture brightens the rendered image. These are simulated exposure controls over a virtual scene.

V116 exposes a photo over its shutter interval: motion, cutscene pose/camera movement and supported hand/gyroscope shake are sampled and averaged. Slow shutters can blur moving subjects; faster shutters can freeze them. Hand-held and Stabilised change simulated shake, and **SHAKE** warns when the shutter is slow for the current lens/holding conditions. RAW uses the same exposure path. This later implementation supersedes a stale “Not yet: a photo's motion blur” sentence still present at the bottom of the camera document.

CPL is a virtual polarizer model, not recovered polarization data from the game's materials. With CPL enabled, a two-finger twist turns the ring instead of the view. The lens, sensor treatment and image styles are Reader presentation features, not evidence that DMC3 originally rendered the scene that way.

## 12. Framing and gesture reference

| Frame setting | Choices |
| --- | --- |
| Aspect ratio | **As view**, 16:9, 3:2, 4:3, 1:1, 21:9, 2.35:1, 9:16, 2:3. |
| Grid | Off, **rule of thirds**, 4×4, golden ratio, center cross. |
| Settings line | **Show** or hide. |

The viewfinder's frame is the intended capture rectangle. The renderer adjusts projection/zoom for the selected shape instead of merely stretching the saved image.

| Gesture or button in Camera mode | Result |
| --- | --- |
| One-finger drag | Aim/turn the free camera; with gyro enabled, adjust without physically turning the phone. |
| Pinch | Change zoom / focal length. |
| Two-finger drag | Move sideways and up/down. |
| Hold one finger; move another | Dolly forward/back along the view direction. |
| Two-finger twist | Turn enabled CPL. |
| Two-finger tap | Level roll and re-center gyro. |
| Double tap | Focus at the touched point using available depth. |
| Long press | Toggle AF Lock / return to autofocus. |
| Slide along the right edge | Manual focus ring: up farther, down nearer, logarithmic steps. |
| Three-finger twist | Roll the camera. |
| Three fingers up/down | Adjust EV; the document specifies one step per 50 dp. |
| Three-finger tap | Take a photo. |
| Volume button | Photo shutter; hold to start/stop video. |
| Walking enabled | Physical steps/crouching move the virtual camera. |

The ordinary viewer's three-finger sideways variant swipe and four-finger follow tap are disabled in Camera mode. **ℹ in the camera bar** explains that mode's controls. Dolly is separately configurable in gesture settings; model views have a target and do not dolly through the model, while a stage can be entered.

## 13. Gyroscope, drone, walking and orientation

**3DoF gyroscope** uses phone rotation to turn the virtual camera; it does not measure physical walking. Enable it with **🧭** in the camera bar or Camera settings. Sensitivity is **0.5×, 1×, 1.5×, 2×** and smoothing is **Off, Low, Medium, High**. Touch, sticks and phone rotation can contribute to aiming together.

**World horizon** retains world-relative roll as the phone tilts, so a saved picture can be tilted. **Screen horizon** ignores phone roll and behaves more like a gimbal. Roll is carried by the camera projection and applies to images, video and RAW.

**Drone** uses two sticks: left moves, right looks. It is the same free camera used in Camera mode with sticks added. Gyro can still turn it.

**6DoF walking** uses optional **ARCore / Google Play Services for AR**. Enable **🚶** or Walking in Camera settings. The first-use flow explains ARCore and requests camera permission; unsupported devices report that walking is unavailable while gyro remains usable. The branch documents using the phone camera for tracking without displaying or saving that camera image.

| Walking setting | Choices / behavior |
| --- | --- |
| Movement scale | 0.25, 0.5, **1**, 2, 5, 10, 20 metres of virtual scene per physical metre. |
| Height | **Follows you** for standing/crouching, or fixed. |
| Tracking loss | Re-acquired tracking starts fresh to avoid a position jump. |
| Stop conditions | Leaving Camera mode, turning walking off, or pausing the app. |
| Optional dependency | The app can run without ARCore; walking requires compatible hardware/services. |

V114 changes orientation handling so turning the phone does not recreate the Activity or reopen the scene. With gyro/walking active, orientation is held because phone movement controls the view. **📱** changes and locks screen orientation; holding it allows normal phone-driven rotation again. This is the source implementation; the earlier session-restore regression still needs an exact-current-APK physical retest.

Sources for sections 8–13: [camera guide and full settings table](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/CAMERA_PHOTO_VIDEO.md), [camera core](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/include/dmcresource/spider/camera.h), [camera movement](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp/modules/camera_control.cpp), [Android camera and recording UI](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/java/com/dmcrengine/nativeviewer/MainActivity.java).

## 14. Graphics, shaders and performance

The Android GPU backend uses **OpenGL ES**, offscreen framebuffers and MSAA, GPU postprocessing and depth for depth of field, plus direct presentation through a SurfaceView. V113 removes the GPU → CPU → GPU round trip for eligible display frames; V117 extends GPU drawing to lines, markers, labels and bones. Pictures, encoding/readback paths, some inspection views and CPU export work still have their own routes. A desktop Mesa result is not an Android GPU certification.

| Settings / inspection | Available choices or purpose |
| --- | --- |
| Renderer | GPU or CPU/software. |
| Show frames (GPU) | Direct screen presentation or Through a picture; the latter is a diagnostic fallback for a black/stale view. |
| Shaders (GPU) | DMC3 or Reader's own. |
| Stage lights | Experimental LIG2 lighting, **Off by default**. |
| MSAA | Off, 2×, 4×, 8×, subject to hardware/memory limits. |
| Mipmaps | On / Off for distant texture shimmer. |
| Anisotropic filtering | Off, 2×, 4×, 8×, 16×, subject to device support. |
| Render long side | 360, 540, 720, 1024, 2048, 3840, 5120, 6144, 7680 px; this is separate from photo capture megapixels. |
| While moving | Fast half-size preview or full quality. |
| Model textures / light | Pixel-original or Smooth; camera lighting or flat. |
| Background / shadows | Dark, Grey, Light or Black; initial shadows On/Off. |
| Lines and overlays | Separate mesh/collision line widths 1, 2, 3, 4, 6 at the reference 720 size; room-wireframe opacity 0–100%. |
| Animation | 20/30/60 FPS and playback speed ¼×, ½×, 1×, 2×. |
| Benchmark | Ten-second maximum-FPS comparison using current settings, with frame-stage timings and retained results during the session. |

The **67 GLSL ES 3.00 shaders** are a clean-room library based on behavior specifications, not copied original shader source. The GPU compiles them, but only appropriate pairs are linked/selected for actual batches. Models, vertex-colored stages and supported fog use selected DMC3 equivalents; other paths retain Reader shaders. Other compiled library members wait for the required data or rendering passes. **67 compiled shaders does not mean 67 active effects or complete visual parity.** Renderer Info identifies shaders used in the last frame.

LIG2 lighting progressed through V124–V128: V126 documents the game's configuration-slot-1 lighting source, while later work refines light selection, masks and scale. **Stage lights remains experimental and disabled by default** because selection/projection and visual equivalence are still bounded. Do not repeat the older V124 uncertainty as if no later evidence existed, or treat later source evidence as a complete visual acceptance pass.

The V109 CPU camera-processing benchmark reports **212 ms → 69 ms per 1080p frame on a four-core desktop**, approximately three times faster for that specific pass/settings combination. It is **not a threefold speedup of the entire Android application**. V110 moves camera development to GPU; V113/V117 reduce display readback. Use the phone's benchmark for its own settings rather than importing desktop numbers as phone FPS.

Sources: [shader mapping and active paths](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/SHADERS.md), [performance history](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/PERFORMANCE.md), [graphics settings code](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/java/com/dmcrengine/nativeviewer/MainActivity.java).

## 15. Export without editing the original

| Output | Purpose / boundary |
| --- | --- |
| Texture PNG | Export a supported decoded image. |
| UV PNG | Export a UV map; stage archives can expose per-texture UV galleries. |
| Rendered scene image | Capture the view without application UI. |
| JPEG / lossless WebP | Developed photo output from the virtual camera. |
| DNG / DNG + JPEG | RAW output or paired RAW and developed image from one capture. |
| H.264 / H.265 video | Silent rendered recording, subject to the phone encoder. |
| Info / diagnostic report | Save inspection or diagnostic information through the corresponding action. |
| MOD/SCM editing | Not a Native Reader authoring feature. |
| PAC/NBZ repacking | Not a Native Reader workflow. |

A reader export creates an image, UV presentation, video or report derived from reading/rendering. It does not imply model editing, arbitrary format conversion, rebuilding archives, creating mods or original-game acceptance of generated resources. Full editors, including collision authoring tools, belong to separate products.

## 16. Known regressions and troubleshooting

### Orientation restores an older resource

The first physical V73 pass used source **`55f874d575203792d86fd88cc49c3209b8db4f14`**, workflow **36587531306**. Installation, both opening routes, basic MOD/SCM/DDS/PTX opening and avoiding stale effects on reopen passed. External-file-manager rotation preserved its resource in that pass.

A separate sequence failed: open **PL000**, replace it internally with **EM028**, then rotate; PL000 returns and EM028 disappears. Comparable slot/costume selection can reset. V114's orientation implementation is intended to retain session/camera/motion state, but **this guide does not claim a physical confirmation on the V142 HEAD**. Record the exact APK/build and opening sequence when reproducing it.

### A chosen stage file became a diagnostics log

The branch documents a **V72–V134 file-picker/export request-code collision**: selecting stage effects could route the chosen file into diagnostic export and overwrite it. **V135 separates the codes and adds a build check for duplicate request codes.** File Health can identify a resource containing the Reader's diagnostics log.

This matters even though normal Reader operations are read-only: that historical defect wrote to a selected URI through the wrong action. Use the corrected build for the affected workflow and retain untouched originals. The fix is in the pinned source; no independent current-APK reproduction/acceptance was performed for this guide.

### Missing effects, unusual lighting or incomplete playback

- Check that the matching stage bank/configuration is attached; orange “another stage's” identifies a mismatch.
- Stage Sounds being unfinished does not mean standalone OGG or attached cutscene sound is absent; they are separate paths.
- Check Info, the supported child resources and motion mode. Raw MOT playback does not execute every scripted/gameplay condition.
- Some effects depend on player/AI/world state or an unverified nested path; no whole-game effect parity is claimed.
- For light discrepancies, inspect the configuration and remember that Stage lights is experimental. Switch it off to compare the normal Reader rig.
- For a black/stale GPU display, compare **Show frames → Through a picture**, then record Renderer Info and settings.
- For failed large captures or video modes, lower resolution/MSAA or select a codec supported by the device. Keep the reported error rather than assuming a successful file was written.

## 17. What has and has not been accepted

| Subsystem | Pinned branch implementation / documentation | Full V142 Android acceptance established here? |
| --- | --- | --- |
| MOD / SCM | Present; historical baseline opening evidence exists. | No. |
| DDS / PTX | Present; historical baseline opening evidence exists. | No. |
| PAC / PNST | Container inspection and supported assemblies. | No. |
| MOT / skeleton | Motion, transforms and supported scripted bindings. | No. |
| FXBANK V/E/G/P | Substantial, bounded/partial effect runtime and component comparisons. | No. |
| Stage Ops | Stage/base-room separation, companions, five presets and spawn workflows. | No. |
| HITS / collision | Visualization, reports and bounded collision behavior. | No. |
| Cutscenes | Script/resource/timeline coordination. | No. |
| OGG / WMV | Implemented media paths and attachment workflows. | No. |
| GPU / shaders | GPU backend, shader library and selected active paths. | No. |
| Photo / RAW | Implemented camera/export paths with device-memory limits. | No. |
| Video | Implemented recording path with device-encoder limits. | No. |
| Gyro / Drone / ARCore | Implemented controls; walking has optional hardware/service requirements. | No. |
| File Health | Implemented checks and a documented 30-file sample result. | No. |
| V142 Next spot | POS-first traversal, floor fallback. | No. |

The repository records parser tests, particle/generator comparisons, shader checks and desktop GPU/Mesa results. They are valuable **component evidence**. This website update ran neither a full exact-head native CTest/build nor physical Android acceptance. No fabricated screenshot or new retail-asset test is presented.

Still unconfirmed as a complete product: all gameplay/AI-dependent behavior; every effect graph and unknown format field; original lighting/rendering equivalence; every UI transition after V142; 200 MP capture and sustained 4K/60 on arbitrary devices. The V141 report's **151 JNI native functions** counts native bridge entries, not buttons, formats or completed workflows.

To close an exhaustive audit, bind all results to one HEAD and APK: inventory every Java control/handler, trace it through JNI into C++, run the required build/tests, verify artifact identity and perform physical device/corpus acceptance. This guide does not claim to have checked every button line by line.

## 18. Architecture and pinned sources

The read-side flow is **resource/container → bounded probe and NativeModuleRegistry → canonical/native parsing and typed resource session → rendering/inspection → Android UI through JNI**. The architecture is native-first **C++23**; Android is the platform shell.

| Component | Responsibility |
| --- | --- |
| DMCNativeReader::Core | Portable C++23 resource/inspection core. |
| NativeModuleRegistry | Route recognized resources to bounded modules. |
| resource_session | Keep opened resources, children and session relationships. |
| Spider | Core session/resource operations and typed presentation contracts. |
| view_renderer / view_gpu | Build CPU/GPU-facing views and frames. |
| gles_view_backend | Android OpenGL ES backend. |
| Black Widow | Interface state and permitted actions. |
| Java UI / JNI | Android controls, file/service interaction and core calls. |

DMC Rengine remains the canonical reverse-engineering and shared-format authority where the Reader consumes those contracts. The development branch's scene workspace and visualization do not authorize a second independent writer or replace GDSpaces' canonical resolver/materializer/provenance responsibilities.

- [Exact V142 source snapshot](https://github.com/VrUaCom/DMC-Native-Reader/tree/7f8caeacac9b168de3954ee15fa42beb01d61f15) — immutable version used for this guide.
- [Status, history and evidence boundaries](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/STATUS.md) — includes later V142 notes despite retained older header metadata.
- [Initial Android acceptance record](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/reviews/NR_LUNA_V73_ANDROID_ACCEPTANCE_PASS01_2026-09-29.md) — historical APK evidence, not V142 acceptance.
- [Camera controls and settings](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/CAMERA_PHOTO_VIDEO.md) — use later implementation notes when an older trailing statement conflicts.
- [Shader library and actual use](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/SHADERS.md) — separates compiled library members from active rendering.
- [Performance measurements](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/PERFORMANCE.md) — dated desktop/component results.
- [Cutscene research](https://github.com/VrUaCom/DMC-Native-Reader/blob/7f8caeacac9b168de3954ee15fa42beb01d61f15/docs/CUTSCENES.md) — scripts, companions, sound, movie and timeline details.
- [Native C++ implementation](https://github.com/VrUaCom/DMC-Native-Reader/tree/7f8caeacac9b168de3954ee15fa42beb01d61f15/app/src/main/cpp) — code remains the implementation authority.
