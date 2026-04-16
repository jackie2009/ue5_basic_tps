#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
 

#include "FVfxSpawnConfig.h"
 
#include "SkillLogicBase.generated.h"

class UCombatResultWrapper;
struct FEffectContext;
struct FBuffVo;
UCLASS(Blueprintable, Abstract, EditInlineNew, DefaultToInstanced)
class USkillLogicBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void ExecuteOnStart(const  FEffectContext& EffectContext);
	
 

	// 核心逻辑入口：必须没有返回值，才能在 EventGraph 中作为事件节点使用
	UFUNCTION(BlueprintNativeEvent, Category = "Skill")
	void OnExecuteSkillLogic(UCombatResultWrapper *CombatResult);

	// 默认实现（可以为空）
	virtual void OnExecuteSkillLogic_Implementation(UCombatResultWrapper* CombatResult) {}
	 
};