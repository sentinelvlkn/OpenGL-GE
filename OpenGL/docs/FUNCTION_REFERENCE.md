# Function Reference

This document aims to explain the **main** engine functions.
Not every line of code is listed here.

## Documentation template

The format for new entries:

```md
functionName(...)

**File:** `src/source.cpp`

**Purpose:**  
Short explanation of what the function does.

**Signature:**
returnType functionName(float sth, int sth);

**Called by:**  
Where this function is usually called from.

**Arguments:**

- typeName1: what it means
- typeName2: what it means

**Result:**  

- changes player position
- changes OpenGL state
- changes global debug state
- loads a texture/model
- writes to debug log

**Important:**  

- rules that must stay true
- common mistakes
- assumptions

**Related to:**  

- Room loading
- Rendering
- UI
- Debug terminal
```

# Window and application setup

## `initializeWindow(...)`

**File:** `src/WindowSystem.cpp`


**Signature:**
```cpp
bool initializeWindow(
    WindowState& windowState,
    AppState& appState,
    GameState& gameState,
    AppContext& appContext,
    int width,
    int height,
    const char* title,
    bool enableMouseLook = false
);
```

**Purpose:**  
Initializes GLFW, creates the window, creates the OpenGL context, loads GLAD, 
registers callbacks, and enables basic OpenGL state.

**Called by:**  
`source.cpp`, during startup.

**Arguments:**

- `windowState`: stores the created GLFW window handle
- `appState`: stores framebuffer size, mouse state, and app-level runtime data
- `gameState`: connected to callbacks through `AppContext`
- `appContext`: stores pointers to `AppState` and `GameState`
- `width`: initial window width
- `height`: initial window height
- `title`: window title
- `enableMouseLook`: whether mouse and scroll callbacks should be enabled

**Result:**

- initializes GLFW
- creates the game window
- creates the OpenGL context
- stores the GLFW window handle in `WindowState`
- connects `AppContext` to the GLFW window user pointer
- loads GLAD
- registers framebuffer resize callback
- optionally registers mouse and scroll callbacks
- enables depth testing
- initializes framebuffer size in `AppState`

**Important:**

- `AppContext` must remain alive while the window exists.
- `glfwSetWindowUserPointer(...)` stores the address of `AppContext`.
- The function returns `false` if setup fails.

**Related to:**

- Window system
- AppState
- GameState
- AppContext
- GLFW callbacks

---

## `shouldWindowClose(...)`

**File:** `src/WindowSystem.cpp`

**Signature:**
```cpp
bool shouldWindowClose(const WindowState& windowState);
```

**Purpose:**  
Checks whether the window should close.

**Called by:**  
`source.cpp`, as the main loop condition.

**Arguments:**

- `windowState`: stores the GLFW window handle

**Result:**

- returns `true` if the window should close
- returns `true` if the window handle is null
- returns `false` while the game should continue running

**Important:**

- This wraps `glfwWindowShouldClose(...)`.
- It keeps direct GLFW calls out of `source.cpp`.

**Related to:**

- Window system
- Main loop

---

## `swapWindowBuffers(...)`

**File:** `src/WindowSystem.cpp`

**Signature:**
```cpp
void swapWindowBuffers(const WindowState& windowState);
```

**Purpose:**
Present the rendered frame by swapping the window buffers

**Called by:**
`source.cpp` at the end of each frame.

**Arguments:**
- `windowState`: stores the GLFW window handle

**Result:**
- displays the completed frame

**Important:**
- it should be called after scene, screne, UI and debug terminal rendering

**Related to:**
- Window system
- Game loop
- Rendering

---

## pollWindowEvents()

**File:** `src/WindowSystem.cpp `

**Signature:**
```cpp
void pollWindowEvents();
```

**Purpose:**
Process pending GLFW events.

**Called by:**
- `source.cpp` every frame

**Arguments:**
- ` `

**Result:**
- keyboard/mouse input is processed
- resize events processed
- triggers registered callbacks

**Important:**
- should be called once per frame
- GLFW callbacks may run during event pooling

**Related to:**
- Window system
- Input
- GLFW callbacks

---

## shutdownWindow()

**File:** `src/WindowSystem.cpp`

**Signature:**
```cpp
void shutdownWindow(WindowState& windowState);
```

**Purpose:**
destroy the GLFW window and shut down GLFW.

**Called by:**
- `source.cpp` at the end of main() during shutdown.

**Arguments:**
- `WindowState& windowState` stores the GLFW window handle

**Result:**
- destroys the GLFW window
- cleans the stored window handle
- calls glfwTerminate()

**Important:**
- call it after freeing OpenGL resources from other systems.
- instead of glfwTerminate(), use shutdownWindow() in `source.cpp`

**Related to:**
- Window system
- Shutdown

---

# Frame timing

## initializeFrameTiming()

**File:** `src/FrameTimingSystem.cpp`

**Signature:**
```cpp
void initializeFrameTiming(AppState& appState);
```

**Purpose:**
Initialize the frame timing values before the main loop and prevent the first frame from using invalid `deltaTime`

**Called by:**
- `source.cpp` during setup

**Arguments:**
- `AppState& appState`: stores frame timing values

**Result:**
- Sets `appState.timing.currentTime`
- Sets `appState.timing.lastFrameTime`
- resets `appState.timing.deltaTime` to `0.0f`

**Important:**
- Should be called before the game loop starts

**Related to:**
- AppState
- Main loop
- Movement
- Fade

---

## updateFrameTiming()

**File:** `src/FrameTimingSystem.cpp`

**Signature:**
```cpp
void updateFrameTiming(AppState& appState);
```

**Purpose:**
Updates frame timing values for the current frame.

**Called by:**
- `source.cpp` at the start of each frame

**Arguments:**
- `AppState& appState`: keeps track of current, delta and max delta time

**Result:**
- update `appState.timing` values
- clamps delta time to avoid big simulation jumps after lag

**Important:**
- Should be the first call in the main loop
- Systems should use `appState.timing.deltaTime` instead of calculating it themselves

**Related to:**
- AppState
- Main loop
- Player movement
- Fade system
- Lighting flicker
- Screen effects

---

# Startup resources

## ShaderState::ShaderState()

**File:** `src/ShaderSystem.cpp`

**Signature:**
```cpp
ShaderState::ShaderState();
```

**Purpose:**
Construct all engine shader objects from their shader file paths.

**Called by:**
- `source.cpp` when `ShaderState shaders;` is created

**Arguments:**
- ` `

**Result:**
- loads scene shader files
- loads screen shader files
- loads note/UI shader files
- loads text shader files
- loads UI rectangle shader files

**Important:**
- Shader file paths should remain relative to the project root.
- OpenGL must already be initialized before shader objects are created.

**Related to:**
- Shader setup
- Rendering
- UI

---

## initializeShaders()

**File:** `src/ShaderSystem.cpp`

**Signature:**
```cpp
void initializeShaders(ShaderState& shaders);
```

**Purpose:**
Set initial shader uniform values after `shaders` is created

**Called by:**
- `source.cpp` during setup after `shaders` is created

**Arguments:**
- `ShaderState& shaders`: stores all shader programs used by the engine

**Result:**
- sets screen shader texture uniforms
- sets UI/note shader texture uniforms


**Related to:**
- Shader system
- Screen rendering
- UI rendering

---

## ModelState::ModelState();

**File:** `src/ModelSystem.cpp`

**Signature:**
```cpp
ModelState::ModelState();
```

**Purpose:**
Construct shared model objects.

**Called by:**
- `source.cpp` when `ModelState& modelState;` is created

**Result:**
- loads playerModel

**Important:**
- Room models aren't loaded through `ModelState`

**Related to:**
- Model setup
- Scene rendering
- Asset cache

---

## initializeModels()

**File:** `src/ModelSystem.cpp`

**Signature:**
```cpp
void initializeModels(ModelState& modelState);
```

**Purpose:**
Complete shared models setup.

**Called by:**
- `source.cpp` during setup

**Arguments:**
- `ModelState& modelState`: stores shared loaded models

**Result:**
- model setup

**Related to:**
- Model setup
- Scene rendering

---

## initializeUi()

**File:** `src/UiSystem.cpp`

**Signature:**
```cpp
bool initializeUi(UiState& uiState);
```

**Purpose:**
Initializes UI rendering resources with text rendering and rectangle rendering.

**Called by:**
- `source.cpp` during setup

**Arguments:**
- `UiState& uiState`: stores UI renderer objects and UI configuration

**Result:**
- loads UI font
- initializes TextRenderer
- initializes UiRenderer

**Important:**
- fonts should be located in `src/ui/fonts/`
- call this function before drawing prompts, notes or debug terminal

**Related to:**
- UI setup
- Text rendering
- Debug terminal
- Interaction prompt

---

## initializeScreenQuad()

**File:** `src/QuadMeshSystem.cpp`

**Signature:**
```cpp
void initializeScreenQuad(QuadMeshState& quadMeshState);
```

**Purpose:**
Create the full-screen quad for screen rendering and note overlay

**Called by:**
- `source.cpp` during setup

**Arguments:**
- `QuadMeshState& quadMeshState`: stores the quad VAO and VBO

**Result:**
- creates quad VAO, VBO
- uploads quad vertex data
- configures quad vertex attributes

**Important:**
- Quad data comes from `include/test_vertices.h`
- The quad is destroyed with `destroyScreenQuad()` during shutdown

**Related to:**
- Screen rendering
- Note overlays
- OpenGL resources

---

## destroyScreenQuad()

**File:** `src/QuadMeshSystem.cpp`

**Signature:**
```cpp
void destroyScreenQuad(QuadMeshState& quadMeshState);
```

**Purpose:**
Deletes the OpenGL resources owned by the screen quad.

**Called by:**
- `source.cpp` during shutdown, near the end of main()

**Arguments:**
- `QuadMeshState& quadMeshState`: stores the quad VAO and VBO

**Result:**
- deletes the quad VAO and VBO
- resets stored OpenGL handles to 0

**Important:**
- call before `shutdownWindow()`

**Related to:**
- Screen rendering
- Note overlay
- Shutdown

---

# Framebuffer resources

## initializeFramebuffer()

**File:** `src/FramebufferSystem.cpp`

**Signature:**
```cpp
bool initializeFramebuffer(FramebufferState& framebufferState, const AppState& appState);
```

**Purpose:**
Create an offscreen framebuffer used for rendering the 3D scene before it is drawn to the window.

**Called by:**
- `source.cpp` during setup

**Arguments:**
- `FramebufferState& framebufferState`: stores the framebuffer, color texture and renderbuffer handles
- `const AppState& appState`: provides framebuffer width and height

**Result:**
- creates an OpenGL framebuffer object
- creates the framebuffer color texture
- creates the depth/stencil renderbuffer
- attaches texture and renderbuffer to the framebuffer
- returns true if framebuffer setup is successful, false otherwise 

**Important:**
- Call after OpenGL is initialized
- `destroyFramebuffer(...)` is used to get rid of the framebuffer

**Related to:**
- Framebuffer system
- Scene rendering
- Screen rendering

---

## resizeFramebufferAttachments()

**File:** `src/FramebufferSystem.cpp`

**Signature:**
```cpp
void resizeFramebufferAttachments(FramebufferState& framebufferState, AppState& appState);
```

**Purpose:**
Resize the framebuffer texture and renderbuffer after window framebuffer size changes.

**Called by:**
- `source.cpp` in the main loop when `appState.framebufferResized = true`

**Arguments:**
- `FramebufferState& framebufferState`: stores framebuffer texture and renderbuffer handles
- `AppState& appState`: provides the framebuffer size and stores the resize flag

**Result:**
- resizes framebuffer color texture, depth/stencil renderbuffer
- clears `appState.framebufferResized`
- may write resize information to the debug log

**Important:**
- it should be called before rendering the next frame begins

**Related to:**
- Window resize callback
- AppState
- Scene and Screen rendering

---

## destroyFramebuffer()

**File:** `src/FramebufferSystem.cpp`

**Signature:**
```cpp
void destroyFramebuffer(FramebufferState& framebufferState);
```

**Purpose:**
Deletes the OpenGL framebuffer resources.

**Called by:**
- `source.cpp` during shutdown, near the end of main()

**Arguments:**
- `FramebufferState& framebufferState`: stores the framebuffer handles that will be destroyed

**Result:**
- deletes the renderbuffer, framebuffer color texture and framebuffer objects
- resets stored OpenGL handles to 0

**Important:**
- call before shutdownWindow(...)
- do not manually delete framebuffer resources

**Related to:**
- Framebuffer system
- Shutdown

---

# Game startup and room loading

## initializeGameState()

**File:** `src/GameBootstrap.cpp`

**Signature:**
```cpp
void initializeGameState(GameState& gameState, const char* initialRoomJson);
```

**Purpose:**
Load the starting room and initialize the first playable game state.

**Called by:**
- `source.cpp` during setup

**Arguments:**
- `GameState& gameState`: stores the active room, room model pointer, player camera and gameplay runtime state
- `const char* initialRoomJson`: path to the first room JSON file

**Result:**
- loads the initial room
- stores the room in `gameState.room`
- retrieves the active room model
- stores the model pointer in `gameState.roomModel`
- moves the player to the room spawn
- updates the room camera

**Important:**
- This is where the starting room should be loaded
- Do not create separate `Room, Player` objects
- Rendering and gameplay should read from the same `GameState` object

**Related to:**
- GameState
- Room loading
- Asset cache
- Camera system

---

## loadRoomFromJson()

**File:** `src/Room.cpp`

**Signature:**
```cpp
Room loadRoomFromJson(const char* path);
```

**Purpose:**
Load the room definition from its corresponding JSON file.

**Called by:**
- `initializeGameState(...)`
- `updateRoomTransition(...)`

**Arguments:**
- `const char* path`: path to the room JSON file

**Result:**
- reads all room attributes (e.g metadata, player spawn, camera zones and etc.)
- converts Blender-authored coordinates into engine coordinates
- returns a populated `Room` object

**Important:**
- Blender-authored positional data must be converted consistently.
- Room model paths should remain valid from the project root.
- If new JSON fields are added, parsing should be added here.

**Related to:**
- Room JSON
- Blender pipeline
- Room transitions
- Asset loading

---

## getRoomModel()

**File:** `src/AssetCache.cpp`

**Signature:**
```cpp
Model& getRoomModel(const Room& room);
```

**Purpose:**
Gets the loaded model for a room using the asset cache.

**Called by:**
- `initializeGameState(...)`
- `updateRoomTransition(...)`

**Arguments:**
- `const Room& room`: room whose model should be loaded or retrieved

**Result:**
- loads the room if it is not cached yet
- reuses cached model if it was already loaded
- returns a reference to the room model

**Important:**
- returned model ref is stored as a pointer in gameState.roomModel.
- room rendering should use *gameState.roomModel.

**Related to:**
- Asset cache
- Room loading
- Scene rendering

---

## getCachedModel()

**File:** `src/AssetCache.cpp`

**Signature:**
```cpp
Model& getCachedModel(const std::string& modelPath);
```

**Purpose:**
Gets a model by path using the asset cache.

**Called by:**
- prop rendering
- room/model loading helpers

**Arguments:**
- `const std::string& modelPath`: model path to load or retrieve

**Result:**
- loads the model if it isn't cached yet
- reuses the cached model if it was loaded
- returns a reference to the model

**Important:**
- meant for reusable prop models

**Related to:**
- Asset cache
- Props
- Scene rendering

---

# Input and gameplay

## keyPressedOnce()

**File:** `src/Input.cpp`

**Signature:**
```cpp
bool keyPressedOnce(GLFW* window, int key);
```

**Purpose:**
Returns true only on the first frame the key is pressed

**Called by:**
- `GameplaySystem`
- `DebugInputSystem`
- `LightingSystem`

**Arguments:**
- `GLFW* window`: active GLFW window
- `int key`: **GLFW key constant**

**Result:**
- returns true on the first pressed frame
- stores previous key state internally

**Important:**
- created for on-time actions like pressing E, toggling F1 and etc.

**Related to:**
- Input
- Interactions
- Hotkeys

---

## processInput()

**File:** `src/Input.cpp`

**Signature:**
```cpp
void processInput(GLFWwindow* window, GameState& gameState);
```

**Purpose:**
Process general input every frame

**Called by:**
- `source.cpp` every frame in the game loop

**Arguments:**
- `GLFWwindow* window`: active GLFW window
- `GameState& gameState`: input access to gameplay state 

**Result:**
- triggers whatever action linked with the input

**Important:**
- Continuous player movement is handled by `updateGameplay(...)` through `updatePlayerMovement(...)`

**Related to:**
- Input
- Fade system
- Main loop

---

## updateGameplay()

**File:** `src/GameplaySystem.cpp`

**Signature:**
```cpp
std::string updateGameplay(GLFWwindow* window, const AppState& appState, GameState& gameState);
```

**Purpose:**
Updates one frame of gameplay logic and returns the current interaction prompt text

**Called by:**
- `source.cpp` every frame

**Arguments:**
- `GLFWwindow* window`: active GLFW window
- `const AppState& appState`: provides frame timing
- `GameState& gameState`: stores room, player, camera, transition, fade and interaction state

**Result:**
- updates player movement
- checks for nearby doors or interactions
- returns prompt text
- may start room transition or open message/note interaction and close it

**Important:**
- should return early while a room transition is active
- doesn't render or draw UI
- uses gameState and no individual definitions of `Room, Player, Camera`

**Related to:**
- Player movement
- Interactions
- Room transitions
- UI prompts

---

## updatePlayerMovement()

**File:** `src/PlayerMovement.cpp`

**Signature:**
```cpp
void updatePlayerMovement(
    GLFWwindow* window,
    Player& player,
    const Camera& camera,
    const Room& room,
    float deltaTime
);
```

**Purpose:**
Update player movement using camera-relative controls and room collision.

**Called by:**
- `updateGameplay(...)`

**Arguments:**
- `GLFWwindow* window`: active GLFW window
- `Player& player`: player to move
- `const Camera& camera`: active camera used for camera-relative movement
- `const Room& room`: current room containing collision data
- `float deltaTime`: frame time used for frame-rate-independent movement

**Result:**
- changes `player.position`
- tests movement against colliders

**Important:**
- movement is skipped when an interaction is active and during room transition
- collision uses room collider data, e.g "room_walls"

**Related to:**
- Player movement
- Collision
- Gameplay system
- Fixed camera

---

# Interactions

## getNearbyInteraction()

**File:** `src/InteractionSystem.cpp`

**Signature:**
```cpp
NearbyAction getNearbyAction(const Player& player, Room& room);
```

**Purpose:**
Finds the nearest door or interaction to the player.

**Called by:**
- `updateGameplay(...)`

**Arguments:**
- `const Player& player`: for the player's position
-  `Room& room`: current room with its doors and interactions

**Result:**
- returns a `NearbyAction` like `Door` or `Interaction` or `nothing lol`

**Important:**
- It only detects nearby action, doesn't draw prompts
- Prompr drawing happens in `GameUIRenderer`

**Related to:**
- Interactions
- Doors
- Gameplay system
- UI prompt

---

## runInteraction()

**File:** `src/InteractionSystem.cpp`

**Signature:**
```cpp
void runInteraction(Interaction& interaction, InteractionRuntimeState& interactionState);
```

**Purpose:**
Runs an interaction that the player activated

**Called by:**
- `updateGameplay()`

**Arguments:**
- `Interaction& interaction`: interaction data from the current room
- `InteractionRuntimeState& interactionState`: runtime state for interactions

**Related to:**
- Interaction system
- Gameplay System
- UI rendering

---

## closeActiveInteraction()

**File:** `src/InteractionSystem.cpp`

**Signature:**
```cpp
void closeActiveInteraction(InteractionRuntimeState& interactionState)
```

**Purpose:**
Closes the currently active interaction

**Called by:**
- `updateGameplay(...)`

**Arguments:**
- `InteractionRuntimeState& interactionState`: runtime interaction state to clear

**Result:**
- clears active interaction values

**Related to:**
- Interaction system
- Gameplay system
- Note overlay
- Interaction prompt

---

# Fade and room transition

## updateFade()

**File:** `src/FadeSystem.cpp`

**Signature:**
```cpp
void updateFade(GameState &gameState, const AppState &appState);
```

**Purpose:**
Handles fade screen using current frame delta time

**Called by:**
- `source.cpp` every frame

**Arguments:**
- `GameState &gameState`: stores active fade state
- `const AppState &appState`: provides deltaTime

**Result:**
- updates `gameState.fade.value`

**Important:**
- call it before rendering the scene/screen so the screen shader uses that update fade value

**Related to:**
- Fade system
- Room transitions
- Screen rendering

---

## beginFadeToBlack()

**File:** ` `

**Signature:**
```cpp
void beginFadeToBlack(FadeState &fadeState);
```

**Purpose:**
Start the fade from visible screen to black

**Called by:**
- `beginRoomTransition(...)`
- `processInput(...)`

**Arguments:**
- `FadeState &fadeState`: the state to manipulate

**Result:**
- sets fade direction and state so `updateFade(...)` moves towards black

**Related to:**
- Fade system
- Room transition
- Screen rendering

---

## beginFadeFromBlack();

**File:** ` `

**Signature:**
```cpp
void beginFadeFromBlack(FadeState &fadeState);
```

**Purpose:**
Starts fade from black to visible screen.

**Called by:**
- `updateRoomTransition(...)`
- `processInput(...)`

**Arguments:**
- `FadeState &fadeState`: the fade state to manipulate

**Result:**
- sets `fadeState` so that `updateFade(...)` moves towards visible 

**Related to:**
- Fade system
- Room transitions
- Screen rendering

---

## beginRoomTransition()

**File:** ` `

**Signature:**
```cpp
void beginRoomTransition(
							GameState& gameState,
							const std::string& targetRoomJson,
							const glm::vec3& targetSpawn
)
```

**Purpose:**
Starts the room transition through a door

**Called by:**
- `updateGameplay()`

**Arguments:**
- `GameState& gameState`: stores transition, fade, room, model, player and camera state
- `const std::string& targetRoomJson`: JSON file for the target room
- `const glm::vec3& targetSpawn`: where the player should be spawned

**Result:**
- stores pending target room path and player spawn
- marks room transition as active
- calls `beginFadeToBlack(...)`

**Important:**
- this only begins the room transition, it doesn't load the room
- the room loads after the fade is fully black

**Related to:**
- Room transitions
- Doors
- Fade system
- Room loading

---

## updateRoomTransition()

**File:** `src/RoomTransitionSystem.cpp`

**Signature:**
```cpp
void updateRoomTransition(GameState& gameState)
```

**Purpose:**
Updates active room transition.

**Called by:**
- `source.cpp` every frame

**Arguments:**
- `GameState& gameState`: stores room transition, fade, room, model, player and camera state

**Result:**
- loads pending room JSON
- updates `gameState.room`, `gameState.roomModel`
- moves the player to spawn
- update room camera
- calls `beginFadeFromBlack(...)`
- end the transition when the fade has ended

**Important:**
- gameplay shouldn't happen while a transition is happening

**Related to:**
- Room transitions
- Fade system
- Room loading
- Asset cache
- Camera system

---

# Camera

## updateRoomCamera()

**File:** ` `

**Signature:**
```cpp
void updateRoomCamera(GameState &gameState);
```

**Purpose:**
Update the active fixed camera based on player's position.

**Called by:**
- `initializeGameState()`
- `source.cpp`

**Arguments:**
- `GameState &gameState`: stores current room, player and camera

**Result:**
- checks in which camera zone the player is located
- applies the active room camera

**Important:**
- call after gameplay movement so the function receives update player.position

**Related to:**
- Fixed camera
- Room data
- Player movement
- Scene rendering

---

# Lighting debug

## updateLightningDebugInput()

**File:** `src/LightingSystem.cpp`

**Signature:**
```cpp
void updateLightningDebugInput(GLFWwindow *window, const AppState &appState, GameState &gameState);
```

**Purpose:**
Update lighting debug controls for the current frame.

**Called by:**
- `source.cpp` every frame

**Arguments:**
- `GLFWwindow *window`: current window for key input
- `const AppState &appState`: provides frame delta time
- `GameState &gameState`: stores room lighting data and debug state

**Result:**
- may trigger forced preview
- toggle flicker behaviour
- force a flash
- print light debug info
- update lighting debug timers/state

**Important:**
- This is for lighting debug purposes only

**Related to:**
- Lighting system
- Debug input
- AppState
- GameState

---

## 

**File:** ` `

**Signature:**
```cpp
float calculateFlickerMultiplier(	const PointLight &light, 
									float time,
									int lightIndex,
									const LightingDebugState &debugState)
```

**Purpose:**
Calculate the brightness multiplier for a flickering point light

**Called by:**
- `uploadRoomLights()` in `SceneRenderer.cpp`

**Arguments:**
- `const PointLight &light`: light whose flicker should be calculated
- `float time`: current app time
- `int lightIndex`: light index to vary flicker behavior
- `const LightingDebugState &debugState`: debug state that can force or disable flicker behavior

**Result:**
Returns a multiplier applied to light intensity

**Important:**
- this function doesn't apply uniforms directly

**Related to:**
- Lighting system
- Scene rendering
- Debug lighting

---

# Rendering

## renderSceneToFBO()

**File:** ` `

**Signature:**
```cpp
void renderSceneToFBO(
							const FramebufferState& fboState,
							ShaderState& shaders,
							ModelState& models,
							const AppState& appState,
							GameState& gameState
)
```

**Purpose:**
Renders the 3D world into offscreen framebuffer.

**Called by:**
- `source.cpp` every frame

**Arguments:**
- `const FramebufferState& fboState`: provides the framebuffer handle
- `ShaderState& shaders`: provides the screen shader
- `ModelState& models`: provides shared models like player model
- `const AppState& appState`: provides framebuffer size and current time
- `GameState& gameState`: provides room, active room model, player, camera and lighting debug

**Result:**
- binds the offscreen framebuffer
- sets the scene viewport
- clears color and depth buffers
- upload camera matrices
- upload room lighting uniforms
- draws the room model
- draws props
- draws the player model
- changes OpenGL render state

**Important:**
- Room models should use identity transforms
- Do not rotate the full room in C++ render time
- The active room model should come from `gameState.roomModel`
- Emissive state should be reset after drawing emissive props and before drawing the player

**Related to:**
- Scene rendering
- Framebuffer system
- Room data
- Lighting system
- Model system

---

## void renderScreen()

**File:** `src/ScreenRenderer.cpp`

**Signature:**
```cpp
void renderScreen(
							const QuadMeshState& quadMeshState, 
							ShaderState& shaders,
							const FramebufferState& fboState, 
							const AppState& appState,
							const GameState& gameState
)
```

**Purpose:**
Draws the framebuffer texture to the window

**Called by:**
- `source.cpp` every frame after `renderSceneToFbo(...)`

**Arguments:**
- `const QuadMeshState& quadMeshState`: provides full-screen quad VAO
- `ShaderState& shaders`: provides screen shader
- `const FramebufferState& fboState`: provides framebuffer color texture
- `const AppState& appState`: provides framebuffer size and current time
- `const GameState& gameState`: provides fade state

**Result:**
- binds default framebuffer
- disables or adjusts depth state as needed
- binds the framebuffer color texture
- draws the full-screen quad
- applies screen shader effects
- changes OpenGL render state

**Important:**
- call after 3D scene has been rendered to the framebuffer
- UI should be drawn after this function so UI appears on top

**Related to:**
- Screen rendering
- Framebuffer system
- Fade system
- UI rendering

---

# UI rendering

## drawInteractionPrompt()

**File:** `src/GameUIRenderer.cpp`

**Signature:**
```cpp
void drawInteractionPrompt(
                            const std::string& promptText,
                            UiState& uiState,
                            ShaderState& shaders,
                            const AppState& appState,
                            const GameState& gameState
)
```

**Purpose:**
Draw the on-screen interaction prompt.

**Called by:**
- `source.cpp` every frame after the screen pass.

**Arguments:**
- `const std::string& promptText`: text returned by `updateGameplay(...)`
- `UiState& uiState`: provides text and rectangle renderers
- `ShaderState& shaders`: provides text and UI rectangle shaders
- `const AppState& appState`: provides framebuffer size for UI placement
- `const GameState& gameState`: provides interaction and transition state

**Result:**
- draws the prompt background rectangle
- draws prompt text

**Important:**
- this function only draws the prompt, it does not decide nearby interactions

**Related to:**
- UI rendering
- Interaction system
- Gameplay system

---

## drawNoteOverlay()

**File:** `src/GameUIRenderer.cpp`

**Signature:**
```cpp
void drawNoteOverlay(
                            const GameState& gameState,
                            ShaderState& shaders,
                            const QuadMeshState& quadMeshState
)
```

**Purpose:**
Draw the note overlay when a note interaction is open.

**Called by:**
- `source.cpp`

**Arguments:**
- `const GameState& gameState`: provides note interaction runtime state
- `ShaderState& shaders`: provides the note/UI shader
- `const QuadMeshState& quadMeshState`: provides the full-screen quad VAO

**Result:**
- draws the active note texture as an overlay

**Important:**
- message interaction text is separate from note overlay behavior

**Related to:**
- UI rendering
- Interaction system
- Note interactions

---

## drawDebugTerminal()

**File:** `src/DebugTerminal.cpp`

**Signature:**
```cpp
void drawDebugTerminal(	UiState& uiState,
						ShaderState& shaders,
						const AppState &appState
)
```

**Purpose:**
Draw the in-game debug terminal.

**Called by:**
- `source.cpp`

**Arguments:**
- `UiState& uiState`:
- `ShaderState& shaders`:
- `const AppState &appState`:

**Result:**
- draws debug terminal background and wraps debug log text

**Related to:**
- Debug terminal
- Debug log
- UI rendering
- AppState

---

# Debug systems

## updateDebugInput()

**File:** `src/DebugInputSystem.cpp`

**Signature:**
```cpp
void updateDebugInput(
    GLFWwindow* window,
    AppState& appState,
    const GameState& gameState
)
```

**Purpose:**
Handle general debug hotkeys.

**Called by:**
- `source.cpp` every frame

**Arguments:**
- `GLFWwindow* window`: active GLFW window used for debug key input
- `AppState& appState`: stores debug terminal visibility
- `const GameState& gameState`: provides current room/player/camera data for debug printing

**Result:**
- toggles `appState.debugTerminalOpen`
- writes debug text through `DebugLog`

**Important:**
- use `keyPressedOnce(...)` for toggles

**Related to:**
- Debug input
- Debug terminal
- Debug log
- AppState
- GameState

---

## DebugLog::log()

**File:** `src/DebugLog.cpp`

**Signature:**
```cpp
void DebugLog::log(const std::string &text);
```

**Purpose:**
Add a debug message to the log and prints it to the external console.

**Called by:**
- engine systems that need to report state or errors
- debug input
- collision helpers
- boot/setup systems

**Arguments:**
- `const std::string &text`: message to print

**Result:**
- prints to both the in-game debug terminal and external terminal

**Important:**
- prefer this over `std::cout << ...`

**Related to:**
- Debug log
- Debug terminal
- Debug input

---

## DebugLog::getLines()

**File:** `src/DebugLog.cpp`

**Signature:**
```cpp
const std::vector<std::string>& DebugLog::getLines();
```

**Purpose:**
Return the stored debug log lines.

**Called by:**
- `drawDebugTerminal(...)`

**Result:**
- returns stored debug log messages

**Important:**
- should be used to read log lines, not add new messages

**Related to:**
- Debug log
- Debug terminal

---

## DebugLog::clear()

**File:** `src/DebugLog.cpp`

**Signature:**
```cpp
void DebugLog::clear();
```

**Purpose:**
Clear the stored debug log lines.

**Called by:**
- debug tools or systems that need to reset the displayed log

**Result:**
- removes stored debug messages

**Important:**
- this clears what the in-game debug terminal can display

**Related to:**
- Debug log
- Debug terminal

---

# File paths

## FileSystem::pathFromRoot()

**File:** `src/include/FileSystem.h`

**Signature:**
```cpp
static std::string FileSystem::pathFromRoot(const std::string &loc);
```

**Purpose:**
Build a full path from the project root directory and a relative project path.

**Called by:**
Room/Shader/Model/texture loading helpers and systems that need project-relative assets

**Arguments:**
- `const std::string &loc`: path relative to the project root

**Result:**
- returns a project-root-based path string

**Important:**
- depends on the `PROJECT_ROOT_DIR` CMake definition
- asset paths should be written relative to the project root

**Related to:**
- File loading
- Room loading
- Shaders
- Models
- Textures

---

## 

**File:** ``

**Signature:**
```cpp

```

**Purpose:**

**Called by:**

**Arguments:**
- ` `: 

**Result:**

**Important:**

**Related to:**

