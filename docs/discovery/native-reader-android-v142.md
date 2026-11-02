# Complete Android guide — source and coverage review

Review date: 2026-11-02. Public route: `/native-reader/android/` (existing URL).

Implementation reference: `VrUaCom/DMC-Native-Reader`, `NR-Luna-v73`, exact HEAD `7f8caeacac9b168de3954ee15fa42beb01d61f15`, commit subject `v142: Next spot steps through the config's spawn points`.

## Supplied audit → published guide coverage

| Audit area | Guide chapters |
| --- | --- |
| Overall assessment / metrics | 1, 3, 6, 14, 17 |
| Architecture | 18 |
| Formats and modules | 3 |
| Viewer buttons / Info / File Health | 2, 4 |
| Stage Ops / five base rooms / POS / CAM | 5 |
| Characters, motions, effects | 6 |
| Camera entry / photo / video / optics / exposure | 8–11 |
| Framing / gestures / gyro / ARCore / Drone | 12–13 |
| GPU / shaders / lighting / performance | 14 |
| Cutscenes / sound / movies | 7 |
| Export / read-only product boundary | 15 |
| Regressions / tests / limits / result matrix | 16–17 |
| Remaining exhaustive-audit work | 17 |

All requested settings ranges, distinctive counts, original evidence limitations and two historical regressions are retained. Source clarifications are deliberate:

- The earlier `main` registry contains 16 initial module entries. V142 has 18 initial entries plus module-factory expansions; these are not interchangeable format counts.
- `STATUS.md`'s historical `41358b89...` review head and older header date do not identify the current V142 source.
- Next spot is a menu action; POS-first traversal and its floor fallback match V142.
- White balance exposes the documented presets within 2000–10000 K, not every intermediate value.
- V126 supplies later LIG2-source evidence after V124's uncertainty. Stage lights stays experimental; full original visual parity is not claimed.
- Photo motion blur is implemented in later V116 notes despite a stale trailing “not yet” sentence in the camera document.
- The 875 figure counts EXE spawn call sites; 151 counts JNI natives; 67 counts compiled shader-library members, not simultaneous draws or finished effects.
- The existing Downloads APK remains code72/1.0.45 at 7d1188e; no V142 binary promotion or device acceptance is implied.

## Validation scope

The guide Markdown is the sole full-content source for both GitHub and the generated page. An escaped, dependency-free renderer supplies accessible headings, chapter anchors, inline links/code, and scrollable tables. Invalid table structure and unsafe link schemes fail rather than silently losing content.

Checks include the four existing builder/catalog suites, guide-renderer safety and coverage tests, the full generated-public-surface workflow step, and browser QA of the chapter links and responsive widths 320/390/768/1024. Nineteen unique pinned repository file/directory link targets were checked against the exact source checkout. No new game assets, APK build, native CTest result or Android device pass is created by this website task.

Production acceptance adds this existing route as sample 32, after the prior 31 routes, preserving their established indices and the 113-page inventory. It checks the rendered manual, source SHA and final chapter on the live origin.
