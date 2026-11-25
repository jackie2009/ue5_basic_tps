using UnrealBuildTool;

// 道具系统 Runtime 模块：数据 + 逻辑，不含 UI，不依赖 basic_tps（战斗）模块。
public class ItemSystem : ModuleRules
{
	public ItemSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects" });
	}
}
