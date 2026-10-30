"""Read-only profile inspection. Writes reports only under Saved/AssassinProfile."""
import json
import os
import re
import unreal

OUT = os.path.join(unreal.Paths.project_saved_dir(), "AssassinProfile")
os.makedirs(OUT, exist_ok=True)
BP_PATH = "/Game/AssassinGirl/BP_Hero_Assassin"
bp = unreal.load_asset(BP_PATH)
cls = unreal.EditorAssetLibrary.load_blueprint_class(BP_PATH)
cdo = unreal.get_default_object(cls)

def values(obj, names):
    result = {"object": obj.get_path_name(), "class": obj.get_class().get_name()}
    for name in names:
        try:
            value = obj.get_editor_property(name)
            result[name] = value if isinstance(value, (bool, int, float, str)) else str(value)
        except Exception as exc:
            result[name] = "UNAVAILABLE: " + str(exc)
    return result

movement_names = ["max_walk_speed", "max_walk_speed_crouched", "max_acceleration", "braking_deceleration_walking", "ground_friction", "braking_friction", "braking_friction_factor", "use_separate_braking_friction", "rotation_rate", "orient_rotation_to_movement", "use_controller_desired_rotation", "air_control", "jump_z_velocity", "gravity_scale", "min_analog_walk_speed"]
arm_names = ["target_arm_length", "socket_offset", "target_offset", "relative_location", "relative_rotation", "use_pawn_control_rotation", "inherit_pitch", "inherit_yaw", "inherit_roll", "enable_camera_lag", "camera_lag_speed", "camera_lag_max_distance", "enable_camera_rotation_lag", "camera_rotation_lag_speed", "use_camera_lag_substepping", "camera_lag_max_time_step", "do_collision_test", "probe_size"]
camera_names = ["field_of_view", "relative_location", "relative_rotation", "use_pawn_control_rotation", "constrain_aspect_ratio", "aspect_ratio", "post_process_blend_weight"]
report = {"hero": values(cdo, ["use_controller_rotation_yaw", "use_controller_rotation_pitch", "use_controller_rotation_roll"]), "blueprint_variables": values(cdo, [name for name in dir(cdo) if re.search(r"walk|sprint|speed|running|camera|sensitivity", name) and not callable(getattr(cdo, name, None))]), "components": []}
subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
library = unreal.SubobjectDataBlueprintFunctionLibrary
for handle in subsystem.k2_gather_subobject_data_for_blueprint(bp):
    data = library.get_data(handle)
    obj = library.get_object(data)
    if isinstance(obj, unreal.SpringArmComponent):
        report["components"].append(values(obj, arm_names))
    elif isinstance(obj, unreal.CameraComponent):
        report["components"].append(values(obj, camera_names))
    elif isinstance(obj, unreal.CharacterMovementComponent):
        report["components"].append(values(obj, movement_names))
report["movement_cdo"] = values(cdo.get_editor_property("character_movement"), movement_names)
report["exporters"] = [name for name in dir(unreal) if "Exporter" in name and ("Blueprint" in name or "Object" in name)]
for asset, label in [(bp, "assassin"), (unreal.load_asset("/Game/Characters/BP_Hero"), "hero")]:
    task = unreal.AssetExportTask()
    task.object = asset
    task.filename = os.path.join(OUT, label + ".t3d")
    task.automated = True
    task.prompt = False
    task.exporter = unreal.ObjectExporterT3D()
    try:
        report[label + "_export"] = unreal.Exporter.run_asset_export_task(task)
    except Exception as exc:
        report[label + "_export"] = str(exc)
with open(os.path.join(OUT, "before.json"), "w", encoding="utf-8") as stream:
    json.dump(report, stream, ensure_ascii=False, indent=2)
unreal.log("ASSASSIN_PROFILE_INSPECTED " + OUT)
