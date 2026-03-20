#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
 

#include "FVfxSpawnConfig.h"
 
#include "MagicEffect.generated.h"
 

UCLASS()
class BASIC_TPS_API AMagicEffect : public AActor
{
	GENERATED_BODY()
    
public:	
	AMagicEffect();

	// 核心初始化接口（值传递）
 
	virtual void InitializeEffect(FEffectContext InContext,USceneComponent* InAttachComp,FTransform & SpawnTransform);
	UFUNCTION(BlueprintCallable, Category = "MagicEffect")
	static AMagicEffect* SpawnMagicEffect(
		const UObject* WorldContextObject, const TSubclassOf<AMagicEffect> ClassToSpawn,
		const FEffectContext& InContext,const FVector& location=FVector(0.f, 0.f, 0.f),const FQuat& rotation=FQuat());

	UFUNCTION(BlueprintCallable, Category = "MagicEffect")
	AMagicEffect* SpawnNextMagicEffect();

	// 在 Actor 类内部
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties))
	FSkillVfxConfig EffectConfig;
	
	
protected:
  
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MagicEffect")
	UPrimitiveComponent* MainCollision;
	// 特效挂点（在蓝图子类里可以调整位置）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	USceneComponent* EffectAnchor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	UAudioComponent* AudioComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MagicEffect")
	FEffectContext MyContext;
	 
	UFUNCTION()
	void OnFlySphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION()
	void OnEffectOverlap(UPrimitiveComponent* OverlappedComponent, 
						 AActor* OtherActor, 
						 UPrimitiveComponent* OtherComp, 
						 int32 OtherBodyIndex, 
						 bool bFromSweep, 
						 const FHitResult& SweepResult);
private:
 
};