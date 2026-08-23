import bpy
import json
import os
from mathutils import Vector

#config variables that should be set before running this script

ROOM_ID = "testRoom"

ROOM_NAME = ROOM_ID
MODEL_PATH = f"src/models/{ROOM_ID}/{ROOM_ID}.obj"
JSON_PATH = f"src/models/{ROOM_ID}/{ROOM_ID}.json"

CAMERA_NAMES = ["Cam01", "Cam02"]

# Mesh used to calculate outer wall bounds
ROOM_WALLS_OBJECT = "room_walls"

# Thickness of generated perimeter wall colliders
WALL_THICKNESS = 0.115

# Optional: set True if you want one hardcoded test door
USE_TEMP_DOOR = False

AMBIENT_COLOR = [0.12, 0.12, 0.14]

TEMP_DOORS = [
    {
        "min": [-1.0, -1.0, 0.0],
        "max": [1.0, 1.0, 2.0],
        "targetRoom": "src/models/testRoom02/testRoom02.json",
        "targetSpawn": [0.0, 0.0, 0.0]
    }
]

# If True, also export mesh objects whose names start with Col_
EXPORT_EXTRA_COL_OBJECTS = True

#helpers

def round_float(x):
    return round(float(x), 5)

def vec3(v):
    return [round_float(v.x), round_float(v.y), round_float(v.z)]

def get_obj(name):
    return bpy.data.objects.get(name)

def world_aabb(obj):
    """
    Returns world-space AABB min/max for a mesh object.
    Output is in Blender coordinates.
    """
    if obj is None:
        raise RuntimeError("world_aabb called with None")

    if obj.type != "MESH":
        raise TypeError(f"{obj.name} must be a mesh object")

    if len(obj.data.vertices) == 0:
        raise RuntimeError(f"{obj.name} has no vertices")

    min_v = Vector((1e9, 1e9, 1e9))
    max_v = Vector((-1e9, -1e9, -1e9))

    mw = obj.matrix_world

    for v in obj.data.vertices:
        w = mw @ v.co

        if w.x < min_v.x: min_v.x = w.x
        if w.y < min_v.y: min_v.y = w.y
        if w.z < min_v.z: min_v.z = w.z

        if w.x > max_v.x: max_v.x = w.x
        if w.y > max_v.y: max_v.y = w.y
        if w.z > max_v.z: max_v.z = w.z

    return min_v, max_v

def make_perimeter_colliders_from_bounds(min_v, max_v, thickness):
    """
    Creates 4 AABB wall slabs in Blender space from room_walls bounds.

    Blender:
      X/Y = floor plane
      Z   = up
    """

    x0 = min_v.x
    x1 = max_v.x

    y0 = min_v.y
    y1 = max_v.y

    z0 = min_v.z
    z1 = max_v.z

    inner_y0 = y0 + thickness
    inner_y1 = y1 - thickness

    inner_x0 = x0 + thickness
    inner_x1 = x1 - thickness

    return [
        {
            "min": [round_float(x0), round_float(y0), round_float(z0)],
            "max": [round_float(x1), round_float(inner_y0), round_float(z1)]
        },
        {
            "min": [round_float(x0), round_float(inner_y1), round_float(z0)],
            "max": [round_float(x1), round_float(y1), round_float(z1)]
        },
        {
            "min": [round_float(x0), round_float(y0), round_float(z0)],
            "max": [round_float(inner_x0), round_float(y1), round_float(z1)]
        },
        {
            "min": [round_float(inner_x1), round_float(y0), round_float(z0)],
            "max": [round_float(x1), round_float(y1), round_float(z1)]
        }
    ]

def parse_color_prop(value, default=(1.0, 0.75, 0.45)):
        if value is None:
            return list(default)

        if isinstance(value, str):
            parts = value.replace(",", " ").split()
            if len(parts) >= 3:
                try:
                    return [
                        round_float(parts[0]),
                        round_float(parts[1]),
                        round_float(parts[2])
                    ]
                except ValueError:
                    return list(default)

            return list(default)

        try:
            if len(value) >= 3:
                return [
                    round_float(value[0]),
                    round_float(value[1]),
                    round_float(value[2])
                ]
        except Exception:
            pass

        return list(default)

def get_scene_ambient(default=(0.12, 0.12, 0.14)):
    value = bpy.context.scene.get("ambientColor", None)

    if value is None:
        return list(default)

    if isinstance(value, str):
        parts = value.replace(",", " ").split()
        if len(parts) >= 3:
            try:
                return [
                    round_float(parts[0]),
                    round_float(parts[1]),
                    round_float(parts[2])
                ]
            except ValueError:
                return list(default)

    try:
        if len(value) >= 3:
            return[
                round_float(value[0]),
                round_float(value[1]),
                round_float(value[2])
            ]
    except Exception:
        pass

    return list(default)

#export

def export_room_json():
    print("\n=== Exporting room JSON ===")
    print(f"ROOM_ID: {ROOM_ID}")
    print(f"MODEL_PATH: {MODEL_PATH}")

    #-------------- player spawn --------------
    spawn_obj = get_obj("PlayerSpawn")

    if spawn_obj is None:
        print("WARNING: Missing PlayerSpawn. Defaulting to [0.0, 0.0, 0.0]")
        player_spawn = [0.0, 0.0, 0.0]
    else:
        player_spawn = vec3(spawn_obj.matrix_world.translation)

    #-------------- cameras --------------
    cameras = []

    for cam_name in CAMERA_NAMES:
        pos_obj = get_obj(f"{cam_name}_Pos")
        target_obj = get_obj(f"{cam_name}_Target")
        zone_obj = get_obj(f"{cam_name}_Zone")

        if pos_obj is None:
            raise RuntimeError(f"Missing object: {cam_name}_Pos")
        if target_obj is None:
            raise RuntimeError(f"Missing object: {cam_name}_Target")
        if zone_obj is None:
            raise RuntimeError(f"Missing object: {cam_name}_Zone")

        zone_min, zone_max = world_aabb(zone_obj)

        # Optional custom fov property on Pos or Zone object
        fov = pos_obj.get("fov", zone_obj.get("fov", 95.0))

        cameras.append({
            "position": vec3(pos_obj.matrix_world.translation),
            "target": vec3(target_obj.matrix_world.translation),
            "zoneMin": vec3(zone_min),
            "zoneMax": vec3(zone_max),
            "fov": float(fov)
        })

    #-------------- room walls colliders--------------
    room_walls = get_obj(ROOM_WALLS_OBJECT)

    if room_walls is None:
        raise RuntimeError(f"Missing object: {ROOM_WALLS_OBJECT}")

    room_min, room_max = world_aabb(room_walls)

    colliders = make_perimeter_colliders_from_bounds(
        room_min,
        room_max,
        WALL_THICKNESS
    )

    #-------------- extra collider objects --------------
    if EXPORT_EXTRA_COL_OBJECTS:
        for obj in bpy.data.objects:
            if obj.type == "MESH" and obj.name.startswith("Col_"):
                mn, mx = world_aabb(obj)
                colliders.append({
                    "min": vec3(mn),
                    "max": vec3(mx)
                })

    #-------------- doors --------------
    doors = []

    for obj in bpy.data.objects:
        if obj.type == "MESH" and obj.name.startswith("Door_"):
            mn, mx = world_aabb(obj)

            target_room = str(obj.get("targetRoom", ""))

            spawn_name = obj.name + "_Spawn"
            door_spawn = get_obj(spawn_name)

            if door_spawn is None:
                print(f"WARNING: {obj.name} has no matching spawn empty named {spawn_name}")
                target_spawn = [0.0, 0.0, 0.0]
            else:
                target_spawn = vec3(door_spawn.matrix_world.translation)

            doors.append({
                "min": vec3(mn),
                "max": vec3(mx),
                "targetRoom": target_room,
                "targetSpawn": target_spawn
            })

    if USE_TEMP_DOOR:
        doors.extend(TEMP_DOORS)
    
    #--interactions--
    interactions = []

    for obj in bpy.data.objects:
        if obj.type == "MESH" and obj.name.startswith("Interact_"):
            mn, mx = world_aabb(obj)

            interaction_id = str(obj.get("id", obj.name))
            interaction_type = str(obj.get("type", "message"))

            message = str(obj.get("message", ""))
            asset = str(obj.get("asset", ""))
            target_id = str(obj.get("targetId", ""))
            
            repeatable = bool(obj.get("repeatable", True))

            interaction = {
                        "id": interaction_id,
                        "type": interaction_type,
                        "min": vec3(mn),
                        "max": vec3(mx),
                        "repeatable": repeatable
                    }

            if message:
                interaction["message"] = message

            if asset:
                interaction["asset"] = asset

            if target_id:
                interaction["targetId"] = target_id

            interactions.append(interaction)

    #--lights--

    lights = []

    for obj in bpy.data.objects:
        if not obj.name.startswith("Light_"):
            continue

        if obj.name.endswith("_Target"):
            continue

        light_type = str(obj.get("lightType", "point"))

        target_name = obj.name + "_Target"
        target_obj = get_obj(target_name)
        if target_obj is None:
            print(f"WARNING: {obj.name} has no matching target empty named {target_name}")
            target = vec3(pos)
        else:
            target = vec3(target_obj.matrix_world.translation)

        pos = obj.matrix_world.translation

        color = parse_color_prop(obj.get("color", None))
        intensity = float(obj.get("intensity", 4.0))
        radius = float(obj.get("radius", 14.0))
        
        inner_cutoff = float(obj.get("innerCutoff", 12.0))
        outer_cutoff = float(obj.get("outerCutoff", 28.0))

        flicker_enabled = bool(obj.get("flicker", False))
        flicker_type = str(obj.get("flickerType", "sine"))
        flicker_frequency = float(obj.get("flickerFrequency", 8.0))
        flicker_strength = float(obj.get("flickerStrength", 0.2))
        flicker_randomness = float(obj.get("flickerRandomness", 0.0))
        flicker_threshold = float(obj.get("flickerThreshold", 0.85))
        
        light_json = {
            "type": light_type,
            "position": vec3(pos),
            "color": color,
            "intensity": round_float(intensity),
            "radius": round_float(radius)
        }

        if light_type == "spot":
            light_json["target"] = target
            light_json["innerCutoff"] = round_float(inner_cutoff)
            light_json["outerCutoff"] = round_float(outer_cutoff)

        if flicker_enabled:
            light_json["flicker"] = {
                "enabled": True,
                "type": flicker_type,
                "frequency": round_float(flicker_frequency),
                "strength": round_float(flicker_strength),
                "randomness": round_float(flicker_randomness),
                "threshold": round_float(flicker_threshold)
            }
        else:
            light_json["flicker"] = {
                "enabled": False
            }

        lights.append(light_json)
       
    #--ambient color--

    ambientColor = get_scene_ambient()

    #-------------- final json--------------
    room_json = {
        "name": ROOM_NAME,
        "model": MODEL_PATH,
        "playerSpawn": player_spawn,
        "cameras": cameras,
        "colliders": colliders,
        "doors": doors,
        "interactions": interactions,
        "lights": lights,
        "ambientColor": ambientColor
    }

    # Save next to the .blend file if the .blend is saved
    blend_dir = bpy.path.abspath("//")

    # If the .blend file is not saved, fallback to /tmp
    if not blend_dir or blend_dir == "//":
        blend_dir = "/tmp/"

    output_path = os.path.join(blend_dir, f"{ROOM_ID}.json")

    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(room_json, f, indent=2)

    print("\n----- ROOM JSON -----")
    print(json.dumps(room_json, indent=2))
    print(f"\nSaved JSON to: {output_path}")

# run
export_room_json()
