# ADR-0002 — Initial MIT License

## Status

Accepted for the initial public foundation.

## Context

The clean C++ repository needs an explicit software license before broader contribution. The project also needs separate content and clean-room policies because a software license does not authorize redistribution of game assets or third-party material.

## Decision

Use the MIT License for original repository code and documentation unless a file states otherwise.

The license applies only to material owned or licensable by repository contributors. It does not apply to Capcom game files, third-party trademarks, leaked source, or user-supplied proprietary artifacts.

## Consequences

- low friction for use, research, integration, and contribution;
- forks may use the code under MIT terms;
- project policies must separately enforce evidence, clean-room, and repository-content boundaries;
- a future license change for new contributions would require governance review and cannot retroactively revoke existing MIT grants.

## Review trigger

Revisit before 1.0 if the contributor community prefers file-level or strong copyleft, dual licensing, or additional patent terms.
