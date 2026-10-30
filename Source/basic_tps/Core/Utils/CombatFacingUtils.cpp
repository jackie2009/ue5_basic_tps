#include "CombatFacingUtils.h"
#include "basic_tps/Core/Targeting/CombatTargetableComponent.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

bool UCombatFacingUtils::FaceToSelectedTargetImmediately(AActor* Source, bool bUpdateControllerYaw)
{
	if (!IsValid(Source)) return false;
	auto* Targeting = Source->FindComponentByClass<UCombatTargetingComponent>();
	return Targeting && FaceToTargetImmediately(Source, Targeting->GetActionTarget(), bUpdateControllerYaw);
}

bool UCombatFacingUtils::FaceToTargetImmediately(AActor* Source, AActor* Target, bool bUpdateControllerYaw)
{
	if (!IsValid(Source) || !IsValid(Target) || Source == Target || Source->IsActorBeingDestroyed()
		|| Target->IsActorBeingDestroyed() || Source->GetWorld() != Target->GetWorld()) return false;
	auto* Targetable = Target->FindComponentByClass<UCombatTargetableComponent>();
	if (Targetable && !Targetable->CanBeTargetedBy(Source)) return false;
	const FVector Location = Targetable ? Targetable->GetTargetLocation() : Target->GetActorLocation();
	FVector Delta = Location - Source->GetActorLocation();
	Delta.Z = 0.;
	if (Delta.IsNearlyZero()) return false;
	FRotator Rotation = Source->GetActorRotation();
	Rotation.Yaw = Delta.Rotation().Yaw;
	Source->SetActorRotation(Rotation);
	if (bUpdateControllerYaw)
	{
		const auto* Pawn = Cast<APawn>(Source);
		if (auto* Controller = Pawn ? Pawn->GetController() : nullptr)
		{
			FRotator Control = Controller->GetControlRotation();
			Control.Yaw = Rotation.Yaw;
			Controller->SetControlRotation(Control);
		}
	}
	return true;
}
