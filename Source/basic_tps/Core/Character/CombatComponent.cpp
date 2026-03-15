// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "basic_tps/Core/Utils/CombatCalculator.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UCombatComponent::TryHurtTarget(ACombatCharacter* Target, int32 SkillID)
{
	if (Target==nullptr) return;
	auto skillBaseVoPtr=UTableDataManagerSubsystem::Get(this)->SkillBaseMap.Find(SkillID);
	if (skillBaseVoPtr==nullptr)return;
	auto attacker=Cast<ACombatCharacter>(GetOwner());
	auto rst=UCombatCalculator::CalHurtPoint(attacker,Target,*(*skillBaseVoPtr)[0]);
	Target->FindComponentByClass<UCombatComponent>()->HandleHurt(rst,attacker);
	
}

void UCombatComponent::HandleHurt(const FCombatResult& Result, ACombatCharacter* Attacker)
{
	if (Result.bIsMiss)
	{
		
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("--------------Miss------------------"));
		return;
		
	} 

	

	//   增加仇恨
	if (Attacker&&Result.Damage>0)
	{
		AddAggro(Attacker, FMath::Max(1, Result.Damage));
	}
	// 1. 调用属性组件扣血
	if (auto* AttrComp = GetOwner()->FindComponentByClass<UCharacterDataComponent>())
	{
		AttrComp->CostCurrentHP(Result.Damage);
	}
	// 3. 触发受击反馈 (声音、特效、动画)
	// 这里建议通过派发事件或调用接口处理，保持组件纯粹
}

void UCombatComponent::AddAggro(ACombatCharacter* Target, int32 Amount)
{
 
	if (!Cast<ACombatCharacter>(GetOwner())->IsAlive())return;
	if (!Target) return;
    
	int32& CurrentAggro = AggroTargets.FindOrAdd(Target);
	CurrentAggro += Amount;
}