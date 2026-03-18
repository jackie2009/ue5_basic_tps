// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_CombatActionLock.generated.h"

/**
 * 
 */
UCLASS()
class BASIC_TPS_API UANS_CombatActionLock : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	// 在这里你可以暴露给策划：这个锁是哪种类型的？
	UPROPERTY(EditAnywhere, Category = "Settings")
	FGameplayTag LockTag; 

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};