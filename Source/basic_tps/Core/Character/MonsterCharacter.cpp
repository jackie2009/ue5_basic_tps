// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterCharacter.h"

#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/MonsterDataComponent.h"
 


// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
	MonsterDataComp=CreateDefaultSubobject<UMonsterDataComponent>(TEXT("MonsterDataComp"));
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
 
}

void AMonsterCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
 
}

void AMonsterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (MonsterBaseID>0&&MonsterDataComp!=nullptr)
	{
	

		MonsterDataComp->InitWithBaseID(MonsterBaseID);
	}
}

// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

 
 
