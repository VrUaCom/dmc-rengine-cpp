# Changelog

All notable changes to the clean C++ generation of DMC Rengine are documented here.

The project is pre-1.0 and may change APIs rapidly. Historical research is recorded in `docs/history/`, public Evidence Packets, and Drive authority records rather than presented as completed releases.

## [Unreleased]

### Added

#### Table extents corrected against absorbed string pools

- **blind spot closed.** The limitation recorded below is not a pool that resembles a table; it is a real table whose extent ran past its own end into the pool that follows it. A pool of short names is padded to the alignment the grid uses, and because its strings fall at a fixed sub-multiple of the stride the payload offsets stay consistent, so the pool rejection never fires;
- `StringTableScanner` now trims a run whose payload-bearing elements form a strict suffix containing at least one text payload. Payload from the first element is a record whose second field the constant-stride scan cannot see, and trimming that would delete a real table, so the rule requires a suffix. A head too short to be a table then fails the minimum and the run goes entirely: three names and a pool is evidence of a pool;
- `FogColor` at RVA 0x506F68 is corrected from 22 elements to 16 and `Maguma.ogg` at RVA 0x36F288 from 30 to 28; four runs are trimmed of 14 absorbed elements in total, giving 223 runs and 5,813 entries;
- trimmed elements are not consumed, so the tail is read again on its own terms: the tail of 0x506F68 comes back as a separate run at RVA 0x507068 with a stride of 8 and 11 entries, all 11 named by a constant index;
- **the check that found it:** a constant index is folded into its displacement, so the spacing of a table's references measures its element size independently of anything read from the bytes. Both corrected runs were indexed at a pitch of 8 under a declared stride of 16. Across the image, references the scan could only place inside an element fall from 16 to 7 while total references rise from 202 to 206; the 7 that remain sit at offsets of 13, 16 and 20, which is not the sub-multiple pitch of an absorbed pool.

#### One load or two: pointer members separated from subobjects

- a load out of the object at a non-zero offset is the vtable of what sits there only for an **embedded subobject**, whose vtable pointer is at its own offset zero. For a **pointer member** the value is an address and the vtable is a second load away. Counting the load depth separates them: of **1,134** dispatches in the `this` family, 352 read the vtable straight out of the object and **782 read it through a pointer**, 485 of those through a single offset — `+224`, a pointer at a fixed place in a widely shared base;
- a pointer member's vtable belongs to the pointee and the enclosing class's layout describes the pointer, so those sites are counted and **left unresolved** rather than resolved against the wrong class;
- **the distinction was nearly lost.** The field carrying the depth went into the wrong slot of an aggregate initialiser — setting the multiplier instead — so all 782 came out labelled as direct subobject loads. The jump from 352 to 1,134 with *zero* at depth two gave it away. Every construction now names its fields;
- **what a pointer field holds comes from a factory.** 187 stores of a returned value into the object exist and **not one callee is a constructor**. The commonest by far (RVA 0x2E7CA0, 86 stores) I first called an allocator, inferring it from the function's size and its lack of a vtable binding — **it is not one**. Its call sites pass a small constant and it returns an object filed away on the very next instruction: at RVA 0x8D361 the selector is computed with `lea ecx,[r9-7]`, and the result goes into `this+168` with no constructor in between. It takes four arguments and copies a 64-byte default transform out of `.rdata` when its third is null. The field's type is what that factory returns for that selector, so reaching it means reading the factory's body, not hunting a constructor that is not there;
- **a register-to-register move now carries whatever the source held**, not only the first argument it was built for. Indexed reads 1,963 → **1,966**, consistent array walks 531 → **534**, recovered arrays 278 → **281**, pointer stores 183 → **187**, code graph unchanged.

#### Calls on a subobject resolve through that subobject's vtable

- a load through the first argument at a **non-zero** offset is the vtable pointer of whatever sits there, since a subobject's vtable pointer is at its own offset zero. Reading the offset as well as the fact of the load takes dispatches on `this` from 175 to **352**, of which 177 are on something inside the object;
- where the offset is one the **RTTI** records for one of the class's own vtables, the receiver is a base subobject and the call goes into *that* vtable — not the class's primary one, which holds a different function at the same slot. Resolved dispatches rise 30 → **47**, 17 of them through a subobject;
- **the RTTI settles it without any constructor store**, which is what makes it work: a derived class inherits its layout from a base whose constructor did the storing, so the classes making these calls (`CEm025`, `CEm002`, `CEm007`) are not the classes whose constructors write the vtables. Waiting for a store would have resolved none of them;
- confirmed against the instructions: the `CEm025Shl00` method at RVA 0x116900 loads the vtable at `this+96`, passes `this+96` as the receiver's own first argument with a `lea`, and calls slot 29 — the adjusted `this` of a base-subobject call;
- `exe_class_field.member_vtable_rva` and `exe_resolved_dispatch.receiver_field_offset` carry this into SQL.

#### Class layout from constructor stores

- a constructor writes its class's vtable into the object at offset zero, which names the function without reading a symbol, and everything it writes at a non-zero offset names what sits there. The register analysis already knows which register holds the first argument and which holds an address a `lea` produced, so both fall out of it;
- **270 stores** of a taken address into the first argument, **265 of them a known vtable** — 98%, which is a check on the register following rather than a result of it. **146 constructors and destructors identified** by their offset-zero store;
- **44 typed fields recovered**, 15 of them embedded members of a different class. `CScene` holds a `CLightMgr` at +64, a `CGameW` at +88, a `CChain` at +112, a `CEventMission` at +592, a `CGameLoader` at +2688, a `CList<CNonPlayer>` at +3312, a second `CChain` at +4256, a `CLightStatic` at +7552 and a `CCameraMiniDemo` at +22832;
- **29 offsets confirmed twice over.** Where the vtable belongs to the constructor's own class the offset is a base subobject, and the RTTI records that vtable's subobject offset independently — an instruction the compiler emitted and a structure the compiler emitted saying the same thing. `CEm021`, `CEm029Cart` and `CNonPlayer` carry bases at +96, +208, +272, +384 and +592 on that footing;
- `FunctionFacts.constructs_class` names the class each constructor builds; `exe_class_field` and `v_exe_class_layout` carry the layout into SQL. Offsets and type names only — no field contents.

#### Register analysis as block dataflow

- the register analysis now runs as its own pass over the instructions the walk decoded, with a state per instruction and a **meet at every join** — a fact survives only when every path into the point agrees on it. That replaces a single pass whose facts depended on the order the traces happened to run. The walk is untouched and the code graph comes out **byte-identical**, which is the check that says so;
- indexed reads with a nameable base 1,524 → **1,963**, consistent array walks 368 → **531**, recovered arrays 190 → **278**, dispatches on `this` 105 → **175**, resolved dispatches 19 → **30**;
- **the earlier reading was wrong about the ceiling.** Dataflow was said to be what stood between 105 `this` dispatches and the 2,487 displacements in single-vtable methods. It lifted the figure by two thirds, and the rest is not an analysis problem: the method at RVA 0x2A9A0 reads a container out of a field of `this`, takes an element pointer from it and dispatches on the element, whose contents are built at run time. Most sites are that shape, so the ceiling was never reachable by following `this`;
- tests pin the meet in both directions: a fact two paths disagree on does not survive the join, one they agree on does.

#### Virtual dispatch: a census, and the receivers that can be named

- recording each call through a register base rather than only its displacement gives **10,274 dispatch sites** where 3,410 distinct displacements were known;
- **no receiver is statically known.** The one shape that would resolve — a global object whose vtable pointer is baked into the image — occurs zero times. Receivers arrive as arguments or through pointers loaded from writable data, so whole-image devirtualisation by reading the file is not available. That is a property of the programme, not a shortfall of the walk;
- **`this` is knowable without analysing what any pointer holds.** The Microsoft x64 convention puts the first argument in rcx, so a load through rcx — or through the saved register a method copies it into at entry — is the object's vtable pointer. The enclosing method's own binding says which vtable, the displacement says which slot, and the target is read out of the vtable. 105 sites dispatch on `this`, 98 inside a method the RTTI binds, and **19 in a method bound into exactly one vtable**, which is what makes the class unambiguous; those resolve to a class, slot and target across 15 classes. A method bound into several vtables is inherited and is left unresolved rather than attributed to the first class that fits;
- only 19 because the walk gives up its register state at every trace root, since a block reached by a branch has a state depending on which predecessor ran. Block-level dataflow with a merge at join points is what would lift it;
- `exe_resolved_dispatch` and `v_exe_virtual_call_edge` carry the resolved calls into SQL.

#### The calling convention is evidence, not an assumption

- the walk gave up every register at a call, on the grounds that which ones survive was a claim it had no right to make. It is not a claim: the Microsoft x64 convention names rax, rcx, rdx and r8–r11 volatile and the rest preserved, and compiler-generated code follows it. Forgetting only the volatile ones lifts recall everywhere the register state is used — indexed reads with a nameable base 1,357 → **1,524**, consistent array walks 304 → **368**, recovered arrays 178 → **190**. A test pins both halves: a base in rcx is lost across a call, a base in rbx is not.

#### Image-base reads: a layout not invented

- 778 indexed reads reach their array against the image base, where the array's start is folded into the displacement, so one read cannot separate base from field. Grouping reads whose addresses fall within one element of each other yields 18 arrays and **is unsound**: grouping instead by what is actually shared — one function, one index register, one element size — shows that **53 of the 60** multi-read groups span more than one element, meaning the register was reused for a different array. Proximity would have merged them and invented a layout for each;
- the 7 surviving groups are recorded with `base_measured = false`: the array may begin before the lowest address observed, so that address bounds the base from above and only the offsets between reads are measured. `IndexedAccess` now carries the index register, which is what makes this measured rather than guessed;
- **two of the seven corroborate**: they reproduce a base and element size a register was separately seen holding, so they merge into the measured entry rather than becoming a second array. 178 arrays over 173 bases, 5 of them with a conflicting element size;
- the conflict counter now compares element *sizes* rather than counting entries, so a base reached by both routes at the same element size reads as agreement, which it is.

#### Indexed data arrays, and element sizes above eight

- a SIB scale encodes only 1, 2, 4 and 8, so any other element size is carried in the **index**: `lea reg,[a+a*k]` multiplies by k+1, `imul` by its constant, a shift by a power of two. The image holds 933 of the `lea` form alone. `X86LengthDecoder` now also reports the immediate's width and sign-extended value, and the walk carries the multiplier through to the read — 40 reads then show element sizes of 6, 10, 12, 20, 24, 40 and 72 bytes that no scale field could express. The weak half of the previous finding is now measured: **no name run is indexed at any stride**, not only at 8;
- **173 indexed arrays recovered** over 168 bases, with their element size and the field offsets the code reads inside them. On a held base the displacement is a field offset, so it must land inside the element — a free consistency check that splits 579 reads into 304 consistent array walks, 259 inlined character scans (a negative offset), and **16 at or past the element**, a 2.8% error bar stated rather than hidden;
- **the failures named the missing instruction.** That third figure started at 32, and those sites were not noise: the function at RVA 0x274730 multiplies its index by five with a `lea`, doubles it with `add rbx,rbx`, then reads at a scale of eight — eighty bytes. The doubling was unmodelled, so the element read as 40 and every offset from +8 to +28 fell outside it. Adding the self-add form and the shift form took inconsistent reads from 32 to 16 and consistent walks from 267 to 304; `0x597150` now reads as an 80-byte element with eight 4-byte fields. The check flagged the gap and the gap named the instruction to add;
- `X86LengthDecoder` also reports the ModRM rm operand with REX.B applied, which the shift and self-add forms need to say which register they write — for a shift the reg field is the group selector and names no register at all;
- 28 arrays are read at more than one offset and so carry a partial element layout: `0x597150` is an 80-byte element at +0…+28, `0x5D08A0` a 20-byte element at +4 +8 +12 +16, `0x5DE5B0` a 24-byte element at +0 +8 +16. Layout only — where an array is, how wide its element is, which offsets are read — never contents;
- **5 of 168 bases are read at two element sizes**, which cannot both be right. The count is reported rather than a winner picked, since nothing in the encodings says which;
- `exe_indexed_array` and `exe_indexed_array_field` carry this into SQL, with `v_exe_array_layout` for the arrays whose layout is more than a single observation. The importer refuses a field offset outside its element rather than storing a layout that contradicts itself.

#### Following a table base through a register

- `X86LengthDecoder` surfaces the addressing fields it already parsed: REX.R/X/B-extended register numbers for the ModRM reg operand and the SIB base and index, and the scale. No length logic changed. Three encodings needed care — a SIB index of 4 without REX.X is *no index* (objdump's `riz`) while with REX.X it is r12, and a SIB base of 5 under mod 0 is a bare disp32 with no base. Cross-validated against objdump over every indexed memory operand in the image: **16,503 compared, 16,503 agree**;
- `CodeGraphBuilder` remembers what a RIP-relative `lea` put in a register and gives it up on any instruction naming that register, on every call, and after 64 instructions. Reads invalidate as readily as writes: the decoder models no mnemonics, so the honest response is to claim less and let validation on the other side carry the weight;
- **1,303 indexed reads** have a base the walk can name, and **749 hold the image base** — MSVC's `lea r13,[__ImageBase]` followed by a read at a displacement, the same construct switch recovery depended on, now readable for data tables generally;
- **no recovered name run or record is indexed as an array.** Zero of the 1,303 reach one at its own element size. The four whose base register holds a run's base exactly are all the inlined character scan `[base + index*1 - 1]` over the *first* name — over a literal. The runs are a layout fact about how the linker packed these literals, not an access fact about how the game reads them, and the fixed stride is the packer's alignment rather than an array's element size;
- the evidence is strong at stride 8, where a scale of 8 is the natural encoding and 211 such reads exist elsewhere with none naming a run; weaker above 8, where a SIB scale cannot express the stride and the element address is computed first, which this walk does not follow. Recall is a floor, not a census: objdump counts 16,559 indexed operands and the walk names a base for 1,303;
- `NameTableUsage.indexed_sites` and the summary's `indexed_accesses`, `image_base_indexed_accesses` and `indexed_table_accesses` carry the measurement; `exe_name_table.indexed_sites` carries it into SQL.

#### Pool strings that cross an element boundary, and folded empty strings

- **the fifth pool mode.** Names packed to an eight-byte boundary whose lengths all round up to the same figure produce a flawless fixed-stride run with nothing but NUL padding in every element. The first thing that breaks the pattern is a name rounding up to less: it begins inside the element and runs out through its far edge, so its terminator falls outside — and the text-payload test required the payload to terminate *inside* the element, excluding precisely the case it most needed. A field of a record always terminates within its element; a pool string crossing the edge never does. Accepting it takes trimmed runs from 4 to 73 and absorbed elements from 14 to 83;
- **a reference into padding is an empty string.** Four functions load one address 213 bytes into record 15 of the layout at RVA 0x36B308 — not a field offset, but the second NUL in a field's padding — and pass it to a `"%s%s%s%s"` call. The linker folds `""` into any NUL byte in the image, and a name table's padding is nothing but NUL bytes. Rejecting an address whose byte is NUL takes references that could only be placed inside an element from 4 to **0**: every reference into a recovered table now lands on an element or a field;
- **caveat on constant indices.** A reference whose offset is a whole multiple of a run's stride was read as a constant index folded into the displacement. It is equally the shape of code loading one literal at that offset, and the instruction cannot tell them apart. The run at RVA 0x506C38 settles its own case: the function at RVA 0x2D83E0 reaches all eight elements through eight separate `lea` instructions at eight different addresses and never holds the base — eight literals, not a table of eight. Coverage by constant index is evidence that code reaches those bytes, not that it indexes them;
- no reference was lost to any of this: `string_references` rose by exactly the 13 the tables gave up.

#### Record interiors reconciled against the stride scan

- a record whose name fields are all one width contains a constant-stride grid by construction, so both scans describe the same bytes and both report them. 179 of 223 runs coincide with a block of equal-width fields in one of the 25 recovered record layouts, leaving 44 that stand on their own; the 264-byte record at RVA 0x35D640 alone was reported as 20 separate tables, one per record, being fifteen 16-byte name fields followed by one of 24;
- interiors are marked rather than deleted — the record is the fuller reading of the same bytes, and counting both as tables counts the bytes twice;
- **a grid does not stop at the end of a field block.** Nothing halts a constant-stride walk at a field of a different width whose name terminates inside the walk's stride, so 87 runs overrun their block by 270 elements in total: eleven runs claim eight 40-byte elements where the localisation record's block is seven fields, and the eighth is the next record's 32-byte field read as though it were 40. The record's widths repeat with a measured period and are the stronger reading, so the run is cut back;
- a block shorter than the minimum entry count is kept. The minimum exists to stop a stride hypothesis being reported on a few coincidences; a seven-field block two scans agree on is not a hypothesis. Erasing them was the first thing this pass got wrong;
- **the references caught it.** `FunctionMapBuilder` checked only the span with the nearest preceding base, so a cut-back interior shadowed the record enclosing it and 14 references vanished. Considering every span that can contain an address and preferring the record reading restores all 206 references and moves 18 from a grid position to a record and a field, taking record-layout references from 8 to 26. The count coming back exactly is the check that reconciliation removed duplication rather than evidence.

#### Research SQLite: executable reverse layer

- `research/sql/007_executable_reverse.sql` indexes the `analyze-exe` and `map-functions` reports across 10 tables and 3 views, so cross-cutting questions — which functions read a given table, call a maths import and are bound to a class — are a query rather than a walk over a two-megabyte document;
- `v_exe_table_coverage` exposes `interior_references` against each run's declared stride, which is the extent check above as a standing query rather than a one-off; `v_exe_independent_table` excludes each record's interior, so tables are not counted twice;
- `extent_status` records how far each run's extent is settled: `STRUCTURAL_CONFIRMED`, `RECORD_INTERIOR`, `EXTENT_TRIMMED`, `SEMANTIC_CANDIDATE`, or `EXTENT_UNCLASSIFIED` for a run known only through the function map;
- the importer refuses reports whose artifact SHA-256 differ and never invents a table to hang a reference on; a run the analysis report's capped list omits is created from the map's own layout facts, with payload measurements left unknown rather than guessed, so all 206 references land;
- no function carries a recovered name: `exe_function.recovered_name` is NULL throughout with `name_evidence_status='PRESERVED_UNDECODED'`;
- the database is generated locally and is not committed. Importer guardrails run in CI.

#### Constant table indices

- each table reference now resolves its element: a constant index is folded into the displacement, so an offset that is a whole multiple of the element size names one element outright, and for a record layout the remainder picks the field. 186 of 202 references are constant, 16 computed;
- per-table coverage reports how many elements are named from code; three pure name arrays are fully or nearly fully covered (9/9, 12/12, 12/13);
- **limitation recorded:** the pool rejection has a blind spot. A pool whose strings share a length class produces consistent payload offsets and passes, which is how the `FogColor` run at RVA 0x506F68 survives and over-extends into an adjacent region of four- and five-character extension variants. Payload-offset consistency is evidence of a record only when string lengths vary, and coverage over a payload-bearing run is an upper bound.

#### Code-to-table linkage

- `FunctionMapBuilder` matches each function's RIP-relative data references against recovered name tables, reporting the table, the offset addressed within it, and per-table referrer counts: 45 functions linked to 27 tables, with 218 tables addressed in ways direct-reference matching cannot follow;
- **correction:** a third false-positive mode removed. The stride acceptance test weakens as the stride grows — at 384 bytes "a name terminated inside the field" constrains nothing, since a dense pool always terminates a name somewhere inside. An element's payload must now be all zero or text at a consistent offset, giving 220 runs and 5,785 entries in place of 222 and 5,692, and dropping linkage from 174 functions to 45;
- the phantom was caught by its own referrers: 118 of its 128 referring functions addressed one interior offset and only 3 a multiple of the stride, which makes reference-offset distribution an independent check on any recovered table.

#### Multi-field name record layouts

- `StringTableScanner` gains a gap-period pass that finds records whose name fields differ in width, which no constant-stride hypothesis can cover: 25 layouts holding 5,076 name fields, with 2, 4, 9, 12, 16 or 32 fields per record;
- the pass independently re-derives the 352-byte cutscene localisation record — same base, widths, extensions and record count as the hand analysis;
- a chosen period is reduced to the smallest divisor at which both field widths *and* extensions repeat; testing extensions alone would misreport the 16-field record at RVA 0x35FE68, whose extensions repeat every four fields but whose widths do not;
- payload after a name terminator is now reported as two measurements — text-likeness and offset consistency — instead of implying a record: inconsistent text payload is an alignment-padded pool, and two parameter-name pools were misreadable as records without it;
- runs also report the smallest period at which their extensions repeat.

#### Resource name tables

- `StringTableScanner` recovers runs of fixed-width NUL-padded name fields from read-only data, reporting base, stride, entry count, longest name and whether elements carry payload after the terminator; only one representative name per run is retained, since table contents are game data;
- elements must begin immediately after a terminator — without that constraint a stride locks onto a phantom grid that slices through real names and marches across unrelated tables; a wandering-spacing fixture guards it;
- `analyze-exe` gains a `name_tables` section and a `--no-name-tables` switch;
- on the canonical target: 222 runs holding 5,692 entries, largest 1,747 entries at stride 24;
- 352-byte cutscene localisation record resolved exactly — 32-byte archive name plus eight 40-byte per-language message names, no residue, validated across all 49 records;
- extension census adds ADX, OGG, SFD, FXH and TM2 to `resource_family_hints`; SFD corroborates the FMV subsystem alongside the RTTI `FullMotionVideo` classes and the Media Foundation imports.

#### Construction sites, dispatch census and semantic anchors

- functions referencing a recovered class vtable are recorded as construction sites — 710 on the canonical target, led by `CWork` (75), `CConstraint` (55) and `CPlayerWeapon` (48);
- indirect dispatch census: displacements of `call [reg + disp]` sites collected per function and aggregated — 10,958 sites across 2,070 functions over 145 distinct offsets;
- `resource_family_hints` classifies a literal's text against the documented resource families, with per-function families and a repository-wide census;
- measured why literal attribution is narrow in this image: `.pac` names are a packed table (median gap 24 bytes, 99.1 percent under 64) and `.hlsl` paths sit inside DXBC bytecode, so names are data-table content rather than code constants;
- three candidate resource-resolution functions recorded at `high` confidence with their supporting observations stated separately: NBZ volume-path construction, PTX extension matching, and MOT/CLT extension matching — the last being evidence that CLT shares MOT's path.

#### Switch dispatch and prologue recovery

- switch-table recovery behind register-indirect jumps: candidate bases from RIP-relative `lea` targets *and* non-RIP 32-bit displacements, since a compiler holding the image base in a register reaches tables through `base + disp32`; a candidate is accepted only when consecutive entries land inside the same function, and both offset readings are tried;
- 637 tables and 6,867 block addresses recovered on the canonical target, lifting walk coverage from 85.8 to **98.6 percent** of the unwind extent and cutting functions with no structural referrer from 2,101 to 848;
- unwind code decoding: per-function prologue size, stack reservation, pushed/saved register counts, frame-pointer register and handler flags, with an unrecognised operation stopping the walk rather than desynchronising it;
- decoder now reports displacement width and operand shape (`register_indirect`, `rip_relative_lea`);
- **corrections:** the code-graph and reachability evidence records are superseded with post-switch figures; `is_leaf_frame` no longer treats a function that establishes a frame pointer as a leaf.

#### Function-level reverse

- `X86LengthDecoder`: bounded, fail-closed x86-64 instruction length decoder reporting control-flow role, RIP-relative displacement and direct branch displacement, cross-validated against GNU objdump at 875,067 in-function positions with 99.93 percent agreement;
- `CodeGraphBuilder`: recursive-descent walks that fold `UNW_FLAG_CHAININFO` continuation ranges into the function that owns them, and never enter the switch jump tables MSVC embeds inside function extents;
- `FunctionMapBuilder`: joins the code graph against the import, export and RTTI tables — callers/callees, reachability, vtable slot bindings, imported symbols called (including through unwind-less thunks) and referenced literals;
- `dmc-rengine map-functions` emitting a deterministic JSON function map;
- **correction:** the canonical target's 12,235 exception-directory entries resolve to 7,389 functions plus 4,846 continuation ranges; the evidence record supersedes the earlier inventory claim;
- function-level results recorded as evidence: 593,458 instructions, 26,322 call edges, 2,412 attributed functions, 3,588 of 13,894 vtable slots bound to code.

#### Executable structural reverse

- `PeReader` extended with COFF timestamp/characteristics, DLL characteristics, alignments, checksum, code/data sizes and the data-directory array;
- `PeDirectoryReader`: imports (delay imports and ordinal-only entries included), exports with forwarder detection, the x64 exception directory as a function inventory, the debug directory with CodeView GUID/age/PDB identity, base relocations, TLS with its callback array, and the resource tree — each recovered independently so one malformed table does not cost the others;
- `RttiScanner`: MSVC type descriptors, complete-object locators, class hierarchy and base-class descriptors, and vtables, grouped by type with per-subobject vtable offsets; locators are accepted only when their self-reference matches, and MSVC decorated names are reconstructed with an explicit completeness flag;
- `dmc-rengine analyze-exe` emitting a deterministic JSON analysis report;
- synthetic PE32+ fixtures covering every directory, the RTTI hierarchy and their failure paths;
- structural reverse of the canonical DMC3 HD target recorded as evidence: build identity, 12,235 unwind-backed function ranges over 89.1 percent of `.text`, the 26-module import surface, the `dmc3_main` export, and 396 polymorphic types across 915 vtables.

#### Runtime host layer

- new responsibility boundary: platform, fixed-step frame loop and rendering device abstraction for playable targets (Specification 010);
- `IPlatform` with `HeadlessPlatform` (deterministic, display-free) and `AndroidPlatform` (lifecycle- and surface-aware);
- `FrameClock` fixed-step accumulator with a bounded catch-up budget that drops a stall instead of replaying it;
- `IRenderDevice` abstraction, implemented `NullRenderDevice` reference backend, and a `RenderBackendRegistry` in which declared-but-unimplemented backends fail closed rather than substituting `null`;
- `ResourceBridge`: the layer's only door to resource bytes, over `SourceRegistry`, with residency accounting and typed failures;
- `StageHost`: runtime mirror of a Stage Ops `StageBundle` that projects a deterministic draw list and never synthesizes geometry;
- `RuntimeApplication` loop handling surface loss/recreation, suspend/resume and focus throttling;
- separate `DMCRengine::Runtime` target on C++23 by default, selectable across C++20/23/26 with automatic step-down, keeping the core library on its C++20 baseline;
- `std::expected` and `std::move_only_function` used where the toolchain provides them, with API-identical fallbacks otherwise;
- `dmc-rengine-runtime-probe` host tool and 7 runtime test suites, green in both the standard and fallback builds;
- Android Gradle project, JNI bridge and Java shell (minSdk 26, targetSdk 36, `arm64-v8a` + `x86_64`);
- `Runtime` CI workflow covering C++20/23 on Ubuntu and Windows, a forward-looking C++26 job, and an Android APK job.

#### Core and build

- C++20/CMake core library and CLI;
- Windows/Ubuntu CI validation;
- CMake presets, warning policy, formatting, and editor configuration;
- compiler-level `/UNDEBUG` / `-UNDEBUG` test invariant;
- SHA-256 implementation and known-vector tests;
- bounds-checked binary reader;
- expanded CTest integration stack, reaching 68 validated tests per platform after Binary Inspector Cross-Port Wave 1.
- optional CLI (`DMC_RENGINE_BUILD_CLI`) and runtime (`DMC_RENGINE_BUILD_RUNTIME`) targets so the Android NDK build excludes the desktop tool and the CTest suite.

#### Evidence and Canon

- confidence model, locations, records, tags, supersession, and `EvidenceRegistry`;
- `ArtifactIdentity` and versioned `EvidencePacket`;
- deterministic Evidence Packet JSON export;
- strict untrusted Evidence Packet JSON import;
- parser size/depth/count limits, duplicate-key and duplicate-ID rejection, and cross-reference validation;
- CLI `validate-evidence`;
- public packets for the canonical DMC3 executable, Item runtime, and PC-save Pass 31/32 findings;
- Drive/GitHub reverse-authority registry and implementation receipts.

#### GDSpaces and integration

- `ResourceId`, `ResourceRef`, `ResourcePayload`, and diagnostics;
- safe read-only `LocalDirectorySource` with root-containment protection;
- `SourceRegistry`, `ResourceGraph`, `OpenRouter`, and game profiles;
- centralized path/extension/magic classification and post-read correction;
- typed `StageBundle` and deterministic `StageBundleAssembler`;
- generic read-only container contracts, parser registry, synthetic slot-container fixtures, stable child identity, empty-slot preservation, diagnostics, and graph edges;
- revisioned `WorkingCopy` with expected-byte edits, variable-size replacement, history, reset, and undo;
- Project Workspace, Project Graph, append-only workspace events, and deterministic manifests;
- canonical tool and format capability registries;
- shared Stage Ops/ModViz stage views.

#### Binary Inspector domain

- overflow-safe `ByteRange`;
- structural regions and kinds;
- typed fields and parent-child structures;
- ownership claims;
- annotations and Evidence links;
- owner, field, and annotation selection context;
- selected-range overlap context across regions, fields, ownership, and annotations;
- union coverage, unknown gaps, structural conflicts, and ownership conflicts;
- deterministic metadata manifests;
- format adapters including the canonical HITS model;
- deterministic offset-aligned byte diff with equal, modified, inserted, and removed spans;
- byte-diff summary counters and stable left/right ranges;
- Shannon entropy maps with configurable windows and step size;
- entropy-window zero ratio, unique-byte count, and visualization bands;
- explicit heuristic boundaries for entropy and non-resynchronizing diff behavior;
- web-to-C++20 Binary Inspector capability parity matrix and staged cross-port roadmap.

#### EXE and patching

- generic read-only PE32/PE32+ parser;
- checked file offset, RVA, and VA conversions;
- PE section/range diagnostics;
- known executable target model and DMC3 Phase 12 registry;
- target recognition by SHA-256 and PE metadata;
- Evidence Address Resolver and executable workspace manifests;
- `GuardedPatchPlan` with source hash, expected bytes, ranges, overlap, and atomicity checks;
- evidence-gated patch-plan compilation;
- copied-output in-memory patch execution;
- output SHA-256 and verified rollback plans;
- manifests proving that the original file was not written.

#### Stage and formats

- DMC3 110 × 4 stage-table descriptor;
- `st001` role plan, path normalization, and resource matching;
- `DMC3StageWorkspaceBuilder` and Stage Workspace manifests;
- compiled modules and tests for HITS, DCA, LIG2, and Stage TXT;
- Resource Analyzer integration.

#### Item and Trial Chamber

- Item Workspace and Item runtime Evidence Packet;
- runtime requests, graph nodes, events, and manifests;
- validation plans and requirements;
- evidence-gated Item runtime patch compilation;
- guarded copied-output and rollback provenance.

#### Source integration and custom builds

- `SourceModificationPackage`;
- `IntegrationProject` state and dependency/conflict graph;
- deterministic source-integration manifests;
- `CustomBuildIdentity` and `CustomBuildRecord`;
- compiler, linker, target, flags, dependency-lock, and recovered-source identity;
- source-unit/source-line/recovered-symbol mappings to output offsets, RVAs, and VAs;
- test, release, attestation, revocation, and rollback gates;
- EXE reopen lineage by executable SHA-256.

#### HITS

- correction of the obsolete `HITS$`/fixed-marker model;
- header-driven `HITS` parser;
- exact `0x38` triangle-plane records;
- spatial grid and signed `-1`-terminated reference lists;
- source 0/member 3 and source 1/member 6 identity;
- Binary Inspector semantic adapter;
- runtime-derived grid conversion, flattening, broadphase, deduplication, and reject-mask behavior;
- candidate and contact result contracts;
- topology-preserving safe editing;
- normal and plane-D recomputation;
- deterministic DMC Rengine SAT spatial writer;
- canonical parser/writer round trips and stable surface identity;
- spatial corpus differential validator;
- deterministic per-cell/per-surface JSON reports and precision/recall/Jaccard metrics;
- GDSpaces-backed `compare-hits-spatial` CLI with SHA-256 identities and unique bit-exact geometry mapping across reorder.

#### DMC3 PC save

- exact `0x4A30` file model;
- 21 integrity envelopes;
- global, summary, and detailed-payload record layouts;
- four-byte `recordState + checksum` trailers;
- one's-complement end-around-carry checksum validation and generation;
- packed-BCD date/time handling;
- rejection of the former standalone `0x28` block interpretation;
- conservative open-semantic boundaries;
- Pass 31 and Pass 32 Evidence Packets, tests, CI, and Drive receipts.

#### CLI

- `version`;
- `doctor`;
- `scan`;
- `hash`;
- `validate-evidence`;
- `route`;
- `inspect-exe`;
- `list-tools`;
- `list-formats`;
- `integration-status`;
- `inspect-workspace`;
- `compare-hits-spatial`.

#### Process and documentation

- MIT license;
- governance, maintainer, contribution, security, support, conduct, and clean-room policies;
- issue and pull-request templates;
- DMC Rengine Constitution;
- SDD specifications and ADR system;
- architecture, phase map, blockers, risks, JSON status, history, and Canon documents;
- public brand Canon defining the Sect of Neuroslop as the DMC Rengine community, the Monks of Binary Code as creators and recognized core contributors, and the Order of the Inverted Triangle as the core Team alias;
- public Long Descent, Monastery, chamber, ritual, campaign, and evidence-presentation vocabulary;
- GitHub implementation truth separated from newer Drive research truth;
- Binary Inspector Web → C++20 cross-port rules, parity tracking, and Wave 1–4 plan;
- current status and machine-readable state reconciled after PR #47.

### Fixed

- corrected Evidence JSON escaped-newline test expectation;
- fixed Windows Release test crashes caused by `NDEBUG` removing side-effectful `assert` expressions;
- rejected an unreliable forced-include assertion workaround and standardized `/UNDEBUG` / `-UNDEBUG` for test targets;
- removed stale status claims that strict Evidence import, Binary Inspector fields, guarded copy execution, source integration, HITS runtime/writer work, and PC-save Pass 31/32 were still planned;
- documented that `HITS$` and `0x18060001` as a universal record marker are rejected historical assumptions;
- removed the obsolete public-lore model that treated the Sect as an AI-only inner wing and used `Monks of Reverse` as the contributor identity;
- removed the new Binary Inspector aggregate-initialization warning by fully initializing `ByteDiffResult`;
- corrected status documentation that still described Binary Inspector Diff and entropy analysis as unimplemented after Wave 1.

### Research boundaries

- production PAC/PNST/NBZ/AFS source expansion remains incomplete;
- the first game-backed `st001` StageBundle remains open;
- HITS Capcom offline-builder equivalence is not confirmed;
- Binary Inspector diff is currently offset-aligned and not structure-aware or resynchronizing;
- Binary Inspector Analysis Cache, generic diagnostics, unknown-region analysis, templates, EXE bridges, and native UI remain open;
- Wide Pass 33 remains research-ready and product-promotion-pending;
- full DMC3 decompilation and a working rebuilt executable are not complete.

## [0.1.0] — 2026-08-02

### Added

- initial C++20/CMake foundation;
- minimal CLI;
- initial `ResourceId` model;
- cross-platform build workflow;
- initial architecture, roadmap, reverse-engineering rules, and README;
- proprietary-data exclusions.
