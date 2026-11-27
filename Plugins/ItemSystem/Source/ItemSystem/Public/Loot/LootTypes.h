#pragma once

#include "CoreMinimal.h"
#include "Data/ItemSystemTypes.h"
#include "LootTypes.generated.h"

/** 怪物掉落阶级（决定默认品质概率表与零配置兜底方案） */
UENUM(BlueprintType)
enum class EMonsterLootTier : uint8
{
	Normal = 0 UMETA(DisplayName = "普通怪"),
	Elite = 1 UMETA(DisplayName = "精英怪"),
	Champion = 2 UMETA(DisplayName = "领主/头目"),
	Boss = 3 UMETA(DisplayName = "Boss")
};

/** 掉落池单条类型 */
UENUM(BlueprintType)
enum class ELootPoolEntryType : uint8
{
	FixedItem = 0 UMETA(DisplayName = "固定道具"),
	DynamicEquip = 1 UMETA(DisplayName = "按等级动态装备"),
	Gold = 2 UMETA(DisplayName = "金币"),
	SubPool = 3 UMETA(DisplayName = "嵌套子池")
};

/** 品质权重配置（白 / 蓝 / 黄 / 暗金） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootQualityPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float NormalWeight = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float MagicWeight = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float RareWeight = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float UniqueWeight = 0.5f;
};

/** 暗黑2 风格 MF（寻宝率）边际递减参数：EffectiveMF = MF * K / (MF + K)，K <= 0 表示无递减 */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootMFConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float MagicK = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float RareK = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float UniqueK = 250.0f;

	static float ApplyDiminishingReturns(float RawMF, float K)
	{
		const float ClampedMF = FMath::Max(0.0f, RawMF);
		if (K <= 0.0f) return ClampedMF;
		return (ClampedMF * K) / (ClampedMF + K);
	}
};

/** 掉落池条目 */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootPoolEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	ELootPoolEntryType Type = ELootPoolEntryType::FixedItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float Weight = 100.0f;

	/** FixedItem：指定道具 BaseID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 ItemID = 0;

	/** FixedItem：-1 表示走品质表随机，0~4 表示强制品质 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 ForcedQuality = -1;

	/** DynamicEquip：允许的 EquipType 槽位列表（空=所有可随机掉落的装备） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<int32> Slots;

	/** DynamicEquip：相对怪物等级的筛选下界偏移（如 -8） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 LevelMinOffset = -8;

	/** DynamicEquip：相对怪物等级的筛选上界偏移（如 +3） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 LevelMaxOffset = 3;

	/** Gold：基于怪物基础金币的随机倍率区间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float GoldScaleMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float GoldScaleMax = 1.25f;

	/** SubPool：子池 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName SubPoolId = NAME_None;
};

/** 掉落池（轻量版 TreasureClass） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootPoolConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float NoDropWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FLootPoolEntry> Entries;
};

/** 必掉项（可用于 MonsterLootProfiles.json，也可直接在怪物蓝图/关卡实例上临时追加） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FGuaranteedDropConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 ItemID = 0;

	/** -1 = 走品质随机，0~4 = 强制指定品质 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 ForcedQuality = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** 是否仅首杀生效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bFirstKillOnly = false;
};

/** 随机抽取组 */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootRollGroupConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName PoolId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 RollCountMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 RollCountMax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bAllowDuplicate = true;
};

/** 单个怪物的完整掉落方案（MonsterLootProfiles.json） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FMonsterLootProfileConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	EMonsterLootTier Tier = EMonsterLootTier::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float GoldChance = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float GoldScaleMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float GoldScaleMax = 1.25f;

	/** 可选：覆写本怪物暗金权重倍率（默认 1.0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float UniqueWeightMultiplier = 1.0f;

	/** 可选：覆写本怪物稀有(黄)权重倍率（默认 1.0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float RareWeightMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FGuaranteedDropConfig> GuaranteedDrops;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FLootRollGroupConfig> RollGroups;
};

/** 掉落结算输入上下文（由怪物/宝箱在触发掉落时组装） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 MonsterLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	EMonsterLootTier MonsterTier = EMonsterLootTier::Normal;

	/** 怪物基础金币（来自 MonsterBaseVo.dropMoney） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 BaseGold = 10;

	/** 玩家寻宝率 MF（百分比，例如 100 表示 +100% MF） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	float MagicFind = 0.0f;

	/** 是否首次击杀 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bIsFirstKill = false;

	/** 额外必掉项（例如关卡里某只特定摆放怪物追加的钥匙/任务品） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FGuaranteedDropConfig> ExtraGuaranteedDrops;
};

/** 单件掉落结算产物（要么是金币，要么是已 Roll 好品质与随机词缀的 FItemInstance） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootDropResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	bool bIsGold = false;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 GoldAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	FItemInstance Item;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	bool bFromGuaranteed = false;
};

/** 模拟 N 次击杀的掉落统计报告 */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FLootSimulationReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	FName ProfileId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 TotalItems = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int64 TotalGold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	TArray<int32> QualityCounts;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	TMap<int32, int32> ItemCounts;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	FString FormattedSummary;
};
