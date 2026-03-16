// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
 
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

class AMagicEffect;
class USkillVfxDataAsset;
struct FCombatResult;
class ACombatCharacter;
 
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USkillComponent();

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	 AMagicEffect*   MagicEffect;
	UFUNCTION(BlueprintCallable, Category = "Combat")
    void  UseSkill(   int32 SkillID,int32 SkillLevel=1);
 

private:
 	 
};


