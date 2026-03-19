// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacter.h"

#include "CombatComponent.h"
#include "SkillComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ACombatCharacter::ACombatCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	 
}

// Called when the game starts or when spawned
void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();
 
}

void ACombatCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	CharacterDataComp = NewObject<UCharacterDataComponent>(this);
	CharacterDataComp->RegisterComponent();
	
	CombatComp = NewObject<UCombatComponent>(this);
	CombatComp->RegisterComponent();

	SkillComp = NewObject<USkillComponent>(this);
	SkillComp->RegisterComponent();
	
 
}

void ACombatCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
 
 
}

void ACombatCharacter::SelfOnHurt(float Damage, FVector HitLocation)
{
	OnHurt(Damage, HitLocation);
}

void ACombatCharacter::SelfOnDead()
{
	// 延迟到下一帧执行真正的碰撞关闭
	GetWorldTimerManager().SetTimerForNextTick(this, &ACombatCharacter::ExecutePhysicsDisabled);
	
	
	OnDead();
}
void ACombatCharacter::ExecutePhysicsDisabled()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 2. 如果开启了 Ragdoll（布娃娃），Mesh 需要保留物理但关闭对 Pawn 的阻挡
	//GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	// 确保尸体不会被玩家踢飞或挡住玩家
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 3. 停止移动组件逻辑
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	 
}
// Called every frame
void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
bool ACombatCharacter::IsAlive() const
{
	return    CharacterDataComp!=nullptr && ( CharacterDataComp->GetCurrentHP()>0);
}
