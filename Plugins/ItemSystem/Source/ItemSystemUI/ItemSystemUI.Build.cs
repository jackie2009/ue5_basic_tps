using UnrealBuildTool;

// 道具系统 UI 模块（UMG）。Widgets/ 与 Logic/ 永久保留；CodeLayout/ 是临时的代码搭建布局，将来换成 WBP 后删除。
public class ItemSystemUI : ModuleRules
{
	public ItemSystemUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UMG", "InputCore", "DeveloperSettings", "ItemSystem" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
