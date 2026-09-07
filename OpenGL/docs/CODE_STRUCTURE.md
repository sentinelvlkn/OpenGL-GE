# Code Structure

This document explains what each major file or system is responsible for.

For details about state objects such as `AppState`, `GameState`, `ShaderState`, `UiState`, and `FramebufferState`, see:

```text
docs/STATE_OBJECTS.md
```

For the exact runtime order, see:

```text
docs/MAIN_LOOP_FLOW.md
```

---

## Main idea

The engine is split into focused systems.

Each system has a clear responsibility.

`source.cpp` should stay small and should mainly describe:

- what initializes first
- what updates each frame
- what renders each frame
- what shuts down last

Detailed logic should live in the relevant system files.

---

# Entry point

## `source.cpp`

Main entry point for the engine.

**Owns:**
- creation of main state objects
- startup order
- main loop order
- shutdown order

**Calls into:**
- window setup
- shader setup
- game bootstrap
- model setup
- UI setup
- framebuffer setup
- input
- gameplay
- rendering
- debug systems
- cleanup systems

**And it should not own:**
- detailed gameplay logic
- detailed rendering logic
- shader loading details
- framebuffer creation details
- UI font loading details
- GLFW callback implementations
- room transition internals
- debug hotkey internals

`source.cpp` should read like a table of contents for the engine.

---

# Startup and resource systems

## `WindowSystem.h/.cpp`

Owns window and OpenGL context setup.

**Responsible for:**
- initializing GLFW
- creating the window
- setting the GLFW user pointer
- registering GLFW callbacks
- loading GLAD
- enabling basic OpenGL state
- swapping buffers
- polling events
- shutting down the window and GLFW

**Main functions:**

```cpp
initializeWindow(...);
shouldWindowClose(...);
swapWindowBuffers(...);
pollWindowEvents();
shutdownWindow(...);
```

---

## `ShaderSystem.h/.cpp`

Loads and stores shader programs.

**Responsible for:**
- scene shader setup
- screen shader setup
- note/UI shader setup
- text shader setup
- UI rectangle shader setup
- default shader values

**Main functions:**

```cpp
initializeShaders(shaders);
```

---

## `ModelSystem.h/.cpp`

Loads shared models.

**Currently responsible for:**
- player model setup

**Main functions:**
```cpp
initializeModels(modelState);
```

**Important:**
Room models are loaded through the asset cache, not through `ModelSystem`.

---

## `GameBootstrap.h/.cpp`

Initializes the starting game world.

**Responsible for:**
- loading the initial room JSON
- retrieving the initial room model
- placing the player at the room spawn
- setting the starting camera

**Main functions:**
```cpp
initializeGameState(gameState, initialRoomJson);
```

---

## `FramebufferSystem.h/.cpp`

Owns the offscreen framebuffer.

**Responsible for:**
- framebuffer creation
- color texture creation
- depth/stencil renderbuffer creation
- framebuffer resize handling
- framebuffer cleanup

**Main functions:**
```cpp
initializeFramebuffer(fboState, appState);
resizeFramebufferAttachments(fboState, appState);
destroyFramebuffer(fboState);
```

---


## `QuadMeshSystem.h/.cpp`

Owns the full-screen quad mesh.


**Responsible for:**
- quad VAO creation
- quad VBO creation
- quad vertex data upload
- quad cleanup

**Main functions:**
```cpp
initializeScreenQuad(quadMeshState);
destroyScreenQuad(quadMeshState);
```


**Used by:**
- `ScreenRenderer`
- `GameUIRenderer`

---

## `UiSystem.h/.cpp`

Owns UI renderers and UI setup.


**Responsible for:**
- storing `TextRenderer`
- storing `UiRenderer`
- loading the UI font
- storing UI configuration such as prompt scale

**Main functions:**
```cpp
initializeUi(uiState);
```

---

## `FrameTimingSystem.h/.cpp`

Owns frame timing updates.

**Responsible for:**
- initializing timing values
- updating current time
- calculating delta time
- clamping delta time

**Main functions:**
```cpp
initializeFrameTiming(appState);
updateFrameTiming(appState);
```

---

# Gameplay systems

## `Input.h/.cpp`

Handles basic input.


**Responsible for:**
- Escape-to-close behavior
- debug fade keys
- one-shot key detection


**Main functions:**
```cpp
processInput(window, gameState);
keyPressedOnce(window, key);
```

---

## `GameplaySystem.h/.cpp`

Handles one frame of gameplay logic.

**Responsible for:**
- player movement update
- nearby door checks
- nearby interaction checks
- prompt text generation
- interaction open/close behavior
- starting room transitions

**Main functions:**
```cpp
updateGameplay(window, appState, gameState);
```

**Notes:**
This system returns prompt text. It does not draw the prompt.

---

## `PlayerMovement.h/.cpp`

Handles player movement and collision.

**Responsible for:**
- reading movement direction
- moving relative to the current camera
- testing room colliders
- blocking invalid movement

**Called by:**

```cpp
GameplaySystem
```

---

## `InteractionSystem.h/.cpp`

Handles interaction detection and runtime interaction behavior.

**Responsible for:**
- finding nearby doors
- finding nearby interactions
- building interaction prompt text
- running message interactions
- running note interactions
- closing active interactions

**Key types:**
```cpp
NearbyAction
InteractionRuntimeState
```

---

## `RoomTransitionSystem.h/.cpp`

Handles door-based room transitions.

**Responsible for:**
- starting transition state
- fading to black
- loading the pending room
- updating the active room model
- moving the player to the target spawn
- updating camera after room load
- fading back from black

**Main functions:**
```cpp
beginRoomTransition(gameState, targetRoomJson, targetSpawn);
updateRoomTransition(gameState);
```

---

## `CameraSystem.h/.cpp`

Handles fixed-camera updates.

**Responsible for:**
- checking room camera zones
- applying the active room camera to the runtime camera

**Main functions:**
```cpp
updateRoomCamera(gameState);
```

---

## `FadeSystem.h/.cpp`

Handles fade state.

**Responsible for:**
- updating fade value
- starting fade to black
- starting fade from black
- detecting fully black / fully visible states

**Main functions:**
```cpp
updateFade(gameState, appState);
beginFadeToBlack(fadeState);
beginFadeFromBlack(fadeState);
isFullyBlack(fadeState);
isFullyVisible(fadeState);
```

---

# Rendering system

## `SceneRenderer.h/.cpp`

Renders the 3D scene into the offscreen framebuffer.

**Responsible for:**
- binding the framebuffer
- setting scene viewport
- clearing color/depth
- setting camera matrices
- uploading room lighting
- drawing room geometry
- drawing props
- drawing player model

**Main functions:**
```cpp
renderSceneToFBO(
    fboState,
    shaders,
    modelState,
    appState,
    gameState
);
```

**Private helpers include:**
```cpp
uploadRoomLights(...);
drawRoom(...);
drawProps(...);
drawPlayer(...);
```

---

## `ScreenRenderer.h/.cpp`

Draws the framebuffer texture to the actual window.

**Responsible for:**
- binding the default framebuffer
- drawing the full-screen quad
- applying screen-space effects

**Main functions:**
```cpp
renderScreen(
    quadMeshState,
    shaders,
    fboState,
    appState,
    gameState
);
```

---

## `GameUIRenderer.h/.cpp`

Draws gameplay UI.

**Responsible for:**
- interaction prompt
- note overlay

**Main functions:**
```cpp
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
```

---

## `DebugTerminal.h/.cpp`

Draws the in-game debug terminal.

**Responsible for:**
- terminal background
- debug line layout
- text wrapping
- debug text rendering

**Main functions:**
```cpp
drawDebugTerminal(uiState, shaders, appState);
```

---

# Debug systems

## `DebugLog.h/.cpp`

Stores and prints debug messages.

**Responsible for:**
- storing log lines
- printing to external console
- exposing lines to the in-game debug terminal
- clearing stored lines

**Important:**
`DebugLog` should not handle keyboard input.

---

## `DebugInputSystem.h/.cpp`

Handles debug hotkeys.

**Responsible for:**
- toggling the debug terminal
- printing current debug/player/room state

**Main functions:**
```cpp
updateDebugInput(window, appState, gameState);
```

---

## `LightingSystem.h/.cpp`

Handles lighting debug input and light flicker calculations.

**Responsible for:**
- forced lightning flash
- lightning preview mode
- flicker enable/disable
- light debug printing
- spot-as-point debug mode
- flicker multiplier calculation

**Main functions:**
```cpp
updateLightningDebugInput(window, appState, gameState);
calculateFlickerMultiplier(light, time, index, lightingDebugState);
```

---

# Data and loading systems

## `Room.h/.cpp`

Defines room data and loads room JSON.

**Responsible for:**
- room name
- room model path
- player spawn
- camera zones
- colliders
- doors
- interactions
- lights
- props
- JSON parsing
- coordinate conversion for Blender-authored data

---

## `AssetCache.h/.cpp`

Caches loaded models.

**Responsible for:**
- room model caching
- prop model caching
- avoiding duplicate model loads


**Main functions:**
```cpp
getRoomModel(room);
getCachedModel(modelPath);
```

---

## `TextureUtils.h/.cpp`

Loads texture resources.

**Responsible for:**
- regular 2D texture loading
- note texture loading
- cubemap loading if used

**Main functions:**
```cpp
loadTexture(...);
loadTexture2D(...);
loadCubemap(...);
```

---

## `FileSystem.h/.cpp`

Builds paths relative to the project root.


**Main functions:**
```cpp
FileSystem::pathFromRoot(...);
```

---

# UI and render helper classes

## `TextRenderer.h/.cpp`

Renders text using FreeType.

**Used by:**
- interaction prompts
- debug terminal

---

## `UiRenderer.h/.cpp`

Draws simple UI rectangles.

**Used by:**
- interaction prompt background
- debug terminal background

---

# Data types

## `Interaction.h`

Defines interaction data loaded from room JSON.

**Used by:**
- room loading
- interaction detection
- interaction runtime behavior

---

## `AABB.h`

Defines axis-aligned bounding boxes.

**Used by:**
- collision
- camera zones
- doors
- interactions

---

## `Player.h`

Defines player data.

**Used by:**
- movement
- collision
- rendering
- interaction range checks

---

## `Camera.h`

Defines camera behavior.

**Used by:**
- fixed-camera rendering
- camera zone application
- mouse look if enabled
- scroll zoom if enabled

---

# Where new code should go

## New gameplay behavior

Put it in:
```text
GameplaySystem.h/.cpp
```
or a new focused gameplay system if it becomes large.

## New movement behavior

```text
PlayerMovement.h/.cpp
```

## New interactions

Detection/runtime behavior in:
```text
InteractionSystem.h/.cpp
```

Pure interaction data in:
```text
Interaction.h
```

## New room JSON fields

Parsing and coordinate conversion in:
```text
Room.h/.cpp
```

## New rendering behavior

3D world rendering in:
```text
SceneRenderer.h/.cpp
```

Screen-space effects in:
```text
ScreenRenderer.h/.cpp
```

UI drawing in:
```text
GameUIRenderer.h/.cpp
```

## New debug hotkeys

```text
DebugInputSystem.h/.cpp
```

## New setup resources

Create small system with state object when the resource has setup and cleanup responsibilities, e.g:
```text
FramebufferSystem + FramebufferState
ShaderSystem + ShaderState
UiSystem + UiState
```

---

# Organization guidelines

## Keep headers light

When a full type isn't required in a header file, prefer forward declarations for faster builds.

## Have implementation details in `.cpp` files

Private helper function should be in the `.cpp` files.
Use the `static` keyword or anonymous namespace for helpers that aren't called outside the file.

## High-level systems should use state objects

```cpp
updateGameplay(window, appState, gameState);
renderSceneToFBO(fboState, shaders, modelState, appState, gameState);
```

## Low-level helpers receive exact objects

```cpp
drawRoom(shader, roomModel);
drawPlayer(shader, playerModel, player);
calculateFlickerMultiplier(light, time, index, lightingDebugState);
```

That is with the idea to keep some form of flexibility

## Avoid duplicate runtime states

Do not define each object for ```cpp Room, Player, Camera, FadeState and etc```. 
```cpp GameState``` exists to group those objects together.

```cpp
gameState.room;
gameState.player;
gameState.camera;
gameState.fade;
```
