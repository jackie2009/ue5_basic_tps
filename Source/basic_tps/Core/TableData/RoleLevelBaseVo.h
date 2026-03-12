#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RoleLevelBaseVo.generated.h"

// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FRoleLevelBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 ID;					// 技能序号
	UPROPERTY(EditAnywhere)
	int32 level;				//等级
	UPROPERTY(EditAnywhere)
	int32 Exp;				   // 经验
	UPROPERTY(EditAnywhere)
	TArray<int32> Attributes;  // 战斗属性
 

};