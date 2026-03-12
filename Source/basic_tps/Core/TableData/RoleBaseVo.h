#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RoleBaseVo.generated.h"

// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FRoleBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 ID;                  // 技能序号
	UPROPERTY(EditAnywhere)
	FString animalname;           //角色名
 
	UPROPERTY(EditAnywhere)
	TArray<int32> skills;  //技能

	UPROPERTY(EditAnywhere)
	TArray<int32>   initBagProps;//新建角色背包物品
	UPROPERTY(EditAnywhere)
	TArray<int32> initPocketProps;//新建角色快捷栏物品
	UPROPERTY(EditAnywhere)
	TArray<int32>initEquipProps;//新建角色装备栏物品

	UPROPERTY(EditAnywhere)
	TArray<int32>initSkills;//新建角色拥有技能

	 
  

};