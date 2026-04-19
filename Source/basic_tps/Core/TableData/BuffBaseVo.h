#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BuffBaseVo.generated.h"
 

 
 
// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FBuffBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 ID;                  // 技能序号
	UPROPERTY(EditAnywhere)
	int32  group;//分组，同组替换
 
	UPROPERTY(EditAnywhere)
	FString name;//名称

	UPROPERTY(EditAnywhere)
	int32  attribute;//作用属性


 

	UPROPERTY(EditAnywhere)
	int32   tick;//每几秒作用一次
	UPROPERTY(EditAnywhere)
	FString info;//buff说明
	UPROPERTY(EditAnywhere)
	FString icon;//
	UPROPERTY(EditAnywhere)
	FString  buffEffectRes;//buff 效果
 

	UPROPERTY(EditAnywhere)
	FString tipOnAdd;//添加时提示
	UPROPERTY(EditAnywhere)
	FString tipOnRemove;//结束时提示

};