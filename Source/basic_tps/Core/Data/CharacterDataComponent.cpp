// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterDataComponent.h"
 
#include "Kismet/GameplayStatics.h"

 
UCharacterDataComponent::UCharacterDataComponent()
{
 
	PrimaryComponentTick.bCanEverTick = false;
	Attributes[AttributeEnum::HP]=100;
	SetCurrentHP(Attributes[AttributeEnum::HP]);
 
}

int32 UCharacterDataComponent::GetCurrentHP() 
{
	return  Hp;
}

int32 UCharacterDataComponent::GetMaxHP() 
{
	return Attributes[AttributeEnum::HP];
}

void UCharacterDataComponent::SetCurrentHP(int value) 
{
	Hp=value;
}

void UCharacterDataComponent::AddCurrentHP(int value) 
{
	Hp = FMath::Clamp(Hp + value, 0, GetMaxHP());
}

void UCharacterDataComponent::CostCurrentHP(int value) 
{
	Hp = FMath::Clamp(Hp - value, 0, GetMaxHP());
}
  

