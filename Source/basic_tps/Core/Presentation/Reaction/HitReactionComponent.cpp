// Fill out your copyright notice in the Description page of Project Settings.


#include "HitReactionComponent.h"

#include "AIController.h"
#include "AnimHitReactionInterface.h"
#include "BrainComponent.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"


// Sets default values for this component's properties
UHitReactionComponent::UHitReactionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UHitReactionComponent::ApplyHit(float powScale)
{
	AddImpulse(FVector(2000*powScale,0,0));
	TickTimeTotal=0;
	bCleared=false;
	bIsStunned=false;
}

// Called when the game starts
void UHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	if (bCleared)return;
	if (TickTimeTotal>AnimTime)
	{
		if (!bCleared)
		{
			ClearHit();
		}
		return;
	}
	 
	
	TickTimeTotal+=DeltaTime;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FVector ToTarget = Target - Position;
	if (Target.X>Position.X&&Velocity.X<0)
	{
		Velocity=FVector::Zero();
		Position=Target;
		if (!bCleared)
		{
			ClearHit();
		}
		return;
	}
	// ====== 计算弹簧力 ======
	
	// ====== 应用到Actor（如果你需要）======
	//GetOwner()->SetActorLocation(Position);

	// ====== 偏离计算 ======
	float Dist = ToTarget.Size();

	float Normalized = FMath::Clamp(Dist / MaxDistance, 0.0f, 1.f);

	Deviation01 = Normalized;

	// 关键：符号逻辑（你要的 -1~1）
	float VelocityDir = FVector::DotProduct(Velocity, ToTarget.GetSafeNormal());

	bReturningToTarget = VelocityDir >0;
	// ...


	float HitStartTime=0;
	if (bReturningToTarget)
	{
		HitStartTime=FMath::Lerp(AnimTime,MiddleTime,Deviation01);
	}else
	{
		HitStartTime =MiddleTime*Deviation01 ;
	}
 
	TObjectPtr<USkeletalMeshComponent> HitAnimMesh=GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (HitAnimMesh)
	{
		float HitWeight=1- FMath::Pow( FMath::Clamp( HitStartTime/AnimTime,0,1),2);
		UAnimInstance* AnimInst =HitAnimMesh->GetAnimInstance();
		if (AnimInst && AnimInst->GetClass()->ImplementsInterface(UAnimHitReactionInterface::StaticClass()))
		{
			IAnimHitReactionInterface::Execute_SetHitReaction(
			  AnimInst,
			  HitStartTime,
			  HitWeight
		  );
		}
		//Cast<UAnimHitReactionInterface>(AnimInst)->SetHitReaction(startTime,); 
	
	 
 
	}


	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;
	if (HitStunTime>0&&!bIsStunned)
	{
		bIsStunned=true;
		Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
			  
		auto& TimerManager = GetWorld()->GetTimerManager();

		TimerManager.ClearTimer(RecoverHandle);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("STOP_MOVE"));
		TimerManager.SetTimer(
			RecoverHandle,
			this,
			&UHitReactionComponent::RecoverMovement,
			HitStunTime,
			false
		);
	}
}
void UHitReactionComponent::RecoverMovement()
{
	 
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("RecoverMovement"));
	// 恢复行走模式，CMC 会在这一帧重新接管坐标更新
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    
	// 如果 AI 没动，手动调用一次，给路径系统一个信号重新开始
	if (AAIController* AIC = Cast<AAIController>(Character->GetController()))
	{
		// 这一步是为了让行为树或路径组件意识到“我已经可以移动了”
		AIC->ReceiveMoveCompleted.Broadcast(FAIRequestID::AnyRequest, EPathFollowingResult::Success);
	}
 
}
void UHitReactionComponent::ClearHit()
{
	bCleared=true;
	 
	TObjectPtr<USkeletalMeshComponent> HitAnimMesh=GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (HitAnimMesh)
	{
	 
		UAnimInstance* AnimInst =HitAnimMesh->GetAnimInstance();
		if (AnimInst && AnimInst->GetClass()->ImplementsInterface(UAnimHitReactionInterface::StaticClass()))
		{
			IAnimHitReactionInterface::Execute_SetHitReaction(
			  AnimInst,
			  0,
			  0
		  );
		}
 
	}
}

