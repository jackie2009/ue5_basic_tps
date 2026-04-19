// BuffComponent.h
#pragma once
#include "CoreMinimal.h"
#include "CharacterComponent.h"
#include "Components/ActorComponent.h"
#include "../Data/CombatTypes.h"
#include "BuffComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class  BASIC_TPS_API UBuffComponent : public UCharacterComponent {
	GENERATED_BODY()

public:
	UBuffComponent();
	UPROPERTY(BlueprintReadOnly)
	FCombatResult    TempCombatResult;

	UFUNCTION(BlueprintCallable)
	void AddBuff(UBuffLogicBase *NewBuff);

	UFUNCTION(BlueprintCallable)
	int32 GetBuffValue(int32 BuffAttType) const;
	
	UFUNCTION(BlueprintCallable)
	int32 CostBuffValue(int32 BuffAttType,int32 value);
 
	UFUNCTION(BlueprintCallable)
	void RemoveBuff(UBuffLogicBase *BuffLogic, bool bReplaceMode=false);
	
 
	void RemoveBuff(int32 Index, bool bReplaceMode=false);
	
	UFUNCTION(BlueprintCallable)
	void RemoveAllBuffs();
	
	void BroadcastOnTakeDamage(FCombatResult& Result);
	
	 
	

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CalBuffAttributes();
	void HandleOwnerDeath(ACombatCharacter* Victim);
	
	bool bIsOwnerDead=false;
	bool bIsMovementAllowed;
	bool bIsAttackAllowed;
	bool bAIStoppedByBuff;
private:
	UPROPERTY()
	TArray<TObjectPtr<UBuffLogicBase>> BuffList;

	

 
	void TryDestroyBuffView(UBuffLogicBase *BuffLogic);
	
 
};