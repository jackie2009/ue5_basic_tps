// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
 
#include "CombatCameraUtils.generated.h"

struct FCombatResult;
class ACombatCharacter;
class USkillVfxDataAsset;
struct FSkillBaseVo;
/**
 * 
 */
UCLASS()
class BASIC_TPS_API UCombatCameraUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	 
	/** 1. 获取准星在世界中的第一碰撞点 */
	UFUNCTION(BlueprintCallable, Category = "SkillUtils", meta = (WorldContext = "WorldContextObject"))
	static bool GetCameraViewHitLocation(const UObject* WorldContextObject, float MaxRange, FVector& OutHitLocation);

	/** 2. 将一个空间点垂直投影到地面 */
	UFUNCTION(BlueprintCallable, Category = "SkillUtils", meta = (WorldContext = "WorldContextObject"))
	static bool ProjectLocationToGround(const UObject* WorldContextObject,float MaxRange,FVector& OutHitLocation);

 };