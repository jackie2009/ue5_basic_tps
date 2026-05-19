// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterComponent.h"
#include "CombatNetworkComponent.generated.h"


struct FEffectContext;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UCombatNetworkComponent : public UCharacterComponent
{
	GENERATED_BODY()
public:    
	UCombatNetworkComponent();

	// 1. 本地触发入口
	UFUNCTION(BlueprintCallable, Category = "CombatNetwork")
	void LocalAndCast_PlayMontage(UAnimMontage* SkillMontage);
	UFUNCTION(BlueprintCallable, Category = "CombatNetwork")
	void LocalAndCast_SpawnMagicEffect(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext& InContext, const FVector& Location, const FQuat& Rotation, bool bForceUseTransform);
	UFUNCTION(BlueprintCallable, Category = "CombatNetwork")
	void LocalAndCast_HandleHurt(const FCombatResult& Result);
	UFUNCTION(BlueprintCallable, Category = "CombatNetwork")
	void CastByServer_HandleDead( ACombatCharacter* Target);
	UFUNCTION(BlueprintCallable, Category = "CombatNetwork")
	UBuffLogicBase* LocalAndCast_AddBuff(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue);
	
protected:
	// 2. Server RPC
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* SkillMontage);
	UFUNCTION(Server, Reliable)
	void Server_SpawnMagicEffect(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext InContext, const FVector Location, const FQuat Rotation, bool bForceUseTransform);
	UFUNCTION(Server, Reliable)
	void Server_HandleHurt(const FCombatResult& Result);
	UFUNCTION(Server, Reliable)
	void Server_AddBuff(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue);
	// 3. Multicast RPC
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMontage(UAnimMontage* SkillMontage);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnMagicEffect(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext InContext, const FVector Location, const FQuat Rotation, bool bForceUseTransform);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleHurt(const FCombatResult& Result);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleDead( ACombatCharacter* Target);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddBuff(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue);
private:
	// 内部通用播放逻辑
	void Internal_PlayMontage(UAnimMontage* SkillMontage);
    
	// 辅助函数：安全获取宿主的 AnimInstance
	class UAnimInstance* GetOwnerAnimInstance() const;
	//内部接受广播后真正执行的本地操作
	UBuffLogicBase* ExecuteLocal_AddBuff(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue);
	
};
