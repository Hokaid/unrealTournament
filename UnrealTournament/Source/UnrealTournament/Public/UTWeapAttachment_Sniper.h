// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTWeaponAttachment.h"
#include "Particles/ParticleSystemComponent.h"
#include "UTWeapAttachment_Sniper.generated.h"

UCLASS(CustomConstructor)
class AUTWeapAttachment_Sniper : public AUTWeaponAttachment
{
	GENERATED_UCLASS_BODY()

	AUTWeapAttachment_Sniper(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
	{}
};
