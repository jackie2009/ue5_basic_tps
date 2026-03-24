// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroDataComponent.h"

#include "CharacterDataComponent.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

 
UHeroDataComponent::UHeroDataComponent()
{
 
	PrimaryComponentTick.bCanEverTick = false;


	 
 
}

void UHeroDataComponent::BeginPlay()
{
	Super::BeginPlay();
    UpdateLevel(1);
	auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;
    
	 
	if ( BaseDataComp)
	{
		BaseDataComp->SetCurrentHP(BaseDataComp->Attributes[AttributeEnum::HP]);
	}
	
 }

void UHeroDataComponent::UpdateLevel(int32 level)
{
	// 传入 this，函数会自动判断当前环境是否能拿 Subsystem
	auto* TableManager = UTableDataManagerSubsystem::Get(this);
    
	if (!TableManager) return; // 如果在构造函数或非法时机调用，这里会直接拦截
 
	if (auto dataPtr=TableManager->RoleLevelBaseMap.Find(Job*1000+level))
	{
		levelBaseVo=*dataPtr;
		 GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,  FString::Printf(TEXT("UHeroDataComponent%d"),levelBaseVo->Exp));
	}
	CalBaseAttributes();
}
  void UHeroDataComponent::CalBaseAttributes()
{
	// 1. 数据源校验（原本的判空保留，这是对的）
	if (!levelBaseVo) return;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("UHeroDataComponent_1"));
	// 2. 缓存 Owner 上的组件。现实工程中，如果这个函数调用频繁，建议把 baseDataComp 变成成员变量并在 BeginPlay 缓存
	auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;
    
	// 3. 必须对目标组件进行有效性检查，防止在某些极端生命周期下崩溃
	if (!BaseDataComp) return;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("UHeroDataComponent_2"));
	// 4. 重置数据：UE 原生数组或 TArray 建议用更直观的方式
	// 假设 Attributes 是 int32 Attributes[AttributeEnum::MAX] 
	FMemory::Memzero(BaseDataComp->Attributes, sizeof(BaseDataComp->Attributes));

	// 5. 核心逻辑：同步属性
	// 使用 FMath::Min 这种防御性写法，是主流工程中最常见的“妥协”：
	// 即便 levelBaseVo 的配置表配错了，多配或少配了，程序也不会因为数组越界而 Crash。
	const int32 CopyCount = FMath::Min((int32)AttributeEnum::MAX, levelBaseVo->Attributes.Num());

	for (int32 i = 0; i < CopyCount; ++i)
	{
		BaseDataComp->Attributes[i+1] = levelBaseVo->Attributes[i];
	}
	 
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("UHeroDataComponent_3"));
}
  

