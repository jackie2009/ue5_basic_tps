// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "basic_tps/Core/Presentation/comm/VirtualSpringComponent.h"
#include "HitReactionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UHitReactionComponent : public UVirtualSpringComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHitReactionComponent();

	UFUNCTION(BlueprintCallable)
	float GetDeviation01() const { return Deviation01; }

	UFUNCTION(BlueprintCallable)
	bool IsReturningToTarget() const { return bReturningToTarget; }

	UFUNCTION(BlueprintCallable)
	void ApplyHit(float powScale=1.0f);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// ====== 输出 ======
	float Deviation01 = 0.f;
	bool bReturningToTarget;

	// 最大距离（归一化用）
	UPROPERTY(EditAnywhere, Category="HitReaction")
	float MaxDistance = 100.f;
 

	UPROPERTY(EditAnywhere, Category="HitReaction",DisplayName="动作幅度极限的时间点")
	float MiddleTime = 0.5f;
	UPROPERTY(EditAnywhere, Category="HitReaction",DisplayName="动画总时长")
	float AnimTime = 1.0f;
	UPROPERTY(EditAnywhere, Category="HitReaction",DisplayName="受伤动画逻辑暂停时间")
	float HitStunTime = 0.3f;

	

	float TickTimeTotal=0;
	bool bCleared = false;
	bool bIsStunned=false;
	
	// 恢复移动（定时器回调）
 
	void RecoverMovement();
 // 定时器句柄（必须是成员变量）
	FTimerHandle RecoverHandle;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void ClearHit();
};
