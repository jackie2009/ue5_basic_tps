#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Loot/LootTypes.h"
#include "LootDropComponent.generated.h"

class ACombatCharacter;
class ALootPickupActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLootDropped, const TArray<FLootDropResult>&, Results);

/**
 * 怪物 / 宝箱通用掉落组件（自动挂在 AMonsterCharacter 上，也可添加到任意宝箱/可破坏物蓝图）。
 *
 * 3 级配置优先级：
 *   1. 在蓝图/关卡中直接下拉选择 LootProfileId（读取 MonsterLootProfiles.json）
 *   2. 留空时自动读取 MonsterBaseVo.dropID 映射（DropIdMap）
 *   3. 仍未匹配时按怪物等级与阶级自动兜底 Default_Normal / Elite / Champion / Boss
 */
UCLASS(ClassGroup = (Loot), meta = (BlueprintSpawnableComponent))
class BASIC_TPS_API ULootDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULootDropComponent();

	/** 是否开启掉落 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bEnableDrop = true;

	/**
	 * 怪物掉落方案 ID（留空则自动按 MonsterBaseVo.dropID 或默认阶级兜底）。
	 * 选项自动从 Plugins/ItemSystem/Content/Data/MonsterLootProfiles.json 读取。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (GetOptions = "GetAvailableLootProfileIds"))
	FName LootProfileId = NAME_None;

	/** 是否手动覆盖怪物阶级（关闭时自动根据 MonsterBaseVo.nameColor 识别普通/领主/Boss） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bOverrideTier = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (EditCondition = "bOverrideTier"))
	EMonsterLootTier TierOverride = EMonsterLootTier::Normal;

	/** 可选：非怪物 Actor（如宝箱）使用的等级；怪物默认读取 MonsterBaseVo.Lvl */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 FallbackMonsterLevel = 1;

	/** 可选：非怪物 Actor（如宝箱）使用的基础金币；怪物默认读取 MonsterBaseVo.dropMoney */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0"))
	int32 FallbackBaseGold = 50;

	/** 额外必掉项（适合给关卡里某只特定摆放的怪/宝箱直接塞钥匙或专属装备） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TArray<FGuaranteedDropConfig> ExtraGuaranteedDrops;

	/** 地面掉落物 Actor 类（默认 ALootPickupActor，可替换为自定义蓝图子类） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Visual")
	TSubclassOf<ALootPickupActor> PickupActorClass;

	/** 掉落物扇形散开最小半径（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Visual", meta = (ClampMin = "0.0"))
	float SpreadRadiusMin = 75.0f;

	/** 掉落物扇形散开最大半径（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Visual", meta = (ClampMin = "0.0"))
	float SpreadRadiusMax = 175.0f;

	/** 掉落物抛物线高度（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Visual", meta = (ClampMin = "10.0"))
	float LaunchArcHeight = 95.0f;

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnLootDropped OnLootDropped;

	/** 手动触发掉落（怪物死亡会自动调用；宝箱开启时可在蓝图调用） */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FLootDropResult> TriggerDrop(float MagicFind = 0.0f, bool bIsFirstKill = false);

	/** 工具函数：把一组掉落结果在指定中心点喷泉式生成到地面 */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SpawnDropActorsAt(const FVector& Origin, const TArray<FLootDropResult>& Results) const;

	/** 供 UE5 编辑器 Details 面板下拉框动态列出 MonsterLootProfiles.json 中的全部方案名 */
	UFUNCTION()
	static TArray<FString> GetAvailableLootProfileIds();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleOwnerDeath(ACombatCharacter* DeadCharacter);
	void BuildLootContext(float MagicFind, bool bIsFirstKill, FLootContext& OutContext, int32& OutDropId) const;
	FVector FindGroundPoint(const FVector& Origin, float AngleRad, float Radius) const;

	bool bHasDropped = false;
	FDelegateHandle DeathDelegateHandle;
};
