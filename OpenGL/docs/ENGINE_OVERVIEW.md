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
- framebuffer-based screen effects

## Core architecture

The engine is organized around state objects and systems.

In general, state objects store data and systems perform actions using state's data.

This keeps `source.cpp` focused on program flow rather than handling gameplay, rendering, UI and setup logic.

## Main state objects

### `AppState`

`AppState` stores application-level runtime data.

This includes:

- framebuffer width
- framebuffer height
- framebuffer resize flag
- debug terminal visibility
- mouse tracking values
- frame timing values

```cpp
appState.framebufferWidth;
appState.framebufferHeight;
appState.framebufferResized;
appState.debugTerminalOpen;
appState.timing.deltaTime;
appState.timing.currentTime;
```

`AppState` should be used for app-level systems such as:

- window resize handling
- framebuffer resizing
- frame timing
- UI layout
- debug terminal visibility
- screen effects that depend on time

`AppState` should not own world gameplay data such as the current room, player, camera, or active interaction.

### `GameState`

`GameState` stores active gameplay/world data.

This includes:

- current room
- active room model
- player
- camera
- fade state
- room transition state
- interaction runtime state
- lighting debug state

Examples:

```cpp
gameState.room;
gameState.roomModel;
gameState.player;
gameState.camera;
gameState.fade;
gameState.roomTransition;
gameState.interaction;
gameState.lightingDebug;
```

The main loop should use `GameState` instead of keeping separate duplicate runtime variables.

Avoid this:

```cpp
Room testRoom;
Player player;
Camera camera;
FadeState fadeState;
InteractionRuntimeState interactionState;
```

Prefer this:

```cpp
GameState gameState;

gameState.room;
gameState.player;
gameState.camera;
gameState.fade;
gameState.interaction;
```

This prevents bugs where gameplay updates one object but rendering reads from another.

### `AppContext`

GLFW only allows one user pointer per window.

The engine uses `AppContext` to give GLFW callbacks access to both `AppState` and `GameState`.

```cpp
struct AppContext {
    AppState* appState = nullptr;
    GameState* gameState = nullptr;
};
```

This is used by:

- framebuffer resize callback
- mouse callback
- scroll callback

The window stores the context like this:

```cpp
glfwSetWindowUserPointer(window, &appContext);
```

## Setup state objects

The engine also uses smaller setup/resource state objects.

These keep `source.cpp` from owning every OpenGL object directly.

### `WindowState`

Stores the GLFW window handle.

Used by:

- `initializeWindow`
- `shouldWindowClose`
- `swapWindowBuffers`
- `pollWindowEvents`
- `shutdownWindow`

### `ShaderState`

Stores the engine shader programs.

This includes:

- scene shader
- screen shader
- UI note shader
- text shader
- UI rectangle shader

High-level rendering/UI functions receive `ShaderState`.

Low-level helpers can still use individual `Shader&` values.

### `ModelState`

Stores shared loaded models.

Currently this includes:

- player model

Room models are loaded and cached through the room model cache.

### `FramebufferState`

Stores the offscreen framebuffer resources.

This includes:

- framebuffer object
- framebuffer color texture
- depth/stencil renderbuffer

Used by:

- scene rendering
- screen rendering
- framebuffer resize
- framebuffer cleanup

### `QuadMeshState`

Stores the VAO and VBO for the full-screen quad.

Used by:

- screen rendering
- note overlay rendering

### `UiState`

Stores UI rendering resources and UI configuration.

This includes:

- `TextRenderer`
- `UiRenderer`
- prompt scale


## Main runtime flow

The current main loop performs roughly this sequence:

```text
1. Update frame timing
2. Handle framebuffer resize
3. Process player/system input
4. Update lighting debug hotkeys
5. Update debug terminal hotkeys
6. Update fade
7. Handle room transition
8. Update gameplay
9. Update active room camera
10. Render 3D scene to the framebuffer
11. Render the framebuffer to screen
12. Draw UI prompt if necessary
13. Draw note overlay if the player interacts with one
14. Draw debug terminal if open
15. Swap buffers and poll events
```

A simplified version of the main loop looks like this:

```cpp
while (!shouldWindowClose(windowState)) {
    updateFrameTiming(appState);

    if (appState.framebufferResized) {
        resizeFramebufferAttachments(fboState, appState);
    }

    processInput(windowState.handle, gameState);

    updateLightningDebugInput(windowState.handle, appState, gameState);

    updateDebugInput(windowState.handle, appState, gameState);

    updateFade(gameState, appState);

    updateRoomTransition(gameState);

    std::string promptText = updateGameplay(
        windowState.handle,
        appState,
        gameState
    );

    if (!gameState.roomTransition.transitioning) {
        updateRoomCamera(gameState);
    }

    renderSceneToFBO(
        fboState,
        shaders,
        modelState,
        appState,
        gameState
    );

    renderScreen(
        quadMeshState,
        shaders,
        fboState,
        appState,
        gameState
    );

    drawInteractionPrompt(
        promptText,
        uiState,
        shaders,
        appState,
        gameState
    );

    drawNoteOverlay(
        gameState,
        shaders,
        quadMeshState
    );

    drawDebugTerminal(
        uiState,
        shaders,
        appState
    );

    swapWindowBuffers(windowState);
    pollWindowEvents();
}
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

UI is draw after the screen quad, on top of the final rendered scene.

## Scene rendering

The scene renderer draws the 3D world.

```cpp
renderSceneToFBO(
    fboState,
    shaders,
    modelState,
    appState,
    gameState
);
```

The scene pass draws:
- room model
- props
- player model
- room lighting
- ambient color

The pass renders into the framebuffer inside a `FramebufferState`.

## Screen rendering

The screen renderer draws the framebuffer texture to the real window.

```cpp
renderScreen(
    quadMeshState,
    shaders,
    fboState,
    appState,
    gameState
);
```

This is wehere screen shader effects are applied.

The screen pass uses:
- `FramebufferState` for the rendered scene texture
- `QuadMeshState` for the full-screen quad
- `ShaderState` for the screen shader
- `AppState` for screen size and time
- `GameState` for fade state

## UI rendering

UI is drawn on top of the 3D scene and screen pass.

Current UI:
- interaction prompts
- note overlays
- debug terminal

Promp is rendered with:

```cpp
drawInteractionPrompt(
    promptText,
    uiState,
    shaders,
    appState,
    gameState
);
```

Note rendering:

```cpp
drawNoteOverlay(
    gameState,
    shaders,
    quadMeshState
);
```

Debug terminal rendering:

```cpp
drawDebugTerminal(
    uiState,
    shaders,
    appState
);
```

## Room transition

Door transitions are handled through `RoomTransitionSystem`.

The room transition flow:

```text
1. Player interacts near a door
2. Gameplay starts the room transition
3. Fades to black
4. When the screen is black, the next room is loaded
5. The room model cache loads/returns the room model
6. Player is moved to the door target spawn
7. Camera updates for the new room
8. Fade disappears until the whole screen is visible
```

The transition is started with:

```cpp
beginRoomTransition(
    gameState,
    targetRoomJson,
    targetSpawn
);
```

The transition is updated in the main loop with:

```cpp
updateRoomTransition(gameState);
```

## Interaction flow

`GameplaySystem` checks nearby door and interactions.

It returns prompt text to the main loop:

```cpp
std::string promptText = updateGameplay(
    windowState.handle,
    appState,
    gameState
);
```

That prompt text is passed to `GameUIRenderer`.

Interactions currently open:
- messages
- notes

Active interaction data lives in `gameState.interaction`

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

### High-level systems should receive state objects

Good:

```cpp
updateGameplay(windowState.handle, appState, gameState);
renderSceneToFBO(fboState, shaders, modelState, appState, gameState);
drawDebugTerminal(uiState, shaders, appState);
```

Avoid this in high-level systems:

```cpp
renderSceneToFBO(
    framebuffer,
    sceneShader,
    playerModel,
    room,
    player,
    camera,
    width,
    height,
    time,
    lightingDebugState
);
```

Passing whole state objects keeps the main loop readable and avoids bugs where systems read from different 
copies of the same state.

### Low-level helpers should receive exact objects

Low-level helper functions do not need every state object.

Good:

```cpp
drawRoom(shader, roomModel);
drawPlayer(shader, playerModel, player);
calculateFlickerMultiplier(light, time, index, lightingDebugState);
```

This keeps private helpers simple and avoids unnecessary coupling.

## Current architecture goal

The goal is for `source.cpp` to be readable at a glance.

It should answer:

- what initializes first
- what updates first
- what renders first
- what shuts down last

Detailed implementation should live in the relevant system files.

The main loop should describe the engine flow.

The system files should contain the actual implementation details.
