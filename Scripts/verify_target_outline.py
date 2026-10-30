"""Render isolated outline fixtures in a separate editor using -ExecCmds="py <script>".
No existing map or Blueprint is saved. Images are written under Saved/Tests/TargetOutline.
"""
import os
import unreal

world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
material = unreal.load_asset("/Game/Combat/Targeting/M_TargetOutline")
assert material
unreal.MaterialEditingLibrary.recompile_material(material)
unreal.MaterialEditingLibrary.get_statistics(material)

subject = actors.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(0, 0, 0))
subject.set_actor_rotation(unreal.Rotator(pitch=0, yaw=-90, roll=0), False)
mesh = subject.get_component_by_class(unreal.SkeletalMeshComponent)
mesh.set_skeletal_mesh_asset(unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"))
mesh.set_custom_depth_stencil_value(250)
mesh.set_render_custom_depth(True)
for body_material in mesh.get_materials():
    if body_material:
        unreal.MaterialEditingLibrary.get_statistics(body_material)

light = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 300), unreal.Rotator(pitch=-35, yaw=-135, roll=0))
light.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(10.0)

capture_actor = actors.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(360, 0, 95), unreal.Rotator(pitch=0, yaw=180, roll=0))
capture = capture_actor.get_component_by_class(unreal.SceneCaptureComponent2D)
capture.set_editor_property("capture_every_frame", False)
capture.set_editor_property("capture_on_movement", False)
capture.set_editor_property("always_persist_rendering_state", True)
capture.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
capture.set_editor_property("post_process_blend_weight", 1.0)
capture.set_editor_property("fov_angle", 45.0)
texture = unreal.RenderingLibrary.create_render_target2d(world, 640, 480, unreal.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_property("texture_target", texture)
capture.add_or_update_blendable(material, 1.0)
settings = capture.get_editor_property("post_process_settings")
settings.set_editor_property("override_auto_exposure_method", True)
settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
settings.set_editor_property("override_auto_exposure_bias", True)
settings.set_editor_property("auto_exposure_bias", 8.0)
capture.set_editor_property("post_process_settings", settings)
capture.add_or_update_blendable(material, 1.0)
unreal.log("OUTLINE_DIAGNOSTIC " + str(capture.get_editor_property("post_process_blend_weight")) + " " + str(capture_actor.get_actor_rotation()) + " " + str(mesh.get_editor_property("custom_depth_stencil_value")))
output = os.path.join(unreal.Paths.project_saved_dir(), "Tests", "TargetOutline")
os.makedirs(output, exist_ok=True)

def render(name):
    capture.capture_scene()
    unreal.RenderingLibrary.export_render_target(world, texture, output, name)
    unreal.log("TARGET_OUTLINE_RENDER " + name)

wall = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(180, 0, 95))
wall_mesh = wall.get_component_by_class(unreal.StaticMeshComponent)
wall_mesh.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube"))
unreal.MaterialEditingLibrary.get_statistics(wall_mesh.get_material(0))
wall.set_actor_scale3d(unreal.Vector(0.1, 3.0, 3.0))
wall_mesh.set_visibility(False)

# Scene capture changes must land on different engine frames; a commandlet's one Python
# invocation can reuse the same render state for all four captures and produce false evidence.
elapsed = 0.0
stage = 0
def tick(delta):
    global elapsed, stage, callback
    elapsed += delta
    if elapsed < 3.0:
        return
    elapsed = 0.0
    if stage == 0:
        render("selected.png")
        mesh.set_render_custom_depth(False)
    elif stage == 1:
        render("unselected.png")
        mesh.set_render_custom_depth(True)
        wall_mesh.set_visibility(True)
    elif stage == 2:
        render("occluded.png")
        mesh.set_render_custom_depth(False)
    elif stage == 3:
        render("occluded_unselected.png")
        assert os.path.isfile(os.path.join(output, "selected.png"))
        unreal.log("TARGET_OUTLINE_RENDER_COMPLETE")
        unreal.unregister_slate_post_tick_callback(callback)
        unreal.SystemLibrary.quit_editor()
    stage += 1
callback = unreal.register_slate_post_tick_callback(tick)
