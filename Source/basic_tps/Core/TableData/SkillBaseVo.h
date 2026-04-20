#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillBaseVo.generated.h"

// 仅用于 DataTable 导入，不向蓝图暴露任何属性
USTRUCT(BlueprintType)
struct FSkillBaseVo : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 ID;                  // 技能序号
	UPROPERTY(EditAnywhere)
	FString MagName;           // 技能名字
 
	 
	UPROPERTY(EditAnywhere)
	FString skillVisualDataID;  //技能视觉部分配置ID
 
	UPROPERTY(EditAnywhere)
	int32 level;               // 技能等级
	UPROPERTY(EditAnywhere)
	int32 effectLevel;         // buff有效等级
	UPROPERTY(EditAnywhere)
	int32 TargetType;          // 0敌方 1自己 2友方
	UPROPERTY(EditAnywhere)
	int32 GroupTargets;        // 群体目标
 
	UPROPERTY(EditAnywhere)
	int32 allowMiss;           // 允许miss
	UPROPERTY(EditAnywhere)
	int32 EffectType;          // 效果类型
 
	UPROPERTY(EditAnywhere)
	int32 Spell;               // 每次使用技能使用的魔法值
	UPROPERTY(EditAnywhere)
	TArray<int32> Power;       // 技能的伤害值参数
	UPROPERTY(EditAnywhere)
	int32 needLevel;           // 技能升级需要玩家等级
	UPROPERTY(EditAnywhere)
	int32 needExp;             // 技能升级需要技能经验
	UPROPERTY(EditAnywhere)
	int32 distanceTarget;      // 释放距离
	UPROPERTY(EditAnywhere)
	int32 CD;                  // 冷却延迟
	UPROPERTY(EditAnywhere)
	int32 MaxCharges;                  // 冷却延迟
	UPROPERTY(EditAnywhere)
	FString icon;              // 图标
	UPROPERTY(EditAnywhere)
	FString desc;              // 描述
	UPROPERTY(EditAnywhere)
	TArray<int32> extraValues; // 特殊自定义参数

};