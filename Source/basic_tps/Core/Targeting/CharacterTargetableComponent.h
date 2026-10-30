#pragma once
#include "CombatTargetableComponent.h"
#include "CharacterTargetableComponent.generated.h"

/** The only targeting adapter that knows about character health/factions. */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UCharacterTargetableComponent : public UCombatTargetableComponent
{
	GENERATED_BODY()
public:
	virtual bool CanBeTargetedBy_Implementation(AActor* Seeker) const override;
};
