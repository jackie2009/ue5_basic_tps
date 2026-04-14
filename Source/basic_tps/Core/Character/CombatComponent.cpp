// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "BuffComponent.h"
#include "CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Effect/FVfxSpawnConfig.h"
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


void UCombatComponent::TryHurtTarget(ACombatCharacter* Target, const FEffectContext &  EffectContext)
{
	if (Target==nullptr) return;
	auto skillBaseVoPtr=EffectContext.SkillBaseVo;
	if (skillBaseVoPtr==nullptr)return;
	auto attacker=Cast<ACombatCharacter>(GetOwner());
	 
	auto rst=UCombatCalculator::DamagePipeline(attacker,Target,EffectContext);

	 if (rst.OnDamageFinishBuffVo!=nullptr&&rst.OnDamageFinishBuffVo->EffectRole!=nullptr)
	 {
		 rst.OnDamageFinishBuffVo->EffectRole->BuffComp->AddBuff(*rst.OnDamageFinishBuffVo);
	 }
	
	//rst.SkillVo->isBuffForSelf
	
	
}

void UCombatComponent::HandleHurt(const FCombatResult& Result)
{
	auto character=Cast<ACombatCharacter>(GetOwner());
	if (character->IsAlive()==false)return;
	if (Result.bIsMiss)
	{
		
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("--------------Miss------------------"));
		return;
		
	}
	if (Result.bIsBlock)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("--------------Block------------------"));
	}
	if (Result.bIsDeathBlow)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("--------------DeathBlow------------------"));
	}
	if (Result.bIsCritical)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("--------------Critical------------------"));
	}
	if (Result.FinalDamage>0)
	{
	   character->SelfOnHurt(Result.FinalDamage, FVector::Zero());
	}
	// 通知蓝图显示伤害数字、播受击动画
	
	

	//   增加仇恨
	if (Result.Attacker&&Result.FinalDamage>0)
	{
		AddAggro(Result.Attacker, FMath::Max(1, Result.FinalDamage));
	}
	// 1. 调用属性组件扣血
	if (auto* AttrComp = GetOwner()->FindComponentByClass<UCharacterDataComponent>())
	{
		AttrComp->CostCurrentHP(Result.FinalDamage);
		
	}
	if (character->IsAlive()==false)
	{
		character->SelfOnDead();
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

// 在你的技能执行类或角色类中
USceneComponent* UCombatComponent::GetEffectAttachSource(FName SocketName)
{
	auto character = Cast<ACombatCharacter>(GetOwner());
	// 1. 先在角色自身的 Mesh 上找
	if (character->GetMesh()->DoesSocketExist(SocketName))
	{
		return character->GetMesh();
	}


	if (character->CurrentWeaponMesh && character->CurrentWeaponMesh->DoesSocketExist(SocketName))
	{
		return character->CurrentWeaponMesh;
	}


	return character->GetMesh();
}
