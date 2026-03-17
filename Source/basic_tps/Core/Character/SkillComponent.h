// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "basic_tps/Core/Data/FVfxSpawnConfig.h"

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
	 
	AMagicEffect*   FirstSkillMagicEffect;
	FEffectContext   FirstSkillVfxContext;
	UFUNCTION(BlueprintCallable, Category = "Combat")
    void  UseSkill(   int32 SkillID,int32 SkillLevel=1);
 
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void  SpawnFirstMagicEffect();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void  SpawnFlyMagicEffect();
private:
 	 
};


