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
    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "CharacterData")
    int32 GetCurrentHP();
    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "CharacterData")
    int32 GetMaxHP();
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    void SetCurrentHP(int value);
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    void AddCurrentHP(int value);
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    void CostCurrentHP(int value);
    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "CharacterData")
    int32 GetAttribute(int32 AttributeID);
private:
    int32 Hp = 100;
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombatFaction Faction = ECombatFaction::Neutral;
    
   
    int32 Attributes[AttributeEnum::MAX];
      
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat")
    int32 Level = 1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat")
 
    
    FString displayName ="???";
 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AutoAttackInterval = 1.0f;

    
};
