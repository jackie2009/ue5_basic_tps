#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BuffBaseVo.generated.h"
 

//100开始 避开 人物属性 buff表里同时支持2种
UENUM(BlueprintType)
enum class EBuffAttribute : uint8
{
	 NONE=0,
	INDEX_OFFSET = 100,
	OnPoison = 101,//中毒 持续效果 
	AddHP = 102,//回血 持续效果 
	Execution = 103,//斩杀 瞬时效果 伤害前计算
	LifeSteal=104,// 吸血 瞬时效果 伤害后计算
	IgnoreArmor=105//忽视防御
	
};
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
	FString effectRes;//名称

	UPROPERTY(EditAnywhere)
	int32   tick;//每几秒作用一次
	UPROPERTY(EditAnywhere)
	FString info;//buff说明
	UPROPERTY(EditAnywhere)
	FString icon;//
	UPROPERTY(EditAnywhere)
	int32  buffEffectID;//buff 效果


	UPROPERTY(EditAnywhere)
	FString tipOnAdd;//添加时提示
	UPROPERTY(EditAnywhere)
	FString tipOnRemove;//结束时提示

};