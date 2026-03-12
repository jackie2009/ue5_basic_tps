#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponTypeBaseVo.generated.h"

// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FWeaponTypeBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 ID;                  // 技能序号
 
 
	UPROPERTY(EditAnywhere)
	FString name;//名称

 


	UPROPERTY(EditAnywhere)
	FString anmController1;//动画控制器名称

	UPROPERTY(EditAnywhere)
	int32   attackDistance;//普通攻击距离
	 

};