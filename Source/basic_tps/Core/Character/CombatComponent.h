// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

struct FEffectContext;
struct FCombatResult;
class ACombatCharacter;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

 
public:
	TMap<TObjectPtr<ACombatCharacter>, int32> AggroTargets;
	bool IsAttacking() const;
	bool IsHurting() const;
	UFUNCTION(BlueprintCallable, Category = "Combat")
    void  TryHurtTarget(  ACombatCharacter* Target, const FEffectContext &  EffectContext);
	// 处理受击逻辑 (整合之前讨论的 FCombatResult)
	void HandleHurt(const FCombatResult& Result);

	// 仇恨管理
	void AddAggro(ACombatCharacter* Target, int32 Amount);
	USceneComponent* GetEffectAttachSource(FName SocketName);

private:
	void Die();
};
