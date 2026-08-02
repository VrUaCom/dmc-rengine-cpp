# ADR-0001 — Defer GUI Technology Selection

## Status

Accepted.

## Context

DMC Rengine needs a desktop shell, Spider Hub, dockable editors, 3D viewports, menu/HUD editing, evidence panels, and deployment on Windows. Candidate technologies include Qt 6, Dear ImGui, and custom rendering integrations.

Selecting a GUI stack before the resource, evidence, patch, and executable contracts stabilize risks allowing UI state to define domain ownership and reintroduce direct file access.

## Decision

Defer the final GUI technology decision until the following are stable and green in CI:

- GDSpaces resource/source contracts;
- Evidence Packet schema;
- read-only PE inspection;
- working-copy/guarded patch boundary;
- one StageBundle assembly vertical slice;
- Binary Inspector domain model.

CLI vertical slices remain the primary integration test surface.

## Consequences

Positive:

- domain APIs remain independent from UI framework types;
- easier unit/integration testing;
- reduced risk of a second resolver in view models;
- clearer evaluation of Qt/ImGui/rendering requirements.

Negative:

- no immediate polished Spider Hub application;
- visual design work remains mockup/specification work;
- some deployment questions remain open.

## Future evaluation criteria

- native Windows desktop integration;
- docking and multi-window support;
- accessibility and localization;
- 3D viewport embedding;
- high-DPI behavior;
- plugin/module boundaries;
- packaging and update strategy;
- testability;
- license compatibility;
- ability to keep file/source ownership outside UI widgets.
