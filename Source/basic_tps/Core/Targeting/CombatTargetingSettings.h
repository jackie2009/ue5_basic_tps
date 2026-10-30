#pragma once
#include "CoreMinimal.h"
#include "CombatTargetingSettings.generated.h"

UENUM(BlueprintType)
enum class ECombatTargetingDirection : uint8 { Camera, Movement };

/** Project tuning values, not reverse-engineered Hogwarts Legacy constants. Distances are cm. */
USTRUCT(BlueprintType)
struct FCombatTargetingSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	ECombatTargetingDirection Direction = ECombatTargetingDirection::Camera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="1"))
	float AcquireRange = 2500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="1"))
	float RetainRange = 2800.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="1", ClampMax="89"))
	float AcquireHalfAngle = 35.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="1", ClampMax="89"))
	float RetainHalfAngle = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="0"))
	float AngleWeight = 0.85f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="0"))
	float DistanceWeight = 0.15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="0"))
	float SwitchAdvantage = 0.12f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="0"))
	float SwitchDwellSeconds = 0.15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting", meta=(ClampMin="0"))
	float OcclusionGraceSeconds = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	TEnumAsByte<ECollisionChannel> VisibilityChannel = ECC_Visibility;
};
