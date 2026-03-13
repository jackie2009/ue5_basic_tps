// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacter.h"

#include "basic_tps/Core/Data/CharacterDataComponent.h"


// Sets default values
ACombatCharacter::ACombatCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterDataComp=CreateDefaultSubobject<UCharacterDataComponent>(TEXT("NewCombatData_02"));
 
	 
}

// Called when the game starts or when spawned
void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
	return   (CharacterDataComp!=nullptr)&& ((CharacterDataComp->Hp)>0);
}
