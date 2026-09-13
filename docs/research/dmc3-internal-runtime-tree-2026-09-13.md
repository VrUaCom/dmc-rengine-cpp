# DMC3 — internal runtime tree, address graph and fresh EXE census

Date: 2026-09-13. Working branch: `reverse/mod-completion-20260907`.
Baseline: branch `e954e28` reconciled with `main@660cd29909863dac4f8980b12d070ac3afd3036f`.

## User requirement and scope correction

The requested tree is **the internal structure of Devil May Cry 3**, not the
GitHub directory layout and not an isolated MOD/SCM format map. Recover the
subsystems, classes, inheritance, subobjects, globals, registries, dispatch,
functions, resource paths and relationships connecting them. GitHub stores
the recovered structure and subsequent C++ reconstruction.

The target remains full game reconstruction on C++, editable and rebuildable,
with Android, iOS and other platform targets. C++20/23/26 are permitted when
needed. This pass adds C++20 research code. It neither introduces another
implementation language nor declares working mobile game ports.

Source directory names are not encoded by RTTI. The historical 00–19 folders
are a research organization of runtime domains, not proof of original Capcom
source paths. The real structure is a graph: inheritance, containment, calls,
dispatch and resource references are different relation kinds. Do not force
multiple inheritance or shared services into a single-parent folder tree.

## Exact artifact

- Input: 6,356,432-byte canonical `dmc3.exe`.
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.
- PE32+ AMD64, image base `0x140000000`, eight sections.
- SHA verified again in this pass; the input EXE is not committed.
- The 6,567,320-byte executable in Drive's Vanilla folder is a separate build.

## Machine-readable internal tree

Fresh, deterministic C++ outputs are in
[`data/reverse/runtime-tree-20260913`](../../data/reverse/runtime-tree-20260913).

| Output | Meaning |
| --- | --- |
| `class_tree.md` | Every hierarchy-linked class with its address and direct bases |
| `types.tsv` | 407 validated hierarchy-linked type descriptors, exact decorated names |
| `type_candidates.tsv` | 408 decorated-name candidates, with hierarchy membership |
| `inheritance.tsv` | 438 unique direct derived-to-base relations |
| `inheritance_evidence.tsv` | Each relation's CHD, preorder indices and base descriptor |
| `subobjects.tsv` | Base occurrences and raw PMD displacements in each most-derived context |
| `locators.tsv` | 915 complete object locators and secondary-subobject offsets |
| `vtable_anchors.tsv` | 915 vftable anchors, class identity, offset and first code entry |
| `runtime_ranges.tsv` | 12,235 unwind ranges, flags and CHAININFO targets |
| `entry_calls.tsv` | 19 manually boundary-reviewed direct calls from `dmc3_main` |
| `scene_dispatch.tsv` | Ten selector-to-case addresses from the scene factory jump table |
| `drive-reconciliation.json` | Exact comparison against the existing Inheritance tab |

Counts are structural observations, not completion percentages. A vtable
anchor does not prove the table's full extent or every method's signature.
The first entry may be a destructor, thunk or another method. The 12,235
unwind records include 4,846 CHAININFO ranges and 2,210 handler-bearing ranges;
they must not be called 12,235 independently recovered C++ functions.

## Five missing inheritance links recovered

The Knowledge Base `Inheritance!A1:D500` returns 433 distinct old relations.
All 433 are retained. Walking every validated flattened base array, including
relations inside base subtrees, adds exactly these five:

| Derived | Base | Derived TypeDescriptor VA | Base TypeDescriptor VA |
| --- | --- | --- | --- |
| `IMMNotificationClient@DMC3` | `IUnknown` | `0x1405D52D8` | `0x1405D5248` |
| `CItem` | `CWork` | `0x1405D6E78` | `0x1405D5410` |
| `CParticle` | `CEffectBase` | `0x1405D7700` | `0x1405D76D8` |
| `CRealParticle` | `CParticle` | `0x1405D7720` | `0x1405D7700` |
| `CFakeParticle` | `CParticle` | `0x1405D7748` | `0x1405D7700` |

Status: **EXE_CONFIRMED, structural inheritance only**. Exact CHD/BCD witnesses
are retained in `inheritance_evidence.tsv`; offsets remain scoped to their
most-derived hierarchy. They are not automatically standalone object sizes.
No virtual-base occurrence is observed in these validated hierarchies.

The 408-versus-407 distinction is also resolved: `.?AV_com_error@@` at
`0x1405D8EA8` is present as a decorated type candidate but is absent from the
validated COL-linked hierarchy set. This is not deletion or proof that the
type is unused. Exception-type ownership remains a separate investigation.

## Addressed runtime roots

These are research navigation paths. A semantic domain label does not assert
that the executable contains that source directory.

| Internal domain | Addressed nodes | Authority in this pass |
| --- | --- | --- |
| Entry / bootstrap | `dmc3_main 0x1402C5DF0` | Direct callsites reacquired |
| Memory | arena initializer `0x140030190`, callsite `0x1402C5E03` | Call reacquired; semantic name agrees with Drive |
| Scene factory | `CSceneFactoryApp`, vftable `0x1404C5FA0`, dispatch `0x140240090` | RTTI, vftable entry and jump table reacquired |
| Scene/global connection | `0x140C8F970` passed to `0x14004F120` and `0x14004F1E0` | RCX setup and calls at `0x1402C5E48` / `0x1402C5F9D` reacquired |
| Actor foundation | `CActor` derives from `CWork`, `IActor`, `ICollisionHandle` | RTTI reacquired; separate vftables at offsets 0, 96, 208 |
| Player | `CPlayer` → `CActor`; primary vftable `0x1404DF778` | RTTI reacquired; gameplay implementation not recovered by this relation |
| Nonplayer | `CNonPlayer` → `CActor`; primary vftable `0x1404DC5E8` | RTTI reacquired |
| Enemy factory | `CFactoryEnemy` → `IFactoryEnemy`; vftable `0x1404DBEC0` | RTTI reacquired; 46-selector dispatch remains a separate code trace |
| Item | `CItem` → `CWork`; `CItemKey` / `CItemOrb` → `CItem` | Corrected inheritance path reacquired |
| Effects / particles | `CEffectBase` → derived particle family | Four-level family linked by fresh RTTI witnesses |
| Application/save compatibility | `CMcAppli`, vftable `0x140508728`, first entry `0x1403295B0` | RTTI reacquired; detailed semantics remain owned by existing save research |

The scene factory bounds-checks selector 0..9 and reads the RVA jump table at
`0x1402402B8`. The case sequence matches the Drive map: Boot, Opening,
StartMenu, MisSelect, Game, GameMain, Demo, MisStart, Result, Ending. Those
names come from the existing reviewed factory map; the newly emitted table
records raw case addresses. This pass does not claim that scene transition
policy, constructors or all ten scene implementations are complete.

The reacquired entry code has distinct paths after testing global byte
`0x140CA8800`: one calls `0x140048970`; the other calls `0x140030530` and then
the `0x140D6D3D0` / `0x140C8F970` / `0x140CAC970` paths. Those callees need
their own body/ownership traces. Do not rename an unknown callee merely to
make the navigation tree look complete.

## Existing full domain organization to preserve

The Drive recovered-source tree already groups research into:

`00 Engine Foundation`, `01 Application Platform`, `02 Resource Runtime`,
`03 Scene Runtime`, `04 Rendering`, `05 Audio`, `06 Input`, `07 Camera`,
`08 Animation Constraints`, `09 Gameplay`, `10 Enemy Runtime`,
`11 Player Runtime`, `12 Stage Runtime`, `13 Item Runtime`,
`14 Effect Runtime`, `15 UI HUD`, `16 Save Progression`,
`17 Demo Cutscene`, `18 Steam Platform`, `19 Debug Crash Policy`.

Retain those domain roots and add evidence-linked class/function/global nodes
under them. RTTI establishes inheritance, not subsystem ownership: remaining
classification must follow constructors, global initialization, users and
dispatch paths rather than name prefixes alone.

## Reproduction and validation

Optional CMake target:

```sh
cmake -S . -B build/tree -DDMC_RENGINE_BUILD_REVERSE_TOOLS=ON
cmake --build build/tree --target dmc3-runtime-tree
build/tree/dmc3-runtime-tree --self-test
build/tree/dmc3-runtime-tree /path/to/canonical/dmc3.exe /path/to/output
```

This environment has GCC 13.3 but no `cmake` executable. Direct compilation
uses exactly the checked-in sources:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wconversion -Iinclude \
  tools/reverse/dmc3_runtime_tree.cpp src/core/sha256.cpp \
  src/exe/pe_image.cpp src/exe/pe_reader.cpp -o dmc3-runtime-tree
```

Self-tests cover nested and sibling inheritance, malformed descendant spans,
file-backed bounds including virtual-only tails, unanchored decorated names,
a complete positive RTTI/vftable chain, invalid COL self-RVA, oversized base
arrays and non-code vtable entries. Input size and SHA are checked before
writing any outputs. The CMake integration has not been executed here.

Validation results: GCC warning-enabled C++20 build PASS; self-tests PASS;
canonical image census PASS; a separate AddressSanitizer/UndefinedBehaviorSanitizer
build passes both self-tests and the full canonical census. All generated
files are byte-identical between the optimized and sanitized builds.
LeakSanitizer cannot inspect `/proc` under this execution environment, so
the successful sanitizer runs use `ASAN_OPTIONS=detect_leaks=0`; leak checking
is not claimed. No original-game runtime or Android/iOS acceptance was run.

## Remaining whole-tree work

1. Resolve every runtime range's fragment/CHAININFO relationship without
   equating an unwind family with a logical C++ function automatically.
2. Recover vtable extents, adjustor thunks, full virtual method sets and
   constructor/destructor object layouts; attach evidence to each relation.
3. Trace entry callees and the global initializer graph into the established
   subsystem roots. Recover singleton ownership and initialization order.
4. Extend factory/registry maps through enemy, scene, effect, input, audio,
   UI, resource and platform dispatch. Keep resource paths as their own graph.
5. Recover behavior in C++, validate it against the original, and replace
   platform dependencies through separately verified C++ implementations.

Full internal-tree semantic closure, full game decompilation and mobile
recompilation remain **incomplete**. No recovered method bodies or mobile
game binaries are claimed by this structural pass.

## Sources

- [Master Architecture](https://docs.google.com/document/d/10ctsZbqXQvJtJPaGaJ9dG5c294AX1p7r9q9x7yUSTdw/edit)
- [Address Map](https://docs.google.com/document/d/1KdmzRsvQVAefpmi54zqaDRtz5SfaHTPuwdFjs3Sih-0/edit)
- [Knowledge Base / Inheritance](https://docs.google.com/spreadsheets/d/1tmNY1O5b2dwYSRBzlPFtKndKBvRknmd1bUqkaTEahBU/edit)
- [Recovered C_CPP Source Tree](https://drive.google.com/drive/folders/1e6DJ8co7CnkzbWDgLyXY8FgEdyeJHMCI)
- [LLVM MicrosoftCXXABI.cpp](https://github.com/llvm/llvm-project/blob/main/clang/lib/CodeGen/MicrosoftCXXABI.cpp), RTTI record layout and descendant-count construction
- [Microsoft x64 exception handling](https://learn.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170), runtime range and chained unwind layout
