// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterDataComponent.h"
 
#include "Kismet/GameplayStatics.h"

 
UCharacterDataComponent::UCharacterDataComponent()
{
 
	PrimaryComponentTick.bCanEverTick = false;
	Attributes[AttributeEnum::HP]=1;
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
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red,   FString::Printf(TEXT("--------------CostCurrentHP:%d------------------"),value));
	Hp = FMath::Clamp(Hp - value, 0, GetMaxHP());
	 
}

int32 UCharacterDataComponent::GetAttribute(int32 AttributeID)
{
	 
	// 检查：不能小于0，且必须小于枚举的最大值
	if (AttributeID >= 0 && AttributeID <  AttributeEnum::MAX)
	{
		 
		 return Attributes[AttributeID];
	}
	return 0;
}

 
