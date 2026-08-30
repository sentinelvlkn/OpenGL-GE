# Function Reference

This document aims to explain the **main** engine functions.
Not every line of code is listed here.

## Documentation template

The format for new entries:

```md
functionName(...)

File: src/source.cpp

Purpose:  
Short explanation of what the function does.

Called by:  
Where this function is usually called from.

Inputs:

- inputName1: what it means
- inputName2: what it means

Result:  

- changes player position
- changes OpenGL state
- changes global debug state
- loads a texture/model
- writes to debug log

Important:  

- rules that must stay true
- common mistakes
- assumptions

Related to:  

- Room loading
- Rendering
- UI
- Debug terminal
```


# Main loop helper functions

## `keyPressedOnce(GLFWwindow* window, int key)`

**File:** `src/source.cpp`

**Purpose:**  
Returns `true` only on the first frame a key is pressed.

This prevents actions from triggering every frame while the key is held.

**Called by:**  

- door interaction logic
- note/message close logic
- debug hotkeys
- terminal toggle

**Important inputs:**

- `window`: active GLFW window
- `key`: GLFW key constant

**Side effects:**

- stores previous key state in a static array

**Important rules:**

Use this for one-shot actions such as:

- pressing `E`
- toggling `F1`
- forcing lightning with `F5`

Do not use this for continuous movement keys.

## `processInput(GLFWwindow* window)`

**File:** `src/source.cpp`

**Purpose:**  
Processes general input such as closing the game.

**Called by:**  
Main loop.

# Player and collision

## `updatePlayerMovement(GLFWwindow* window, Player& player, const Camera& camera, const Room& room, float deltaTime)`

**File:** `src/source.cpp`

**Purpose:**  
Updates player movement using camera-relative controls and room collision.

The player moves relative to the active fixed camera direction.

**Called by:**  
Main loop, only when:

- not transitioning
- no message/note is open

**Important inputs:**

- `player`: player position and speed
- `camera`: active room camera
- `room`: current room colliders
- `deltaTime`: frame time

**Side effects:**

- changes `player.position`
- may log collision hits through `DebugLog::log`

**Important rules:**

- Delta time is clamped to reduce tunneling.
- Movement uses substeps.
- Collision uses circle-vs-AABB in the XZ plane.
- Movement should not update while messages or notes are open.

**Common bugs:**

- Calling this while `messageOpen == true` allows player movement while reading.
- Logging collision every frame can spam the debug terminal.

