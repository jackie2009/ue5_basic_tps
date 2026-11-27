#pragma once

#include "CoreMinimal.h"
#include "HeroStaminaTypes.generated.h"

/**
 * 英雄体力四段区域枚举（配合 AHeroCharacter::StaminaZoneCapacities 数组按索引使用）
 */
UENUM(BlueprintType)
enum class EStaminaZone : uint8
{
	/** 0 - 无力区：严重疲劳，不能攻击/跳跃/闪避，仅可普通移动 */
	Exhausted = 0 UMETA(DisplayName = "无力区 (Exhausted)"),
	/** 1 - 疲劳区：可以作战但受影响，攻击收益约为正常七成 */
	Fatigued = 1 UMETA(DisplayName = "疲劳区 (Fatigued)"),
	/** 2 - 普通区：正常战斗状态，100% 正常收益，自然恢复上限 */
	Normal = 2 UMETA(DisplayName = "普通区 (Normal)"),
	/** 3 - 活力区：额外强化区（药水/增益激活），获得额外攻击收益 */
	Vigorous = 3 UMETA(DisplayName = "活力区 (Vigorous)"),

	Count = 4 UMETA(Hidden)
};

namespace HeroStaminaDefaults
{
	constexpr int32 ZoneCount = 4;
	constexpr int32 DefaultExhaustedCapacity = 20;
	constexpr int32 DefaultFatiguedCapacity = 30;
	constexpr int32 DefaultNormalCapacity = 50;
	constexpr int32 DefaultVigorousCapacity = 20;
}
