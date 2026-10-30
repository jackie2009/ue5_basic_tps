#include "CombatTargetableComponent.h"
#include "CombatTargetRegistry.h"
#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UCombatTargetableComponent::UCombatTargetableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatTargetableComponent::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UCombatTargetRegistry>()->RegisterTarget(this);
}

void UCombatTargetableComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UWorld* World = GetWorld())
		if (auto* Registry = World->GetSubsystem<UCombatTargetRegistry>()) Registry->UnregisterTarget(this);
	Super::EndPlay(Reason);
}

void UCombatTargetableComponent::SetTargetable(bool bEnabled)
{
	if (bTargetable == bEnabled) return;
	bTargetable = bEnabled;
	if (!bEnabled && GetWorld())
		GetWorld()->GetSubsystem<UCombatTargetRegistry>()->OnTargetInvalidated.Broadcast(this);
}

bool UCombatTargetableComponent::CanBeTargetedBy_Implementation(AActor* Seeker) const
{
	return bTargetable && IsValid(Seeker) && IsValid(GetOwner()) && Seeker != GetOwner()
		&& !GetOwner()->IsActorBeingDestroyed() && !GetOwner()->IsHidden();
}

FVector UCombatTargetableComponent::GetTargetLocation_Implementation() const
{
	if (!GetOwner()) return FVector::ZeroVector;
	if (auto* Mesh = GetOwner()->FindComponentByClass<UMeshComponent>())
	{
		if (!AimSocket.IsNone() && Mesh->DoesSocketExist(AimSocket)) return Mesh->GetSocketLocation(AimSocket);
		if (Mesh->Bounds.SphereRadius > KINDA_SMALL_NUMBER) return Mesh->Bounds.Origin;
	}
	return GetOwner()->GetActorLocation();
}
