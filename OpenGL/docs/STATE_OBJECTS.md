# State Objects

The engine uses state objects to reduce long argument lists, avoid scattered globals and keep data together.

A state object stores data, a system manipulates that data.

## State object guidlines

High-level systems use state objects.

```cpp
updateGameplay(windowState.handle, appState, gameState);

renderSceneToFBO(
    fboState,
    shaders,
    modelState,
    appState,
    gameState
);

drawDebugTerminal(
    uiState,
    shaders,
    appState
);
```

Low-level helper functions should receive only the exact objects they operate on.

```cpp
drawRoom(shader, roomModel);
drawPlayer(shader, playerModel, player);
calculateFlickerMultiplier(light, time, index, lightingDebugState);
```

## `AppState`

**File:** `src/include/AppState.h`

Stores application-level runtime state.

`AppState` is for data that belongs to the running application, window, frame, or UI shell.

It does not store world gameplay data.

### Contains
- framebuffer width
- framebuffer height
- framebuffer resize flag
- debug terminal visibility
- mouse tracking values
- frame timing values

### Used by
- `WindowSystem`
- `FrameTimingSystem`
- `FramebufferSystem`
- `ScreenRenderer`
- `SceneRenderer`
- `GameUIRenderer`
- `DebugTerminal`
- `DebugInputSystem`
- `GameplaySystem`

## `FrameTiming`

**File:** `src/include/AppState.h`

Stores frame timing values inside `AppState`.

### Contains
- current frame time
- previous frame time
- delta time
- maximum allowed delta time

### Updated by
```cpp
updateFrameTiming(appState);
```

### Important
`deltaTime` is clamped to prevent very large simulation jumps after lag, breakpoints, or window stalls.

## `GameState`

**File:** `src/include/GameState.h`

Stores active gameplay and world state.
GameState is what the game world depends on.

### Contains
- current room
- active room model pointer
- player
- camera
- fade state
- room transition state
- interaction runtime state
- lighting debug state

### Used by
- `GameBootstrap`
- `Input`
- `GameplaySystem`
- `RoomTransitionSystem`
- `CameraSystem`
- `SceneRenderer`
- `ScreenRenderer`
- `GameUIRenderer`
- `DebugInputSystem`
- `LightingSystem`

### Important
Avoid creating duplicate runtime objects such as:

```cpp
Room testRoom;
Player player;
Camera camera;
FadeState fadeState;
InteractionRuntimeState interactionState;
LightingDebugState lightingDebugState;
```

Keep everything to GameState.

## `AppContext`

**File:** `src/include/AppContext.h`

Connects GLFW callbacks to both `AppState` and `GameState`.
GLFW only allows one user pointer per window.

`AppContext` lets that one pointer provide access to both major state objects.

```cpp
glfwSetWindowUserPointer(window, &appContext);
```

### Used by
- framebuffer resize callback
- mouse callback
- scroll callback

### Important
`AppContext` does not own the states.

It only points to states created in `main()`.

```cpp
struct AppContext {
    AppState* appState = nullptr;
    GameState* gameState = nullptr;
};
```

## `WindowState`

**File:** `src/include/WindowSystem.h`

Stores the GLFW window handle.

```cpp
struct WindowState {
    GLFWwindow* handle = nullptr;
};
```

### Used by
- `initializeWindow`
- `shouldWindowClose`
- `swapWindowBuffers`
- `pollWindowEvents`
- `shutdownWindow`

Window ownership belongs to `WindowSystem`.

## `ShaderState`

**File:** `src/include/ShaderSystem.h`

Stores all shader programs used by the engine.

### Contains
- scene shader
- screen shader
- UI note shader
- text shader
- UI rectangle shader

### Used by
- `SceneRenderer`
- `ScreenRenderer`
- `GameUIRenderer`
- `DebugTerminal`

### Important
High-level render and UI functions should receive `ShaderState`.

Private low-level helper functions may still receive individual `Shader&` values.

## `ModelState`

**File:** `src/include/ModelSystem.h`

Stores shared loaded models.

### Currently contains
- player model

### Used by
- `SceneRenderer`

### Important
Room models are handled separately by the room model cache inside `GameState`.

## `FramebufferState`

**File:** `src/include/FramebufferSystem.h`

Stores offscreen framebuffer resources.

### Contains
- framebuffer object
- framebuffer color texture
- depth/stencil renderbuffer


### Used by
- `initializeFramebuffer`
- `resizeFramebufferAttachments`
- `destroyFramebuffer`
- `renderSceneToFBO`
- `renderScreen`

Do not manually delete framebuffer resources in `source.cpp`.

## `QuadMeshState`

**File:** `src/include/QuadMeshSystem.h`

Stores the VAO and VBO for the full-screen quad.

### Contains
- quad VAO
- quad VBO

### Used by
- `ScreenRenderer`
- `GameUIRenderer`

The same full-screen quad is used for screen rendering and note overlays.
Do not manually delete quad resources in `source.cpp`.

## `UiState`

**File:** `src/include/UiSystem.h`

Stores UI rendering resources and UI configuration.

### Contains
- text renderer
- UI rectangle renderer
- prompt scale

### Used by
- `GameUIRenderer`
- `DebugTerminal`

`UiState` owns UI renderer objects.

## Summary

The current state object structure is:

```text
AppState
    application/window/frame/UI-shell timing state

GameState
    current gameplay and world state

AppContext
    bridge between GLFW callbacks and state objects

WindowState
    GLFW window handle

ShaderState
    shader programs

ModelState
    shared loaded models

FramebufferState
    framebuffer resources

QuadMeshState
    full-screen quad resources

UiState
    UI renderers and UI configuration
```

Related data is grouped together so the code structure stays readable and each system receives the necessary state, 
lowering the risk of confusing data objects.
