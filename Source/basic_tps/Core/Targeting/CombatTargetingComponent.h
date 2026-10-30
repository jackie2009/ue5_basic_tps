#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTargetingSettings.h"
#include "CombatTargetingComponent.generated.h"

class UCombatTargetableComponent;
class APlayerController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatTargetChanged, AActor*, PreviousTarget, AActor*, NewTarget);

/** Selection only. Does not rotate actors, move cameras, cast spells, or draw outlines. */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class BASIC_TPS_API UCombatTargetingComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCombatTargetingComponent();
	virtual void Deactivate() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Targeting")
	FCombatTargetingSettings Settings;
	/** Automatic selection runs only for the locally controlled player pawn. AI supplies its target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Targeting")
	bool bAutoSelect = false;
	UPROPERTY(BlueprintAssignable, Category="Combat|Targeting")
	FCombatTargetChanged OnTargetChanged;
	UFUNCTION(BlueprintPure, Category="Combat|Targeting")
	AActor* GetSelectedTarget() const;
	/** Revalidates health, hostility, range and both sight lines; never returns a grace-period target. */
	UFUNCTION(BlueprintPure, Category="Combat|Targeting")
	AActor* GetActionTarget() const;
	/** For AI/manual use. Null clears selection. Does not enable automatic selection. */
	UFUNCTION(BlueprintCallable, Category="Combat|Targeting")
	bool SetSelectedTarget(AActor* Target);
	UFUNCTION(BlueprintCallable, Category="Combat|Targeting")
	void SetTargetLocked(bool bLocked);
	UFUNCTION(BlueprintPure, Category="Combat|Targeting")
	bool IsTargetLocked() const { return bTargetLocked; }
	/** One call per input press/flick. Positive = screen right; negative = screen left. */
	UFUNCTION(BlueprintCallable, Category="Combat|Targeting")
	bool SwitchTarget(float ScreenDirection);
	UFUNCTION(BlueprintCallable, Category="Combat|Targeting")
	void RefreshTarget();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTick) override;
private:
	struct FCandidate
	{
		UCombatTargetableComponent* Target = nullptr;
		float Score = 0.f;
		float ScreenX = 0.f;
	};
	APlayerController* GetLocalController() const;
	bool IsEligible(UCombatTargetableComponent* Target, float Range) const;
	bool HasLineOfSight(UCombatTargetableComponent* Target) const;
	bool Evaluate(UCombatTargetableComponent* Target, bool bRetain, FCandidate& Out) const;
	void ChangeTarget(UCombatTargetableComponent* Target);
	void OnInvalidated(UCombatTargetableComponent* Target);
	TWeakObjectPtr<UCombatTargetableComponent> Selected;
	TWeakObjectPtr<UCombatTargetableComponent> Challenger;
	FDelegateHandle InvalidationHandle;
	double ChallengerSince = 0.;
	double OccludedSince = -1.;
	bool bTargetLocked = false;
};
