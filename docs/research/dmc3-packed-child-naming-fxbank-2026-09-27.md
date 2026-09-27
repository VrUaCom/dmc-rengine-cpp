# DMC3 HD packed-child naming authority — FXBANK / PAC / PNST

Date: 2026-09-27  
Branch: `reverse/em034-lady-runtime-20260927`

## Target

- executable: `dmc3.exe`
- size: `6,356,432`
- SHA-256: `e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`
- ImageBase: `0x140000000`

The executable used for this pass was hash-verified before disassembly.

## Scope

This pass answers one narrow question: whether lower-level children of packed PAC/PNST resources receive original runtime filenames, and specifically how `em034_028.pnst` FXBANK records are identified by the canonical executable.

It does not assign gameplay semantics to effect kinds and it does not invent extraction names.

## 1. FXBANK outer wrapper

Function `0x1402C04C0` reads a PNST-like relative-slot container:

- `[container+0x04]` -> slot count;
- offset at `+0x08` -> physical slot 0;
- offset at `+0x0C` -> physical slot 1.

For a bank it resolves:

- physical slot 0 -> manifest text pointer;
- physical slot 1 -> nested PNST record-container pointer.

It then enters the main bank walk at `0x1402C0500`.

No filename string is recovered from the outer PNST header.

## 2. FXBANK main walk

At `0x1402C0500` the runtime:

1. initializes the text scanner;
2. tokenizes manifest text with `0x140322AB0`;
3. terminates when the current token begins with `#`;
4. parses the next token as a decimal integer through `0x140322A60`;
5. selects the next physical record from the nested PNST;
6. dispatches by the first character of the manifest kind token.

The relevant instruction window is `0x1402C05D0..0x1402C06F5`.

The manifest therefore supplies exactly two record-identity values:

```text
kind: one ASCII letter
id:   decimal integer -> passed/stored as u16
```

There is no filename-construction call in this loop.

## 3. Registrar call ABI

The loader passes the following arguments.

### A / C / E / G / P / V

```text
RCX = global registrar/table object
RDX = record payload pointer
R8  = u16 id
```

Calls:

| kind | registrar |
|---|---:|
| A | `0x140322990` |
| C | `0x1402D3BE0` |
| E | `0x1402E87A0` |
| G | `0x1402ECBD0` |
| P | `0x140314B80` |
| V | `0x140325030` |

### T

```text
RCX = texture table object
RDX = record payload pointer
R8  = u16 id
R9  = mode byte
additional stack flag used by texture construction path
```

Registrar: `0x140322F20`.

### M

```text
RCX = model/effect-model table object
RDX = u16 id
R8  = current record payload pointer
R9  = next physical slot payload pointer
```

Registrar: `0x1402E35D0`.

The M branch advances one additional physical slot before registration, so the companion has no separately parsed manifest kind/id.

## 4. Registrar storage proves ID-based identity

The registrar prologs do not receive a resource-name pointer.

Recovered storage patterns:

- `A @ 0x140322990`: searches existing u16 IDs, then stores `id + payload pointer`;
- `E @ 0x1402E87A0`: searches existing u16 IDs, then stores `id + payload pointer`;
- `G @ 0x1402ECBD0`: searches existing u16 IDs, then stores `id + payload pointer`;
- `V @ 0x140325030`: searches existing u16 IDs, then stores `id + payload pointer`;
- `M @ 0x1402E35D0`: searches existing u16 IDs, then stores `id + record pointer + companion pointer`;
- `T @ 0x140322F20`: searches existing u16 IDs, then stores `id + mode + source pointer + materialized texture pointer`;
- `P @ 0x140314B80`: searches u16 IDs, parses the payload, then stores `id + parsed object pointer`;
- `C @ 0x1402D3BE0`: searches/stores by u16 ID and payload-derived fields.

For the FXBANK path, the canonical runtime key is therefore:

```text
(kind, u16 id)
```

not a child filename.

## 5. Generic packed PNST traversal agrees

The independent packed-child dispatcher `0x1401B9FA0` recursively traverses PNST children by:

1. reading the PNST count;
2. resolving the next child pointer from the relative offset table;
3. recursively calling `0x1401B9FA0(child_pointer)`.

No child filename or path argument is passed during this packed traversal.

Typed children are selected from their bytes (for example MOD/EFM/SCM/SHW prefixes), not from a generated nested filename.

This is separate from the top-level resource resolver and from the loose `.lst` fallback.

## 6. Naming-authority consequence

For packed PAC/PNST children, the current executable evidence does **not** establish an original runtime filename generated from:

- parent stem;
- physical slot;
- extracted ordinal;
- effect kind;
- effect ID.

In particular, no evidence supports runtime filenames such as:

```text
G214.*
V010.*
E745.*
em034_028_000.*
slot_0000.*
```

unless a separate stored-name, embedded-name, external extraction metadata, or loose-list source proves that exact spelling.

The FXBANK manifest is a runtime identity manifest for `kind + id`; it is not a filename list.

## 7. Native Reader correction

Current Native Reader code at commit `a15329906e498d468ee5641b1ca5643aaf1761b0`,
`app/src/main/cpp/modules/module_effects.cpp::record_filename()`, constructs presentation names with:

```cpp
"%c%03u"
T -> ".dds"
M -> ".efm" or ".mod"
others -> ".fx" + lowercase(kind)
```

That function is **TOOL_SYNTHETIC**.

It is not recovered from `dmc3.exe`, and its zero-padding and `.fxA/.fxE/... `-style suffixes must not be used as historical/runtime naming evidence.

Also, a physical T record is descriptor + DDS. The nested DDS payload may legitimately be exposed as a DDS child view, but the whole physical T record is not itself a raw DDS file.

## 8. Relation to .index / .lst

Existing canonical reverse remains unchanged:

- external `.index` is extraction/display metadata, not recovered original-runtime packed-child naming authority;
- `.lst` is an EXE-confirmed loose representation: names are used to locate loose children before an in-memory packed container is synthesized;
- after packed materialization, child traversal is pointer/slot/content based.

Therefore extraction naming and runtime packed identity must remain separate.

## 9. Export policy from this evidence

Until a stronger naming authority is recovered:

1. preserve physical slot identity separately;
2. change only a file's **format extension** when its format is proven;
3. do not insert semantic labels such as `model`, `generator`, `costume`, etc. into filenames;
4. do not use `kind+id` as a filename unless the product explicitly labels it as a synthetic display alias;
5. keep a neutral stem for unnamed packed records;
6. retain the FXBANK `kind/id` pair as metadata, not as a claimed original filename.

## Status

| claim | status |
|---|---|
| FXBANK outer slot0 = manifest, slot1 = nested record PNST | EXE_CONFIRMED |
| manifest parser supplies one-letter kind + decimal id | EXE_CONFIRMED |
| FXBANK registrars receive child filename/path | REJECTED for recovered call ABI |
| registrar identity is keyed by u16 id | EXE_CONFIRMED |
| M companion receives independent manifest id/name | REJECTED |
| packed PNST recursion passes child filename | REJECTED for `0x1401B9FA0` |
| Native Reader `%c%03u` names are original runtime names | REJECTED |
| exact historical extractor spelling for unnamed children | RESEARCH_REQUIRED |

## Next reverse gate

Continue from three surfaces:

1. `0x1401B7C70..0x1401B85C0` — loose `.lst` child-name acquisition and how those names are discarded/retained during synthesis;
2. embedded slot-0 name-list cases (for example stage resources) — determine whether any original runtime consumer uses them as names or whether they are data/tool-side aliases only;
3. every em034 nested PAC/PNST family — classify which descendants have a real stored/loose-list name and which are intrinsically ID/slot-addressed records.
