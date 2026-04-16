#include "SkillLogicBase.h"

#include "basic_tps/Core/Character/BuffComponent.h"

void USkillLogicBase::SetBaseSkillBaseHarm(int32 attackPoint)
{
	CombatResult.SkillBaseHarm=attackPoint;
}

void USkillLogicBase::AddBuffImmediate(FBuffVo BuffVo)
{
	BuffVo.InitBaseData();
	if (BuffVo.EffectRole)BuffVo.EffectRole->BuffComp->AddBuff(BuffVo);
}

void USkillLogicBase::AddBuffDelay(FBuffVo BuffVo)
{
	BuffVo.InitBaseData();
	CombatResult.OnDamageFinishBuffVo=BuffVo;
}

void USkillLogicBase::AddBuffForTemp(FBuffVo BuffVo)
{
	BuffVo.InitBaseData();
	CombatResult.WorkingBuffVo=BuffVo;
}
