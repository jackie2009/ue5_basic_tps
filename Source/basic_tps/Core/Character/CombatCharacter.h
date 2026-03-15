// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatCharacter.generated.h"


class UCombatComponent;
class UCharacterDataComponent;

UCLASS()
class BASIC_TPS_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatCharacter();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCharacterDataComponent> CharacterDataComp = nullptr;
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatComponent> CombatComp = nullptr;
	

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
