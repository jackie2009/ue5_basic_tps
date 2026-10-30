"""Reload and render the saved Assassin camera in an isolated UE 5.7 editor.

Run with -RenderOffscreen -ExecCmds="py E:/basic_tps_demo/Scripts/preview_assassin_profile.py".
This replaces the helper editor's map. Never run in an editor with unsaved work.
No content assets or maps are saved; output goes to Saved/AssassinProfile.
"""
import json
import os
import unreal

output = os.path.join(unreal.Paths.project_saved_dir(), "AssassinProfile")
os.makedirs(output, exist_ok=True)
world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
hero = actors.spawn_actor_from_class(
    unreal.load_class(None, "/Game/AssassinGirl/BP_Hero_Assassin.BP_Hero_Assassin_C"),
    unreal.Vector(x=0, y=0, z=96), unreal.Rotator(pitch=0, yaw=0, roll=0))
hero.set_actor_location(unreal.Vector(x=0, y=0, z=96), False, False)
camera = hero.get_component_by_class(unreal.CameraComponent)
arm = hero.get_component_by_class(unreal.SpringArmComponent)
movement = hero.get_component_by_class(unreal.CharacterMovementComponent)
assert camera.get_editor_property("field_of_view") == 80
assert arm.get_editor_property("target_arm_length") == 480
assert hero.get_editor_property("MoveSpeed") == 420
assert movement.get_editor_property("max_walk_speed") == 420
assert movement.get_editor_property("max_acceleration") == 1100
assert movement.get_editor_property("braking_deceleration_walking") == 1000
assert movement.get_editor_property("orient_rotation_to_movement")
assert not hero.get_editor_property("use_controller_rotation_yaw")
assert not hero.get_editor_property("use_controller_rotation_roll")
for name, expected in [("relative_location", (0, 0, 20)), ("target_offset", (0, 0, 35)),
                       ("socket_offset", (0, 70, 130))]:
    value = arm.get_editor_property(name)
    assert tuple(round(getattr(value, axis), 3) for axis in "xyz") == expected
assert abs(camera.get_editor_property("relative_rotation").pitch + 8) < 0.001
assert movement.get_editor_property("rotation_rate").yaw == 420

# Only this unsaved preview instance uses a frozen idle pose.
mesh = hero.get_editor_property("mesh")
mesh.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
mesh.play_animation(unreal.load_asset("/Game/AssassinGirl/Animation/Sequence/Idle/Unarmed_Idle_Seq"), True)
mesh.set_update_animation_in_editor(True)
mesh.set_position(0.0, False)
for material in mesh.get_materials():
    if material:
        unreal.MaterialEditingLibrary.get_statistics(material)

floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x=0, y=0, z=-10))
floor_mesh = floor.get_component_by_class(unreal.StaticMeshComponent)
floor_mesh.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube"))
floor.set_actor_scale3d(unreal.Vector(x=80, y=80, z=0.2))
unreal.MaterialEditingLibrary.get_statistics(floor_mesh.get_material(0))
for x, y, height in [(700, -350, 150), (1000, 250, 200), (1700, -100, 280)]:
    marker = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x=x, y=y, z=height / 2))
    marker_mesh = marker.get_component_by_class(unreal.StaticMeshComponent)
    marker_mesh.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube"))
    marker.set_actor_scale3d(unreal.Vector(x=1, y=1, z=height / 100))

light = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(x=0, y=0, z=500),
                                     unreal.Rotator(pitch=-45, yaw=20, roll=0))
light.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(150)
fill = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(x=-200, y=100, z=500),
                                    unreal.Rotator(pitch=-35, yaw=-160, roll=0))
fill.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(45)
actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(x=0, y=0, z=0))

capture_actor = actors.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(x=0, y=0, z=300))
capture = capture_actor.get_component_by_class(unreal.SceneCaptureComponent2D)
capture.set_editor_property("capture_every_frame", False)
capture.set_editor_property("capture_on_movement", False)
capture.set_editor_property("always_persist_rendering_state", True)
capture.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
capture.set_editor_property("post_process_blend_weight", 1.0)
capture.set_editor_property("fov_angle", camera.get_editor_property("field_of_view"))
texture = unreal.RenderingLibrary.create_render_target2d(world, 1280, 720, unreal.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_property("texture_target", texture)
settings = capture.get_editor_property("post_process_settings")
settings.set_editor_property("override_auto_exposure_method", True)
settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
settings.set_editor_property("override_auto_exposure_bias", True)
settings.set_editor_property("auto_exposure_bias", 8)
capture.set_editor_property("post_process_settings", settings)

elapsed = 0.0
stage = 0
def tick(delta):
    global elapsed, stage, callback
    elapsed += delta
    if elapsed < 4:
        return
    elapsed = 0
    try:
        if stage == 0:
            # Read the actual spring-arm socket after editor component ticks/collision.
            location = camera.get_world_location()
            rotation = camera.get_world_rotation()
            assert abs(location.x + 480) < 1, str(location)
            assert abs(location.y - 70) < 1, str(location)
            assert abs(location.z - 281) < 1, str(location)
            assert abs(rotation.pitch + 8) < 0.01, str(rotation)
            capture_actor.set_actor_location_and_rotation(location, rotation, False, False)
            with open(os.path.join(output, "reload-verification.json"), "w", encoding="utf-8") as stream:
                json.dump({"status": "passed", "camera_location": [location.x, location.y, location.z],
                           "camera_rotation": [rotation.pitch, rotation.yaw, rotation.roll],
                           "FOV": 80, "MoveSpeed": 420}, stream, indent=2)
        elif stage == 1:
            capture.capture_scene()
        elif stage == 2:
            capture.capture_scene()
            unreal.RenderingLibrary.export_render_target(world, texture, output, "assassin-camera-preview.png")
            unreal.log("ASSASSIN_PROFILE_RELOAD_AND_RENDER_VERIFIED")
            unreal.unregister_slate_post_tick_callback(callback)
            unreal.SystemLibrary.quit_editor()
        stage += 1
    except Exception:
        unreal.unregister_slate_post_tick_callback(callback)
        unreal.SystemLibrary.quit_editor()
        raise
callback = unreal.register_slate_post_tick_callback(tick)
