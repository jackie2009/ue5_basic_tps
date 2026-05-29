#pragma once
#include "CoreMinimal.h"
#include "BuffEffect_Base.h"
#include "GameFramework/Actor.h"
#include "basic_tps/Core/Data/CombatTypes.h"

#include "FVfxSpawnConfig.h"
#include "Algo/Replace.h"
#include "basic_tps/Core/Character/BuffComponent.h"
#include "basic_tps/Core/Character/CombatNetworkComponent.h"
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
	
	
	 
	UPROPERTY(EditAnywhere)
	int32  group;//分组，同组替换
 
    UPROPERTY(EditAnywhere)
	int32   tick;//每几秒作用一次
	 
	 
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABuffEffect_Base>  buffEffectRes;//buff 效果

	UFUNCTION(BlueprintCallable)
	void InitBaseData(ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue)
	{
		this->EffectRole = InEffectRole;
		this->FromRole = InFromRole;
	 
		this->Duration = InDuration;
		this->Value = InValue;
		 
		 
	}
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> View;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<ACombatCharacter> EffectRole;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<ACombatCharacter> FromRole;

	 
	const FSkillBaseVo* FromSkill;

 

	UPROPERTY(BlueprintReadOnly)
	float DieTime;

	UPROPERTY(BlueprintReadOnly)
	float NextEffectTime;

	UPROPERTY(BlueprintReadWrite)
	float Duration; // 建议用 float，UE 时间轴全是 float

	UPROPERTY(BlueprintReadWrite)
	int32 Value;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuffAddMode BuffAddMode= EBuffAddMode::Replace;

	UFUNCTION(BlueprintCallable)
	// 功能函数
	int32 UseAmount(int32 Amount)
	{
		Value -= Amount;
		Value=FMath::Max(0,Value);
		 
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
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="特殊同组合并buff处理"))
	void CustomMergeBuff(UBuffLogicBase* NewBuff);



	//buff系统的这3个函数用法 与技能系统形式不同 是因为这里找到了更好的模式 但还需要观察段时间
	UPROPERTY(EditAnywhere,meta=(ToolTip="是否修改伤害流水线计算"))
	bool  bModifyDamageProcess;//分组，同组替换
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable ,meta=(ToolTip="伤害公式计算前修正参数 比如 忽视目标防御"))
	FCombatResult PreDamageProcess(const FCombatResult& InResult);
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable  ,meta=(ToolTip="伤害公式计算后修正结果 伤害抵扣"))
	FCombatResult AdjustFinalDamage(const FCombatResult& InResult);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable ,meta=(ToolTip="扣除目标血量后做处理 比如吸血"))
	FCombatResult PostDamageProcess(const FCombatResult& InResult);
	
};

inline FCombatResult UBuffLogicBase::PreDamageProcess_Implementation(const FCombatResult& InResult)
{
	return InResult;
}
inline FCombatResult UBuffLogicBase::AdjustFinalDamage_Implementation(const FCombatResult& InResult)
{
	return InResult;
}

inline FCombatResult UBuffLogicBase::PostDamageProcess_Implementation(const FCombatResult& InResult)
{
	return InResult;
}
