"""Apply the screenshot-inspired exploration camera/movement profile to Assassin only.
Run in UE 5.7 PythonScript commandlet with the project's editor closed.
"""
import json
import os
import shutil
import unreal

PATH = "/Game/AssassinGirl/BP_Hero_Assassin"
OUTPUT = os.path.join(unreal.Paths.project_saved_dir(), "AssassinProfile")
os.makedirs(OUTPUT, exist_ok=True)
source = os.path.join(unreal.Paths.project_content_dir(), "AssassinGirl", "BP_Hero_Assassin.uasset")
assert shutil.disk_usage(source).free > 1024 ** 3, "At least 1 GiB free space required before saving"
backup = os.path.join(OUTPUT, "BP_Hero_Assassin.before.uasset")
if not os.path.exists(backup):
    shutil.copy2(source, backup)
bp = unreal.load_asset(PATH)
cls = unreal.EditorAssetLibrary.load_blueprint_class(PATH)
cdo = unreal.get_default_object(cls)
old_speed = cdo.get_editor_property("MoveSpeed")
direction_curve = cdo.get_editor_property("TpsRunSpeedByDir")
assert direction_curve is not None, "Existing directional speed curve must remain intact"

movement_values = {
    "max_walk_speed": 420.0,
    "max_walk_speed_crouched": 160.0,
    "min_analog_walk_speed": 30.0,
    "max_acceleration": 1100.0,
    "braking_deceleration_walking": 1000.0,
    "ground_friction": 5.0,
    "use_separate_braking_friction": True,
    "braking_friction": 1.5,
    "braking_friction_factor": 1.0,
    "rotation_rate": unreal.Rotator(pitch=0, yaw=420, roll=0),
    "orient_rotation_to_movement": True,
    "use_controller_desired_rotation": False,
    "air_control": 0.18,
    "jump_z_velocity": 450.0,
    "gravity_scale": 1.25,
}
arm_values = {
    "target_arm_length": 480.0,
    "relative_location": unreal.Vector(x=0, y=0, z=20),
    "relative_rotation": unreal.Rotator(pitch=0, yaw=0, roll=0),
    "target_offset": unreal.Vector(x=0, y=0, z=35),
    "socket_offset": unreal.Vector(x=0, y=70, z=130),
    "use_pawn_control_rotation": True,
    "inherit_pitch": True,
    "inherit_yaw": True,
    "inherit_roll": False,
    "enable_camera_lag": True,
    "camera_lag_speed": 10.0,
    "camera_lag_max_distance": 35.0,
    "enable_camera_rotation_lag": True,
    "camera_rotation_lag_speed": 16.0,
    "use_camera_lag_substepping": True,
    "camera_lag_max_time_step": 1.0 / 60.0,
    "do_collision_test": True,
    "probe_size": 14.0,
}
camera_values = {
    "field_of_view": 80.0,
    "relative_location": unreal.Vector(x=0, y=0, z=0),
    "relative_rotation": unreal.Rotator(pitch=-8, yaw=0, roll=0),
    "use_pawn_control_rotation": False,
    "constrain_aspect_ratio": False,
}

def apply(obj, properties):
    obj.modify()
    for name, value in properties.items():
        obj.set_editor_property(name, value)

bp.modify()
apply(cdo, {"MoveSpeed": 420.0, "use_controller_rotation_yaw": False,
            "use_controller_rotation_pitch": False, "use_controller_rotation_roll": False})
apply(cdo.get_editor_property("character_movement"), movement_values)
subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
library = unreal.SubobjectDataBlueprintFunctionLibrary
edited = set()
for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
    data = library.get_data(handle)
    original = library.get_object(data)
    properties = arm_values if isinstance(original, unreal.SpringArmComponent) else (
        camera_values if isinstance(original, unreal.CameraComponent) else None)
    if properties is None:
        continue
    # Creates an inheritable component override owned by this child Blueprint.
    obj = library.get_object_for_blueprint(data, bp)
    assert obj and obj.get_path_name().startswith(PATH + "."), "Refusing to mutate a parent component"
    if obj.get_path_name() not in edited:
        apply(obj, properties)
        edited.add(obj.get_path_name())
assert len(edited) == 2, "Expected exactly one camera and one spring arm override"
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, only_if_is_dirty=False)
# UE can return success even after a low-level disk-full write failure.
assert os.path.getsize(source) >= os.path.getsize(backup), "Saved package is unexpectedly truncated"

# Validate the constructed Blueprint, including the inherited construction script.
world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
actor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
    unreal.EditorAssetLibrary.load_blueprint_class(PATH), unreal.Vector(x=0, y=0, z=100))
assert actor
camera = actor.get_component_by_class(unreal.CameraComponent)
arm = actor.get_component_by_class(unreal.SpringArmComponent)
move = actor.get_component_by_class(unreal.CharacterMovementComponent)
for obj, properties in [(camera, camera_values), (arm, arm_values), (move, movement_values)]:
    for name, expected in properties.items():
        actual = obj.get_editor_property(name)
        if isinstance(expected, float):
            assert abs(actual - expected) < 0.001, (name, actual, expected)
        elif isinstance(expected, bool):
            assert actual == expected, (name, actual, expected)
assert actor.get_editor_property("MoveSpeed") == 420.0
report = {"asset": PATH, "previous_MoveSpeed": old_speed, "MoveSpeed": 420,
          "directional_speed_curve": direction_curve.get_path_name(), "overrides": sorted(edited),
          "movement": {k: str(v) for k, v in movement_values.items()},
          "spring_arm": {k: str(v) for k, v in arm_values.items()},
          "camera": {k: str(v) for k, v in camera_values.items()}}
with open(os.path.join(OUTPUT, "applied.json"), "w", encoding="utf-8") as stream:
    json.dump(report, stream, indent=2, ensure_ascii=False)
unreal.log("ASSASSIN_PROFILE_APPLIED_AND_CONSTRUCTION_VERIFIED")
