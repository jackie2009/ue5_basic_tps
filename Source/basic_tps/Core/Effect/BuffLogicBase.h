#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "basic_tps/Core/Data/CombatTypes.h"

#include "FVfxSpawnConfig.h"
 
#include "BuffLogicBase.generated.h"

 
struct FEffectContext;
 
UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced)
class UBuffLogicBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACombatCharacter> Owner;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UBuffComponent> OwnerBuff;

	UFUNCTION(BlueprintCallable)
	void InitBaseData(ACombatCharacter* EffectRole,ACombatCharacter* FromRole, int BaseID,float Duration,int Value)
	{
		this->EffectRole = EffectRole;
		this->FromRole = FromRole;
		this->BaseID = BaseID;
		this->Duration = Duration;
		this->Value = Value;
		 
		BaseVo=nullptr;
		auto buffBaseVoPtr=UTableDataManagerSubsystem::Get()->BuffBaseMap.Find(BaseID);
		if (buffBaseVoPtr!=nullptr){ BaseVo=*buffBaseVoPtr;}
	}
	UPROPERTY(BlueprintReadOnly)
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

	UPROPERTY(BlueprintReadOnly)
	float DieTime;

	UPROPERTY(BlueprintReadOnly)
	float NextEffectTime;

	UPROPERTY(BlueprintReadWrite)
	float Duration; // 建议用 float，UE 时间轴全是 float

	UPROPERTY(BlueprintReadWrite)
	int32 Value;
	// 核心修复：定义“相等”即“BaseID相同”
	bool operator==(const UBuffLogicBase& Other) const
	{
		return BaseID == Other.BaseID;
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
	
	//给蓝图子类扩展
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="每几秒执行一次效果"))
	void OnIntervalTick();
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="受到伤害时执行"))
	void OnTakeDamage();
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="对角色属性的修改"))
	void ApplyAttributesModify();
	
	
};