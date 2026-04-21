#include "SkillLogicBase.h"

#include "BuffLogicBase.h"
#include "basic_tps/Core/Character/BuffComponent.h"

void USkillLogicBase::SetSkillBaseHarm(int32 attackPoint)
{
	CombatResult.SkillBaseHarm=attackPoint;
}

void USkillLogicBase::Init(ACombatCharacter* InOwner, USkillComponent* InSkillComp)
{
	Owner=InOwner;
	SkillComp=InSkillComp;
}

 
 

 