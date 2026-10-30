"""Run with UE 5.7 PythonScript commandlet. Only creates/updates our dedicated material.

UnrealEditor-Cmd.exe basic_tps.uproject -run=pythonscript
  -script=Scripts/create_target_outline.py -EnablePlugins=PythonScriptPlugin -unattended
"""
import unreal

PATH = "/Game/Combat/Targeting"
NAME = "M_TargetOutline"
library = unreal.MaterialEditingLibrary
material = unreal.load_asset(f"{PATH}/{NAME}")
if material is None:
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        NAME, PATH, unreal.Material, unreal.MaterialFactoryNew())
assert material is not None
material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
# After tonemapping keeps the two-pixel white rim independent of scene exposure.
material.set_editor_property("blendable_location", unreal.BlendableLocation.BL_SCENE_COLOR_AFTER_TONEMAPPING)
library.delete_all_material_expressions(material)

custom = library.create_material_expression(material, unreal.MaterialExpressionCustom, 400, 0)
custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
custom.set_editor_property("description", "Visible outer silhouette only; reserved stencil 250")
custom.set_editor_property("code", r"""
float2 uv = GetDefaultSceneTextureUV(Parameters, 25);
float2 pixel = GetSceneTextureViewSize(25).zw;
float centre = abs(Stencil.r - 250.0) < 0.5 ? 1.0 : 0.0;
float edge = 0.0;
const float2 offsets[8] = {
    float2(1,0), float2(-1,0), float2(0,1), float2(0,-1),
    float2(0.7071,0.7071), float2(-0.7071,0.7071),
    float2(0.7071,-0.7071), float2(-0.7071,-0.7071)
};
[unroll] for (int i=0; i<8; ++i)
{
    float2 p = ClampSceneTextureUV(uv + offsets[i] * pixel * OutlineWidth, 25);
    float mask = abs(SceneTextureLookup(p, 25, false).r - 250.0) < 0.5 ? 1.0 : 0.0;
    float targetDepth = SceneTextureLookup(p, 13, false).r;
    float sceneDepth = SceneTextureLookup(p, 1, false).r;
    // Suppress hidden parts AND prevent dilation painting onto a foreground occluder.
    float visible = (targetDepth <= sceneDepth + 1.0 && targetDepth <= Depth.r + 1.0) ? 1.0 : 0.0;
    edge = max(edge, mask * visible);
}
return lerp(Scene.rgb, float3(1,1,1), saturate(edge * (1.0-centre)));
""")
names = ["Scene", "Stencil", "Depth", "CustomDepth", "OutlineWidth"]
inputs = []
for name in names:
    entry = unreal.CustomInput()
    entry.set_editor_property("input_name", name)
    inputs.append(entry)
custom.set_editor_property("inputs", inputs)

# Explicit graph dependencies make UE compile the required scene textures for custom HLSL.
for i, (name, texture_id) in enumerate([
    ("Scene", unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0),
    ("Stencil", unreal.SceneTextureId.PPI_CUSTOM_STENCIL),
    ("Depth", unreal.SceneTextureId.PPI_SCENE_DEPTH),
    ("CustomDepth", unreal.SceneTextureId.PPI_CUSTOM_DEPTH),
]):
    node = library.create_material_expression(material, unreal.MaterialExpressionSceneTexture, 0, i * 160)
    node.set_editor_property("scene_texture_id", texture_id)
    assert library.connect_material_expressions(node, "Color", custom, name)

width = library.create_material_expression(material, unreal.MaterialExpressionScalarParameter, 0, 640)
width.set_editor_property("parameter_name", "OutlineWidth")
width.set_editor_property("default_value", 2.0)
assert library.connect_material_expressions(width, "", custom, "OutlineWidth")
assert library.connect_material_property(custom, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
library.recompile_material(material)
assert unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
unreal.log("TARGET_OUTLINE_MATERIAL_SAVED " + material.get_path_name())
