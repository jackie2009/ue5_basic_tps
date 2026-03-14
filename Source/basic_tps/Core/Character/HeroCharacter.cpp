// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroCharacter.h"

#include "basic_tps/Core/Data/CharacterDataComponent.h"
#include "basic_tps/Core/Data/HeroDataComponent.h"
 


// Sets default values
AHeroCharacter::AHeroCharacter()
{
	 
}

// Called when the game starts or when spawned
void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
 
}

void AHeroCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
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

 
 
