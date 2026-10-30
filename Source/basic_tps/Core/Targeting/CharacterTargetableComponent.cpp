#include "CharacterTargetableComponent.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"

bool UCharacterTargetableComponent::CanBeTargetedBy_Implementation(AActor* Seeker) const
{
	if (!Super::CanBeTargetedBy_Implementation(Seeker)) return false;
	const auto* Target = Cast<ACombatCharacter>(GetOwner());
	const auto* Source = Cast<ACombatCharacter>(Seeker);
	if (!Target || !Source || !Target->IsAlive() || !Source->IsAlive()
		|| !Target->CharacterDataComp || !Source->CharacterDataComp) return false;
	const ECombatFaction A = Source->CharacterDataComp->Faction;
	const ECombatFaction B = Target->CharacterDataComp->Faction;
	const bool bAAllied = A == ECombatFaction::Hero || A == ECombatFaction::Pet;
	const bool bBAllied = B == ECombatFaction::Hero || B == ECombatFaction::Pet;
	return (bAAllied && B == ECombatFaction::Monster) || (A == ECombatFaction::Monster && bBAllied);
}
