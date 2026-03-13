#pragma once

#include "CoreMinimal.h"
 
#include "CombatTypes.h"
#include "CharacterDataComponent.generated.h"

UCLASS()
class BASIC_TPS_API UCharacterDataComponent : public UActorComponent
{
    GENERATED_BODY()
public:	
    
    UCharacterDataComponent();
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombatFaction Faction = ECombatFaction::Neutral;

    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat")
    int32 Hp = 100;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FString displayName ="???";
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 MaxHp = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 MaxMp = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 BaseAttack = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 BaseDefense = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AutoAttackInterval = 1.0f;
};
