// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"

#include "CombatCharacter.h"
#include "CombatComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Effect/MagicEffect.h"
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

void USkillComponent::UseSkill(int32 SkillID, int32 SkillLevel)
{
	auto attacker=Cast<ACombatCharacter>(GetOwner());
	
	//动作限制状态检测
	if (attacker==nullptr)return;
	if (attacker->CharacterDataComp->ActionTags.HasTag(FCombatTags::State_LockSkill))return;
	
	// cd mp check
	auto skillBaseVoPtr=UTableDataManagerSubsystem::Get(this)->SkillBaseMap.Find(SkillID);
	if (skillBaseVoPtr==nullptr)return;
	auto skillVo=(*skillBaseVoPtr)[SkillLevel];
	 



	
	//float Duration = AnimInst->Montage_Play(SkillData->SkillMontage);
	if (!skillVo->skillVisualDataID.IsEmpty())
	{

		auto visualData=  GetSkillVisualDataById(skillVo->skillVisualDataID);
		if (!IsValid(visualData))
		{
			UE_LOG(LogTemp, Error, TEXT("SkillVisualDataID [%s] 加载失败或无效！"), *skillVo->skillVisualDataID);
		    return;
		}
		UAnimInstance* AnimInst = attacker->GetMesh()->GetAnimInstance();
		AnimInst->Montage_Play(visualData->SkillMontage);
		 
		 
		FirstSkillVfxContext.Instigator=attacker;
		FirstSkillVfxContext.TargetActor=nullptr;
		FirstSkillVfxContext.SkillBaseVo=skillVo;
		FirstSkillVfxContext.VfxConfig=visualData->VfxDataAsset;
	
	 
	}
 
	
}

void USkillComponent::SpawnFirstMagicEffect()
{
	FirstSkillMagicEffect=nullptr;
	if (FirstSkillVfxContext.SkillBaseVo==nullptr)return;
	 
	FirstSkillMagicEffect=AMagicEffect::SpawnMagicEffect(this,FirstSkillVfxContext);
	FirstSkillVfxContext.SkillBaseVo=nullptr;
}

void USkillComponent::SpawnFlyMagicEffect()
{
	if (FirstSkillMagicEffect==nullptr)return;
	FirstSkillMagicEffect->SpawnNextMagicEffect();
	FirstSkillMagicEffect=nullptr;
}
  