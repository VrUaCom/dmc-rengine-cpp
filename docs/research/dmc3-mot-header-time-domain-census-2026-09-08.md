# DMC3 HD MOT — em000 header/time-domain census (2026-09-08)

**Branch:** `reverse/mot-em000-20260908`  
**Corpus:** `em000-extract.zip`  
**Corpus SHA-256:** `306130125f09824811289366324f4208c3c1aba880c5a7efa3953a88d566d07b`  
**Scope:** all 82 signature-confirmed MOT payloads, 5,118 tracks, 94,875 known-compression keys.

## Purpose

This pass tests the four raw header values at `+0x08/+0x0C/+0x10/+0x14`, the two raw u16 values at `+0x18/+0x1A`, track-local `+0x06`, and the high bit of the key time/control word against the complete em000 MOT corpus.

The goal is to separate what the bytes prove from names found in historical/community tooling.

---

## 1. Header float/u32 domain

Across all 82 MOT resources:

```text
+0x08 interpreted as f32 = 0.0     82 / 82
+0x0C f32 == +0x14 f32             82 / 82
+0x10 f32 <= +0x14 f32             82 / 82
```

`+0x0C/+0x14` are finite, non-zero, integer-valued floats in the bound corpus. They have 69 distinct observed values and range from `1.0` to `471.0`.

`+0x10` has only seven observed values:

```text
0, 24, 36, 78, 125, 297, 349
```

Histogram-level fact:

```text
+0x10 == 0.0 : 76 / 82
+0x10 != 0.0 :  6 / 82
```

The six non-zero cases are:

```text
end/mirror 53   raw +0x10 36
end/mirror 26   raw +0x10 24
end/mirror 237  raw +0x10 125
end/mirror 141  raw +0x10 78
end/mirror 298  raw +0x10 297
end/mirror 350  raw +0x10 349
```

A historical/community DMC3 importer names these four fields approximately as two start/end frame pairs. The corpus is compatible with that interpretation, but **the semantic names are not promoted here** because the canonical executable consumer has not yet been rebound.

Safe status:

```text
+0x08  CORPUS_CONFIRMED zero in em000, semantics open
+0x0C  CORPUS_CONFIRMED mirrored finite frame/time-like scalar
+0x10  CORPUS_CONFIRMED bounded secondary scalar
+0x14  CORPUS_CONFIRMED mirror of +0x0C in em000
```

---

## 2. Header +0x18 and +0x1A

`+0x18` observed values:

```text
0 : 63
1 : 19
```

`+0x1A` observed values:

```text
0  : 51
22 : 27
3  :  3
4  :  1
```

The important structural relationship is:

> Whenever `+0x1A` is non-zero, it equals `channel_domain_count` at `+0x1C`.

That is true for all `31 / 31` non-zero instances.

Joint histogram `(raw18, raw1A, channel_domain_count)`:

```text
(0,  0, 22) : 40
(0, 22, 22) : 21
(1,  0, 22) : 10
(1, 22, 22) :  6
(1,  3,  3) :  2
(1,  4,  4) :  1
(0,  0,  3) :  1
(0,  3,  3) :  1
```

Safe conclusion:

- `+0x18` is a two-valued raw control field in em000 (`0/1`);
- `+0x1A` is either zero or a mirror/reference to the complete channel-domain count;
- neither field receives a high-level runtime name until direct EXE evidence exists.

---

## 3. Track +0x06 is zero in the complete em000 set

Every one of the 5,118 tracks has:

```text
track +0x06 u16 = 0
```

Historical/community code calls this value a track start time. The em000 corpus cannot prove that semantic because it provides no variation.

Status:

```text
RESERVED_OBSERVED_ZERO for em000 at the value level
PRESERVED_UNDECODED globally
```

It must remain serialized/preserved in the parser and future writer.

---

## 4. Header end-like scalar versus observed key times

After applying the proven structural projection:

```text
key_time_index = time_control & 0x7FFF
```

we compared each document's maximum observed key index against `header +0x0C/+0x14`.

Result:

```text
header +0x0C == maximum key_time_index : 77 / 82
```

The five exceptions are instructive:

1. one constant-pose clip has header value `10` while every key index is `0`;
2. one clip has header `63` while some tracks reach `67`;
3. one clip has header `43` while three single-key tracks carry index `125` and the animated runs close at `43`;
4. one compact 0x30-header clip has header `49` while keys reach `61`;
5. one three-track clip has header `1` while all key indices are `0`.

Therefore it is unsafe to define `+0x0C/+0x14` merely as:

```text
max(serialized key time)
```

The field is clearly time/frame-domain related, but clips can contain constants, preroll/postroll-like key positions, or other key-domain details not represented by a naïve maximum. Exact evaluation behavior belongs to the CMotion evaluator reverse.

---

## 5. Key high bit is strongly compression-correlated

Known-compression key census:

```text
compression 2 keys:     459
compression 3 keys:  94,416
total:               94,875
```

High bit of `time_control`:

```text
compression 2:
    bit15 set     0 / 459
    bit15 clear 459 / 459

compression 3:
    bit15 set    94,410 / 94,416
    bit15 clear       6 / 94,416
```

The six compression-3 exceptions are real and occur at ordinary masked time indices (`7`, `20`, `43`) rather than only at document boundaries.

This rejects two oversimplifications:

```text
bit15 is just the sign bit of a signed time        REJECTED
bit15 is always synonymous with compression == 3  REJECTED
```

Safe interpretation remains:

```text
bits 0..14 -> key time index       CORPUS_CONFIRMED
bit 15     -> separate control bit PRESERVED_UNDECODED
```

The very strong compression correlation makes bit15 a high-value evaluator target, but it is not enough to name it as tangent/interpolation/segment behavior.

---

## 6. Relationship to old st001 evidence

The historical 63,440-byte st001 MOT had its first key component interpreted as signed s16, producing apparent stamps:

```text
-32768 ... -32118
```

The current decomposition correctly reads the same raw bit pattern as:

```text
0x8000 ... 0x828A
```

therefore:

```text
time_index = 0 ... 650
bit15      = 1
```

and the resulting `0..650` domain agrees with the header's `650.0` scalar. This independently supports the lower-15-bit time rule while leaving bit15 semantics separate.

---

## 7. Next direct-EXE gates

The highest-value machine-code targets are now extremely narrow:

1. load of header `+0x08/+0x0C/+0x10/+0x14` into CMotion state;
2. branch or copy involving `+0x18/+0x1A`;
3. compression switch for values `2/3`;
4. exact use of `time_control & 0x7FFF` and its bit15 branch;
5. interpolation boundary behavior for the five clips where header scalar != raw maximum key index;
6. channel-mask bit selection -> CMotionJoint transform component.

Until those consumers are recovered, the parser stays structurally exact and semantics-neutral.
