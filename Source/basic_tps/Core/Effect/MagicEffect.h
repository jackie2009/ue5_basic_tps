#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
 

#include "basic_tps/Core/Data/FVfxSpawnConfig.h"
 
#include "MagicEffect.generated.h"
 

UCLASS()
class BASIC_TPS_API AMagicEffect : public AActor
{
	GENERATED_BODY()
    
public:	
	AMagicEffect();

	// 核心初始化接口（值传递）
 
	virtual void InitializeEffect(FEffectContext InContext,USceneComponent* InAttachComp);
 
	static AMagicEffect* SpawnMagicEffect(
		const UObject* WorldContextObject, 
		const FEffectContext& InContext,const FVector& location=FVector::ZeroVector,const FQuat& rotation=FQuat::Identity);

	UFUNCTION(BlueprintCallable, Category = "MagicEffect")
	AMagicEffect* SpawnNextMagicEffect();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	class USphereComponent* CollisionSphere;
	// 特效挂点（在蓝图子类里可以调整位置）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	USceneComponent* EffectAnchor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	UAudioComponent* AudioComp;
	 
	FEffectContext MyContext;
	UFUNCTION()
	void OnFlySphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	 
private:
	// 内部创建特效组件的逻辑
	void Internal_SetupVisuals();
};