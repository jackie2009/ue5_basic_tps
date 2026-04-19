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
	Frozen =106,//冻结
	IceShield=107,//寒冰盾 加护甲 并冻结攻击者
 
	/** 1. 支持：枚举 == 数字 (EBuffAttribute == int32) */

};
FORCEINLINE bool operator==(EBuffAttribute Attr, int32 Value)
{
	return static_cast<int32>(Attr) == Value;
}

/** 2. 支持：数字 == 枚举 (int32 == EBuffAttribute) */
FORCEINLINE bool operator==(int32 Value, EBuffAttribute Attr)
{
	return Value == static_cast<int32>(Attr);
}

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