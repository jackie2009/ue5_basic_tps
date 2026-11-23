#include "SkillLogicBase.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "BuffLogicBase.h"
#include "MagicEffect.h"
#include "basic_tps/Core/Character/BuffComponent.h"
#include "basic_tps/Core/Character/SkillComponent.h"

void USkillLogicBase::SetSkillBaseHarm(int32 attackPoint)
{
	CombatResult.SkillBaseHarm=attackPoint;
}

void USkillLogicBase::Init(ACombatCharacter* InOwner, USkillComponent* InSkillComp,FEffectContext InInitEffectContext)
{
	Owner=InOwner;
	SkillComp=InSkillComp;
	InitEffectContext=InInitEffectContext;
	MagicEffectEmitterInstance=nullptr;
	if (MagicEffectEmitterClass!=nullptr)
	{
		MagicEffectEmitterInstance= AMagicEffect::SpawnMagicEffect(Owner,MagicEffectEmitterClass,InitEffectContext);
	}
	
	UAnimMontage* Montage = SelectSkillMontage();
	if (!Montage)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Owner->GetMesh();
	if (UAnimInstance* AnimInst = Mesh ? Mesh->GetAnimInstance() : nullptr)
	{
		AnimInst->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
	}
}

UAnimMontage* USkillLogicBase::SelectSkillMontage_Implementation()
{
	return SkillMontage.Get();
}

 
void USkillLogicBase::PreDamageProcess_Ref(FCombatResult& InOutResult)
{
	// 这种赋值操作在 C++ 层面是内存拷贝（Memcpy）
	this->CombatResult = InOutResult;
	this->PreDamageProcess();
	InOutResult = this->CombatResult;
}

void USkillLogicBase::AdjustFinalDamage_Ref(FCombatResult& InOutResult)
{
	// 这种赋值操作在 C++ 层面是内存拷贝（Memcpy）
	this->CombatResult = InOutResult;
	this->AdjustFinalDamage();
	InOutResult = this->CombatResult;
}

void USkillLogicBase::PostDamageProcess_Ref(FCombatResult& InOutResult)
{
	// 这种赋值操作在 C++ 层面是内存拷贝（Memcpy）
	this->CombatResult = InOutResult;
	this->PostDamageProcess();
	InOutResult = this->CombatResult;
}


 