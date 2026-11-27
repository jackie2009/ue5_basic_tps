// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "CombatCharacter.h"
#include "basic_tps/Core/Data/HeroStaminaTypes.h"

#include "HeroCharacter.generated.h"

class UAnimMontage;
class USkillVisualDataAsset;
class UHeroDataComponent;
class UTargetOutlineComponent;
class USkillLogicBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroStaminaZoneChanged, EStaminaZone, PreviousZone, EStaminaZone, NewZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroRageChanged, float, NewRage, int32, AvailableCharges);

struct FSkillBaseVo;

UCLASS()
class BASIC_TPS_API AHeroCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHeroCharacter();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Targeting")
	TObjectPtr<UTargetOutlineComponent> TargetOutlineComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHeroDataComponent> HeroDataComp = nullptr;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TMap<int32, TObjectPtr<USkillLogicBase>> WeaponVisualCollection;

	// ==================== 英雄专属：体力（能量）四段区域系统 ====================

	/**
	 * 体力四段区域容量配置（4位 int 数组，与 EStaminaZone 枚举一一对应）
	 * 索引 0 = 无力区(20), 1 = 疲劳区(30), 2 = 普通区(50), 3 = 活力区(20)
	 * 总上限 = 四段之和；自然恢复上限 = 无力 + 疲劳 + 普通
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0"))
	TArray<int32> StaminaZoneCapacities;

	/** 疲劳区攻击伤害倍率（约七成收益） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float FatiguedAttackDamageMultiplier = 0.7f;

	/** 普通区攻击伤害倍率（100% 正常收益） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float NormalAttackDamageMultiplier = 1.0f;

	/** 活力区攻击伤害倍率（额外攻击力奖励，默认 +15%） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float VigorousAttackDamageMultiplier = 1.15f;

	/** 当进攻技能表格 Spell <= 0（如普攻）时使用的默认体力消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0"))
	int32 DefaultAttackStaminaCost = 10;

	/** 跳跃消耗的体力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0"))
	int32 JumpStaminaCost = 15;

	/** 闪避消耗的体力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0"))
	int32 DodgeStaminaCost = 20;

	/** 站立与普通移动时的自然体力恢复速率（点/秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenRate = 22.0f;

	/** 消耗体力后的恢复延迟（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenDelay = 0.8f;

	/** 可选显式配置的闪避蒙太奇（未配置时自动识别名称包含 Dodge 的蒙太奇） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	TArray<TSoftObjectPtr<UAnimMontage>> DodgeMontages;

	/** 体力区域变化委托 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Stamina")
	FOnHeroStaminaZoneChanged OnStaminaZoneChanged;

	// ==================== 英雄专属：怒气系统（敌人无怒气） ====================

	/** 最大怒气点数（对应 HUD 火焰上限，默认 5 点） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxRageCharges = 5;

	/** 当前精确怒气值（整数部分为已激活可用点数，小数部分为下一格积累进度，如 2.4 = 2 点可用 + 第 3 格积累 40%） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage", meta = (ClampMin = "0.0"))
	float CurrentRage = 0.0f;

	/** 每次普通攻击积累的怒气值（默认 0.2，5 次普攻激活 1 点） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage", meta = (ClampMin = "0.0"))
	float NormalAttackRageGain = 0.2f;

	/** 是否仅在普攻有效命中敌人时积累怒气（默认 true；关闭则普攻出手即积累） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage")
	bool bGainRageOnlyOnHit = true;

	/** 普通攻击技能 ID（对应 IA_Fire，默认 100003） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage")
	int32 NormalAttackSkillID = 100003;

	/** 现有技能默认消耗怒气点数（默认 1 点；普攻固定为 0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage", meta = (ClampMin = "0"))
	int32 DefaultSkillRageCost = 1;

	/** 可选按 SkillID 覆盖单个技能的怒气消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Rage")
	TMap<int32, int32> SkillRageCostOverrides;

	/** 怒气变化委托 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Rage")
	FOnHeroRageChanged OnRageChanged;

	/** 当前精确怒气值 */
	UFUNCTION(BlueprintPure, Category = "Combat|Rage")
	float GetCurrentRage() const;

	/** 当前已完整激活、可直接消耗的整数怒气点数 */
	UFUNCTION(BlueprintPure, Category = "Combat|Rage")
	int32 GetAvailableRageCharges() const;

	/** 下一格未激活怒气点的当前积累进度 [0, 1) */
	UFUNCTION(BlueprintPure, Category = "Combat|Rage")
	float GetChargingRageProgress() const;

	/** 检查是否拥有足够的已激活怒气点数 */
	UFUNCTION(BlueprintPure, Category = "Combat|Rage")
	bool HasEnoughRage(int32 Cost) const;

	/** 设置当前怒气值（自动钳制在 [0, MaxRageCharges]） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Rage")
	void SetCurrentRage(float NewRage);

	/** 增加怒气（例如普攻 +0.2） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Rage")
	void AddRage(float Amount);

	/** 消耗整数点怒气（保留未满一格的积累进度） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Rage")
	bool ConsumeRage(int32 Cost);

	/** 判断给定技能是否为普通攻击 */
	bool IsNormalAttackSkill(const FSkillBaseVo* SkillVo) const;

	/** 查询指定技能 ID 需要消耗的怒气点数（普攻为 0，其他技能默认 1） */
	UFUNCTION(BlueprintPure, Category = "Combat|Rage")
	int32 GetSkillRageCostByID(int32 SkillID) const;

	int32 GetSkillRageCost(int32 SkillID, const FSkillBaseVo* SkillVo) const;

	/** 普通攻击触发怒气积累（+NormalAttackRageGain） */
	void OnNormalAttackAccumulateRage();

	// ==================== 技能键位配置（供 HUD 动态读取技能名） ====================

	/** Q 键绑定的技能 ID（对应 IA_Skill1，默认 100001 盾牌冲锋） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skills")
	int32 SkillQ_ID = 100001;

	/** E 键绑定的技能 ID（对应 IA_Skill2，默认 200010 盾牌投掷） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Skills")
	int32 SkillE_ID = 200010;

	/** 从 SkillBaseVo 技能表动态查询技能显示名称 */
	UFUNCTION(BlueprintPure, Category = "Combat|Skills")
	FString GetSkillDisplayNameByID(int32 SkillID, int32 SkillLevel = 1) const;

	UFUNCTION(BlueprintPure, Category = "Combat|Skills")
	FString GetSkillQDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Combat|Skills")
	FString GetSkillEDisplayName() const;

	// ==================== 体力区域查询与控制 API ====================

	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	UHeroDataComponent* GetHeroDataComp() const { return HeroDataComp; }

	/** 确保数组固定为 4 位且非负，并同步最大能量（MP）到基础属性 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	void SyncStaminaZoneConfig();

	/** 按枚举读取对应区域的长度/容量 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	int32 GetStaminaZoneCapacity(EStaminaZone Zone) const;

	/** 按枚举修改单个区域的容量，并同步最大体力与 HUD 比例 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	void SetStaminaZoneCapacity(EStaminaZone Zone, int32 NewCapacity);

	/** 一次性设置全部 4 个区域的容量（自动规整为 4 位） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	void SetStaminaZoneCapacities(const TArray<int32>& NewCapacities);

	/** 获取规范化后的 4 位区域容量数组 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	TArray<int32> GetNormalizedStaminaZoneCapacities() const;

	/** 普通体力上限（无力 + 疲劳 + 普通），即自然恢复终点 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	int32 GetNormalStaminaCap() const;

	/** 总体力上限（无力 + 疲劳 + 普通 + 活力），对应 MaxMP */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	int32 GetTotalStaminaMax() const;

	/** 获取某一区域的起始与结束体力阈值 [OutStart, OutEnd] */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	void GetStaminaZoneBounds(EStaminaZone Zone, int32& OutStart, int32& OutEnd) const;

	/** 获取给定体力值所属的区域 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	EStaminaZone GetStaminaZoneForValue(float StaminaValue) const;

	/** 获取英雄当前所处的体力区域 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	EStaminaZone GetCurrentStaminaZone() const;

	/** 获取当前平滑体力值（用于 HUD 平滑刷新） */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetCurrentStaminaFloat() const;

	/** 获取某区域当前填充进度 [0, 1]（供 HUD 四段分段进度条使用） */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetStaminaZoneFillPercent(EStaminaZone Zone) const;

	/** 获取某区域在整条体力条中的宽度占比 [0, 1] */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetStaminaZoneWidthRatio(EStaminaZone Zone) const;

	/** 当前体力状态是否允许攻击 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool CanHeroAttack() const;

	/** 当前体力状态是否允许跳跃 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool CanHeroJump() const;

	/** 当前体力状态是否允许闪避 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool CanHeroDodge() const;

	/** 获取指定体力区域对应的攻击伤害倍率 */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetAttackDamageMultiplierForZone(EStaminaZone Zone) const;

	/** 获取本次攻击结算应使用的伤害倍率（优先使用出手瞬间的区域快照） */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetCurrentAttackDamageMultiplier() const;

	/** 消耗体力（允许在非无力区透支归零并落入无力区），同时重置自然恢复延迟 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	bool ConsumeStamina(int32 Cost, bool bAllowInExhausted = false);

	/** 尝试执行闪避体力判定并扣除体力（蓝图可直接调用） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	bool TryConsumeDodgeStamina();

	/** 额外补充体力（药水/增益用，可突破普通上限进入活力区，上限为 TotalStaminaMax） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	void AddBonusStamina(int32 Amount);

	/** 记录本次攻击出手时的体力区域，避免攻击消耗体力后命中瞬间误按更低区间结算 */
	void CaptureAttackStaminaZoneSnapshot();

	/** 当前是否处于阻止体力自然恢复的动作中（攻击、跳跃、闪避） */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool IsStaminaRegenBlocked() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void EnsureStaminaArrayNormalized();
	void UpdateStaminaRegen(float DeltaTime);
	void CheckAndBroadcastZoneChange();
	bool IsDodgeMontage(const UAnimMontage* Montage) const;

	UFUNCTION()
	void HandleMontageStarted(UAnimMontage* Montage);

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	float StaminaRegenAccumulator = 0.0f;
	double LastStaminaConsumeTime = -1000.0;
	bool bIsDodging = false;
	TWeakObjectPtr<UAnimMontage> ActiveDodgeMontage;

	EStaminaZone LastBroadcastZone = EStaminaZone::Normal;
	EStaminaZone LastAttackZoneSnapshot = EStaminaZone::Normal;
	double LastAttackSnapshotTime = -1000.0;

	int32 CachedZoneCapacities[HeroStaminaDefaults::ZoneCount] = {
		HeroStaminaDefaults::DefaultExhaustedCapacity,
		HeroStaminaDefaults::DefaultFatiguedCapacity,
		HeroStaminaDefaults::DefaultNormalCapacity,
		HeroStaminaDefaults::DefaultVigorousCapacity
	};
};
