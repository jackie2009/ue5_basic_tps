#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Loot/LootTypes.h"
#include "LootGeneratorSubsystem.generated.h"

class UItemDatabaseSubsystem;

/**
 * 暗黑2 风格掉落引擎（读取 Content/Data/LootRules.json 与 MonsterLootProfiles.json）。
 * 支持零编译热重载（Loot.Reload）、批量模拟验算（Loot.Sim）以及编辑器下拉选项查询。
 */
UCLASS()
class ITEMSYSTEM_API ULootGeneratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static ULootGeneratorSubsystem* Get(const UObject* WorldContext);

	/**
	 * 静态读取磁盘上的 MonsterLootProfiles.json 返回所有可用 Profile 名称。
	 * 供怪物蓝图组件 UPROPERTY(meta=(GetOptions="...")) 在非运行期编辑器下拉框直接选择。
	 */
	static TArray<FString> GetAvailableProfileIdsFromDisk();

	/** 热重载 ItemBase.csv + RandomAffixes.json + LootRules.json + MonsterLootProfiles.json 并执行配置校验 */
	UFUNCTION(BlueprintCallable, Category = "ItemSystem|Loot")
	bool ReloadAllLootConfigs();

	/** 3 级优先级解析怪物掉落配置：蓝图直选 > MonsterBaseVo.dropID 映射 > 阶级默认兜底 */
	UFUNCTION(BlueprintPure, Category = "ItemSystem|Loot")
	FName ResolveProfileId(FName ExplicitProfileId, int32 DropId, EMonsterLootTier FallbackTier) const;

	/** 执行一次完整掉落计算（使用随机种子） */
	UFUNCTION(BlueprintCallable, Category = "ItemSystem|Loot")
	bool RollLoot(FName ProfileId, const FLootContext& Context, TArray<FLootDropResult>& OutResults) const;

	/** 使用确定性随机种子执行一次掉落计算（用于自动化测试与可复现调试） */
	bool RollLootWithSeed(FName ProfileId, const FLootContext& Context, int32 Seed, TArray<FLootDropResult>& OutResults) const;

	/** 模拟 N 次击杀，返回掉落统计报表 */
	UFUNCTION(BlueprintCallable, Category = "ItemSystem|Loot")
	FLootSimulationReport SimulateProfile(FName ProfileId, int32 KillCount, const FLootContext& Context, int32 Seed = 12345) const;

	/** 校验全部池子、引用、ItemID 与嵌套循环，返回错误消息列表（空=完全合法） */
	TArray<FString> ValidateConfigs() const;

	const FMonsterLootProfileConfig* FindProfile(FName ProfileId) const { return Profiles.Find(ProfileId); }
	const FLootPoolConfig* FindPool(FName PoolId) const { return Pools.Find(PoolId); }
	const FLootMFConfig& GetMFConfig() const { return MFConfig; }

	static FName GetDefaultProfileIdForTier(EMonsterLootTier Tier);
	static EMonsterLootTier ParseTierString(const FString& Str);

private:
	bool LoadLootRulesJson(const FString& FilePath);
	bool LoadMonsterProfilesJson(const FString& FilePath);
	void RollFromPool(
		FName PoolId,
		const FMonsterLootProfileConfig& Profile,
		const FLootContext& Context,
		TSet<int32>& UsedItemIds,
		bool bAllowDuplicate,
		int32 Depth,
		FRandomStream& Rng,
		TArray<FLootDropResult>& OutResults) const;
	int32 PickDynamicEquipItemId(const FLootPoolEntry& Entry, int32 MonsterLevel, const TSet<int32>& UsedItemIds, bool bAllowDuplicate, FRandomStream& Rng) const;
	int32 RollQualityForItem(const FMonsterLootProfileConfig& Profile, const FLootContext& Context, FRandomStream& Rng) const;
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	TMap<EMonsterLootTier, FLootQualityPreset> QualityPresets;
	FLootMFConfig MFConfig;
	TMap<FName, FLootPoolConfig> Pools;
	TMap<FName, FMonsterLootProfileConfig> Profiles;
	TMap<int32, FName> DropIdMap;

	FString ContentDir;
	TArray<IConsoleObject*> ConsoleCommands;
};
