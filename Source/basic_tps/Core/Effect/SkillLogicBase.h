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
	UFUNCTION(BlueprintImplementableEvent)
	void ExecuteOnStart(const  FEffectContext& EffectContext);
	

	UFUNCTION(BlueprintImplementableEvent)
 
	void ExecuteOnDealSigleTarget();

	//以下函数 不能在 start里调用 这时候还没 CombatResult
	UFUNCTION(BlueprintCallable)
	void SetBaseSkillBaseHarm(int32 attackPoint);
	
	UFUNCTION(BlueprintCallable)
	void AddBuffImmediate(FBuffVo BuffVo);

	UFUNCTION(BlueprintCallable)
	void AddBuffDelay(FBuffVo BuffVo);
	UFUNCTION(BlueprintCallable)
	void AddBuffForTemp(FBuffVo BuffVo);
};