"""Import Control_Hints element textures into /Game/UI/HUD with UI texture settings,
and recompile/save WBP_CombatCharater.
"""
import os
import unreal

SRC_DIR = r"C:\Users\Admin\.gemini\antigravity\brain\79ed5c97-34b5-41ea-abfc-f25eb147da27\scratch\hud_elements"
DEST_PATH = "/Game/UI/HUD"
FILES = [
    "Control_Hints_Ornament.png",
    "Control_Hints_Bg.png",
    "Control_Hints_Keycap.png",
    "Rage_Glow.png",
]

tasks = []
for fn in FILES:
    full_src = os.path.join(SRC_DIR, fn)
    assert os.path.exists(full_src), f"Missing source PNG: {full_src}"
    task = unreal.AssetImportTask()
    task.filename = full_src
    task.destination_path = DEST_PATH
    task.destination_name = os.path.splitext(fn)[0]
    task.replace_existing = True
    task.automated = True
    task.save = False
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for fn in FILES:
    asset_name = os.path.splitext(fn)[0]
    asset_path = f"{DEST_PATH}/{asset_name}"
    tex = unreal.load_asset(asset_path)
    assert isinstance(tex, unreal.Texture2D), f"Failed to import {asset_path}"
    tex.modify()
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property("srgb", True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(tex, only_if_is_dirty=False), f"Failed to save {asset_path}"

# Recompile & save WBP_CombatCharater so CDO picks up the newly imported default textures
wbp = unreal.load_asset("/Game/UI/WBP_CombatCharater")
assert wbp is not None
unreal.BlueprintEditorLibrary.compile_blueprint(wbp)
assert unreal.EditorAssetLibrary.save_loaded_asset(wbp, only_if_is_dirty=False), "Failed to save WBP_CombatCharater"
unreal.log("IMPORT_HUD_HINT_ELEMENTS_OK")
