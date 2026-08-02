## Summary

Describe what changed and why.

## Scope

- [ ] Code
- [ ] Tests
- [ ] Documentation
- [ ] Reverse-engineering evidence
- [ ] Build/CI
- [ ] Brand/UI

## Architecture

- [ ] Resources still enter tools only through GDSpaces.
- [ ] No independent container/path resolver was added.
- [ ] Ownership and lifetime are explicit.
- [ ] Working-copy/export boundaries are preserved.
- [ ] EXE changes, if any, use guarded patch plans.

## Evidence

Confidence: `hypothesis | candidate | low | medium | high | confirmed | corrected | rejected | not-applicable`

Evidence packet, issue, specification, or test references:

## Validation

Commands run:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Additional runtime or fixture validation:

## Legal/content check

- [ ] No proprietary game files or leaked source are included.
- [ ] Fixtures are synthetic or clearly authorized.
- [ ] Logs and paths are sanitized.

## Risks and rollback

Describe compatibility risks, unresolved assumptions, and how to revert the change.
