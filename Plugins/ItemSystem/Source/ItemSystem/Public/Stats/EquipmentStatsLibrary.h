#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EquipmentStatsLibrary.generated.h"

class UEquipmentComponent;

/** 装备属性汇总（独立实现，汇总基础属性 + 随机词缀 DynamicAttributes） */
USTRUCT(BlueprintType)
struct ITEMSYSTEM_API FEquipmentStatSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Defence = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Block = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 BonusHpMax = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 Critical = 0;

	/** 非武器部位附带的攻击（如尖刺手套） */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FIntPoint BaseAttack = FIntPoint::ZeroValue;

	/** 三种武器各自的攻击区间（含 BaseAttack 与攻击强化词缀），没装备时为 0 */
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
