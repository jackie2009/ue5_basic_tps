#pragma once

#include "CoreMinimal.h"
 
#include "CombatTypes.h"
#include "GameplayTagContainer.h"
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
    bool CostCurrentHP(int value);

    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "CharacterData")
    int32 GetCurrentMP();
    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "CharacterData")
    int32 GetMaxMP();
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    void SetCurrentMP(int value);
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    void AddCurrentMP(int value);
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    bool CostCurrentMP(int value);
    
    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "CharacterData")
    int32 GetAttribute(int32 AttributeID);
    
    UFUNCTION(BlueprintCallable, Category = "CharacterData")
    int32 AddAttribute(int32 AttributeID,int32 value);
   
   
private:
   
    int32 Hp = 100;
    int32 Mp = 100;
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombatFaction Faction = ECombatFaction::Neutral;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FGameplayTagContainer ActionTags;
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
