#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EquipmentStatsLibrary.generated.h"

class UEquipmentComponent;

/** 装备属性汇总（独立实现，只用于显示，不写入战斗属性） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FEquipmentStatSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Defence = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Block = 0;

	/** 非武器部位附带的攻击（如尖刺手套） */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FIntPoint BaseAttack = FIntPoint::ZeroValue;

	/** 三种武器各自的攻击区间（含 BaseAttack），没装备时为 0 */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FIntPoint OneHandAttack = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FIntPoint PolearmAttack = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FIntPoint RangedAttack = FIntPoint::ZeroValue;
};

UCLASS()
class ITEMSYSTEM_API UEquipmentStatsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "ItemSystem|Stats")
	static FEquipmentStatSummary Summarize(const UEquipmentComponent* Equipment);
};
