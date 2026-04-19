#pragma once
#include "CoreMinimal.h"
#include "../Character/CombatCharacter.h"
#include "../TableData/BuffBaseVo.h"
#include "../TableData/SkillBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "CombatTypes.generated.h"

class UBuffLogicBase;
class ACombatCharacter;

 
namespace  AttributeEnum
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
//
//  namespace BuffAttributeEnum {
// 	constexpr int8 INDEX_OFFSET = 100;
// 	constexpr int8 OnPoison = 1;
// 	constexpr int8 AddHP = 2;
// 	constexpr int8 AddPet = 3;
// 	constexpr int8 ExtraHarm = 4;
// 	constexpr int8 Dizzy = 5;
// 	constexpr int8 Hidden = 6;
// 	constexpr int8 ForceDizzy = 7;
// 	constexpr int8 HarmReducePercent = 8;
// 	constexpr int8 IgnoreArmor = 9;
// 	constexpr int8 ExtraHarmPercent = 10;
// 	constexpr int8 FinalAttackHarmPercent = 11;
// 	constexpr int8 DeathBlow = 12;
// 	constexpr int8 AddHpFromAttack = 13;
// 	constexpr int8 AddMpFromAttack = 14;
// 	constexpr int8 RemoveSkillCD = 15;
// 	constexpr int8 LessMPCost = 16;
// 	constexpr int8 EscapeToInitPos = 17;
//
// }

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

	UPROPERTY(BlueprintReadWrite)
	int32 Damage = 0;
	
	UPROPERTY(BlueprintReadWrite)
	int32 AttackPoint=0;
	
	UPROPERTY(BlueprintReadWrite)
	int32 DefencePoint=0;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMiss = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsBlock = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsDeathBlow = false; // 是否触发了斩杀


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACombatCharacter* Attacker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACombatCharacter* Victim = nullptr;

 
	const FSkillBaseVo* SkillVo = nullptr;
	 
	 
	 
	int SkillBaseHarm;

	 
	//这次伤害计算结束 给目标挂上去的  buff 持续时间>0，
	 
	 
 
	TArray<TObjectPtr<UBuffLogicBase>> OnDamageFinishBuffLogicArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalDamage = 0; // 经过buff修改后的伤害
	 
};
 

