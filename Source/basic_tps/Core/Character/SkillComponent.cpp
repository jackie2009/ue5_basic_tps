// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"

#include "CombatCharacter.h"
#include "CombatComponent.h"
#include "HeroCharacter.h"
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
	FirstSkillMagicEffect=nullptr;
	// ...
}


// Called when the game starts
void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool USkillComponent::UseSkill(int32 SkillID,int32 CurrentWeaponType, int32 SkillLevel)
{
	auto attacker=Cast<ACombatCharacter>(GetOwner());
	
	//动作限制状态检测
	if (attacker==nullptr)return false;
	
	
	// cd mp check
	TArray<FSkillBaseVo*>* skillBaseVoPtr = UTableDataManagerSubsystem::Get()->SkillBaseMap.Find(SkillID);
	if (skillBaseVoPtr==nullptr)return false;
	auto heroAttacker=Cast<AHeroCharacter>(GetOwner());
	
	const int32 SkillLevelIndex = SkillLevel - 1;

	if (!skillBaseVoPtr->IsValidIndex(SkillLevelIndex)) return false;
	auto skillVo=(*skillBaseVoPtr)[SkillLevelIndex];
	//check cd
	if (!IsSkillReady(SkillID))
	{
		return false;
	}
	//CD 0为 不锁动作
	if (skillVo->CD>0&&attacker->CharacterDataComp->ActionTags.HasTag(FCombatTags::State_LockSkill))return false;

	
	//check mp
	if (heroAttacker)
	{
		if (!heroAttacker->CharacterDataComp->CostCurrentMP(skillVo->Spell)) return false;
	}
	// 1. 获取引用（如果不存在则原地创建一个默认的）
	FSkillChargeState& State = SkillChargeStateMap.FindOrAdd(SkillID);

	// 2. 如果是第一次创建（或者需要强制同步配置），先更新配置
	// 注意：如果你的配置是在别的初始化地方做的，这里可以省略赋值
	State.MaxCharges =FMath::Max(1, skillVo->MaxCharges); 
	State.RechargeDuration = skillVo->CD * 0.001f;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 3. 检查是否是首次初始化（如果 LastRechargeStartTime 是 0，说明从未充能过）
	// 我们需要给它一个初始时间。如果希望第一次直接能用，就设为“全满”状态
	if (State.LastRechargeStartTime <= 0.0f)
	{
		State.LastRechargeStartTime = CurrentTime - ( State.RechargeDuration);
	}
	float MaxAccumulatedTime = State.MaxCharges * State.RechargeDuration;
	float TimeSinceStart = CurrentTime - State.LastRechargeStartTime;
	if (TimeSinceStart >= MaxAccumulatedTime)
	{
		// 如果已经攒满了（甚至溢出了），强制把起点设为“刚好全满”的那个时刻
		// 这样扣除一个 CD 后，它会立刻变成“剩 2 发，且第 3 发刚开始转”的状态
		State.LastRechargeStartTime = CurrentTime - MaxAccumulatedTime;
	}

 

	// 5. 消耗点数：时间轴向后推一格
	State.LastRechargeStartTime += State.RechargeDuration;
 
	



	USkillLogicBase *SkillLogicData=nullptr;
	//float Duration = AnimInst->Montage_Play(SkillData->SkillMontage);
	if (!skillVo->skillVisualDataID.IsEmpty())
	{
		auto DataID=skillVo->skillVisualDataID;
		 if (heroAttacker!=nullptr&& DataID.Equals("weapon"))
		 {
		 	auto weaponVisualDataPtr= heroAttacker->WeaponVisualCollection.Find(CurrentWeaponType);
		 	if (weaponVisualDataPtr)
		 	{
		 		SkillLogicData=*weaponVisualDataPtr;
		 		
		 	}
		 }
		if (SkillLogicData==nullptr)
		{
			//如果不是武器决定的效果 读取技能配置 比如 各自射击由枪械配置决定 不是有技能配置决定
			// 这里的路径必须是绝对路径
			FString FullPackagePath = "/Game/TableDataExtra/Skills" / DataID;
			FString AssetName = FPaths::GetBaseFilename(FullPackagePath);
			FString AssetPath = FString::Printf(TEXT("%s.%s_C"), *FullPackagePath, *AssetName);
			UClass* SkillClass =  LoadObject<UClass>(nullptr, *AssetPath);
		 
			// 3. 实例化并配置表现数据
			if (SkillClass)
			{
				SkillLogicData = NewObject<USkillLogicBase>(GetOwner(), SkillClass);
				
				// 此时 NewSkill 里的属性（如 MontageToPlay, EffectClass）已经是你在蓝图里预设好的了
			}
		}
		SkillLogicData->Init(Cast<ACombatCharacter>( GetOwner()),this);
		if (!IsValid(SkillLogicData))
		{
			UE_LOG(LogTemp, Error, TEXT("SkillVisualDataID [%s] 加载失败或无效！"), *skillVo->skillVisualDataID);
		    return false;
		}
		
		 
		FirstSkillVfxContext.Instigator=attacker;
		FirstSkillVfxContext.TargetActor=nullptr;
		FirstSkillVfxContext.SkillBaseVo=skillVo;
	 
		FirstSkillVfxContext.SkillLogic= nullptr;
		FirstSkillMagicEffectClass=SkillLogicData->MagicEffectClass;
	  FirstSkillVfxContext.SkillLogic=  SkillLogicData;
		if (SkillLogicData->SkillMontage)
		{
			UAnimInstance* AnimInst = attacker->GetMesh()->GetAnimInstance();
			AnimInst->Montage_Play(SkillLogicData->SkillMontage,1,EMontagePlayReturnType::MontageLength,0,false);
		}else
		{
			 
			 SpawnFirstMagicEffect();
			 SpawnFlyMagicEffect();
		}
		 
	   return true;
	
	 
	}
	return false;
 
	
}

void USkillComponent::SpawnFirstMagicEffect()
{
	//GEngine->AddOnScreenDebugMessage(-1,10,FColor::Yellow,FString::Printf( TEXT("SpawnFirstMagicEffect")));
	FirstSkillMagicEffect=nullptr;
	if (FirstSkillVfxContext.SkillBaseVo==nullptr)return;
	 if (FirstSkillVfxContext.SkillLogic!=nullptr)
	 {
	 	if (FirstSkillVfxContext.SkillLogic->Owner==nullptr)
	 	{
	 		FirstSkillVfxContext.SkillLogic=nullptr;
	 		FirstSkillVfxContext.SkillBaseVo=nullptr;
	 		return;	
	 	}
	 	FirstSkillVfxContext.SkillLogic->ExecuteOnStart(FirstSkillVfxContext);
	 }
	FirstSkillMagicEffect=AMagicEffect::SpawnMagicEffect(this,FirstSkillMagicEffectClass,FirstSkillVfxContext);
	FirstSkillVfxContext.SkillBaseVo=nullptr;
	FirstSkillVfxContext.SkillLogic=nullptr;
}

void USkillComponent::SpawnFlyMagicEffect()
{
	if (FirstSkillMagicEffect==nullptr)return;
	FirstSkillMagicEffect->SpawnNextMagicEffect();
	FirstSkillMagicEffect=nullptr;
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