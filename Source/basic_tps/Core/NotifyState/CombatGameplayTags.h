#pragma once
#include "NativeGameplayTags.h"

/**
 * 战斗内核专属标签：在这里定义后，C++ 可以直接用变量名，
 * 且会自动出现在 Project Settings 和所有蓝图下拉菜单中。
 */
namespace FCombatTags
{
	// 定义“技能锁定”标签
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LockSkill);
	// 定义“移动锁定”标签
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LockMove);
 
}