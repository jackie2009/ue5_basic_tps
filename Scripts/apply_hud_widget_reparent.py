"""Reparent /Game/UI/WBP_CombatCharater to /Script/basic_tps.CombatHUDWidget and verify all HUD/Hero/Enemy blueprints.

Run with Unreal Editor closed:
UnrealEditor-Cmd.exe basic_tps.uproject -run=pythonscript -script=Scripts/apply_hud_widget_reparent.py -EnablePlugins=PythonScriptPlugin -unattended -nop4 -nosplash -NullRHI
"""
import os
import shutil
import unreal

WBP_PATH = "/Game/UI/WBP_CombatCharater"
BACKUP_DIR = os.path.join(unreal.Paths.project_saved_dir(), "HUDBackup")
os.makedirs(BACKUP_DIR, exist_ok=True)
SRC_FILE = os.path.join(unreal.Paths.project_content_dir(), "UI", "WBP_CombatCharater.uasset")
BAK_FILE = os.path.join(BACKUP_DIR, "WBP_CombatCharater.before.uasset")
if not os.path.exists(BAK_FILE):
    shutil.copy2(SRC_FILE, BAK_FILE)

wbp = unreal.load_asset(WBP_PATH)
assert wbp is not None, f"Failed to load {WBP_PATH}"

hud_cls = unreal.CombatHUDWidget.static_class()
assert hud_cls is not None, "CombatHUDWidget C++ class not found; build basic_tpsEditor first"

wbp.modify()
unreal.BlueprintEditorLibrary.reparent_blueprint(wbp, hud_cls)
unreal.BlueprintEditorLibrary.compile_blueprint(wbp)
assert unreal.EditorAssetLibrary.save_loaded_asset(wbp, only_if_is_dirty=False), (
    f"Failed to save {WBP_PATH}. Make sure Unreal Editor is closed before running this script."
)

for path in (
    "/Game/UI/WBP_CombatCharater",
    "/Game/UI/NewHUD",
    "/Game/Characters/BP_Hero",
    "/Game/EssentialSwordShieldAnimationPack/Blueprint/BP_Hero_Sword",
    "/Game/AnimalsWolf/Blueprints/BP_Wolf",
    "/Game/Enemies/SkeletonLight/BP_SkeletonLight",
):
    bp = unreal.load_asset(path)
    assert bp is not None, f"Missing blueprint: {path}"
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert bp.generated_class() is not None, f"Failed to compile: {path}"

hero_cls = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Characters/BP_Hero")
hero_cdo = unreal.get_default_object(hero_cls)
zones = list(hero_cdo.get_editor_property("stamina_zone_capacities"))
assert zones == [20, 30, 50, 20], f"Unexpected default stamina zones: {zones}"
unreal.log(f"HUD_REPARENT_AND_VERIFY_OK zones={zones}")
