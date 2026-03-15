#pragma once
#include "CoreMinimal.h"
 
#include "CombatTypes.generated.h"

namespace AttributeEnum
{
	constexpr int8 Luck = 4;
	constexpr int8 HP = 5;
	constexpr int8 MP = 6;
	constexpr int8 Block = 7;
	constexpr int8 Critical = 8;
	constexpr int8 Damage1 = 9;
	constexpr int8 Damage2 = 10;
	constexpr int8 Defence1 = 12;
	constexpr int8 Defence2 = 13;
	constexpr int8 AttackRating = 15;
	constexpr int8 Dodge = 16;
	constexpr int8 AttackSpeed = 17;
	constexpr int8 MAX = 32;
}

namespace SkillEffectTypeEnum {

	constexpr int8  None = 0;

	constexpr int8  DC = 1;

	constexpr int8 MC = 2;

	constexpr int8 SC = 3;

	constexpr int8 Buff = -1;

	constexpr int8 DeBuff = -2;
	
}

 namespace BuffAttributeEnum {
	constexpr int8 INDEX_OFFSET = 100;
	constexpr int8 OnPoison = 1;
	constexpr int8 AddHP = 2;
	constexpr int8 AddPet = 3;
	constexpr int8 ExtraHarm = 4;
	constexpr int8 Dizzy = 5;
	constexpr int8 Hidden = 6;
	constexpr int8 ForceDizzy = 7;
	constexpr int8 HarmReducePercent = 8;
	constexpr int8 IgnoreArmor = 9;
	constexpr int8 ExtraHarmPercent = 10;
	constexpr int8 FinalAttackHarmPercent = 11;
	constexpr int8 DeathBlow = 12;
	constexpr int8 AddHpFromAttack = 13;
	constexpr int8 AddMpFromAttack = 14;
	constexpr int8 RemoveSkillCD = 15;
	constexpr int8 LessMPCost = 16;
	constexpr int8 EscapeToInitPos = 17;

}

namespace SkillGroupTargetsEnum {
	constexpr int8 Selected = 0;
	constexpr int8 Teams = 1;

}

 
 
UENUM(BlueprintType)
enum class ECombatFaction : uint8
{
	Neutral = 0,
	Hero = 1,
	Monster = 2,
	Pet = 3
};

UENUM(BlueprintType)
enum class ESkillTargetType : uint8
{
	Self = 0,
	Friendly = 1,
	Enemy = 2
};

USTRUCT(BlueprintType)
struct FCombatResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Damage = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMiss = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBlock = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDeathBlow = false; // 是否触发了斩杀

 
 
};