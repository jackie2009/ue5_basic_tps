// BuffComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Data/CombatTypes.h"
#include "CharacterComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class  BASIC_TPS_API UCharacterComponent : public UActorComponent {
	GENERATED_BODY()

public:
 
   TObjectPtr<ACombatCharacter> Character;
	 
	

protected:
	void virtual  OnRegister() override;
private:
 
};

 

