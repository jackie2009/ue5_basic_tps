// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterComponent.h"
#include "CombatNetworkComponent.generated.h"


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
protected:
	// 2. Server RPC
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* SkillMontage);
	UFUNCTION(Server, Reliable)
	void Server_SpawnMagicEffect(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext InContext, const FVector Location, const FQuat Rotation, bool bForceUseTransform);
	
	// 3. Multicast RPC
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMontage(UAnimMontage* SkillMontage);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnMagicEffect(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext InContext, const FVector Location, const FQuat Rotation, bool bForceUseTransform);
	
private:
	// 内部通用播放逻辑
	void Internal_PlayMontage(UAnimMontage* SkillMontage);
    
	// 辅助函数：安全获取宿主的 AnimInstance
	class UAnimInstance* GetOwnerAnimInstance() const;
 
};
