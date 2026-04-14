#pragma once
#include "CoreMinimal.h"
#include "../Character/CombatCharacter.h"
#include "../TableData/BuffBaseVo.h"
#include "../TableData/SkillBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "CombatTypes.generated.h"

class ACombatCharacter;

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

	UPROPERTY(BlueprintReadOnly)
	int32 Damage = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 AttackPoint=0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 DefencePoint=0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMiss = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBlock = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDeathBlow = false; // 是否触发了斩杀


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACombatCharacter* Attacker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACombatCharacter* Victim = nullptr;

 
	const FSkillBaseVo* SkillVo = nullptr;
	//这次伤害计算过程中 技能附带的 持续生命为0 的 buff 比如 忽视防御等，
	TSharedPtr<FBuffVo> WorkingBuffVo = nullptr;

	//这次伤害计算结束 给目标挂上去的  buff 持续时间>0，
	TSharedPtr<FBuffVo> OnDamageFinishBuffVo = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalDamage = 0; // 经过buff修改后的伤害
	 
};

 



USTRUCT(BlueprintType)
struct FBuffVo
{
	GENERATED_BODY()

public:
	// 视觉表现：在 UE 中建议使用 TWeakObjectPtr 防止特效销毁后引用失效
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> View;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<ACombatCharacter> EffectRole;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<ACombatCharacter> FromRole;

	// 原始数据引用（假设这些是通过数据表获取的 const 指针）
	const FBuffBaseVo* BaseVo;
	const FSkillBaseVo* FromSkill;

	UPROPERTY(BlueprintReadWrite)
	int32 BaseID;

	UPROPERTY(BlueprintReadWrite)
	float DieTime;

	UPROPERTY(BlueprintReadWrite)
	float NextEffectTime;

	UPROPERTY(BlueprintReadWrite)
	float Duration; // 建议用 float，UE 时间轴全是 float

	UPROPERTY(BlueprintReadWrite)
	int32 Value;
	// 核心修复：定义“相等”即“BaseID相同”
	bool operator==(const FBuffVo& Other) const
	{
		return BaseID == Other.BaseID;
	}
	// 默认构造函数（UE 序列化需要）
	FBuffVo() 
		: View(nullptr), EffectRole(nullptr), FromRole(nullptr)
		, BaseVo(nullptr), FromSkill(nullptr), BaseID(0)
		, DieTime(0.f), NextEffectTime(0.f), Duration(0.f), Value(0)
	{}

	// 带参构造函数直接实现在这里
	FBuffVo(ACombatCharacter* InEffectRole, ACombatCharacter* InFromRole, int32 InBaseID, float InLastTime, int32 InValue, const FSkillBaseVo* InFromSkill = nullptr)
		: View(nullptr)
		, EffectRole(InEffectRole)
		, FromRole(InFromRole)
		, BaseVo(nullptr) // 注意：这里需要在外部或通过单例赋值
		, FromSkill(InFromSkill)
		, BaseID(InBaseID)
		, DieTime(0.f)
		, NextEffectTime(0.f)
		, Duration(InLastTime)
		, Value(InValue)
	{
		BaseVo=nullptr;
		auto buffBaseVoPtr=UTableDataManagerSubsystem::Get(InFromRole)->BuffBaseMap.Find(InBaseID);
		if (buffBaseVoPtr!=nullptr){ BaseVo=*buffBaseVoPtr;}
		// 这里的 BaseData 映射逻辑通常在 BuffComponent 添加时处理更安全
		// 因为头文件里很难直接引用全局数据单例而不产生循环包含
	}

	// 功能函数
	int32 UseAmount(int32 Amount)
	{
		Value -= Amount;
		return Value;
	}

	// 辅助逻辑
	bool IsExpired(float CurrentTime) const 
	{ 
		return CurrentTime > DieTime; 
	}
};