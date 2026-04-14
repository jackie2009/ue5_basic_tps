#include "CharacterComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "CombatCharacter.h"
#include "CombatComponent.h"

void UCharacterComponent::OnRegister()
{
	Super::OnRegister();
    Character=Cast<ACombatCharacter>( GetOwner());
	 
}
