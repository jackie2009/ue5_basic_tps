// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
 
#include "CombatCalculator.generated.h"

struct FCombatResult;
class ACombatCharacter;
class USkillVfxDataAsset;
struct FSkillBaseVo;
/**
 * 
 */
UCLASS()
class BASIC_TPS_API UCombatCalculator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void CaptureAttributeSnapshot( FCombatResult& Result);
	static void PreDamageProcess( FCombatResult& Result);
	static void EvaluateCoreFormula( FCombatResult& Result);
	static void AdjustFinalDamage( FCombatResult& Result);
	static void CommitActualDamage(const FCombatResult& Result);
	static void PostDamageProcess(const FCombatResult& Result);

	/**  伤害流程总入口，包含 预处理 基础伤害 后处理等 全套计算的流水线。普通的持续性buff放这个后面添加
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static FCombatResult DamagePipeline(ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FSkillBaseVo& SkillVo);

 
	

private:
	/** 内部逻辑函数也改用 Result 传递状态 */
	static void InternalCalMultiHurt(FCombatResult& OutResult, ACombatCharacter* Attacker, ACombatCharacter* Defencer, const FSkillBaseVo& SkillVo, int32 SkillAttack);
};