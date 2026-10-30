// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"
#include "basic_tps/Core/Targeting/CombatTargetingComponent.h"
#include "basic_tps/Core/Presentation/TargetOutlineComponent.h"

#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/HeroDataComponent.h"
 


// Sets default values
AHeroCharacter::AHeroCharacter()
{
	TargetingComp->bAutoSelect = true;
	TargetOutlineComp = CreateDefaultSubobject<UTargetOutlineComponent>(TEXT("TargetOutline"));
	TargetOutlineComp->AddTickPrerequisiteComponent(TargetingComp);
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
 
}

void AHeroCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	CharacterDataComp->Faction = ECombatFaction::Hero;
	HeroDataComp= NewObject<UHeroDataComponent>(this);
	HeroDataComp->RegisterComponent();
 
}

void AHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
 
}

// Called every frame
void AHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

 
 
