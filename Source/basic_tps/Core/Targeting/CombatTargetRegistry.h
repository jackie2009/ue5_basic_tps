#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatTargetRegistry.generated.h"

class UCombatTargetableComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatTargetInvalidated, UCombatTargetableComponent*);

/** World-scoped weak registry; no GetAllActors scan, no ownership of actors. */
UCLASS()
class BASIC_TPS_API UCombatTargetRegistry : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	void RegisterTarget(UCombatTargetableComponent* Target);
	void UnregisterTarget(UCombatTargetableComponent* Target);
	const TArray<TWeakObjectPtr<UCombatTargetableComponent>>& GetTargets() const { return Targets; }
	FCombatTargetInvalidated OnTargetInvalidated;
private:
	TArray<TWeakObjectPtr<UCombatTargetableComponent>> Targets;
};
