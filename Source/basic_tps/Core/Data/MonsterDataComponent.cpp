// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterDataComponent.h"

#include "CharacterDataComponent.h"
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/TableData/MonsterBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

 
UMonsterDataComponent::UMonsterDataComponent()
{
 
	PrimaryComponentTick.bCanEverTick = false;


	 
 
}

void UMonsterDataComponent::BeginPlay()
{
	Super::BeginPlay();
     
	 
 }
 
  void UMonsterDataComponent::InitWithBaseID(int32 BaseID)
{
	auto* TableManager = UTableDataManagerSubsystem::Get();
    
	if (!TableManager) return; // 如果在构造函数或非法时机调用，这里会直接拦截
 
	 auto dataPtr=TableManager->MonsterBaseMap.Find(BaseID);
	 if (!dataPtr) return;
	  baseVo= *dataPtr;
	 
	CalBaseAttributes();

	auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;
    
	 
	if (!BaseDataComp) return;
	BaseDataComp->displayName=baseVo->Name;
	BaseDataComp->SetCurrentHP(BaseDataComp->GetMaxHP());
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,   TEXT("--------------------------------"));
}
void UMonsterDataComponent::CalBaseAttributes()
{
	// 2. 缓存 Owner 上的组件。现实工程中，如果这个函数调用频繁，建议把 baseDataComp 变成成员变量并在 BeginPlay 缓存
	auto* BaseDataComp = GetOwner() ? GetOwner()->FindComponentByClass<UCharacterDataComponent>() : nullptr;
    
	// 3. 必须对目标组件进行有效性检查，防止在某些极端生命周期下崩溃
	if (!BaseDataComp) return;
	 
	// 4. 重置数据：UE 原生数组或 TArray 建议用更直观的方式
	// 假设 Attributes 是 int32 Attributes[AttributeEnum::MAX] 
	FMemory::Memzero(BaseDataComp->Attributes, sizeof(BaseDataComp->Attributes));
	
	const int32 CopyCount = FMath::Min((int32)AttributeEnum::MAX, baseVo->Attributes.Num());
	
	for (int32 i = 0; i < CopyCount; ++i)
	{
		BaseDataComp->Attributes[i+1] = baseVo->Attributes[i];
	}

}

  

