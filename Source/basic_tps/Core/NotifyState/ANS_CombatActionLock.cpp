// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_CombatActionLock.h"

#include "basic_tps/Core/Character/CombatCharacter.h"
#include "basic_tps/Core/Data/CharacterDataComponent.h"

// ANS_ActionLock.cpp
void UANS_CombatActionLock::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
	 
		if (ACombatCharacter* RealOwner = Cast<ACombatCharacter>(Owner))
		{
			RealOwner->CharacterDataComp->ActionTags.AddTag(LockTag);
		}
	 
	}
}

void UANS_CombatActionLock::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (ACombatCharacter* RealOwner = Cast<ACombatCharacter>(Owner))
		{
			RealOwner->CharacterDataComp->ActionTags.RemoveTag(LockTag);
		}
	}
}