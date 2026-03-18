#include "CombatGameplayTags.h"
 

namespace FCombatTags
{
	// 参数1：C++ 变量名
	// 参数2：Project Settings 里的字符串路径（必须严格对应）
	// 参数3：显示在编辑器里的注释
	UE_DEFINE_GAMEPLAY_TAG(State_LockSkill, "State.Busy.LockSkill");
	UE_DEFINE_GAMEPLAY_TAG(State_LockMove, "State.Busy.LockMove");
 
}