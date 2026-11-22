"""Compile existing character blueprints in memory only and check inherited targeting setup."""
import unreal

hero_blueprints = (
    "/Game/Characters/BP_Hero",
    "/Game/AssassinGirl/BP_Hero_Assassin",
    "/Game/EssentialSwordShieldAnimationPack/Blueprint/BP_Hero_Sword",
)

for path in hero_blueprints:
    hero_class = unreal.EditorAssetLibrary.load_blueprint_class(path)
    assert hero_class is not None, path
    hero = unreal.get_default_object(hero_class)
    targeting = hero.get_editor_property("targeting_comp")
    outline = hero.get_editor_property("target_outline_comp")
    assert hero.get_editor_property("targetable_comp") is not None, path
    assert targeting is not None and not targeting.get_editor_property("auto_select"), path
    assert outline is not None and not outline.get_editor_property("show_outline"), path
    unreal.log(f"{path}: automatic targeting and outline disabled")

unreal.BlueprintEditorLibrary.compile_blueprint(unreal.load_asset(hero_blueprints[0]))
assert hasattr(unreal.CombatFacingUtils, "face_to_selected_target_immediately")
assert hasattr(unreal.CombatFacingUtils, "face_to_target_immediately")
unreal.log("TARGETING_BLUEPRINT_INTEGRATION_OK")
