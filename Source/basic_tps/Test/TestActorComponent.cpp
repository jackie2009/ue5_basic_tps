// Fill out your copyright notice in the Description page of Project Settings.


#include "TestActorComponent.h"

#include "../Core/TableData/SkillBaseVo.h"
#include "basic_tps/Core/Data//CombatTypes.h"
#include "basic_tps/Core/TableData/MonsterBaseVo.h"
#include "basic_tps/Core/TableData/RoleBaseVo.h"
#include "basic_tps/Core/TableData/RoleLevelBaseVo.h"
#include "basic_tps/Core/TableData/TableDataManagerSubsystem.h"
#include "basic_tps/Core/TableData/WeaponTypeBaseVo.h"
#include "eventtest/EventBusSubsystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UTestActorComponent::UTestActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}
const FString TableFolder=TEXT("/Game/TableData");

// Called when the game starts
void UTestActorComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GI =  UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UEventBusSubsystem* Bus = GI->GetSubsystem<UEventBusSubsystem>())
		{
			OnToastHandle=  Bus->Subscribe("UI.ShowToast",
				[this](const FEventBusMessage& Msg)
				{
					UE_LOG(LogTemp, Log, TEXT("Toast: %s"), *Msg.StringValue);
				});
		}
	}

	
	auto filePath= FPaths::Combine(TableFolder, "SkillBaseVo");
	auto tdmss= UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UTableDataManagerSubsystem>();
	if (TArray<FSkillBaseVo*>* SkillGroupPtr = tdmss->SkillBaseMap.Find(201))
	{
		for (FSkillBaseVo* SkillVo : *SkillGroupPtr)
		{

		//	GEngine->AddOnScreenDebugMessage(-1,5,FColor::Green,FString::Printf( TEXT("ddd,%s,%d"),*SkillVo->MagName,	SkillVo->ID));
		}
		
	}

 
	 
}

void UTestActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UEventBusSubsystem* Bus = GI->GetSubsystem<UEventBusSubsystem>())
		{
			Bus->Unsubscribe("UI.ShowToast", OnToastHandle);
		}
	}

 
}


// Called every frame
void UTestActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

 

 
}

