# Main loop flow

Here is listed the high-level runtime flow of the engine

The main loop lives in `source.cpp`.

`source.cpp` describes the order of operations that are carried out inside the system files.

## `source.cpp`

This file owns:
- startup order
- frame update order
- render order
- shutdown render

It leaves the detailed implementation to the systems:
- GLFW/GLAD setup
- shader setup
- model setup
- UI setup
- framebuffer setup
- screen quad setup
- frame timing math
- gameplay logic
- room transition logic
- rendering internals
- debug input logic

## Startup order

Currently the engine starts in this order:

```text
1. Create core state objects
2. Init window and OpenGL
3. Init shaders
4. Init initial game state
5. Init shared models
6. Init screen quad
7. Init UI
8. Init the framebuffer
9. Init frame timing
```

or in code:

```cpp
AppState appState;
GameState gameState;
AppContext appContext;
WindowState windowState;

if (!initializeWindow(
    windowState,
    appState,
    gameState,
    appContext,
    INITIAL_WIDTH,
    INITIAL_HEIGHT,
    "OPENGL"
)) {
    return -1;
}

ShaderState shaders;
initializeShaders(shaders);

initializeGameState(
    gameState,
    "src/models/test_room/test_room.json"
);

ModelState modelState;
initializeModels(modelState);

QuadMeshState quadMeshState;
initializeScreenQuad(quadMeshState);

UiState uiState;
initializeUi(uiState);

FramebufferState fboState;
if (!initializeFramebuffer(fboState, appState)) {
    destroyScreenQuad(quadMeshState);
    shutdownWindow(windowState);
    return -1;
}

initializeFrameTiming(appState);
```


## Per-frame order

Every frame follows this order:
```text
1. Update frame timing
2. Handle framebuffer resize (if it happens)
3. Process input
4. Update lighting debug input
5. Update debug input
6. Update fade (if it is called)
7. Update room transition (if there is any)
8. Update gameplay
9. Update room camera (if the player moves to other camera zone or room switches)
10. Render scene to framebuffer
11. Render framebuffer to screen
12. Draw interaction prompt (if near an interaction)
13. Draw note overlay (if player interacts)
14. Draw debug terminal (if open)
15. Swap buffers
16. Poll window events
```

## Main loop

A rough idea of what the main loop should look like in code:

```cpp 
while (!shouldWindowClose(windowState)) {
    updateFrameTiming(appState);

    std::string promptText;

    if (appState.framebufferResized) {
        resizeFramebufferAttachments(fboState, appState);
    }

    processInput(windowState.handle, gameState);

    updateLightningDebugInput(
        windowState.handle,
        appState,
        gameState
    );

    updateDebugInput(
        windowState.handle,
        appState,
        gameState
    );

    updateFade(gameState, appState);

    updateRoomTransition(gameState);

    promptText = updateGameplay(
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

## The order explained

### Frame timing happens first

Frame timing updates before gameplay, fade, lighting and animation so that all systems use the same frame time.

### Framebuffer resize happens early

Framebuffer resize is done before rendering to make sure the offscreen framebuffer matches the scale of the window size before drawing

### Input before gameplay

That is to movement, debug keys and interaction keys affect the current frame

### Fade before room transition

Room transitions depend of the fade state because it waits until the screen is fully blacked out and then loads the new room.

### Room transition before gameplay

If a room transition is happening, then gameplay should be paused 

### Camera update after gameplay

Gameplay moves the player so fixed camera zones are checked using the player's position.

### Scene renders before screen pass

The 3D scene is rendered in the framebuffer first and then the framebuffer is rendered on the screen.

### UI renders last

UI is drawn after screen pass so that it is on top the final image.
e.g interaction prompts, note overlay, debug terminal

## Shutdown order

The current shutdown procedure: 

```text
1. Destroy framebuffer resources
2. Destroy screen quad resources
3. Shut down the window and GLFW
```

or in code:

```cpp
destroyFramebuffer(fboState);
destroyScreenQuad(quadMeshState);
shutdownWindow(windowState);
```

## Guidelines

### Do not create duplicate runtime states.

Avoid:
```cpp
Room testRoom;
Player player;
Camera camera;
FadeState fadeState;
```

Use:

```cpp
GameState gameState;

gameState.room;
gameState.player;
gameState.camera;
gameState.fade;
```

### Keep `source.cpp` readable

Avoid putting detailed logic into `source.cpp`.
It is better to move it out into several external files as systems and keep stuff organised to prevent headaches down the line.

## Goal

The goal of the main loop is to be readable like a table of contents.

When reading `source.cpp`, it should be clear:

- what initializes first
- what updates each frame
- what renders each frame
- what shuts down at the end

The detailed code should live in the system files.