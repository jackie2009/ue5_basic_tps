#include "SkillLogicBase.h"

#include "basic_tps/Core/Character/BuffComponent.h"

void USkillLogicBase::SetSkillBaseHarm(int32 attackPoint)
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
	CombatResult.OnDamageFinishBuffVoArray.Add(BuffVo);
}

void USkillLogicBase::AddBuffForTemp(FBuffVo BuffVo)
{
	BuffVo.InitBaseData();
	CombatResult.WorkingBuffVo=BuffVo;
}
