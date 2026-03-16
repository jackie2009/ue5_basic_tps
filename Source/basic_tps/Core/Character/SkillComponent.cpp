// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"

#include "CombatCharacter.h"
#include "CombatComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Effect/MagicEffect.h"
#include "basic_tps/Core/TableData/SkillBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "basic_tps/Core/Utils/CombatCalculator.h"


// Sets default values for this component's properties
USkillComponent::USkillComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	MagicEffect=nullptr;
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

	// cd mp check
	auto skillBaseVoPtr=UTableDataManagerSubsystem::Get(this)->SkillBaseMap.Find(SkillID);
	if (skillBaseVoPtr==nullptr)return;
	auto skillVo=(*skillBaseVoPtr)[SkillLevel];
	Cast<ACombatCharacter>(GetOwner())->CombatComp;
	auto attacker=Cast<ACombatCharacter>(GetOwner());
	if (!skillVo->VfxName.IsEmpty())
	{

		auto vfxSoftPtr=  GetVfxDataById(skillVo->VfxName);
		USkillVfxDataAsset* vfxAsset = vfxSoftPtr.LoadSynchronous();
		FEffectContext   vfxContext;
		vfxContext.Instigator=attacker;
		vfxContext.TargetActor=nullptr;
		vfxContext.SkillBaseVo=skillVo;
		vfxContext.VfxConfig=vfxAsset;
		MagicEffect=AMagicEffect::SpawnMagicEffect(this,vfxContext);
	 
	}
 
	
}
  