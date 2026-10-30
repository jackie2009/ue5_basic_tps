#include "CombatTargetingComponent.h"
#include "CombatTargetableComponent.h"
#include "CombatTargetRegistry.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UCombatTargetingComponent::UCombatTargetingComponent()
{
	bAutoActivate = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UCombatTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	InvalidationHandle = GetWorld()->GetSubsystem<UCombatTargetRegistry>()->OnTargetInvalidated.AddUObject(
		this, &UCombatTargetingComponent::OnInvalidated);
}

void UCombatTargetingComponent::Deactivate()
{
	ChangeTarget(nullptr);
	Super::Deactivate();
}

void UCombatTargetingComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (GetWorld())
		if (auto* Registry = GetWorld()->GetSubsystem<UCombatTargetRegistry>()) Registry->OnTargetInvalidated.Remove(InvalidationHandle);
	ChangeTarget(nullptr);
	Super::EndPlay(Reason);
}

APlayerController* UCombatTargetingComponent::GetLocalController() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	auto* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	return PC && PC->IsLocalController() ? PC : nullptr;
}

AActor* UCombatTargetingComponent::GetSelectedTarget() const
{
	return Selected.IsValid() ? Selected->GetOwner() : nullptr;
}

bool UCombatTargetingComponent::IsEligible(UCombatTargetableComponent* Target, float Range) const
{
	return IsValid(Target) && Target->GetWorld() == GetWorld() && Target->CanBeTargetedBy(GetOwner())
		&& FVector::DistSquared(GetOwner()->GetActorLocation(), Target->GetTargetLocation()) <= FMath::Square(FMath::Max(1.f, Range));
}

bool UCombatTargetingComponent::HasLineOfSight(UCombatTargetableComponent* Target) const
{
	if (!IsValid(Target) || !GetWorld()) return false;
	const FVector End = Target->GetTargetLocation();
	FVector Start;
	FRotator Rotation;
	GetOwner()->GetActorEyesViewPoint(Start, Rotation);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CombatTargetVisibility), false, GetOwner());
	Params.AddIgnoredActor(Target->GetOwner());
	// Both the caster and camera must see it: peeking a camera around a wall cannot target through it.
	if (GetWorld()->LineTraceTestByChannel(Start, End, Settings.VisibilityChannel, Params)) return false;
	if (auto* PC = GetLocalController())
	{
		PC->GetPlayerViewPoint(Start, Rotation);
		if (GetWorld()->LineTraceTestByChannel(Start, End, Settings.VisibilityChannel, Params)) return false;
	}
	return true;
}

AActor* UCombatTargetingComponent::GetActionTarget() const
{
	if (!IsActive()) return nullptr;
	auto* Target = Selected.Get();
	return IsEligible(Target, FMath::Max(Settings.AcquireRange, Settings.RetainRange)) && HasLineOfSight(Target)
		? Target->GetOwner() : nullptr;
}

bool UCombatTargetingComponent::Evaluate(UCombatTargetableComponent* Target, bool bRetain, FCandidate& Out) const
{
	auto* PC = GetLocalController();
	if (!PC || !IsEligible(Target, bRetain ? FMath::Max(Settings.AcquireRange, Settings.RetainRange) : Settings.AcquireRange)) return false;
	FVector Origin;
	FRotator View;
	PC->GetPlayerViewPoint(Origin, View);
	FVector Direction = View.Vector();
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const FVector Movement = Pawn ? Pawn->GetLastMovementInputVector().GetSafeNormal2D() : FVector::ZeroVector;
	const bool bMovement = Settings.Direction == ECombatTargetingDirection::Movement;
	if (bMovement)
	{
		Origin = GetOwner()->GetActorLocation();
		Direction = Movement.IsNearlyZero() ? GetOwner()->GetActorForwardVector().GetSafeNormal2D() : Movement;
	}
	FVector Offset = Target->GetTargetLocation() - Origin;
	if (bMovement) Offset.Z = 0.f;
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Direction, Offset.GetSafeNormal()), -1.f, 1.f)));
	const float AcquireAngle = FMath::Clamp(Settings.AcquireHalfAngle, 1.f, 89.f);
	const float MaxAngle = bRetain ? FMath::Clamp(FMath::Max(Settings.RetainHalfAngle, AcquireAngle), 1.f, 89.f) : AcquireAngle;
	if (!(bRetain && bTargetLocked) && Angle > MaxAngle) return false;
	FVector2D Screen;
	const bool bProjected = PC->ProjectWorldLocationToScreen(Target->GetTargetLocation(), Screen, true);
	int32 Width, Height;
	PC->GetViewportSize(Width, Height);
	if (!bMovement && !(bRetain && bTargetLocked)
		&& (!bProjected || Screen.X < 0 || Screen.Y < 0 || Screen.X > Width || Screen.Y > Height)) return false;
	Out.Target = Target;
	Out.ScreenX = bProjected ? Screen.X : 0.f;
	Out.Score = FMath::Max(0.f, Settings.AngleWeight) * Angle / AcquireAngle
		+ FMath::Max(0.f, Settings.DistanceWeight) * FVector::Distance(GetOwner()->GetActorLocation(), Target->GetTargetLocation()) / FMath::Max(1.f, Settings.AcquireRange);
	return true;
}

void UCombatTargetingComponent::ChangeTarget(UCombatTargetableComponent* Target)
{
	if (Selected.Get() == Target && (Target || Selected.IsExplicitlyNull())) return;
	AActor* Previous = GetSelectedTarget();
	Selected = Target;
	Challenger.Reset();
	OccludedSince = -1.;
	if (!Target) bTargetLocked = false;
	OnTargetChanged.Broadcast(Previous, GetSelectedTarget());
}

bool UCombatTargetingComponent::SetSelectedTarget(AActor* Target)
{
	if (!Target) { ChangeTarget(nullptr); return true; }
	auto* Component = Target->FindComponentByClass<UCombatTargetableComponent>();
	if (!IsEligible(Component, Settings.AcquireRange) || !HasLineOfSight(Component)) return false;
	ChangeTarget(Component);
	return true;
}

void UCombatTargetingComponent::SetTargetLocked(bool bLocked)
{
	if (bLocked && !GetActionTarget()) RefreshTarget();
	bTargetLocked = bLocked && GetActionTarget() != nullptr;
	Challenger.Reset();
}

void UCombatTargetingComponent::OnInvalidated(UCombatTargetableComponent* Target)
{
	if (Selected == Target)
	{
		ChangeTarget(nullptr);
		// Reacquisition on the next update avoids registry mutation/reentrancy during actor teardown.
	}
}

void UCombatTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTick)
{
	Super::TickComponent(DeltaTime, TickType, ThisTick);
	RefreshTarget();
}

void UCombatTargetingComponent::RefreshTarget()
{
	if (!GetWorld() || !IsActive()) return;
	const double Now = GetWorld()->GetTimeSeconds();
	if (!bAutoSelect || !GetLocalController())
	{
		if (!Selected.IsExplicitlyNull() && !IsEligible(Selected.Get(), FMath::Max(Settings.AcquireRange, Settings.RetainRange))) ChangeTarget(nullptr);
		// A formerly possessed hero must not keep its old player's selection.
		if (bAutoSelect && !GetLocalController()) ChangeTarget(nullptr);
		return;
	}
	FCandidate Current;
	bool bCurrentValid = Evaluate(Selected.Get(), true, Current);
	if (bCurrentValid && !HasLineOfSight(Current.Target))
	{
		Challenger.Reset();
		if (OccludedSince < 0.) OccludedSince = Now;
		if (Now - OccludedSince < FMath::Max(0.f, Settings.OcclusionGraceSeconds)) return;
		bCurrentValid = false;
	}
	else OccludedSince = -1.;
	if (!bCurrentValid) ChangeTarget(nullptr);
	if (bCurrentValid && bTargetLocked) return;

	TArray<FCandidate> Candidates;
	for (const auto& Weak : GetWorld()->GetSubsystem<UCombatTargetRegistry>()->GetTargets())
	{
		FCandidate Candidate;
		if (Evaluate(Weak.Get(), false, Candidate)) Candidates.Add(Candidate);
	}
	Candidates.Sort([](const FCandidate& A, const FCandidate& B)
	{
		return A.Score == B.Score ? A.Target->GetOwner()->GetPathName() < B.Target->GetOwner()->GetPathName() : A.Score < B.Score;
	});
	const FCandidate* Best = Candidates.FindByPredicate([this](const FCandidate& C) { return HasLineOfSight(C.Target); });
	if (!Best) { Challenger.Reset(); return; }
	if (!bCurrentValid) { ChangeTarget(Best->Target); return; }
	if (Best->Target == Selected.Get() || Best->Score + FMath::Max(0.f, Settings.SwitchAdvantage) >= Current.Score)
	{
		Challenger.Reset();
		return;
	}
	if (Challenger.Get() != Best->Target)
	{
		Challenger = Best->Target;
		ChallengerSince = Now;
	}
	if (Now - ChallengerSince >= FMath::Max(0.f, Settings.SwitchDwellSeconds)) ChangeTarget(Best->Target);
}

bool UCombatTargetingComponent::SwitchTarget(float ScreenDirection)
{
	auto* PC = GetLocalController();
	if (!PC || FMath::IsNearlyZero(ScreenDirection)) return false;
	FVector2D From;
	if (!GetSelectedTarget() || !PC->ProjectWorldLocationToScreen(Selected->GetTargetLocation(), From, true))
	{
		RefreshTarget();
		return GetSelectedTarget() != nullptr;
	}
	UCombatTargetableComponent* Best = nullptr;
	float BestDelta = TNumericLimits<float>::Max();
	for (const auto& Weak : GetWorld()->GetSubsystem<UCombatTargetRegistry>()->GetTargets())
	{
		auto* Target = Weak.Get();
		FVector2D Screen;
		if (Target == Selected.Get() || !IsEligible(Target, Settings.AcquireRange)
			|| !PC->ProjectWorldLocationToScreen(Target->GetTargetLocation(), Screen, true)) continue;
		int32 Width, Height;
		PC->GetViewportSize(Width, Height);
		if (Screen.X < 0 || Screen.Y < 0 || Screen.X > Width || Screen.Y > Height) continue;
		const float Delta = (Screen.X - From.X) * FMath::Sign(ScreenDirection);
		if (Delta > 1.f && Delta < BestDelta && HasLineOfSight(Target)) { Best = Target; BestDelta = Delta; }
	}
	if (!Best) return false;
	ChangeTarget(Best);
	// An explicit switch is sticky until unlocked; camera movement cannot immediately undo the input.
	bTargetLocked = true;
	return true;
}
