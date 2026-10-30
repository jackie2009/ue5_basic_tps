#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTargetableComponent.generated.h"

/** Opt-in target registration. Override eligibility for destructibles or non-character targets. */
UCLASS(Blueprintable, ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UCombatTargetableComponent : public UActorComponent
{

	GENERATED_BODY()
public:
	UCombatTargetableComponent();
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category="Combat|Targeting")
	bool CanBeTargetedBy(AActor* Seeker) const;
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category="Combat|Targeting")
	FVector GetTargetLocation() const;
	UFUNCTION(BlueprintCallable, Category="Combat|Targeting")
	void SetTargetable(bool bEnabled);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Targeting")
	bool bTargetable = true;
	/** Optional mesh socket. Bounds centre is used if absent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Targeting")
	FName AimSocket = NAME_None;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
