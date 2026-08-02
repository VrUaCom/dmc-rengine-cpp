# Security Policy

## Supported versions

DMC Rengine is pre-1.0. Security fixes are applied to the default branch. Release support windows will be defined when tagged public releases begin.

## Reporting a vulnerability

Do not open a public issue for vulnerabilities that could expose user files, execute arbitrary code, escape configured directories, corrupt game installations, disclose secrets, or abuse CI/release infrastructure.

Contact the maintainers privately using the contact route in `MAINTAINERS.md`. Include:

- affected commit or version;
- operating system and compiler;
- reproduction steps;
- expected and observed behavior;
- impact assessment;
- a minimal proof of concept when safe;
- suggested mitigation, if known.

## Security boundaries

DMC Rengine processes untrusted binary data. Parsers and sources must assume malformed input.

Required protections include:

- bounds-checked reads;
- integer-overflow checks;
- path traversal prevention;
- canonical root containment for local sources;
- no implicit execution of extracted content;
- no writes to original game data without an explicit working-copy/export contract;
- hash and source-byte guards for executable patches;
- deterministic diagnostics instead of silent recovery where corruption is possible.

## Secrets

Never commit API keys, GitHub tokens, signing keys, local credentials, game platform credentials, or private user data. CI secrets must be scoped to the minimum required permissions.

## Game files

Game binaries and proprietary assets are local user inputs. They must not be uploaded in vulnerability reports or committed to the repository.
