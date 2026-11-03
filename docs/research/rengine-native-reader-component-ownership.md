# Rengine owns reusable functionality; applications own platform UX

The standalone HITS Editor is developed inside `dmc-rengine-cpp`, on
`feature/hits-editor-mvp`. Its output applications are independent Windows and
Android products; reusable functionality remains available to the game engine.

| Component | Engine ownership | Current state |
|---|---|---|
| Binary resource parsing | `formats`, `ReaderCore` | Already shared with Native Reader |
| HITS authoring and canonical writer | `hits::editor::Session` | Implemented |
| SCM world-space collision extraction | `hits::scm_import` | Implemented |
| Camera/projection/picking/overlay controller | `hits::viewport::Controller` | Added in HITS Editor preview |
| GDSpaces resource identity and reintegration | `ResourcePayload`, GDS/PAC adapters | Existing boundaries; preview opens loose resources |
| Android document picker, touch, activity | Standalone application shell | New preview shell |
| Windows dialogs, window, mouse input | Standalone application shell | New preview shell |
| Native Reader textured renderer | Engine rendering component after dependency audit | Not migrated by this change |
| Native Reader animation/effects/assembly | Engine runtime components with original evidence gates | Not migrated by this change |
| Native Reader inspection, galleries and resource graph | Engine services plus application-specific presentation | Not migrated by this change |

The engine must eventually expose each reusable Native Reader capability, rather
than require launching the reader application. This is an ownership goal, not a
claim that its current functionality has all been ported. Migration must inventory
the actual Native Reader sources, pin a revision, identify Rengine dependencies,
preserve tested transforms/evidence contracts, then validate each moved module
against both applications. Platform file dialogs and activity lifecycle stay in
shells; formats, scene evaluation, rendering and authoring stay reusable.

The current preview uses a new small untextured orthographic viewport, following
Native Reader's existing CPU-preview/core-shell pattern. Its renderer is not a
replacement for the eventual engine renderer, and does not provide Native Reader
feature parity. The full application and all new source are versioned in Rengine.
