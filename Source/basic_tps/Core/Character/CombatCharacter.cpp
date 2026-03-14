// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacter.h"

#include "basic_tps/Core/Data/CharacterDataComponent.h"
 


// Sets default values
ACombatCharacter::ACombatCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	 
}

// Called when the game starts or when spawned
void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();
 
}

void ACombatCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	CharacterDataComp = NewObject<UCharacterDataComponent>(this);
	CharacterDataComp->RegisterComponent();
 
}

void ACombatCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
 
 
}

// Called every frame
void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
bool ACombatCharacter::IsAlive() const
{
	return    CharacterDataComp!=nullptr && ( CharacterDataComp->GetCurrentHP()>0);
}
