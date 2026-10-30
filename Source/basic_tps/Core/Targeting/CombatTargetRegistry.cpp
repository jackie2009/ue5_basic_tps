#include "CombatTargetRegistry.h"
#include "CombatTargetableComponent.h"

void UCombatTargetRegistry::RegisterTarget(UCombatTargetableComponent* Target)
{
	Targets.RemoveAll([](const auto& Entry) { return !Entry.IsValid(); });
	Targets.AddUnique(Target);
}

void UCombatTargetRegistry::UnregisterTarget(UCombatTargetableComponent* Target)
{
	Targets.Remove(Target);
	OnTargetInvalidated.Broadcast(Target);
}
