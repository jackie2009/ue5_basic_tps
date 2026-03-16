#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
 

#include "basic_tps/Core/Data/FVfxSpawnConfig.h"
#include "MagicEffect.generated.h"
struct FEffectContext;

UCLASS()
class BASIC_TPS_API AMagicEffect : public AActor
{
	GENERATED_BODY()
    
public:	
	AMagicEffect();

	// 核心初始化接口（值传递）
	UFUNCTION(BlueprintCallable, Category = "MagicEffect")
	virtual void InitializeEffect(FEffectContext InContext,USceneComponent* InAttachComp);
	// 静态工厂函数：替代原 UCombatCalculator::SpawnVfxUniversal
	UFUNCTION(BlueprintCallable, Category = "MagicEffect")
	static AMagicEffect* SpawnMagicEffect(
		const UObject* WorldContextObject, 
		const FEffectContext& InContext);

	UFUNCTION(BlueprintCallable, Category = "MagicEffect")
	AMagicEffect* SpawnNextMagicEffect();
protected:
	// 特效挂点（在蓝图子类里可以调整位置）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* EffectAnchor;

	// 存储锦囊快照
	UPROPERTY(BlueprintReadOnly, Category = "MagicEffect")
	FEffectContext MyContext;

	
	 
private:
	// 内部创建特效组件的逻辑
	void Internal_SetupVisuals();
};