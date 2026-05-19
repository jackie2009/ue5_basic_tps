// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterDataComponent.h"
 
#include "basic_tps/Core/Character/CombatCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

 
UCharacterDataComponent::UCharacterDataComponent()
{
 
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	Attributes[AttributeEnum::HP]=1;
	SetCurrentHP(Attributes[AttributeEnum::HP]);
 
}

void UCharacterDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterDataComponent, Hp);
	DOREPLIFETIME(UCharacterDataComponent, Mp);
}

void UCharacterDataComponent::OnRep_Hp(int32 OldHp)
{
	 
	// auto Character=Cast<ACombatCharacter>(GetOwner());
	// if (!Character)return;
	// if (Hp < OldHp)
	// {
	// 	Character->SelfOnHurt(OldHp - Hp, FVector::Zero());
	// }
	// if (OldHp > 0&&Hp <= 0)
	// {
	// 	Character->SelfOnDead();
	// }
}

void UCharacterDataComponent::OnRep_Mp(int32 OldMp)
{
	 
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

bool UCharacterDataComponent::CostCurrentHP(int value) 
{
	 
	Hp = FMath::Clamp(Hp - value, 0, GetMaxHP());
	return true;
 }

int32 UCharacterDataComponent::GetCurrentMP() 
{
	return  Mp;
}

int32 UCharacterDataComponent::GetMaxMP() 
{
	return Attributes[AttributeEnum::MP];
}

void UCharacterDataComponent::SetCurrentMP(int value) 
{
	Mp=value;
 
}

void UCharacterDataComponent::AddCurrentMP(int value) 
{
	Mp = FMath::Clamp(Mp + value, 0, GetMaxMP());
 
}

bool UCharacterDataComponent::CostCurrentMP(int value) 
{
	 if (Mp<value) return false;
	Mp = FMath::Clamp(Mp - value, 0, GetMaxMP());
	return  true;
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

 
int32 UCharacterDataComponent::AddAttribute(int32 AttributeID,int32 value)
{
	 
	// 检查：不能小于0，且必须小于枚举的最大值
	if (AttributeID >= 0 && AttributeID <  AttributeEnum::MAX)
	{
		Attributes[AttributeID]+=value;
		return Attributes[AttributeID];
	}
	return 0;
}
