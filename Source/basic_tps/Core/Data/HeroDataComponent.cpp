// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroDataComponent.h"

#include "CharacterDataComponent.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Character/HeroCharacter.h"
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
	auto* Hero = Cast<AHeroCharacter>(GetOwner());

	if (BaseDataComp)
	{
		BaseDataComp->SetCurrentHP(BaseDataComp->Attributes[AttributeEnum::HP]);
		if (Hero)
		{
			Hero->SyncStaminaZoneConfig();
			// 英雄以普通区满体力进入战斗（活力区需通过药水或增益激活）
			BaseDataComp->SetCurrentMP(Hero->GetNormalStaminaCap());
		}
		else
		{
			BaseDataComp->SetCurrentMP(BaseDataComp->Attributes[AttributeEnum::MP]);
		}
	}
}

void UHeroDataComponent::UpdateLevel(int32 level)
{
	// 传入 this，函数会自动判断当前环境是否能拿 Subsystem
	auto* TableManager = UTableDataManagerSubsystem::Get();

	if (!TableManager) return; // 如果在构造函数或非法时机调用，这里会直接拦截

	if (auto dataPtr = TableManager->RoleLevelBaseMap.Find(Job * 1000 + level))
	{
		levelBaseVo = *dataPtr;
	}
	CalBaseAttributes();
}

void UHeroDataComponent::CalBaseAttributes()
{
	// 1. 数据源校验
	if (!levelBaseVo) return;
	auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;

	// 2. 必须对目标组件进行有效性检查，防止在某些极端生命周期下崩溃
	if (!BaseDataComp) return;

	// 3. 重置数据
	FMemory::Memzero(BaseDataComp->Attributes, sizeof(BaseDataComp->Attributes));

	// 4. 同步属性
	const int32 CopyCount = FMath::Min((int32)AttributeEnum::MAX - 1, levelBaseVo->Attributes.Num());

	for (int32 i = 0; i < CopyCount; ++i)
	{
		BaseDataComp->Attributes[i + 1] = levelBaseVo->Attributes[i];
	}

	// 5. 英雄能量(MP)作为体力使用，总上限由英雄四段体力区域数组总和决定
	if (AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner()))
	{
		Hero->SyncStaminaZoneConfig();
	}
}

int32 UHeroDataComponent::GetStaminaZoneCapacity(EStaminaZone Zone) const
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	return Hero ? Hero->GetStaminaZoneCapacity(Zone) : 0;
}

EStaminaZone UHeroDataComponent::GetCurrentStaminaZone() const
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	return Hero ? Hero->GetCurrentStaminaZone() : EStaminaZone::Normal;
}

int32 UHeroDataComponent::GetNormalStaminaCap() const
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	return Hero ? Hero->GetNormalStaminaCap() : 0;
}

int32 UHeroDataComponent::GetTotalStaminaMax() const
{
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwner());
	return Hero ? Hero->GetTotalStaminaMax() : 0;
}
