// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatCharacter.generated.h"


class UBuffComponent;
class USkillComponent;
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
	virtual void ExecutePhysicsDisabled();

public:


		

	
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCharacterDataComponent> CharacterDataComp = nullptr;
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatComponent> CombatComp = nullptr;
	
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkillComponent> SkillComp = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UBuffComponent> BuffComp = nullptr;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsAlive() const;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	virtual void SelfOnHurt(float Damage, FVector HitLocation)  ;
	virtual void SelfOnDead()  ;
	
	
	
	// 定义一个给蓝图用的事件：受伤
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnHurt(float Damage, FVector HitLocation);

	// 定义一个给蓝图用的事件：死亡
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnDead();
};
