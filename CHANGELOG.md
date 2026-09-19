# Changelog

All notable changes to the clean C++ generation of DMC Rengine are documented here.

The project is pre-1.0 and may change APIs rapidly. Historical research is recorded in `docs/history/`, public Evidence Packets, and Drive authority records rather than presented as completed releases.

## [Unreleased]

### Added

#### It is zlib 1.2.5 in raw mode, and the layout I called non-standard is standard

- **the library names itself.** `0x327BE0` clears `0x58` bytes of descriptor, loads **`-15`** into `edx`, the address `0x5086F8` into `r8` and `0x58` into `r9d`, and calls `0x343170`. The bytes at `0x5086F8` are `"1.2.5"`. That is `inflateInit2_(strm, -15, "1.2.5", 0x58)`, and **negative window bits select raw deflate with no wrapper** — so the project's existing "raw DEFLATE" description is now backed by the constant that selects it rather than by the shape of the stream. `.rdata` agrees: `"deflate 1.2.5"` at `0x50AA51`, `"inflate 1.2.5"` at `0x50BEA1`, and the standard message set. Both halves are compiled in; only inflate is reached from the archive path, and what uses deflate is unread;
- with the version fixed, four functions identify themselves: `0x343170` `inflateInit2_`, `0x341900` `inflate`, `0x343110` `inflateEnd`, `0x340010` `crc32`. `inflateEnd` is the clearest — state from `strm+0x28`, deallocator from `strm+0x38`, free the window at `state+0x38` then the state, null `strm+0x28`, `-2` on a null argument;
- **the descriptor I called non-standard is the standard one, computed for the wrong data model.** Yesterday's note said the library "puts a total-in counter between available-in and next-out, and this does not." It does — as a `uLong`, which on **Windows x64 is four bytes**, so it occupies `0x0C` inside what would be padding and `next_out` lands at `0x10`. Every offset then follows, and `0x58` is exactly the size the code passes. Stock `z_stream`. I computed LP64 offsets and reported the mismatch as a property of the image — the kind of error that produces a confident **negative** rather than an undercount;
- **the inflater object is exact: `calloc(1, 0x1080)`**, substream at `0x00`, uncompressed length at `0x08`, produced count at `0x0C`, a `0x1000` input buffer at `0x10`, the `0x58` descriptor at `0x1010`, the exhausted flag at `0x1068`, cursor and end at `0x1070`/`0x1078` — reaching `0x1080` with nothing left over, and the cleared `0x58` is precisely the descriptor with the flag beginning where it ends. The arming also rewinds, and sets the exhausted flag from `length == 0`, so **a member of zero uncompressed length is born exhausted**;
- **exactly one of `+0x38` and `+0x40` is ever set**, enforced not assumed: success nulls `+0x38` and moves the substream into the inflater, failure frees it and nulls it too. That is what makes the destructor's two independent frees safe. **I suspected a leak and there is none** — `0x343110` is `inflateEnd`, the seek calls it then re-initialises, the destructor calls it then frees;
- **and `0x327DB0` is the destructor, not the constructor** as yesterday's note guessed. The constructor is a branch of `0x328360` that the same note had already read for its stored case without noticing it holds the other.

#### Seeking a deflated member re-inflates it from zero, truncation reads as a clean end, and nothing checks the CRC

- `proof-roadmap-2026-09-05.md` marks the NBZ STORE/raw-DEFLATE reader with a warning and names what would close it: `0x140328540`, `0x140328FE0`, and malformed/partial-read semantics. `0x328540` was read earlier the same day; **this reads the other two**. Whether that closes the item is the project's call, not mine;
- **any seek that is not to the current position or to the end re-inflates the member from byte zero.** `0x328FE0` handles origins 0/1/2, returns immediately when the target already equals the current position, clamps to `[0, length]`, and when the target equals the length just sets the exhausted flag and decompresses nothing. Everything else resets the decompressor, re-seeks the compressed substream to the member's start and **discards exactly `target` bytes** in `0x2000` chunks. Cost is linear in the target and **there is no forward path** — one byte ahead of the current position still starts from zero. A port adding the obvious optimisation changes the timing profile of every seek, and since the discard loop drives inflate through a *different* function than the read path, it is not guaranteed to reproduce the same byte counts on a damaged member;
- **truncation is reported as a clean end of stream.** The refill clamps to the compressed bytes remaining; when none remain it sets input cursor and input end equal, writes one zero byte and calls inflate anyway. The step returns `-5`, and the code converts that to **stream end** when the input buffer is empty and to an error (`-1`) when it is not. So a member whose compressed bytes run out short yields a **short read and no error**, and the caller cannot tell it from a member that was genuinely that long. After the end, every further read returns zero rather than failing;
- **two inflate drivers over one object.** The inflate step at `0x341900` has exactly two call sites in the whole image — inside `0x328680` (the seek's discard loop) and inside `0x328820` (the read path, which also takes a flag word in `r9d`). Same field offsets, different drivers;
- **nothing verifies a member against its directory record.** The CRC-32 table at `.rdata 0x508A50` and the routine at `0x340010` are live with **13 call sites, all inside the compression library**; none is in the archive path, and the CRC the decoders store at entry `+0x18` is never read — grepping the six entry-handling functions for displacement `0x18` returns only register spills. Combined with truncation reading as a clean end, **the read path has no integrity check of any kind**;
- **a near-miss worth recording.** The first pass asked who calls `0x340018` and `0x340020`, got zero and one, and was one step from publishing that the CRC machinery is unreachable dead code. It isn't: `0x340018` is a five-byte `jmp` and the real entry is `0x340010`, eight bytes earlier, with 13 callers and **no `.pdata` entry**. Fourth time in one day that `.pdata` hid something — and the first where the consequence would have been an assertion of *absence*. The habit that caught it costs one command: before trusting a caller count of zero, disassemble the bytes immediately before the address.

#### A two-byte function, seventeen vtables, and a limit on our own instruments

- I went to read `0x0629C0` because the correction note had called it out — read and seek keep their archive branch inside the file layer while the size dispatcher jumps outside it — and I had written that "whatever computes an archive entry's size does not live with the rest of the archive code." **That sentence implied an architectural fact and there is none.** `0x0629C0` is `mov eax,[rcx+0x20]; ret`: two bytes of body. Against the entry layout that offset is the uncompressed size, so the jump is semantically exact;
- **and the same two bytes are a vtable slot in 17 classes.** Searching the image for its address as a qword finds it in 17 `.rdata` tables; two dumped in context show an `.rdata` complete-object locator followed by a run of `.text` pointers, agreeing on **every slot but the first**. It cannot be the same thing in both places: the archive entry is `calloc(1, 0x50)` whose offset zero holds a `_strdup` of the archive path, so it has **no vtable pointer** and is not an instance of that family, and the dispatcher reaches the function by a direct tail jump rather than through any table. Folded by the linker or reused by the compiler — the bytes do not say, and it does not matter;
- **what does matter is the limit this puts on our instruments.** This reverse leans on the call graph: the file-access claim was a coupling ratio of 774 callers in against 17 callees out, and the reachability work brackets the image with caller sets and dispatch closures and reports 1,641 unreached functions. Those measures are not in question — they concern functions with real bodies. They carry **no semantic weight over a one-instruction function**, whose callers are not the users of a concept but whoever needed that offset. "X is called from here, therefore X belongs to this subsystem" is unsound at that size, and that is precisely the form of the sentence I wrote;
- **a census to size the exposure, run rather than asserted.** Scanning `.text` for one-instruction `rcx`-relative getters immediately followed by `ret`, over six encodings, finds **33 — every one outside `.pdata`**: 15 `mov eax`, 8 `movss`, 5 `movzx` byte, 3 `mov rax`, 1 `movzx` word, 1 `lea`. The 33 is a **lower bound**, since the padding requirement misses a getter following something else and only six encodings were scanned; what is exact is that none is enumerable from `.pdata` — the **third** instance of that blind spot in one day, after the two missing signature decoders and the third dispatcher;
- the inheritance pattern itself is not offered as a finding: `dmc3-hdc-vtable-slots` already carries 702 base-and-slot pairs and the override ratios, so this is an instance of measured territory. A duplicate-body census that might have decided folding was attempted and is **not** reported as a result: it found 107 groups across 367 `.pdata` ranges, but many of those ranges are one to fourteen bytes and are chained-unwind fragments rather than functions, so the test as run decides nothing.

#### Reading a claim the project already had, and finding my own note short

- I set out to read who registers the mounts and in what order, read it, and then found the answer **already in the repository** — `docs/reverse/dmc3-resource-vertical-proof-reconciliation-2026-09-05.md` §1A and §1B, in more detail than I had reached, plus `docs/roadmap.md` and `docs/status/canonical-status.json`, which names the function by address. The derivation of `<exe-dir>\data\dmc3\`, the flags `0x0C`, the numbered probe from zero, the first-gap stop, the prepend topology and the order that follows — all of it was there, and that document is more careful than this pass on a point I did not make: **a discovered archive whose registration fails is absent from the topology**, so discovery does not prove mounting and an absent volume is not a clean lookup miss;
- **so the worth of this pass is a caveat discharged, not a finding.** That document states plainly that it "does not claim a new raw-byte disassembly ... because a raw canonical executable blob was not available." Sections 1A, 1B and 1D are now read instruction by instruction and nothing in them contradicts the bytes. A change in the grounds of an existing claim, which is worth recording and is not worth dressing up as a discovery;
- **and three errors in my own note from the same day.** The mount-registration note says of the normaliser "all of its bits are now read" and presents the bit map and the `0x0E`/`0x0C` asymmetry as though establishing them; **§1D already carried both**. Reading them independently has value; presenting them without the citation claims origination that is not mine, and it is the rule I applied to the ZIP note and the file-access note and did not apply here;
- **the bit map is also incomplete.** It ends at the `/` → `\` conversion. A second unconditional pass follows at `0x327260`–`0x327295`, copying forward and skipping every `\` after a `\` — a **repeated-separator collapse**, which the 2026-09-05 document has and I did not. Not cosmetic: a query with a doubled separator and an indexed name with a single one are the same key *only* because of the pass I left out, so an index built from my table alone misses on exactly the inputs the collapse exists for;
- **"two functions do the dispatching" is three.** A byte scan for the twelve-byte prologue shared by the read dispatcher at `0x3275C0` and the seek one at `0x3275E0` finds a third at `0x3272A0`, for size — and it is the odd one out: read and seek keep their archive branch inside the file layer, while its branch tail-jumps to `0x0629C0`, outside it. The claim about the shape was right; the count attached to it was stated without running a census, and the scan that corrects it is four lines.

#### Both halves of the name invariant, and the mount struct that embeds its archive

- the mount-list note stated a hazard it had not verified: the lookup is a `bsearch`, so it depends on the array being sorted under exactly the normalisation the query goes through, and a mismatch misses rather than degrades. **Both halves are now read and they are the same call.** `0x327CC0` normalises every stored name in place with `0x327160(name, 0x0E)`, counts by walking, allocates `count * 0x10` **with an overflow guard** (`mul` then `cmovo rax,-1`, so an overflowing product asks for `SIZE_MAX` and fails instead of under-allocating), fills `{name, node}` pairs and `qsort`s with `0x3291D0`; `0x328160` normalises the query with **the same function, the same flag word and the same comparator**. The invariant holds by construction, not by discipline;
- **one mount struct, two kinds, the archive embedded.** `0x326D20` (directory) and `0x326DFC` (archive) are both `calloc(1, 0x58)` pushed onto the head at `0xCF3180` through `+0x50`: kind at 0x00, normalisation flags at 0x04, `_strdup`'d path at 0x08, **the archive object embedded across 0x10–0x4F**, next at 0x50 — 88 bytes with nothing left over. That embedded region is exactly the 0x40 the archive note saw zeroed at `0x328320` without knowing what contained it, so the containing allocation confirms a size previously only inferred from a zeroing loop. Registration pushes onto the head and the resolver takes the first hit, so **the override policy is "last mounted wins"**, expressed as two moves rather than a priority field. An archive mount opens, initialises, closes, indexes, and only then sets its kind and publishes — never visible half built, and the archive file is not held open between mounting and use;
- **the comparator alone would have given a confident wrong answer.** `0x3291D0` is a byte-wise compare returning ±1, which reads as case-sensitive. It is not, because `0x327160` runs first, and all its bits are now read: `0x01` upper-cases ASCII, `0x02` lower-cases ASCII *only when `0x01` is clear*, `0x04` strips leading separators, `0x08` strips trailing ones, and **unconditionally every `/` becomes `\`**. Both sides pass `0x0E`. So lookup is **case-insensitive for ASCII and case-sensitive above `0x7F`** — the folding is a range test on `A`–`Z` that touches no high-bit byte, so two names differing only in the case of a non-ASCII byte are two entries and only one is reachable. A port reaching for a locale-aware or UTF-8 case fold here changes which files resolve;
- **three separators, in a bitmask.** `0x326EE0` appends `\` to a base path unless it already ends in one of a set encoded as `movabs r14, 0x200000000801`, indexed by the last character minus `0x2F`: bits 0, 11 and 45, which are `/`, `:` and `\`. So `C:` is left alone and an existing slash is not doubled — exactly the kind of small thing a port gets wrong silently, because the failure is a path that still opens on one platform;
- one observation about the code's character rather than its behaviour: the layer that spins forever on an I/O error in three of its wrappers is the same layer that clamps `n + 1` in `0x327650` and `count * 16` here. Carefulness about integer arithmetic and carelessness about failure are not the same axis, and this code sits at opposite ends of them.

#### Nothing hands it a handle: a mount list, a 16-byte polymorphic stream, and an entry that opens the archive again

- two notes today ended on the same question from opposite sides — what hands the directory reader a handle, and what the open-entry object is. One answer. `0x327430` opens with **`calloc(1, 0x10)`**: an archive entry at 0x00 or a buffered reader at 0x08, **never both**, and two functions in the `.pdata` hole at `0x3275BD` do all the dispatching in **three instructions and a tail jump** each — load 0x00, test, jump to `0x328F50` or load 0x08 and jump to `0x327910`, and the same shape for seek. A hand-rolled tagged union on a null test, no vtable anywhere near it: a port replaces nine instructions here, not a class hierarchy;
- **resolution walks a mount list and the first hit wins.** A global at `0xCF3180` heads a list linked at `+0x50`; each mount has a kind at `+0x00` — 0 a directory, 1 an archive, anything else skipped. An archive goes through `0x328160` against the index at `+0x10`; a directory is normalised by `0x327160` with the mount's own flags at `+0x04`, joined against the base path at `+0x08` into a 1,024-byte buffer, and opened. **The caller's flags exclude either kind** — bit 1 skips archives, bit 0 skips directories — so one resolver serves look-anywhere, packed-only and loose-only, and **the ordering of the list is the whole of the override policy**;
- **the lookup is a `bsearch`, and it accounts for two fields left open.** `0x328160` normalises with flag word `0x0E` and searches a sorted array of 16-byte pairs at `index+0x30` with the count at `index+0x38`. The archive note zeroed 0x40 bytes at `0x328320` and wrote only up to `+0x28`, leaving `+0x30` and `+0x38` unexplained — **they are the array and its count**, so `0x3289F0` builds the list and `0x328160` searches the array, and the two ends meet. The consequence for a port is sharp: a normalisation mismatch does not degrade the search, it misses;
- **the entry is 80 bytes and carries the directory record at +8**, and four later uses agree on the shift — method at `0x12` against `0x0A`, compressed size `0x1C` against `0x14`, uncompressed `0x20` against `0x18`, local offset `0x34` against `0x2C`. Those four are in three other functions that decode nothing and merely read displacements, so **none of them could have been consistent with the field map by construction** — a stronger check on it than the decoder that produced it could give;
- **every open entry opens the archive again.** `[entry+0x00]` is a `_strdup` of the archive's path, and on first read `0x328540` hands it to `0x327800` — `CreateFileA` on the same file, its own `calloc(1, 0x28)`, its own `malloc(0x4001)` — then frees the copy. So each entry a caller holds open costs **a descriptor and 16 KiB of its own**, however many come from the same archive, and nothing on this path shares an existing handle;
- **the local record's name is consumed, never compared.** `0x328360` skips name and extra in chunks of at most `0x200` rather than parsing them, so a disagreement between the local and central names is invisible here; the data offset is `[entry+0x34] + 0x1E + name + extra`, lengths from the local record and base from the central one. The method at `[entry+0x12]` then picks one of two realisations — zero gives a 24-byte stored substream at `+0x38` (reader, data offset, position, size), anything else an inflater at `+0x40` — and `0x328F50` reads from whichever is set.

#### The handle is not a handle: a 40-byte buffered reader, and five wrappers that retry forever

- what the archive reader passes around as a handle is built by `0x327800`: `CreateFileA` for read access, then `calloc(1, 0x28)` with the handle at offset 0, then **`malloc(0x4001)` — 16,385 bytes, not 16,384** — and a **guard byte `0x4D` written at buffer + 0x4000**, the one extra byte. The rest accounts for the 40 exactly: buffer at 0x08, buffer + 0x4000 at 0x10, current position and end of valid data at 0x18 and 0x20. Its three users agree on that reading — `0x327910` refills when 0x18 equals 0x20 and **bypasses the buffer entirely for requests of 0x4000 or more**, `0x327A80` memmoves the unconsumed bytes to the front, and `0x327B60` resets both after adding `0x18 - 0x20` to a `FILE_CURRENT` displacement, which is minus the unconsumed bytes and exactly the correction the OS file pointer needs. Neither allocation is tested before use;
- **five wrappers share one retry idiom and three cannot leave it.** Call, test the failure value, ask `GetLastError`, compare against the errors worth giving up on, otherwise rebuild the arguments and go again — no counter, no delay, no backoff. `0x327800` gives up on 2 and 3, `0x327720` on 0x12, 2 and 3, and **`0x3277C0` (GetFileSize), `0x327B60` (SetFilePointer) and `0x327A80` (ReadFile refill) give up on nothing at all**: the only exit is the API succeeding, so a closed handle or a disk that has gone away spins the thread between two imports indefinitely. Two details mark it as one piece of source rather than five coincidences — each calls `GetLastError` once per comparison **plus once more whose result is never read**, and each retry rebuilds its arguments rather than reusing the first attempt's. End of file is *not* caught by this, correctly, so the unbounded case is a genuine failure;
- **four gigabytes is the ceiling, and two independent readings say so.** `SetFilePointer` is called with `lpDistanceToMoveHigh` null and the `FILE_CURRENT` correction is 32-bit arithmetic; nothing in the layer carries a 64-bit offset. That converges with the archive reading, where every size and offset field is 32 bits and none of the format's 64-bit extension constants appears in the image. Two readings sharing no instruction reaching the same ceiling. One consequence is readable although unreachable: `GetFileSize` returns `0xFFFFFFFF` both on failure and for a file of exactly 4,294,967,295 bytes, and `0x3277C0` discards the `GetLastError` that would tell them apart;
- **a second axis between the two file layers.** The earlier note separated them by coupling, forty-six users per dependency against two. In layer A each of `CreateFileA`, `GetFileSize` and `SetFilePointer` is called from one function and **twice** from it — attempt and retry — with `GetLastError` appearing 15 times across 7 functions. In layer B they are called **once each**, from `0x049120` and `0x049290`, and neither function calls `GetLastError` at all. One layer retries every transient failure indefinitely; the other never asks what went wrong. Why the engine has both is still open, but it is now two independent differences rather than one;
- **a correction.** The file-access note called layer A's contents "thin wrappers of 43 to 353 bytes around each" import. The bottom of that range is right and the top is not: `0x327910` is a buffered reader with a refill path, a large-request bypass and an end-of-file case, and `0x327800` opens a file *and* constructs the reader object. The census in that note — which imports, from how many functions, with what coupling — was measured and stands. The sentence describing what sits between the imports and the orchestrators was an impression stated with the same confidence as the counts beside it, which is the failure worth naming.

#### What the file layer does with a handle: one archive format, read from the code that reads it

- the file-access note ended on an admission — the orchestrators combine size, seek and read, but nothing said *into what*. This reads that, end to end, and closes the open item. Seven functions, and **six of them have exactly one direct caller apiece**: `0x328C30` finds the trailer, `0x328D80` reads it, `0x3289F0` walks the directory, `0x327E40` and `0x328020` decode the two record sizes, `0x328360` reads the local record, and `0x327650` reads a counted string. The seventh is `0x327650` with three sites, which are the three variable-length parts of one entry. A straight line with a **single entry point at `0x328345`**, which bounds the replacement surface for a port exactly;
- **the trailer is found by scanning backwards in 256-byte windows that overlap by 22.** `0x328C30` refuses a file below 22 bytes, starts at size − 22, reads `min(size − candidate, 256)`, walks the window comparing `'P'`, `'K'`, 5, 6 four bytes at a time, and on no match steps back by **234 = 256 − 22** — so a record straddling a window boundary is still seen whole — clamping at zero. It is not capped at one window; it walks back to offset 0. On a match it validates `offset + 22 + comment_length == size`;
- **the record layouts read straight off the decoders.** `0x327E40` and `0x328020` are one function written twice, assembling every field from single-byte `movzx` loads shifted and or-ed low byte first, each ending in one `cmp`/`sete`/`ret` against `0x02014B50` and `0x04034B50`. Every source and destination offset is an immediate, so the maps are **17 fields over 46 bytes** and **11 fields over 30**. Both mirror the record exactly until the first 32-bit field that would land unaligned, where the destination shifts forward by two and stays shifted — a C struct with natural alignment, and a portability hazard in the other direction: a port that declares these structs and memcpys a record gets different answers;
- **the object and the node are pinned by their allocations.** Each entry is `calloc(1, 0x50)`: 0x30 of decoded scalars, three heap pointers at 0x30/0x38/0x40, a next pointer at 0x48 — exactly 0x50 with nothing left over. The teardown frees precisely those four, which confirms the layout from the other direction. Strings come through `0x327650`, which `malloc`s n+1 with an overflow clamp and writes a NUL at `[n]`, so every variable-length part is a C string whatever the archive says; the trailer's own blob is read without one;
- **46 and 30 are each measured twice from unrelated encodings and agree** — as read lengths at the call sites (`lea r8d,[r9+0x2D]`, `lea r8d,[r9+0x1D]`, each checked by an exact-length test) and as the span of the decoders' source displacements. A field map recovered from displacements cannot know how many bytes the caller read, so this is a real cross-check;
- **neither table was typed.** `research/exe/extract_record_decoders.py` regenerates both from the image, fail-closed: an unrecognised byte aborts naming the offset rather than being skipped, because a skipped instruction moves every field after it. Eight tests synthesise the shapes and need no executable, and **all seven mutations are killed** — including the one distinguishing a store through `rdx` from the same ModRM byte with REX.B set, which is the exact confusion that produced a decoder bug earlier in this reverse. A redundant width check was removed rather than given a test, since a set equal to `range(low, low + width)` already has `width` members;
- **a correction, and what it was worth.** Earlier in this pass I scanned compare immediates function by function, found `0x06054B50` twice and nothing else, and said it was the only such constant in the image. A raw scan of all 6,356,432 bytes finds **four occurrences of three constants** — `0x02014B50` at `0x328011`, `0x04034B50` at `0x328154`, `0x06054B50` at `0x328A69` and `0x328EAF` — and none of the other four the format defines. The earlier scan enumerated functions from `.pdata`, and **both missed decoders have no `.pdata` entry**, so they were never candidates: it was not wrong about what it saw, it was wrong about what it could see, and it reported a census without declaring its population. Every census in this project that enumerates functions is a census of `.pdata`-covered functions, and the 802-byte hole at `0x327E3E` proves that is not the same set. The miscount cost a sentence; correcting it bought the better claim — **the 64-bit extension constants are absent**, so this reader cannot address an archive that needs them, which the narrower scan could not have established at all;
- format identification is kept in its own section and labelled as public interface knowledge, as `WM_DESTROY` was in the window note: the constants, the 22/46/30 fixed sizes and the field names are the published ZIP container, and a reader who distrusts that can delete the section without losing any of the byte-level reading;
- **prior art, stated plainly.** `docs/reverse/gdspaces-blocked-window-acquisition.md` already lists `0x140328540`, `0x140328820`, `0x140328F50` and `0x140328FE0` as the ZIP/inflate side. None of the seven functions read here appears in that list and no layout was recorded anywhere, so this adds the directory half to an area whose compressed-stream half was already claimed. It sharpens an existing claim rather than originating one.

#### How the engine reads files: two layers, one of them a library

- back to the subject matter after several rounds on verification. A game shipping gigabytes of assets reaches the operating system through almost nothing: across the whole image `ReadFile` is called from **4** functions, `CreateFileA` from 2, `GetFileSize` from 2, `SetFilePointer` from 2, `CreateFileW` from 1, and `FindFirstFileA`/`FindClose` from one apiece. **Twelve functions own the entire file surface**, the largest 486 bytes and the smallest 60;
- **57 of them sit in one contiguous range**, RVA 0x326000–0x32A000, 12,895 bytes, and the shape of its edges is what makes it a boundary rather than an address coincidence: **774 functions outside call into it and it calls 17 outside itself** — forty-six users per dependency. Inside it is three tiers: the Win32 imports, thin wrappers of 43 to 353 bytes around each, and orchestrators combining them, one calling the size, seek and read wrappers together. It allocates heavily (`free` 13, `calloc` 7, `malloc` 4) and **names nothing** — one string reference across 57 functions and no resource-family literal, so callers hand it names and it does not know what it is reading;
- **there are two, and they are opposites.** A second range, 0x048000–0x04B000, holds 33 functions over 11,047 bytes and calls the same four imports independently, with **16 callers in and 10 callees out** — two users per dependency against forty-six. Only the second touches `CreateFileW` and the file-time conversions; only the first touches `FindFirstFileA` and `FindClose`. Nothing here says why the engine has both; the claim is about shape, and the shapes differ twenty-fold. A third function, 0x36570, calls `SHGetFolderPathW` beside `CreateDirectoryW` and is the only place in the image asking where a per-user directory is;
- `v_exe_import_owner` and `v_exe_range_coupling` carry it into SQL. The coupling view measures whatever range it is given, so it measures a boundary rather than discovering one. Every figure was computed twice — once from the report, once through SQL — and they agree.

#### The published numbers had drifted, and now they are bound to their counters

- the counters checking each other is bookkeeping inside one report. This is the other direction: do the numbers **published** in evidence still match what the code produces? Three fixes in quick succession — the interior-address rule, reading REX.B, resolving element-size conflicts — had moved figures that four records still stated as current: size-floor counts **326/177/124/39 → 329/192/112/36**, global blocks **3,582/1,044/26 → 3,753/1,130/27**, interior-address figures **1,855/767 → 1,880/785**, constructor stores **270/265/44 → 279/274/47**;
- the last is a different failure from the first three. Its numbers *were* corrected — in a different packet, about the receiver analysis — and nothing linked back. A correction nobody can find from the thing it corrects is not much of a correction. All four are now correction records with `supersedes`, not edits to history;
- **a fifth, in SQL.** Chasing the figures turned up the summary saying `global_blocks_reach_within_the_gap` = 21 while `v_exe_global_block` said 22. The highest block has nothing after it and the view's `CASE` fell through to `REACH_FITS_THE_GAP` — there is no gap for it to fit. It now reports `NO_NEXT_BLOCK` and the two agree;
- **how they were found, and why that is not good enough:** scanning every number in every packet against the current counters gave **44 hits, of which 4 were real**. The rest were RVAs, byte offsets, the *old* side of a stated "was → now", and coincidences. Matching bare numbers in prose is a lead generator, not a verdict, and every one of the 44 had to be read;
- **the durable fix:** an evidence record can now declare the figures it states, bound to the counter each comes from. The evidence schema is strict and rejected the first attempt outright — which is the schema working — so this is a real field with real validation: a figure must be a non-negative whole number, and a string, a negative, a real and a nested object are each refused by a test;
- `research/sql/check_evidence_figures.py` verifies every declared figure against a fresh report, skips records a correction supersedes (their figures are exactly what the correction says no longer hold), and **fails on a figure naming no counter**, because a check that silently checks nothing is worse than no check. Five tests, in CI;
- **102 figures across 33 records** are now bound, up from 20. The method was to propose a binding wherever a record's text holds the *current* value of a counter, then read every proposal — and roughly half were coincidences: "200 of 200 comparisons" against `global_state_blocks` = 200, "sinf (51)" against `name_tables_unreferenced` = 51, "376 bytes — 47 slots" against `field_layout_entries` = 47. The subtlest was "recovered arrays from 278 to 281" against `indexed_arrays` = 278 — a number true in the *past* that happens to equal a counter now, which would have made the check pass for entirely the wrong reason;
- **binding 82 more figures found no new drift.** All 102 agree, so the four found by hand were the whole of it for the counters that exist. Numbers with nothing to bind them to — RVAs, byte offsets, ratios, per-class figures — remain prose and always will. The checker itself runs where the artifact is rather than in CI, since it needs a report; what runs in CI is its guardrails.

#### The counters now check each other, and the check that actually matters

- the `68` against `99` mistake was the **third** counter whose name promised one population and whose value held another: `startup_path_dispatch_sites` counted only call-position dispatches; `indirect_call_sites` (10,958) and `dispatch_sites` (11,434) count different populations side by side; and the SQL table `exe_dispatch_site` held one row per **(function, offset)** — 5,395 — rather than one per site. None is a wrong number; each is a right number of the wrong thing. The table is now `exe_dispatch_offset` with the difference stated in the schema;
- **six partitions and eleven subset relations** are now checked, plus one quantity computed twice by different code (the depth closure and the reachability flag both reach 370). All nineteen hold on the real image, and the importer refuses a report that breaks any of them;
- the dispatch partition needed two new counters, and writing it exposed a gap: the receiver census's headline figure — **9,535 sites, 83.4%, with nothing known** — was published in evidence while the library emitted no such number, so it was not reproducible from the report. It is now, and the partition closes exactly: **1,880 + 12 + 9,535 + 7 = 11,434**;
- **the part that matters: no identity catches the fault that prompted writing them.** Checked against all nineteen, the original 68 fires nothing — it is a good number of a different population and sits happily inside 99, inside 11,434, and inside every total above it. An identity set is blind to a counter that is internally consistent and simply measures the wrong thing. What catches it is measuring the same quantity **twice**, which needs the report to carry the per-function counts the census is made of; it now does, and the check names the fault outright: *summary says 68 but the functions on the path add up to 99*;
- checks live in both places: the library (the four receiver categories partition the census; the per-function count is the same population as the census) and the importer (all nineteen identities plus two summary-against-detail sums, run against the real image on every load). Six tests, each breaking one on purpose, and one of the library tests is built around a call clobbering the argument registers — which is why each receiver category needs its own function to demonstrate;
- what this does not do: make a measurement *correct*. Every identity is bookkeeping. A counter can satisfy all nineteen and still measure something nobody wants — that is what reading the anomalies is for, and this is the other half.

#### The handoff is total: the sound path extends by exactly zero functions

- the dispatch sites the startup path makes were called "the handoff"; reading all of them settles what that means. Of the **99** sites, **88** have nothing known about the receiver, **10** read it through one of the argument registers and **1** through an address the code took. **Not one resolves** to a class and a target, so the number of functions the sound closure gains by following a determined dispatch is **zero** — not few, none;
- the reason is structural rather than a shortfall of the analysis. Resolving a dispatch needs the enclosing function to be one the compiler bound into a vtable, and of the 370 functions on the path **exactly none is**. The platform is brought up by free functions, and the first virtual call is the last thing the file lets anyone follow;
- **correction: 99 dispatch sites on the path, not 68.** The published 68 came from a narrower list than the image-wide census of 11,434 printed beside it — it counts only call-position dispatches and omits tail-position ones, which are the same dispatch by another instruction and which the census does count. Counting from the same list gives 99, of which **31** are in tail position. The figure of 30 functions containing a dispatch is unaffected, and so is everything else the path was said to contain;
- `startup_path_dispatch_sites` now counts from the same list as `dispatch_sites`, with `startup_path_dispatch_sites_in_tail_position`, `startup_path_functions_bound_to_a_class` and `startup_path_extended_by_resolved_dispatch` beside it. A test pins that the startup count and the census agree on a tail-position call, which is the disagreement that produced the wrong figure.

#### The startup path, the one route through the image that is sound end to end

- every caveat flagged in this reverse has now been read, so: the closure of direct calls and tail jumps from the entry point, which assumes nothing about dispatch and which I had never looked at. It reaches **370 of 7,389** functions, runs **17** steps deep and bulges at depths 5–8, where 290 of the 370 functions and 117 KB of code sit;
- **the entry stub hands off by a tail jump, not a call.** A closure following calls alone stops at *two* functions instead of 370 — the first measurement came back as `2` and that is what it meant. Depth therefore counts transfers, and it is **not an execution order**: it says how few steps can reach a function, not when it runs;
- the path contains the **whole platform**: 97 imported symbols across 18 modules, with `d3d11`, `dxgi` and `DINPUT8` each reached by exactly **one** function;
- **not one object is built here.** Of the 370 functions, **zero** are constructors and only 30 contain a dispatch, over 68 sites. The boundary is sharp — the sound path brings the platform up and hands off, and everything the engine builds is built on the far side of a virtual call. That is the same fact that made the rapid-type tightening of the reachability bound collapse to four vtables, now stated at its source;
- **four functions named by what they import**, which is the compiler's record rather than a guess: RVA 0x42890 (2,024 bytes, sole caller of either symbol) calls `CreateDXGIFactory` and `D3D11CreateDeviceAndSwapChain`, its 6 dispatch sites being calls on the interfaces it creates; RVA 0x47B40 (1,217 bytes, sole caller) carries the full `RegisterClassExW`/`CreateWindowExW`/`ShowWindow` sequence together with `DirectInput8Create`; RVA 0x3A9E0 calls `SteamInternal_ContextInit`; RVA 0x41BC0 calls `XInputGetState` guarded by `GetFocus`;
- **one function read end to end.** RVA 0x48970 is 157 bytes with no dispatch sites: `PeekMessageW`, then `TranslateMessage` and `DispatchMessageW` on a message. Its compare chain tests the message against 0x12 and then against the mask `0x50004`, which selects exactly 2, 16 and 18; on those it calls the thunk at 0x3454C8, which jumps to the IAT slot the import directory names `fmod64.dll!FMOD_System_UnlockDSP`, and returns zero. **The audio call is on the way out, not once a frame** — an FMOD call inside a message pump looked wrong enough to check, and checking turned a puzzle into a reading. The Win32 headers name those three values `WM_DESTROY`, `WM_CLOSE` and `WM_QUIT`; that naming is public interface knowledge, not read from this file;
- `exe_function.depth_from_entry`, `v_exe_startup_path` and `v_exe_startup_module` carry it into SQL. The depth closure and the reachability flag are computed separately and agree at 370, which is one checking the other.

#### The last two unread caveats: one holds, one does not

- **the negative displacements hold.** "A negative displacement is the inlined character scan over a string" was published without reading one. All **361** carry displacement exactly **−1**, element size 1, scale 1, and opcode **`3A`** — `cmp r8, r/m8` — across 60 distinct functions. `cmp reg8, byte [base + index*1 - 1]` is a byte-by-byte comparison indexed from one. The reading was right, and the uniformity is far stronger than the claim, which said only that the displacement was negative. Reading a caveat sometimes confirms it;
- **the spanning groups do not.** Image-base groups were dropped when their span exceeded one element, "**meaning the register was reused for a different array**". The spans run from one element to **7.5 MB**; the largest cannot be one array since the image is 6.3 MB, but the smallest is **two bytes**, and **93.1%** have every read a whole multiple of the element from the lowest — which is what one array read at a few constant indices looks like;
- the only provable case is reads that do not all fall inside one section. There are **9** of those, not 102. The remaining **93** are still dropped, but as **undecided** rather than as reuse, and the counts are now reported apart with tests pinning both. Dropping a group is conservative either way; saying *why* is not, and only "nothing here decides it" was supported;
- the separate finding that the image base is held in a register and indexed against — 1,303 indexed reads, 749 at RVA zero — is untouched and stands;
- **method, four rounds on:** 167 sites reading through a constant → a decoder bug; 28 accesses outside their element → resolvable, error bar halved; 361 negative displacements → confirmed and sharpened; 102 spanning groups → reading wrong, 9 not 102. Three corrections and one confirmation, none needing new machinery.

#### The element-size conflicts were resolvable, and reading them halved the error bar

- bases read at more than one element size were published as "a contradiction: at least one of the readings is wrong… **reported rather than resolved, since nothing here says which**". That was honest and it was wrong — the structure does say which;
- an index multiplier is recorded only off a definite `lea a+a*k`, `imul` or `shl`, so it can be **missed but never invented**, and an element size equal to the raw SIB scale is one read with no multiplier seen. Across all **11** such bases in the image the smaller reading both divides the larger and equals its own scale — **11 of 11, no exceptions** — which is a missed multiplier, not a second array;
- resolving to the larger reading halves the accesses whose displacement falls outside its element, **28 → 14**, and raises consistent accesses **541 → 555** by exactly those 14. RVA 0x580D20 now reads as a 24-byte record with fields at 0, 4, 8, 12, 16 and 20, where it was an 8-byte reading plus accesses reported as inconsistent;
- a base whose sizes do **not** divide, or whose smaller reading carries a multiplier of its own, is a real conflict and is now **left out of the layout** rather than given a size it may not have. There are none in this image; a test covers the case with `{24, 12}`, and removing the raw-scale requirement from the rule makes it fail;
- two entries remain doubled in the finished table, at 8 and 48. Both are image-base derived, where the base is only an upper bound and grouping runs per function and index register, so one base can legitimately appear twice. The held-base reasoning does not transfer there, and the counter's scope now says so;
- **method note:** three findings running have come from the same move — take a number published as a caveat and read it. The REX.B bug came from 167 sites in a category that cannot exist; this came from 28 accesses labelled an error bar. Neither needed new machinery.

#### An impossible reading in the receiver census found a decoder bug

- recording the register analysis's **own verdict** at each of the 11,434 dispatch sites, instead of guessing at it with a backward scan, put 167 sites in a category that cannot exist: the register the vtable is read through held a **constant**. A vtable pointer is never a small constant;
- the cause was two layers down, in the decoder. The x86-64 forms that carry their register **inside the opcode** — `mov reg, imm` at `B8`–`BF`, `push`, `pop`, `xchg` — have no ModRM field to extend, so **REX.B is the only thing that distinguishes `rax` from `r8`**. Nothing surfaced it, and the register tracking read the low three bits alone. `mov r8d, imm` sitting between a vtable load and the dispatch through it, a shape the image is full of, was recorded as a write to `rax` — destroying the vtable fact and inventing the constant. The invalidation block had the same blind spot and conservatively forgot *both* halves of the register file, which did most of the damage;
- after surfacing REX.B the impossible category is **empty**, which is the check on the fix, and 21 measured figures move towards more facts recovered: dispatch sites on `this` **1,855 → 1,880**, in a class-bound function **767 → 785**, pointer stores into `this` **187 → 207**, indexed accesses **1,966 → 2,023**, global-block call sites **3,582 → 3,753**, size floors corroborated **189 → 192**;
- **the census itself is the corrected result.** Of the 11,434 sites: **9,535 (83.4%)** with nothing known about the register, **1,892 (16.5%)** read through one of the arguments the convention passes in registers, 7 through a taken address, **0** a constant. Of those through an argument, 1,085 are one load deep — the object's own vtable, the resolvable case — and 782 two loads deep, a pointer member's pointee that the enclosing class's layout cannot describe. Naming a receiver is not resolving a call: 1,880 on `this`, 785 of them in a class-bound function, **105 resolved** across 32 classes. This supersedes the earlier breakdown into `field of a copied register` and `field of a field`, which were artefacts of the backward scan rather than statements about the analysis.

#### Correction: 130 functions take a constant first argument, not 179

- the same REX.B blind spot inflated a published census. `41 b9 imm32` is `mov r9d, imm`, and it was recorded as putting a constant in **`rcx`** — inventing a constant first argument for the following call. "1,311 calls over 179 functions" becomes **1,319 over 130**: the call count barely moves and the function count was overstated by **27%**;
- the separate reading of the function at RVA 0x2E7CA0 is unaffected, because that one was read off its own compare chain rather than inferred from this census. Which is the whole reason the compare chain was read.

#### One instruction was hiding most of the dispatch receivers

- 11,434 virtual call sites and the receiver analysis could name 1,135. Before changing anything, I asked what instruction produced the register each site reads its vtable through: **9,878 (86.4%)** a field load, 116 a stack slot, 24 a global. That kills the obvious suspicion — MSVC spills `rcx`–`r9` to home space in many prologues and I expected the losses there, but it is **1%** of sites. Walking one step further back, **2,529** of those bases were produced by a `lea`;
- **taking the address of something inside an object does not stop it being the object.** A method reaching an embedded member writes `lea rax,[rcx+0x10]` and dispatches through what that points at; the analysis dropped `this` at the `lea` because taking an address was not modelled as carrying the value. Carrying it, with the offset accumulated, moves every dependent figure: dispatch sites on `this` **1,135 → 1,855**, sites inside a class-bound function **258 → 767**, fully resolved virtual calls **47 → 105** across 32 classes, member receivers **17 → 75**, class field layout **44 → 47** entries, size floors corroborated **177 → 189** with lone outliers falling **124 → 114**;
- **the check that had to come with it.** The accumulated offset has to reach the store consumers too, or a vtable written through an interior address reads as a store at offset zero — which is exactly what names a function a constructor of that vtable's class. A test pins the distinction, and removing the accumulation makes it fail. The older check holds as well: stores of a taken address into the first argument were published at 265 of 270 landing on a known vtable, the 98% that says the following is sound rather than accidental, and they now come out at **274 of 279 — 98.2% against 98.1%**. The new stores are the same kind of thing as the old ones;
- **a negative result worth keeping.** The convention passes the first four arguments in `rcx`, `rdx`, `r8` and `r9`, so naming all four should let a dispatch on something a method was handed be told apart from one on `this`. It buys **12 sites out of 11,434**. The engine dispatches on the object a method belongs to, or on something reached through it, and hardly ever on a parameter. `dispatch_sites_on_an_argument` stays because it is free and because a site on argument two is a different thing from an unnamed one, but it is recorded as a measurement that did not pay.

#### The engine's global state: 200 fixed addresses the code operates on

- 396 polymorphic classes and 915 vtables, and **none of them is a global instance**. The other half of the engine is reached differently: the Microsoft x64 convention puts the first argument in `rcx`, which is also where `this` goes, so an address taken with a RIP-relative `lea` and left there is something the callee operates on. There are **3,582** such calls naming **200** distinct addresses;
- **3,562 of them land in `.data`** and 15 in `.rdata`. That split is why this measures state rather than arguments — passing string literals would put almost everything in the read-only section, and almost nothing is there. The head is steep: 17 addresses are reached from twenty or more distinct functions, 112 are passed exactly once. The busiest, RVA 0xD6DC90, takes **600 calls from 347 functions through 34 entry points**;
- **two measurements that did not have to agree.** How far into a block the code reaches comes from instruction displacements inside a callee; how far away the next block is comes from the sorted set of addresses the code takes. A callee lends its reach only to a block it has to itself — every image address it is given is that block, and it has no callers beyond those sites — which drops 575 sites whose callee serves several blocks and 1,044 whose callee is reached some other way. Of the 26 blocks that survive, **21** have a reach that fits inside the gap and **5** run past it. The 5 are a reading, not a disagreement: the next address the code takes is a field inside that block. RVA 0xC99D30 reaches 26,465 bytes against a gap of 26,432;
- **the engine splits cleanly in two.** Exactly one of the 200 blocks is reached by a function the constructor identification names, `DMC3::FullMotionVideoManager` at 0xBEAFA0; the other 199 have no class because nothing writes a vtable into them. What the engine allocates is C++ with full run-time type information, and what it keeps at fixed addresses is plain blocks reached through free functions;
- `exe_global_block` and `v_exe_global_block` carry it into SQL, with a CHECK asserting the overrun flag is exactly the comparison of reach against gap, so it cannot disagree with the numbers beside it.

#### Correction: the constructor half of the size measurement never ran

- the class size floors were published as coming from two kinds of function — methods the compiler bound into a class's vtable, and constructors identified by the vtable they store at offset zero. **Only the first ever contributed.** The measurement read each function's identified class from a field that the constructor identification fills in *further down the same build*, so at the point it ran that field was empty for every function and the constructor branch was dead. The same ordering mistake made the global-block census find no class at all where it should find one;
- nothing published was unsound — a floor from bindings alone is still a floor — and the largest floors are unchanged, `CEm035` still 61,305 bytes and `CPlDante` still 46,625, median still 1,307. The support counts move: **326** classes carry a floor above their own vtable pointer rather than 320, **177** are corroborated rather than 171, **124** rest on a lone outlier rather than 121, **39** have a single source rather than 32, **56** have only the type information rather than 72, and the classes where base placement gives the deeper floor rise from 4 to **14**.

#### Object size floors for 396 classes, from two sources that read no field

- the reverse has known 396 classes, their hierarchies and their vtables for a while, and has known how large one object is for **none** of them. Two sources give a floor. The class hierarchy descriptor places each base subobject at a recorded displacement, and a base carrying a vtable occupies at least the 8 bytes of that pointer — **200** classes get a floor above 8 that way. A method the compiler bound into a class's vtable is handed the object as its first argument, so a memory operand at offset K inside it means the object reaches at least K+1 bytes; where the binding sits at a non-zero subobject offset the reach is that much further into the complete object. **320 of 396** end with a floor above their own vtable pointer, median **1,307 bytes**. Neither source reads what is *at* any offset;
- **support is a column, not a filter.** 171 floors are corroborated by two or more functions independently reaching at least half of them, 121 rest on a lone outlier, 32 have one function speaking, 72 have only the type information. The largest floors are the well-supported ones: `CEm035` at 61,305 bytes has 11 of its 27 speaking functions reaching half, `CPlDante` at 46,625 has 5 of 13;
- **a check nobody asked for:** the four playable-character classes all derive from `IPlayer` and land at 47,273, 46,625, 45,865 and 32,721 bytes — four separately compiled classes of one family agreeing on a size band that nothing in the measurement makes them agree on;
- it is a floor for three reasons, each recorded: only touched offsets count; a function split across ranges is analysed with each range's start knowing nothing, so a field touched only there does not count; and `lea` is excluded, because computing an address is not touching what is at it;
- **the thing that looked wrong and was not.** The code floor exceeds the base floor in 200 of 200 classes where both speak and is never lower, which read as a systematic error. It is the opposite: MSVC lays bases before members, so the deepest member is past the deepest base whenever a class has one. Chasing it sent me through the register analysis hunting a missing invalidation — the invalidation is there and correct (unmodelled register writes clear the fact, a call clears the volatile set, each range's start begins empty), and the second reason above came out of that read;
- `exe_class_size_floor` and `v_exe_class_size` carry it into SQL, with CHECKs asserting the floor is never below either of its own sources — understating a size is the one direction a floor must not be wrong in.

#### Reachability bracketed, instead of a lower bound with nothing beside it

- every reachability figure so far has been published as a lower bound. Direct calls from the entry point reach **370** of the 7,389 inventoried functions — **5%**. Assuming instead that a virtual call reaches whatever sits at its slot in any vtable the image carries gives the other end: **5,748**, or **78%**. That assumption is false as an answer and sound as a bound, because nothing in the file says a receiver's type, so what it could be is the set of vtables carrying the slot. The truth is between, and **the width of the gap is the measurement**: a twentieth of this image's control flow is fixed at compile time and the rest is decided at run time;
- **the standard tightening does nothing here, and that is a fact about the engine.** Restricting candidates to vtables that reachable code installs normally narrows such a bound sharply; of the image's **915** located vtables, directly-reachable code installs **4**. Construction is itself behind dispatch, so the analysis starves before it starts. The bound is reported unrestricted for that reason, not out of caution;
- **1,641 functions sit outside the bound**, and none is exported, an import thunk, or bound to any vtable slot, so no mechanism the file describes reaches them either. 794 are called by something itself outside the bound — closed islands with no way in — and 847 have nothing referring to them at all. That is 587,983 of 3,085,665 bytes, **19% of the code by size**. It replaces the vaguer figure of 848 structurally unreferenced functions, which counted what nothing *pointed at* rather than what nothing could *reach*;
- `exe_function.outside_every_closure`, `v_exe_reachability_bracket` and `v_exe_unreachable_function` carry it into SQL. Build the database with `map-functions --all`, or the percentages describe the exported subset rather than the image.

#### Correction: 210 apparent dispatch tables in data were 10

- runs of consecutive function addresses in read-only data are a dispatch mechanism beside the vtables, and the scan first reported **210 runs holding 1,617 entries**. Wrong. A vtable *is* such a run, and the scan excluded vtables **by base address only**; a slot holding something the function inventory does not cover splits a vtable into fragments whose bases are not the vtable's, so every fragment counted as a table. The three largest supposed tables each sat exactly **376 bytes — 47 slots — inside** a located `CComEm` vtable, which is what gave it away;
- excluding each vtable's **whole extent** leaves **10 runs holding 45 entries**: 97% of the entries were an artefact of a gap in the function inventory. A test pins the distinction — the same bytes inside a vtable's declared extent are not a table, and outside it they are;
- what survives is uniform in a way the inflated figure hid: **all 45 entries name functions the reachability bound does not reach**, and only one of the 10 runs has its address taken by any inventoried function.

#### Correction: `INSERT OR IGNORE` was hiding CHECK failures

- a test written to prove the schema refuses a run shorter than three entries failed to raise, because `INSERT OR IGNORE` suppresses **CHECK** violations and not merely uniqueness conflicts. A malformed row would have vanished silently rather than been refused, in all three tables carrying CHECKs (`exe_name_table`, `exe_base_slot_override`, `exe_function_pointer_run`);
- each now uses a conflict clause scoped to its own unique key, so idempotent re-import still works and a contradiction still raises. The real reports reload clean under the stricter inserts, which is itself a check on the counts they carry.

#### Every vtable slot classified, and each base measured against its inheritors

- a vtable slot is a code address, and decoding only the **first instruction** at that address sorts every one of the image's **13,894 slots** three ways: **276** reach the `_purecall` import, **4,246** start with a return, and 9,372 reach 3,313 distinct functions with code in them;
- the first split is named, not guessed. RVA 0x346BF0 is `jmp [rip+0x87C2]` reaching the import-address-table slot at 0x34F3B8, which the **import directory** names `VCRUNTIME140.dll!_purecall`. The same encoding pointed at any other import means nothing, and the test for it asserts exactly that;
- the second is blunter than expected: all 4,246 slots hold **one address**, 0x24EA30, whose whole body is three bytes — `C2 00 00`, a return. **Thirty-one per cent of the polymorphic surface resolves to a function that does nothing.** The linker folds identical functions, so 3,313 is a lower bound on distinct implementations and 4,246 an upper bound on inert surface;
- **702 base-and-slot pairs** over 53 bases are now measured: for each slot of a base's vtable, how many classes inherit it and how many distinct targets they put there. The comparison runs against the vtable of the base's **own subobject**, at the offset its class hierarchy descriptor records — `IActor` sits 96 bytes into a `CActor`, and comparing primary vtables instead reads an unrelated interface, turning `IActor` slot 1 from **11** implementations into 165. Exactly one base-to-class pairing in the image names an offset with no vtable at it, and it is reported rather than guessed at;
- **the check that found nothing wrong:** slot 0 scores **0.96 to 1.00** for every base with more than twenty inheritors. Nothing told the measurement that MSVC puts a per-class scalar deleting destructor there, so reproducing it is the measure validating itself;
- the spread within one base is the result. `CWork`'s seven slots run **0.96, 0.89, 0.53, 0.52, 0.39, 0.19, 0.13** over the same 264 classes: early slots are per-class behaviour, and the last two are left as the empty body by 200 and 216 of them. `ICollisionHandle`'s two non-destructor slots score **0.011** — 188 classes reach two addresses between them, which is one implementation shared through adjustor thunks;
- **`I` means abstract; `C` means nothing.** Of 53 bases, 21 carry the `I` prefix and **not one defaults a slot to the empty body** — 21 of 21. 19 of 21 declare a slot pure, and the two that do not carry only the destructor slot, so among bases with anything to declare it is 19 of 19. The converse fails: **nine** `C`-named bases declare a slot pure, `CWork` among them;
- this answers the architecture note's open item. `CWork` is an **abstract base with two optional hooks**, not the universal object header it was assumed to be;
- `exe_base_slot_override`, `v_exe_slot_override` and `v_exe_base_interface_shape` carry it into SQL, with CHECKs asserting that a slot's outcomes cannot outnumber the classes carrying it. Slot *semantics* remain unestablished: nothing in the file names a slot, and an override ratio is not evidence of what a slot is for.

#### Table extents corrected against absorbed string pools

- **blind spot closed.** The limitation recorded below is not a pool that resembles a table; it is a real table whose extent ran past its own end into the pool that follows it. A pool of short names is padded to the alignment the grid uses, and because its strings fall at a fixed sub-multiple of the stride the payload offsets stay consistent, so the pool rejection never fires;
- `StringTableScanner` now trims a run whose payload-bearing elements form a strict suffix containing at least one text payload. Payload from the first element is a record whose second field the constant-stride scan cannot see, and trimming that would delete a real table, so the rule requires a suffix. A head too short to be a table then fails the minimum and the run goes entirely: three names and a pool is evidence of a pool;
- `FogColor` at RVA 0x506F68 is corrected from 22 elements to 16 and `Maguma.ogg` at RVA 0x36F288 from 30 to 28; four runs are trimmed of 14 absorbed elements in total, giving 223 runs and 5,813 entries;
- trimmed elements are not consumed, so the tail is read again on its own terms: the tail of 0x506F68 comes back as a separate run at RVA 0x507068 with a stride of 8 and 11 entries, all 11 named by a constant index;
- **the check that found it:** a constant index is folded into its displacement, so the spacing of a table's references measures its element size independently of anything read from the bytes. Both corrected runs were indexed at a pitch of 8 under a declared stride of 16. Across the image, references the scan could only place inside an element fall from 16 to 7 while total references rise from 202 to 206; the 7 that remain sit at offsets of 13, 16 and 20, which is not the sub-multiple pitch of an absorbed pool.

#### Subsystem sizes, measured by reachability

- the **19,420 direct call edges** between inventory functions are now carried through the report and into SQL rather than only counted, because bounding a subsystem is a question about edges. Self-calls and calls to import thunks are not edges, and the importer refuses a self-edge as the schema promises;
- asking which functions reach a module's imports within three calls sizes each layer: audio **70** functions over 17 direct callers, Steam 28 over 12, input 21 over 6, the video shim 14 over 9, and **rendering 4** over a single direct caller for Direct3D and DXGI alike. That last figure is the import list's two symbols saying the same thing a second way — after device creation the renderer is a COM surface and almost nothing funnels into the entry point;
- against those, 3,036 functions reach `KERNEL32` and 2,212 the CRT maths library: two fifths of the image is within three calls of an operating-system service;
- the classes behind the audio layer are **scene** classes (`CSceneGame`, `CSceneDemo`, `CSceneMisStart`, `CSceneStartMenu`), not an audio hierarchy — which agrees with the platform boundary being made of free functions;
- `exe_call_edge` and `v_exe_import_reach` carry this into SQL. The figures are lower bounds — see below for why, which is not the reason first given.

#### Correction: the unresolved indirect jumps were mostly dispatch

- the reachability figures were published noting 1,177 unresolved indirect jumps as holes in the graph. Classifying them shows the caveat was right for the wrong reason. Of **1,814** indirect jumps, **644** are the register-direct form a compiled switch uses and **637 of those have their table recovered — 98.9%**; only 7 register-direct jumps and 10 through a RIP-relative slot are genuinely unresolved, so the true figure is **17, not 1,177**;
- the other **1,160** jump through a memory operand off a register: a virtual call **in tail position**, not a switch whose table went missing. Counting them together made a dispatch site look like a failure of the recovery, and made the recovery look far worse than it is;
- read as dispatch they belong with the call sites, and the census rises from 10,274 to **11,434**. Almost none is on `this` — the receiver analysis gains exactly one — so the reachability figures stay lower bounds because a tail call's target is still unknown. The caveat stands; its cause was misnamed.

#### Platform surface, layers, and the engine's design map

- **what a port has to provide, symbol by symbol.** 26 modules, 221 symbols, 870 call edges. Rendering enters through exactly **two** symbols — `D3D11CreateDeviceAndSwapChain` and `CreateDXGIFactory`, one calling function each — and everything after device creation goes through COM interfaces rather than imports, which is also why the image carries so many indirect calls. Audio is FMOD (36 symbols), video Media Foundation (8), input `XInputGetState`/`XInputSetState` plus a single `DirectInput8Create`, platform services Steam (7). The heaviest module by callers is not the graphics stack but the CRT **maths** library, at 228 functions;
- **three naming conventions, three layers.** Of 407 classes, 360 are `C*` (the engine), 21 are `I*` (its interfaces) and **5** sit in `DMC3::` — and all five are the PC port's video and audio shim, an abstraction and its `PC`-prefixed implementation. That is the whole of what this port added under its own name;
- **the platform boundary is not polymorphic**: of the functions calling FMOD, Direct3D, DXGI, XInput or DirectInput, not one is a virtual method or a constructor. Only Media Foundation reaches classes, and they are the shim's own. `DMC3::PCFullMotionVideo` names `IUnknown` among its bases, as a Media Foundation callback must;
- **the design map, from the hierarchy descriptors.** 1,341 declared base relationships: `CWork` named by **264** classes is the engine's root, `ICollisionHandle` by 188 and `IActor` by 182 — collision is declared more widely than actorhood — `CShell` by 87, `INonPlayer` by 46, and the command triple `ICom`/`IComAction`/`IComActionState` by 29/29/30, one set per enemy type. `IPlayer` has exactly five implementors;
- **11 bases carry no vtable of their own** and would be missing from any class list built from vtable coverage — the interfaces are exactly the part that would have gone. `exe_class_base` and `v_exe_base_reach` read the hierarchy from the descriptors instead.

#### Functions parameterised by a constant

- constants the code puts in a register are now tracked — an immediate move, a register cleared against itself, an address computation over one — which makes any constant first argument readable. **1,311 call sites pass one, over 179 functions**. `0x2C6D90` takes 59 distinct values across 202 calls, `0x1B82C0` a dense 1–21 across 93, `0x8BF30` a sparse scatter of 49 values across 59. What the constants mean is not stated; which functions are parameterised and which values exist is, and that bounds an enumeration without naming it;
- **the function behind a pointer field, read rather than inferred.** RVA 0x2E7CA0 was called an allocator, then a factory keyed by a selector; both were inferences. Its own body settles it: from RVA 0x2E7D20 a compare chain on the first argument sends 0, 1, 2 and 3 to different arms and returns null for anything else. The arms (0x2E3B10, 0x2EBA10, 0x324460) are 125 bytes each and **identical in shape**, differing only in the writable-data address each loads — 0xCAB230, 0xCAE7D0. So the argument picks which of several identical routines runs and what separates them is a data address, **not a type**; the class of the returned object still does not follow, which is where this thread honestly ends;
- `exe_constant_argument_callee`, `exe_constant_argument` and `v_exe_constant_argument` carry the census into SQL.

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
