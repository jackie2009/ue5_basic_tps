// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "CombatCharacter.h"

#include "MonsterCharacter.generated.h"
class UMonsterDataComponent;
class ULootDropComponent;
 

UCLASS()
class BASIC_TPS_API AMonsterCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterCharacter();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MonsterBaseID=0;

	/** 怪物掉落组件：打开怪物蓝图在右侧 Loot 分类下直接选 LootProfileId 或加必掉项即可 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<ULootDropComponent> LootDropComp = nullptr;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
 
	UPROPERTY( BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UMonsterDataComponent> MonsterDataComp = nullptr;
 
public:
	 
	// Called every frame
	virtual void Tick(float DeltaTime) override;

 
};
