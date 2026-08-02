# Clean-Room and Repository Content Policy

This document defines the repository's engineering hygiene. It is project policy, not legal advice.

## Purpose

DMC Rengine documents independently observed interfaces, formats, behavior, and compatibility requirements while keeping proprietary game content outside the public repository.

## Allowed public material

- original C++ implementation written by contributors;
- interface descriptions and independently derived structures;
- hashes and short identifiers needed to distinguish user-supplied versions;
- offsets, RVA/VA references, field layouts, state-machine descriptions, and behavioral summaries;
- small factual byte signatures when necessary for guarded validation;
- original diagrams, documentation, tests, and synthetic fixtures;
- reproducible procedures that require users to supply their own legally obtained files.

## Prohibited material

- game executables, DLLs, archives, textures, models, audio, scripts, videos, fonts, or other proprietary assets;
- large copied binary regions or reconstructed asset packs;
- leaked source code, confidential SDKs, private symbols, or stolen credentials;
- wholesale decompiler output presented as project source;
- circumvention tools whose primary purpose is bypassing access controls rather than interoperability, research, or modding workflows;
- download links to unauthorized copies.

## Evidence handling

Evidence records should identify local artifacts by cryptographic hash, logical role, and user-provided path aliases. Public reports should not embed the artifact itself.

When a short byte sequence is included, document why it is necessary, how it is used, and the exact supported artifact hash. Prefer structural checks over broad copied data.

## Synthetic fixtures

Tests should use generated fixtures that model the required format behavior without copying original content. A synthetic fixture must state:

- what behavior it models;
- which fields are invented;
- which structural facts are independently derived;
- whether round-trip output is expected.

## Recovered source

Recovered C++ is an independent reconstruction of behavior and interfaces. It must not be described as original Capcom source code unless its provenance is legitimately established and publication is authorized.

Every recovered unit should link to an evidence packet and record confidence, unknowns, ABI assumptions, ownership assumptions, and behavioral tests.

## Local-only workspace

Private workspaces may hold user-supplied files for analysis, but repository tools must keep those files outside Git tracking by default. Generated reports should be sanitized before publication.

## Trademark and affiliation

Devil May Cry and related marks belong to their respective owners. DMC Rengine is an independent community project and is not affiliated with or endorsed by Capcom.

## Removal process

If prohibited or questionable content is committed:

1. stop distribution where practical;
2. remove it from the current tree;
3. assess whether history rewriting is required;
4. rotate any exposed credentials;
5. document the corrective action without reproducing the restricted material.
