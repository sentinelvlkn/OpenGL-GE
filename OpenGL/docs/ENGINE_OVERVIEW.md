# Engine Overview

This project is a fixed-camera engine inspired by classic survival-horror games.
It is build using the OpenGL API and is mainly written in C++.

The engine currently supports:

- fixed camera rooms
- JSON room loading
- Blender-authored room helper data
- simple player movement and collision
- door transitions with fade
- note/message interactions
- basic room model caching
- point lights and spotlights with effects
- props for interactions (e.g notes)
- FreeType text rendering
- reusable UI rectangles
- in-game debug terminal

## Main runtime flow

The current main loop performs roughly this sequence:

```text
1. Calculate delta time
2. Handle framebuffer resize
3. Process input
4. Update debug hotkeys
5. Update fade
6. Handle room transition
7. Update gameplay if no transition is happening
8. Update active room camera
9. Render 3D scene to the framebuffer
10. Render the framebuffer to screen
11. Draw UI prompt if necessary
12. Draw note overlay if the player interacts with one
13. Swap buffers and poll events
```

## Render flow

The game renders the 3D world into an offscreen framebuffer first.

```text
3D scene -> framebuffer texture -> screen quad -> UI overlays
```

This allows for screen-space effects:
- fade to black
- vignette
- grain
- saturation changes
- contrast/brightness changes

UI is draw after the screen quad.

## Coordinate system

Blender data is exported in Blender coordinates and converted in engine-space:

```cpp
engine.x = blender.x;
engine.y = blender.z;
engine.z = -blender.y;
```

This applies to gameplay/helper data:

- player spawns
- camera positions
- camera targets
- camera zones
- colliders
- doors
- interactions
- lights
- spotlight targets
- prop positions
- spin axes

This rule applies only to objects that are coordinate-dependent.

## Important engine rules

### Room models should use identity transforms

Room geometry should already by exported correctly from Blender.
Do not rotate the full room in C++ at render time.

Usually this would look like:
```cpp
glm::mat4 model = glm::mat4(1.0f);
shader.setMat4("model", model);
roomModel.Draw(shader);
```

And avoid:
```cpp
model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
```

### Event logs should use `DebugLog::log`

Example:
```cpp
DebugLog::log("[ROOM] Loaded room");
```

instead of:
```cpp
std::cout << "Loaded room\n";
```
The latter is still valid but it will not output in the in-game terminal whereas DebugLog::log sends
to both the external console and in-game.




