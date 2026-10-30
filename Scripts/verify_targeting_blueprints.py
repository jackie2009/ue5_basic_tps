"""Compile existing character blueprints in memory only and check inherited targeting setup."""
import unreal

hero_bp = unreal.load_asset("/Game/Characters/BP_Hero")
assert hero_bp
unreal.BlueprintEditorLibrary.compile_blueprint(hero_bp)
hero_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Characters/BP_Hero")
assert hero_class
hero = unreal.get_default_object(hero_class)
targeting = hero.get_editor_property("targeting_comp")
assert targeting is not None and targeting.get_editor_property("auto_select")
assert hero.get_editor_property("targetable_comp") is not None
assert hero.get_editor_property("target_outline_comp") is not None
assert hasattr(unreal.CombatFacingUtils, "face_to_selected_target_immediately")
assert hasattr(unreal.CombatFacingUtils, "face_to_target_immediately")
unreal.log("TARGETING_BLUEPRINT_INTEGRATION_OK")
