#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatFacingUtils.generated.h"

/** Shared by players and AI; instant yaw only, never a persistent movement/rotation mode. */
UCLASS()
class BASIC_TPS_API UCombatFacingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** False for missing/invalid/occluded selection. Does not choose a replacement or face the camera. */
	UFUNCTION(BlueprintCallable, Category="Combat|Facing", meta=(DefaultToSelf="Source", ReturnDisplayName="Did Rotate"))
	static bool FaceToSelectedTargetImmediately(AActor* Source, bool bUpdateControllerYaw = false);
	/** Use the skill's captured TargetActor here so a mid-cast selection change cannot redirect it. */
	UFUNCTION(BlueprintCallable, Category="Combat|Facing", meta=(DefaultToSelf="Source", ReturnDisplayName="Did Rotate"))
	static bool FaceToTargetImmediately(AActor* Source, AActor* Target, bool bUpdateControllerYaw = false);
};
