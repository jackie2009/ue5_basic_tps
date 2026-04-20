// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "basic_tps/Core/Effect/FVfxSpawnConfig.h"

#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

class AMagicEffect;
class USkillVfxDataAsset;
struct FCombatResult;
class ACombatCharacter;
USTRUCT(BlueprintType)
struct FSkillChargeState {
	GENERATED_BODY() // 必须加这个，用来注入反射代码
	int32 MaxCharges ;
	float RechargeDuration;
    
	// 关键：记录上一次充能彻底清零或消耗时的起始参考时间
	float LastRechargeStartTime;
	// 默认构造函数
	FSkillChargeState() : MaxCharges(1), RechargeDuration(5.0f), LastRechargeStartTime(0.0f) {}

	// 自定义构造函数
	FSkillChargeState(int32 InMax, float InCD, float InTime) 
		: MaxCharges(InMax), RechargeDuration(InCD), LastRechargeStartTime(InTime) {}
};
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
	 UPROPERTY(BlueprintReadOnly,Category = "Combat")
	AMagicEffect*   FirstSkillMagicEffect;
	 
	FEffectContext   FirstSkillVfxContext;
	TSubclassOf<AMagicEffect>  FirstSkillMagicEffectClass;
	UFUNCTION(BlueprintCallable, Category = "Combat",meta=(ExpandBoolAsExecs="ReturnValue"))
    bool  UseSkill(   int32 SkillID,int32 CurrentWeaponType,int32 SkillLevel=1);
 
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void  SpawnFirstMagicEffect();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void  SpawnFlyMagicEffect();


	// 是否CD可用
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool IsSkillReady(int32 SkillID) const;
	UFUNCTION(BlueprintCallable, Category="Combat")
	float GetSkillCDDetails(int32 SkillID,int32& OutCharges) const;
private:
	 
	UPROPERTY()
	TMap<int32, FSkillChargeState> SkillChargeStateMap;
};


