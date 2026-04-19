#include "SkillLogicBase.h"

#include "BuffLogicBase.h"
#include "basic_tps/Core/Character/BuffComponent.h"

void USkillLogicBase::SetSkillBaseHarm(int32 attackPoint)
{
	CombatResult.SkillBaseHarm=attackPoint;
}

void USkillLogicBase::AddBuff(UBuffLogicBase* BuffLogic)
{
	if (BuffLogic==nullptr)return;
	 //如果计算伤害过程时加 需要放入对列延迟加 否则直接加
	 if (bIsInStartState)
	 {
		 if (BuffLogic->EffectRole)BuffLogic->EffectRole->BuffComp->AddBuff(BuffLogic);
	 }else
	 {
	 	CombatResult.OnDamageFinishBuffLogicArray.Add(BuffLogic);
	 }
}

 

 