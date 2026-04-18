#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "basic_tps/Core/Data/CombatTypes.h"

#include "FVfxSpawnConfig.h"
 
#include "SkillLogicBase.generated.h"

 
struct FEffectContext;
 
UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced)
class USkillLogicBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FCombatResult    CombatResult;
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="技能被创建出来时 这时还没碰撞到目标"))
	void ExecuteOnStart(const  FEffectContext& EffectContext);
	
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="开始处理每个目标对象"))
 
	void ExecuteOnDealSigleTarget();
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="伤害公式计算前修正参数 比如 忽视目标防御"))
	void PreDamageProcess();
	
	
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="伤害公式计算后修正结果 伤害抵扣"))
	void AdjustFinalDamage();
	
	UFUNCTION(BlueprintImplementableEvent ,meta=(ToolTip="扣除目标血量后做处理 比如吸血"))
	void PostDamageProcess();

	//以下函数 不能在 start里调用 这时候还没 CombatResult
	UFUNCTION(BlueprintCallable)
	void SetSkillBaseHarm(int32 attackPoint);
	
	UFUNCTION(BlueprintCallable)
	void AddBuffImmediate(FBuffVo BuffVo);

	UFUNCTION(BlueprintCallable)
	void AddBuffDelay(FBuffVo BuffVo);
	UFUNCTION(BlueprintCallable)
	void AddBuffForTemp(FBuffVo BuffVo);
};