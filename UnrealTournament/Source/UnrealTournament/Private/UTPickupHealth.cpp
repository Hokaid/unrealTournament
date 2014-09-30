// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTPickup.h"
#include "UTPickupHealth.h"

AUTPickupHealth::AUTPickupHealth(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	HealAmount = 25;
	BaseDesireability = 0.4f;
}

int32 AUTPickupHealth::GetHealMax_Implementation(AUTCharacter* P)
{
	if (P == NULL)
	{
		return 0;
	}
	else
	{
		return bSuperHeal ? P->SuperHealthMax : P->HealthMax;
	}
}

void AUTPickupHealth::ProcessTouch_Implementation(APawn* TouchedBy)
{
	if (Role == ROLE_Authority)
	{
		AUTCharacter* P = Cast<AUTCharacter>(TouchedBy);
		if (P != NULL && (bSuperHeal || P->Health < GetHealMax(P)))
		{
			Super::ProcessTouch_Implementation(TouchedBy);
		}
	}
}

void AUTPickupHealth::GiveTo_Implementation(APawn* Target)
{
	AUTCharacter* P = Cast<AUTCharacter>(Target);
	if (P != NULL)
	{
		P->Health = FMath::Max<int32>(P->Health, FMath::Min<int32>(P->Health + HealAmount, GetHealMax(P)));
	}
}

float AUTPickupHealth::BotDesireability_Implementation(APawn* Asker, float PathDistance)
{
	AUTCharacter* P = Cast<AUTCharacter>(Asker);
	if (P == NULL)
	{
		return 0.0f;
	}
	else
	{
		float Desire = FMath::Min<int32>(P->Health + HealAmount, GetHealMax(P)) - P->Health;

		if (P->GetWeapon() != NULL && P->GetWeapon()->BaseAISelectRating > 0.5f)
		{
			Desire *= 1.7f;
		}
		if (bSuperHeal || P->Health < 45)
		{
			Desire = FMath::Min<float>(0.025f * Desire, 2.2);
			/*if (bSuperHeal && !WorldInfo.Game.bTeamGame && UTBot(C) != None && UTBot(C).Skill >= 4.0)
			{
				// high skill bots keep considering powerups that they don't need if they can still pick them up
				// to deny the enemy any chance of getting them
				desire = FMax(desire, 0.001);
			}*/
			return Desire;
		}
		else
		{
			if (Desire > 6.0f)
			{
				Desire = FMath::Max<float>(Desire, 25.0f);
			}
			//else if (UTBot(C) != None && UTBot(C).bHuntPlayer)
			//	return 0;
				
			return FMath::Min<float>(0.017f * Desire, 2.0);
		}
	}
}