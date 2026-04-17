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
	if (attacker->CharacterDataComp->ActionTags.HasTag(FCombatTags::State_LockSkill))return false;
	
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


	
	//check mp
	if (heroAttacker)
	{
		if (!heroAttacker->CharacterDataComp->CostCurrentMP(skillVo->Spell)) return false;
	}
	// ⭐ 这里你应该从 SkillBaseVo 里拿CD
	float Cooldown =skillVo->CD*0.001f;  

	float CurrentTime = GetWorld()->GetTimeSeconds();
	// ⭐ 写入CD
	SkillNextAvailableTimeMap.Add(SkillID, CurrentTime + Cooldown);
	



	USkillVisualDataAsset *visualData=nullptr;
	//float Duration = AnimInst->Montage_Play(SkillData->SkillMontage);
	if (!skillVo->skillVisualDataID.IsEmpty())
	{
		auto DataID=skillVo->skillVisualDataID;
		 if (heroAttacker!=nullptr&& DataID.Equals("weapon"))
		 {
		 	auto weaponVisualDataPtr= heroAttacker->WeaponVisualCollection.Find(CurrentWeaponType);
		 	if (weaponVisualDataPtr)
		 	{
		 		visualData=*weaponVisualDataPtr;
		 		
		 	}
		 }
		if (visualData==nullptr)
		{
			//如果不是武器决定的效果 读取技能配置 比如 各自射击由枪械配置决定 不是有技能配置决定
			// 这里的路径必须是绝对路径
			FString FullPackagePath = "/Game/TableDataExtra/Skills" / DataID;
			FString AssetName = FPaths::GetBaseFilename(FullPackagePath);
			FString AssetPath = FString::Printf(TEXT("%s.%s"), *FullPackagePath, *AssetName);
			visualData=  LoadObject<USkillVisualDataAsset>(nullptr, *AssetPath);(skillVo->skillVisualDataID);
		}
		if (!IsValid(visualData))
		{
			UE_LOG(LogTemp, Error, TEXT("SkillVisualDataID [%s] 加载失败或无效！"), *skillVo->skillVisualDataID);
		    return false;
		}
		UAnimInstance* AnimInst = attacker->GetMesh()->GetAnimInstance();
		AnimInst->Montage_Play(visualData->SkillMontage);
		 
		 
		FirstSkillVfxContext.Instigator=attacker;
		FirstSkillVfxContext.TargetActor=nullptr;
		FirstSkillVfxContext.SkillBaseVo=skillVo;
		FirstSkillVfxContext.SkillVisualDataAsset= visualData;
		FirstSkillVfxContext.SkillLogic= nullptr;
		FirstSkillMagicEffectClass=visualData->MagicEffectClass;
		if (visualData->SkillLogicBase)
		{
			FirstSkillVfxContext.SkillLogic=  NewObject<USkillLogicBase>(this, visualData->SkillLogicBase);
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
	 if (FirstSkillVfxContext.SkillLogic!=nullptr) FirstSkillVfxContext.SkillLogic->ExecuteOnStart(FirstSkillVfxContext);
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
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (const float* NextTime = SkillNextAvailableTimeMap.Find(SkillID))
	{
		return CurrentTime >= *NextTime;
	}

	// 没记录过 → 默认可用
	return true;
}
float USkillComponent::GetSkillRemainingCD(int32 SkillID) const
{
	if (!GetWorld()) return 0.f;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (const float* NextTime = SkillNextAvailableTimeMap.Find(SkillID))
	{
		return FMath::Max(0.f, *NextTime - CurrentTime);
	}

	return 0.f;
}