#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MonsterBaseVo.generated.h"

// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FMonsterBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString bodyColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString nameColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 PassiveFight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Appr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Lvl;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ViewRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Exp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 avoidClosed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 dropMoney;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 normalAttackSKill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 attackDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> skillIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> skillIDRnds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 autoHeal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> ignoreBuffs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 dropID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> Attributes;

};