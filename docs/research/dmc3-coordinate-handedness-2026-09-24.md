# DMC3 model and stage coordinates are right-handed

Date: 2026-09-24.

Executable: `dmc3.exe`, SHA-256
`e454272ed0fb0247fcbcf300e5d55d7a3e96d50b89b9ffaff81bb978dcbdd082`.

## Evidence

- The sword attach records of `CPlWpSword` (`0x14058C010`) put state 2 on body
  joint 9 and state 3 on body joint 13. The player motion script names state 2
  the right hand, and it is the state keyed in the sword attacks (see
  `dmc3-player-motion-script-2026-09-24.md`).
- In the rest pose of `pl000` and `pl001`, joint 9 (the end of the 6→9 arm
  chain) is at x = −71 and joint 13 is at x = +71. The toes (joints 18 and 22)
  sit 9.5 units ahead of the ankles on +Z, so the model faces +Z.
- A figure that faces +Z with its right hand on −X lives in a right-handed
  frame with Y up.

## Consequence for viewers

A camera that looks down +Z and puts +X on the right of the screen, which is
the Direct3D left-handed convention, shows every model and every stage as its
mirror image. The Devil May Cry office in `st000.pac` then shows with its
layout reversed, and the sword appears in Dante's left hand.

Native Reader mirrored X in this way up to v61. From v62 its renderer negates
world X before the camera rotation, for positions and for normals. The test in
`core_model_pipeline_test` asserts that a point on +X is drawn to the left of
the origin when the camera looks down +Z.

## Open

This note does not identify where the game's own view or projection matrix
sets its handedness. The data above is enough to place the right hand, but not
to say how the game builds its view.
