using UnrealBuildTool;
using System.IO;

// 道具系统 Runtime 模块：数据 + 逻辑，不含 UI，不依赖 basic_tps（战斗）模块。
public class ItemSystem : ModuleRules
{
	public ItemSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Projects", "Json", "JsonUtilities" });

		// These files are read directly at runtime, so they must remain outside the pak.
		foreach (string DirectoryName in new[] { "Data", "Icons" })
		{
			string DirectoryPath = Path.Combine(PluginDirectory, "Content", DirectoryName);
			foreach (string FilePath in Directory.GetFiles(DirectoryPath, "*", SearchOption.AllDirectories))
			{
				RuntimeDependencies.Add(FilePath, StagedFileType.NonUFS);
			}
		}
	}
}
