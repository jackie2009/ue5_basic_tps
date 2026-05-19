// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatNetworkComponent.h"

#include "BuffComponent.h"
#include "CombatComponent.h"
#include "basic_tps/Core/Effect/BuffLogicBase.h"
#include "basic_tps/Core/Effect/MagicEffect.h"


// Sets default values for this component's properties
UCombatNetworkComponent::UCombatNetworkComponent()
{
 

	PrimaryComponentTick.bCanEverTick = false;

	// 🎯 核心核心：必须在构造函数中告诉引擎，这个组件需要网络同步
	SetIsReplicatedByDefault(true);
}

/////////////////////////////// LocalAndCast /////////////////////////////////////////
void UCombatNetworkComponent::LocalAndCast_PlayMontage(UAnimMontage* SkillMontage)
{
	if (!SkillMontage) return;
	if (!Character->IsLocallyControlled())return;
	// 1. 本地预测播放
    Internal_PlayMontage(SkillMontage);
	 

	// 2. 网络分发机制
	if (Character->HasAuthority())
	{
		Multicast_PlayMontage(SkillMontage);
	}
	else
	{
		Server_PlayMontage(SkillMontage);
	}
}

void UCombatNetworkComponent::LocalAndCast_SpawnMagicEffect(TSubclassOf<AMagicEffect> ClassToSpawn,
	const FEffectContext& InContext, const FVector& Location, const FQuat& Rotation, bool bForceUseTransform)
{
	if (!ClassToSpawn) return;
	//只能自己调用广播 
	if (!Character->IsLocallyControlled())return;
	// 1. 本地手操者：立刻预测生成（实现完美零延迟手感）
	  FVector  CastLocation=Location;
	FQuat  CastRotation=Rotation;
	
	 
	    auto Effect=	AMagicEffect::SpawnMagicEffect(this, ClassToSpawn, InContext, Location, Rotation, bForceUseTransform);
		if (Effect)
		{
			CastLocation=Effect->GetActorLocation();
			CastRotation=Effect->GetActorRotation().Quaternion();
			bForceUseTransform=true;
		}
	 

	// 2. 根据当前权限决定如何发送网络大喇叭
	if (Character->HasAuthority())
	{
		Multicast_SpawnMagicEffect(ClassToSpawn, InContext, CastLocation, CastRotation, bForceUseTransform);
	}
	else
	{
		Server_SpawnMagicEffect(ClassToSpawn, InContext, CastLocation, CastRotation, bForceUseTransform);
	}
}

void UCombatNetworkComponent::LocalAndCast_HandleHurt(const FCombatResult& Result)
{
	if (!Character->IsLocallyControlled())return;
	Result.Victim->CombatComp->HandleHurt(Result);

	if (Character->HasAuthority())
	{
		Multicast_HandleHurt(Result);
	}
	else
	{
		Server_HandleHurt(Result);
	}
}
void UCombatNetworkComponent::CastByServer_HandleDead( ACombatCharacter* Target)
{
	if (!Character->HasAuthority())return;
	Target->SelfOnDead();
    Multicast_HandleDead(Target);
 
}

UBuffLogicBase* UCombatNetworkComponent::LocalAndCast_AddBuff(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue)
{
	if (!Character->IsLocallyControlled())return nullptr;
	auto rst=ExecuteLocal_AddBuff(ClassOfBuff, InEffectRole, InFromRole, InDuration, InValue);

	if (Character->HasAuthority())
	{
		Multicast_AddBuff(ClassOfBuff, InEffectRole, InFromRole, InDuration, InValue);
	}
	else
	{
		Server_AddBuff(ClassOfBuff, InEffectRole, InFromRole, InDuration, InValue);
	}
	return rst;
}
/////////////////////////////// server /////////////////////////////////////////
void UCombatNetworkComponent::Server_PlayMontage_Implementation(UAnimMontage* SkillMontage)
{
	Multicast_PlayMontage(SkillMontage);
}
void UCombatNetworkComponent::Server_SpawnMagicEffect_Implementation(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext InContext, const FVector Location, const FQuat Rotation, bool bForceUseTransform)
{
	// 服务器收到客机请求，直接多播群发
	Multicast_SpawnMagicEffect(ClassToSpawn, InContext, Location, Rotation, bForceUseTransform);
}

void UCombatNetworkComponent::Server_HandleHurt_Implementation(const FCombatResult& Result)
{
	Multicast_HandleHurt(Result);
}
 
void UCombatNetworkComponent::Server_AddBuff_Implementation(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue)
{
		Multicast_AddBuff(ClassOfBuff, InEffectRole, InFromRole, InDuration, InValue);
}
//////////////////////////////////// multi cast //////////////////////////////

void UCombatNetworkComponent::Multicast_PlayMontage_Implementation(UAnimMontage* SkillMontage)
{
	// 旁观者才执行播放，防止发起者重复播放
	if (Character->IsLocallyControlled())
	{
		return; 
	}
		Internal_PlayMontage(SkillMontage);
	 
}
void UCombatNetworkComponent::Multicast_SpawnMagicEffect_Implementation(TSubclassOf<AMagicEffect> ClassToSpawn, const FEffectContext InContext, const FVector Location, const FQuat Rotation, bool bForceUseTransform)
{
	// 🎯 关键拦截：手操发起的那个客户端在第一步已经提前预测生成过了，这里要跳过他，防止生成双份特效
 
	if (Character->IsLocallyControlled())
	{
		return; 
	}

	// 3. 旁观者和服务器（如果服务器不是房主）在自己的单机世界里各自生成一份
	AMagicEffect::SpawnMagicEffect(this, ClassToSpawn, InContext, Location, Rotation, bForceUseTransform);
}
void UCombatNetworkComponent::Multicast_HandleHurt_Implementation(const FCombatResult& Result)
{
	// 旁观者才执行播放，防止发起者重复播放
	if (Character->IsLocallyControlled())
	{
		return; 
	}
	Result.Victim->CombatComp->HandleHurt( Result);
}
void UCombatNetworkComponent::Multicast_HandleDead_Implementation(ACombatCharacter* Target)
{
	// 防止发起者重复播放
	if (Character->HasAuthority())
	{
		return; 
	}
	if (IsValid(Target))
	{
		Target->SelfOnDead();
	}
}
void UCombatNetworkComponent::Multicast_AddBuff_Implementation(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,ACombatCharacter* InFromRole,float InDuration,int InValue)
{
	// 旁观者才执行播放，防止发起者重复播放
	if (Character->IsLocallyControlled())
	{
		return; 
	}
	ExecuteLocal_AddBuff(ClassOfBuff, InEffectRole, InFromRole, InDuration, InValue);
 
}


//////////////////////////////////////execute////////////////////////////////////
void UCombatNetworkComponent::Internal_PlayMontage(UAnimMontage* SkillMontage)
{
	if (UAnimInstance* AnimInst = GetOwnerAnimInstance())
	{
		AnimInst->Montage_Play(SkillMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
	}
}

UAnimInstance* UCombatNetworkComponent::GetOwnerAnimInstance() const
{
 
	if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
	{
		return MeshComp->GetAnimInstance();
	}
 
	return nullptr;
}

UBuffLogicBase* UCombatNetworkComponent::ExecuteLocal_AddBuff(TSubclassOf<UBuffLogicBase> ClassOfBuff, ACombatCharacter* InEffectRole,
	ACombatCharacter* InFromRole, float InDuration, int InValue)
{
	if (!ClassOfBuff || !InEffectRole) return nullptr;

	// 100% 还原你单机的写法，不管是哪台电脑收到，都是在这里现场创建、现场初始化
	UBuffLogicBase* NewBuff = NewObject<UBuffLogicBase>(InEffectRole, ClassOfBuff);
	if (NewBuff)
	{
		NewBuff->InitBaseData(InEffectRole, InFromRole, InDuration, InValue);
		 

		// 挂载到本地的组件上，触发单机的后续逻辑
		InEffectRole->BuffComp->AddBuff(NewBuff);
		return NewBuff;
	}
	return nullptr;
}

 
