// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacter.h"

#include "BuffComponent.h"
#include "CombatComponent.h"
#include "SkillComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "basic_tps/Core/Targeting/CharacterTargetableComponent.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Rendering/SkeletalMeshRenderData.h"


// Sets default values
ACombatCharacter::ACombatCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TargetingComp = CreateDefaultSubobject<UCombatTargetingComponent>(TEXT("CombatTargeting"));
	TargetableComp = CreateDefaultSubobject<UCharacterTargetableComponent>(TEXT("CombatTargetable"));

	 
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

	BuffComp = NewObject<UBuffComponent>(this);
	BuffComp->RegisterComponent();
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
	TargetableComp->SetTargetable(false);
	TargetingComp->SetSelectedTarget(nullptr);
	// 延迟到下一帧执行真正的碰撞关闭
	GetWorldTimerManager().SetTimerForNextTick(this, &ACombatCharacter::ExecutePhysicsDisabled);
	
	OnCharacterDeath.Broadcast(this);
	OnDead();
}
void ACombatCharacter::ExecutePhysicsDisabled()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 2. 如果开启了 Ragdoll（布娃娃），Mesh 需要保留物理但关闭对 Pawn 的阻挡
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
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

static void SetupDroppedPhysics(UPrimitiveComponent* Prim, const FVector& InheritVel, const FVector& OutDir, float ExtraImpulse)
{
	Prim->SetCollisionProfileName(TEXT("PhysicsActor"));
	Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);      // 不挡玩家
	Prim->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);    // 不顶相机
	Prim->BodyInstance.bUseCCD = true;                              // 薄武器防穿地
	Prim->SetSimulatePhysics(true);
	Prim->SetPhysicsLinearVelocity(InheritVel);
	if (ExtraImpulse > 0.f)
	{
		Prim->AddImpulse((OutDir + FVector(0, 0, 0.5f)).GetSafeNormal() * ExtraImpulse, NAME_None, true);
	}
}

void ACombatCharacter::DropWeaponsOnDeath(float ExtraImpulse, FName DropTag)
{
	USkeletalMeshComponent* BodyMesh = GetMesh();
	if (!IsValid(BodyMesh))
	{
		return;
	}

	auto VelAt = [BodyMesh](const FVector& P)
	{
		return BodyMesh->IsSimulatingPhysics() ? BodyMesh->GetPhysicsLinearVelocityAtPoint(P) : FVector::ZeroVector;
	};
	auto OutDirOf = [this](const FVector& P)
	{
		return (P - GetActorLocation()).GetSafeNormal2D();
	};

	// 1) 隐藏网格中的武器材质槽（所有 LOD）
	if (DeathHideMaterialSlots.Num() > 0)
	{
		if (USkeletalMesh* SkelAsset = BodyMesh->GetSkeletalMeshAsset())
		{
			if (FSkeletalMeshRenderData* RenderData = SkelAsset->GetResourceForRendering())
			{
				for (int32 LOD = 0; LOD < RenderData->LODRenderData.Num(); ++LOD)
				{
					const TArray<FSkelMeshRenderSection>& Sections = RenderData->LODRenderData[LOD].RenderSections;
					for (int32 Sec = 0; Sec < Sections.Num(); ++Sec)
					{
						const int32 MatIdx = Sections[Sec].MaterialIndex;
						if (DeathHideMaterialSlots.Contains(MatIdx))
						{
							BodyMesh->ShowMaterialSection(MatIdx, Sec, false, LOD);
						}
					}
				}
			}
		}
	}

	// 2) 在骨骼位置生成独立物理武器（作为本角色的组件，尸体销毁时一起销毁）
	for (const FDeathDropWeapon& W : DeathDropWeapons)
	{
		if (!W.Mesh)
		{
			continue;
		}
		const FTransform BoneTM = (W.Bone != NAME_None && BodyMesh->DoesSocketExist(W.Bone))
			? BodyMesh->GetSocketTransform(W.Bone, RTS_World)
			: BodyMesh->GetComponentTransform();
		const FTransform SpawnTM = W.Offset * BoneTM;

		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, NAME_None, RF_Transient);
		Comp->SetStaticMesh(W.Mesh);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetWorldTransform(SpawnTM);
		Comp->RegisterComponent();
		AddInstanceComponent(Comp);

		SetupDroppedPhysics(Comp, VelAt(SpawnTM.GetLocation()), OutDirOf(SpawnTM.GetLocation()), ExtraImpulse);
	}

	// 3) 已有的、带 DropTag 标签的组件：分离并开启物理
	TArray<UPrimitiveComponent*> Prims;
	GetComponents<UPrimitiveComponent>(Prims);
	for (UPrimitiveComponent* Prim : Prims)
	{
		if (!IsValid(Prim) || Prim == BodyMesh || !Prim->ComponentHasTag(DropTag))
		{
			continue;
		}
		const FVector P = Prim->GetComponentLocation();
		const FVector V = VelAt(P);
		Prim->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		SetupDroppedPhysics(Prim, V, OutDirOf(P), ExtraImpulse);
	}
}
