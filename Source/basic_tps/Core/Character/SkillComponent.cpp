// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"

#include "CombatCharacter.h"
#include "CombatComponent.h"
#include "HeroCharacter.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Effect/MagicEffect.h"
#include "basic_tps/Core/Effect/SkillLogicBase.h"
#include "basic_tps/Core/NotifyState/CombatGameplayTags.h"
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "basic_tps/Core/Utils/CombatCalculator.h"


// Sets default values for this component's properties
USkillComponent::USkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
 
	// ...
}


// Called when the game starts
void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool USkillComponent::UseSkill(int32 SkillID, int32 CurrentWeaponType, int32 SkillLevel, ACombatCharacter* TryToTarget)
{
    auto* Attacker = Cast<ACombatCharacter>(GetOwner());
    auto* Tables = UTableDataManagerSubsystem::Get();
    if (!IsValid(Attacker) || !IsValid(Attacker->CharacterDataComp) || !Tables || !GetWorld()) return false;

    const auto* SkillLevels = Tables->SkillBaseMap.Find(SkillID);
    const int32 SkillLevelIndex = SkillLevel - 1;
    if (!SkillLevels || !SkillLevels->IsValidIndex(SkillLevelIndex)) return false;
    auto* SkillVo = (*SkillLevels)[SkillLevelIndex];
    if (!SkillVo || !IsSkillReady(SkillID)) return false;
    if (SkillVo->CD > 0 && Attacker->CharacterDataComp->ActionTags.HasTag(FCombatTags::State_LockSkill)) return false;

    auto* HeroAttacker = Cast<AHeroCharacter>(Attacker);
    const bool bIsOffensiveSkill = SkillVo->EffectType > 0;
    int32 HeroStaminaCost = SkillVo->Spell;
    int32 HeroRageCost = 0;
    if (HeroAttacker)
    {
        // 无力区禁止发起攻击
        if (bIsOffensiveSkill && !HeroAttacker->CanHeroAttack())
        {
            return false;
        }
        // 怒气判定：现有技能默认消耗 1 点怒气，普攻为 0
        HeroRageCost = HeroAttacker->GetSkillRageCost(SkillID, SkillVo);
        if (HeroRageCost > 0 && !HeroAttacker->HasEnoughRage(HeroRageCost))
        {
            return false;
        }
        if (HeroStaminaCost <= 0 && bIsOffensiveSkill)
        {
            HeroStaminaCost = HeroAttacker->DefaultAttackStaminaCost;
        }
        if (!bIsOffensiveSkill && HeroStaminaCost > 0 && HeroAttacker->CharacterDataComp->GetCurrentMP() < HeroStaminaCost)
        {
            return false;
        }
    }

    // Resolve and validate the logic before committing mana or a charge.
    if (SkillVo->skillVisualDataID.IsEmpty()) return false;
    USkillLogicBase* SkillLogicData = nullptr;
    if (SkillVo->skillVisualDataID.Equals(TEXT("weapon")))
    {
        if (HeroAttacker)
        {
            if (const auto* WeaponLogic = HeroAttacker->WeaponVisualCollection.Find(CurrentWeaponType))
            {
                SkillLogicData = *WeaponLogic;
            }
        }
    }
    else
    {
        const FString FullPackagePath = TEXT("/Game/TableDataExtra/Skills") / SkillVo->skillVisualDataID;
        const FString AssetName = FPaths::GetBaseFilename(FullPackagePath);
        const FString AssetPath = FString::Printf(TEXT("%s.%s_C"), *FullPackagePath, *AssetName);
        UClass* SkillClass = LoadClass<USkillLogicBase>(nullptr, *AssetPath);
        if (SkillClass && !SkillClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
        {
            SkillLogicData = NewObject<USkillLogicBase>(Attacker, SkillClass);
        }
    }
    if (!IsValid(SkillLogicData)) return false;

    if (HeroAttacker)
    {
        if (HeroRageCost > 0 && !HeroAttacker->ConsumeRage(HeroRageCost))
        {
            return false;
        }
        if (bIsOffensiveSkill)
        {
            // 记录攻击出手瞬间的体力区域，供本次攻击命中时的伤害倍率结算
            HeroAttacker->CaptureAttackStaminaZoneSnapshot();
        }
        if (HeroStaminaCost > 0 && !HeroAttacker->ConsumeStamina(HeroStaminaCost, !bIsOffensiveSkill))
        {
            return false;
        }
        // 若配置为普攻出手即积累怒气，在此增加 0.2（默认仅在有效命中时于 PostDamageProcess 积累）
        if (!HeroAttacker->bGainRageOnlyOnHit && HeroAttacker->IsNormalAttackSkill(SkillVo))
        {
            HeroAttacker->OnNormalAttackAccumulateRage();
        }
    }

    FSkillChargeState& State = SkillChargeStateMap.FindOrAdd(SkillID);
    State.MaxCharges = FMath::Max(1, SkillVo->MaxCharges);
    State.RechargeDuration = SkillVo->CD * 0.001f;
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (State.LastRechargeStartTime <= 0.0f)
    {
        State.LastRechargeStartTime = CurrentTime - State.RechargeDuration;
    }
    const float MaxAccumulatedTime = State.MaxCharges * State.RechargeDuration;
    if (CurrentTime - State.LastRechargeStartTime >= MaxAccumulatedTime)
    {
        State.LastRechargeStartTime = CurrentTime - MaxAccumulatedTime;
    }
    State.LastRechargeStartTime += State.RechargeDuration;

    FEffectContext InitSkillVfxContext{};
    InitSkillVfxContext.Instigator = Attacker;
    InitSkillVfxContext.TargetActor = TryToTarget;
    // TargetType uses the DATA TABLE convention: 0 enemy, 1 self, 2 friendly.
    // Capture once. Explicit AI/Blueprint targets still take precedence.
    if (!TryToTarget && SkillVo->TargetType == 0 && Attacker->TargetingComp)
    {
        InitSkillVfxContext.TargetActor = Cast<ACombatCharacter>(Attacker->TargetingComp->GetActionTarget());
    }
    InitSkillVfxContext.SkillBaseVo = SkillVo;
    InitSkillVfxContext.SkillLogic = SkillLogicData;
    ActiveSkillLogic = SkillLogicData;
    ActiveSkillLogic->Init(Attacker, this, InitSkillVfxContext);
    ActiveSkillLogic->ExecuteOnStart();
    return true;
}
bool USkillComponent::IsSkillReady(int32 SkillID) const
{
	// 如果没有记录，默认技能是好的
	if (!SkillChargeStateMap.Contains(SkillID))
	{
		return true;
	}

	const FSkillChargeState& State = SkillChargeStateMap[SkillID];
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 逻辑：计算从起始点到现在一共攒了多少秒
	float TimePassed = CurrentTime - State.LastRechargeStartTime;

	// 只要攒的时间能够抵扣 1 次 CD，技能就是 Ready 的
	// (TimePassed / State.RechargeDuration) >= 1.0f
	return TimePassed >= State.RechargeDuration;

	 
}
// 核心函数定义
float USkillComponent::GetSkillCDDetails(int32 SkillID, int32& OutCharges) const
{
	OutCharges = 0; // 初始化默认值
	if (!GetWorld()) return 0.f;

	const FSkillChargeState* State = SkillChargeStateMap.Find(SkillID);
	if (!State)
	{
		OutCharges = 1; 
		return 0.f;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float TimePassed = CurrentTime - State->LastRechargeStartTime;

	// 计算可用次数并通过引用“传出”
	OutCharges = FMath::Clamp(FMath::FloorToInt(TimePassed / State->RechargeDuration), 0, State->MaxCharges);

	// 返回剩余时间
	if (OutCharges < State->MaxCharges)
	{
		float CurrentElapsed = FMath::Fmod(TimePassed, State->RechargeDuration);
		return State->RechargeDuration - CurrentElapsed;
	}

	return 0.f;
}
