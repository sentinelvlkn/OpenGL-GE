# Room and Blender pipeline

This document explains how rooms are authored, exported, loaded and converted in engine space.

Room data is authored mostly in Blneder and stored in a JSON file.

The engine loads the room JSON through `Room.h/.cpp`.

# Structure

A room model consists of two parts:

```text
1. The visible 3D model
2. JSON gameplay data
```

The JSON contains datasuch as:
- player spawn
- camera zones
- colliders
- doors
- interactions
- lights
- props

The engine loads both the room model and JSON and stores the active room in `gameState.room`.

The active room model pointer is stored in `gameState.roomModel`.

---

# Room model rules

The room models in Blender should be exported in the correct orientation where `blender.z` is `engine.y`; `-blender.y` is `engine.z`.

The engine should render the full room model with an identity transform.

```cpp
glm::mat4 model = glm::mat4(1.0f);
shader.setMat4("model", model);
roomModel.draw(shader);
```

The room shouldn't be rotated at render time and if it appears to face the wrong direction - fix the export.

---

# Coordinate conversion

The engine coordinate convention:

```cpp
engine.x = blender.x;
engine.y = blender.z;
engine.z = -blender.y;
```

In code:

```cpp
glm::vec3 convertBlenderCoords(const glm::vec3& b){
    return glm::vec3(b.x, b.z, -b.y);
}
```

This is a thing because Blender and OpenGL both use the right-handed coordinate system but handle axes differently as shown above.

This converson is applied to data that relies on on positionng:
- player spawn position
- camera position/targets/zones/
- collider bounds
- door trigger bounds and target spawn positions
- interaction trigger bounds
- point light positions
- spotlight positions and target positions
- prop positons/spin axes

---

# AABB conversion

Axis-aligned bounding boxes need additional handling:

```cpp
static void convertBlenderAABB(
    const glm::vec3& minB,
    const glm::vec3& maxB,
    glm::vec3& outMin,
    glm::vec3& outMax
) {
    glm::vec3 centerB = (minB + maxB) * 0.5f;
    glm::vec3 extentB = (maxB - minB) * 0.5f;

    glm::vec3 centerE = convertBlenderCoords(centerB);
    glm::vec3 extentE(extentB.x, extentB.z, extentB.y);

    outMin = centerE - extentE;
    outMax = centerE + extentE;
}
```

This keeps the box valid after axis conversion and applies to colliders, camera zones, door triggers and interaction triggers.

# Room JSON overview

JSONs are used to describe gameplay data for each room.

```json
{
  "name": "test_room",
  "model": "src/models/test_room/test_room.obj",

  "playerSpawn": [0.0, 0.0, 0.0],

  "cameras": [],
  "colliders": [],
  "doors": [],
  "interactions": [],
  "lights": [],
  "props": []
}
```

Not all sections are must-have, the blender scrpt will simply skip them if no instances are found.

## Room name

This variable defines how the room is displayed in debugs and editor/debug output.

```json
{
    "name": "test_room"
}
```

Used mainly by the debug terminal, boot logs and room transition logs.

## Room model path

Points to the 3D room model.

```json
{
    "model": "src/models/test_room/test_room.obj"
}
```

- the model path should be relatve to the project root
- the model should be correctly oriented 

## Player spawn

Where the player spawns when entering the room.

```json
{
    "playerSpawn": [0.0, 0.0, 1.0]
}
```

Loads into `room.playerSpawn` and is used as `gameState.player.position = gameState.room.playerSpawn`.

## Cameras

Currently, those define the fixed camera views inside a room.

A room can have several cameras with different camera zones.

```json
{
    "cameras": [
        {
            "position": [0.0, 1.0, 8.0],
            "target": [0.0, 1.0, 0.0],
            "zoneMin": [-5.0, -1.0, -5.0],
            "zoneMax": [5.0, 3.0, 5.0]
        }
    ]
}
```
Used in:

```cpp
updateRoomCamera(gameState);
```

- camera positions and targets are converted
- camera zone bounds use aabb conversion
- camera updates happen after updating player position

## Colliders

Block player movement.

```json
{
  "colliders": [
    {
      "min": [-1.0, 0.0, -1.0],
      "max": [1.0, 2.0, 1.0]
    }
  ]
}
```

Used in:
```cpp
updatePlayerMovement(...);
```

- colliders are AABBs
- colliders use AABB conversion
- collisions currently work only in the xz plane

---

## Doors

For room transitions.

```json
{
  "doors": [
    {
      "id": "door_to_hallway",
      "prompt": "Open",
      "targetRoom": "src/models/hallway/hallway.json",
      "targetSpawn": [0.0, 0.0, 2.0],
      "triggerMin": [-1.0, 0.0, -1.0],
      "triggerMax": [1.0, 2.0, 1.0]
    }
  ]
}

Used in:
```cpp
updateGameplay(...);
beginRoomTransition(gameState, targetRoomJson, targetSpawn);
updateRoomTransition(gameState);
```

- door trigger bounds use AABB conversion
- door target spawn coords should be converted

## Interactions

Events that the player can trigger other than doors.

Such include notes, messages and more in the future.

```json
{
  "interactions": [
    {
      "id": "note_01",
      "type": "note",
      "prompt": "Read",
      "asset": "src/ui/notes/note_01.png",
      "triggerMin": [-1.0, 0.0, -1.0],
      "triggerMax": [1.0, 2.0, 1.0],
      "repeatable": true
    }
  ]
}
```

## Lights

Rooms can contain point and spot lights.
Light data is loaded from the room JSON and used in scene rendering.

### Point lights

```json
{
  "lights": [
    {
      "type": "point",
      "position": [0.0, 3.0, 0.0],
      "color": [1.0, 0.8, 0.6],
      "intensity": 1.0,
      "radius": 8.0,
      "flicker": true
    }
  ]
}
```

### Spotlights

```json
{
  "lights": [
    {
      "type": "spot",
      "position": [0.0, 4.0, 0.0],
      "target": [0.0, 1.0, 0.0],
      "color": [1.0, 0.9, 0.7],
      "intensity": 2.0,
      "radius": 10.0,
      "innerCutoff": 12.5,
      "outerCutoff": 17.5
    }
  ]
}
```

## Props

Extra models placed in the room. They exist mainly to serve as interaction objects.
They can be either static or have some behaviour like spinning and glowing.

```json
{
  "props": [
    {
      "id": "lamp_01",
      "model": "src/models/props/lamp/lamp.obj",
      "position": [1.0, 0.0, 2.0],
      "scale": 1.0,
      "emissive": true,
      "glowStrength": 1.5
    }
  ]
}
```

- the position and spin axis (if there is such) have coordinates conversion
- prop models are cached like rooms
- props should be data-drive as much as possible

--- 

# Room loading flow

During setup do ```cpp initializeGameState(gameState, "src/models/[model_name]/[model_name].json"); ```

This will:
```text
1. Load the room JSON where the game should first start at
2. Convert Blender coordinates to OpenGL coords
3. Place the room in gameState.room
4. Retrieve the room model and store a pointer to it at gameState.roomModel 
5. Place player at room.playerSpawn
6. Update room camera
```

During room transiton:

```cpp
beginRoomTransition(gameState, targetRoomJson, targetSpawn);
updateRoomTransition(gameState);
```

This will:
```text
1. Start fade to black and wait until the screen is fully blacked out
2. Load the next room json
3. Update gameState.room and gameState.roomModel
4. Move player to target spawn
5. Update the room camera
6. Fade back from black
```

--- 

# Asset caching

Room and prop models are cached through:
```cpp
getRoomModel(room);
getCachedModel(modelPath);
```

That is to skip loading the same model repeatedly.

