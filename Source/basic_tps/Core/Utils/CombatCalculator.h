// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
 
#include "CombatCalculator.generated.h"

class ACombatCharacter;
struct FSkillBaseVo;
/**
 * 
 */
UCLASS()
class BASIC_TPS_API UCombatCalculator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** * 计算战斗结果，但不产生实际修改 
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static FCombatResult CalHurtPoint(ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FSkillBaseVo& SkillVo);

private:
	/** 内部逻辑函数也改用 Result 传递状态 */
	static void InternalCalMultiHurt(FCombatResult& OutResult, ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FSkillBaseVo& SkillVo, int32 SkillAttack);
};