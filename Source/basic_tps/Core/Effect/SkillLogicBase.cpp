#include "SkillLogicBase.h"

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
	
	if (SkillMontage)
	{
		UAnimInstance* AnimInst = Owner->GetMesh()->GetAnimInstance();
		AnimInst->Montage_Play(SkillMontage,1,EMontagePlayReturnType::MontageLength,0,false);
	} 
		 
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


 