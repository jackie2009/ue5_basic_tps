#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ItemSystemTypes.h"
#include "ItemDatabaseSubsystem.generated.h"

class UTexture2D;

/**
 * 道具与随机词缀配置库：
 * 读取插件 Content/Data/ItemBase.csv 与 Content/Data/RandomAffixes.json。
 * 独立于项目的 TableDataManagerSubsystem，保证插件可以整体拆走。
 */
UCLASS()
class ITEMSYSTEM_API UItemDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	static UItemDatabaseSubsystem* Get(const UObject* WorldContext);

	/** 热重载 ItemBase.csv 与 RandomAffixes.json */
	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	void ReloadAll();

	const FItemBaseRow* Find(int32 BaseID) const { return Rows.Find(BaseID); }
	const TMap<int32, FItemBaseRow>& GetAllRows() const { return Rows; }

	/** 按配置顺序排列的 ID 列表 */
	const TArray<int32>& GetOrderedIDs() const { return OrderedIDs; }

	const FRandomAffixRow* FindAffix(int32 AffixID) const { return AffixRows.Find(AffixID); }
	const TMap<int32, FRandomAffixRow>& GetAllAffixes() const { return AffixRows; }
	int32 GetAttackPercentAffixMaxLevel() const { return AttackPercentAffixMaxLevel; }

	FIntPoint GetCellSize(int32 BaseID) const;
	FIntPoint GetCellSize(const FItemInstance& Item) const { return GetCellSize(Item.BaseID); }

	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	UTexture2D* GetIcon(int32 BaseID);

	/** 创建基础实例（按道具 QualityMin 初始化，若 QualityMin >= 1 则自动生成对应随机词缀） */
	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	FItemInstance CreateInstance(int32 BaseID) const;

	/** 按目标品质创建实例并生成随机词缀（0=白 1=蓝 2=稀有/黄 3=暗金/橙） */
	UFUNCTION(BlueprintCallable, Category = "ItemSystem")
	FItemInstance CreateInstanceWithQuality(int32 BaseID, int32 TargetQuality) const;

	/** 使用确定性随机流创建并 Roll 词缀（供掉落模拟与单元测试使用） */
	FItemInstance CreateRolledInstance(int32 BaseID, int32 TargetQuality, FRandomStream& Rng) const;

	/** 品质颜色与名称（可在 LootRules.json 覆写，默认使用暗黑风格配色） */
	UFUNCTION(BlueprintPure, Category = "ItemSystem|Quality")
	static FLinearColor GetQualityColor(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "ItemSystem|Quality")
	static FString GetQualityHexColor(int32 Quality);

	UFUNCTION(BlueprintPure, Category = "ItemSystem|Quality")
	static FString GetQualityDisplayName(int32 Quality);

	/** 生成带魔法前后缀的显示名（例如“巨鲸之皮盔”） */
	UFUNCTION(BlueprintPure, Category = "ItemSystem|Display")
	FString GetItemDisplayName(const FItemInstance& Item) const;

	/** 格式化单条词缀文本（例如“+18 生命”或“+12% 攻击强化”） */
	UFUNCTION(BlueprintPure, Category = "ItemSystem|Display")
	FString FormatAffixText(int32 AffixID, int32 Value) const;

	/** 格式化完整悬停详情文本（品质、名称、等级、基础属性、随机词缀） */
	UFUNCTION(BlueprintPure, Category = "ItemSystem|Display")
	FString FormatItemTooltip(const FItemInstance& Item) const;

	const FString& GetContentDir() const { return ContentDir; }

private:
	void LoadCsv(const FString& FilePath);
	void LoadAffixesJson(const FString& FilePath);
	void RollAffixesForInstance(const FItemBaseRow& Row, FItemInstance& Item, FRandomStream& Rng) const;
	int32 RollSingleAffixValue(const FRandomAffixRow& Affix, int32 ItemLevel, int32 Quality, int32 ScalePercent, FRandomStream& Rng) const;
	int32 PickRandomEnabledAffixID(int32 EquipType, const TMap<int32, int32>& Existing, FRandomStream& Rng) const;

	TMap<int32, FItemBaseRow> Rows;
	TArray<int32> OrderedIDs;
	TMap<int32, FRandomAffixRow> AffixRows;
	TMap<int32, TArray<int32>> EnabledAffixesByEquipType;
	int32 AttackPercentAffixMaxLevel = 42;
	FString ContentDir;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UTexture2D>> IconCache;
};
