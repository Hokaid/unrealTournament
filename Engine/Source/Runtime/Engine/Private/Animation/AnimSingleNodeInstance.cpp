// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UAnimSingleNodeInstance.cpp: Single Node Tree Instance 
	Only plays one animation at a time. 
=============================================================================*/ 

#include "EnginePrivate.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/VertexAnim/VertexAnimation.h"
#include "AnimationRuntime.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpaceBase.h"
#include "Animation/AnimComposite.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AimOffsetBlendSpace1D.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"

/////////////////////////////////////////////////////
// UAnimSingleNodeInstance
/////////////////////////////////////////////////////

UAnimSingleNodeInstance::UAnimSingleNodeInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAnimSingleNodeInstance::SetAnimationAsset(class UAnimationAsset* NewAsset, bool bInIsLooping, float InPlayRate)
{
	FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();
	Proxy.SetAnimationAsset(NewAsset, GetSkelMeshComponent(), bInIsLooping, InPlayRate);

	UAnimMontage* Montage = Cast<UAnimMontage>(NewAsset);
	if ( Montage!=NULL )
	{
		Proxy.ReinitializeSlotNodes();
		if ( Montage->SlotAnimTracks.Num() > 0 )
		{
			Proxy.RegisterSlotNodeWithAnimInstance(Montage->SlotAnimTracks[0].SlotName);
		}
		RestartMontage( Montage );
		SetPlaying(IsPlaying());
	}
	else
	{
		// otherwise stop all montages
		StopAllMontages(0.25f);

		UBlendSpaceBase * BlendSpace = Cast<UBlendSpaceBase>(NewAsset);
		if(BlendSpace)
		{
			BlendSpace->InitializeFilter(&BlendFilter);
		}
	}
}

void UAnimSingleNodeInstance::SetVertexAnimation(UVertexAnimation * NewVertexAnim, bool bIsLooping, float InPlayRate)
{
	GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().SetVertexAnimation(NewVertexAnim, bIsLooping, InPlayRate);

	// reinitialize
	InitializeAnimation();
}


void UAnimSingleNodeInstance::SetMontageLoop(UAnimMontage* Montage, bool bIsLooping, FName StartingSection)
{
	check (Montage);

	int32 TotalSection = Montage->CompositeSections.Num();
	if( TotalSection > 0 )
	{
		if (StartingSection == NAME_None)
		{
			StartingSection = Montage->CompositeSections[0].SectionName;
		}
		FName FirstSection = StartingSection;
		FName LastSection = StartingSection;

		bool bSucceeded = false;
		// find last section
		int32 CurSection = Montage->GetSectionIndex(FirstSection);

		int32 Count = TotalSection;
		while( Count-- > 0 )
		{
			FName NewLastSection = Montage->CompositeSections[CurSection].NextSectionName;
			CurSection = Montage->GetSectionIndex(NewLastSection);

			if( CurSection != INDEX_NONE )
			{
				// used to rebuild next/prev
				Montage_SetNextSection(LastSection, NewLastSection);
				LastSection = NewLastSection;
			}
			else
			{
				bSucceeded = true;
				break;
			}
		}

		if( bSucceeded )
		{
			if ( bIsLooping )
			{
				Montage_SetNextSection(LastSection, FirstSection);
			}
			else
			{
				Montage_SetNextSection(LastSection, NAME_None);
			}
		}
		// else the default is already looping
	}
}

void UAnimSingleNodeInstance::UpdateMontageWeightForTimeSkip(float TimeDifference)
{
	Montage_UpdateWeight(TimeDifference);
}

void UAnimSingleNodeInstance::UpdateBlendspaceSamples(FVector InBlendInput)
{
	GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().UpdateBlendspaceSamples(InBlendInput);
}

void UAnimSingleNodeInstance::RestartMontage(UAnimMontage* Montage, FName FromSection)
{
	FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();

	if( Montage == Proxy.GetCurrentAsset() )
	{
		Proxy.ResetWeightInfo();
		Montage_Play(Montage, Proxy.GetPlayRate());
		if( FromSection != NAME_None )
		{
			Montage_JumpToSection(FromSection);
		}
		SetMontageLoop(Montage, Proxy.IsLooping(), FromSection);
	}
}

void UAnimSingleNodeInstance::NativeInitializeAnimation()
{
	USkeletalMeshComponent* SkelComp = GetSkelMeshComponent();
	SkelComp->AnimationData.Initialize(this);
}

void UAnimSingleNodeInstance::NativePostEvaluateAnimation()
{
	PostEvaluateAnimEvent.ExecuteIfBound();

	Super::NativePostEvaluateAnimation();
}

void UAnimSingleNodeInstance::OnMontageInstanceStopped(FAnimMontageInstance& StoppedMontageInstance) 
{
	FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();
	if (StoppedMontageInstance.Montage == Proxy.GetCurrentAsset())
	{
		Proxy.SetCurrentTime(StoppedMontageInstance.GetPosition());
	}

	Super::OnMontageInstanceStopped(StoppedMontageInstance);
}

void UAnimSingleNodeInstance::Montage_Advance(float DeltaTime)
{
	Super::Montage_Advance(DeltaTime);
		
	FAnimMontageInstance * CurMontageInstance = GetActiveMontageInstance();
	if ( CurMontageInstance )
	{
		FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();
		Proxy.SetCurrentTime(CurMontageInstance->GetPosition());
	}
}

void UAnimSingleNodeInstance::PlayAnim(bool bIsLooping, float InPlayRate, float InStartPosition)
{
	SetPlaying(true);
	SetLooping(bIsLooping);
	SetPlayRate(InPlayRate);
	SetPosition(InStartPosition);
}

void UAnimSingleNodeInstance::StopAnim()
{
	SetPlaying(false);
}

void UAnimSingleNodeInstance::SetLooping(bool bIsLooping)
{
	FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();
	Proxy.SetLooping(bIsLooping);

	if (UAnimMontage* Montage = Cast<UAnimMontage>(GetCurrentAsset()))
	{
		SetMontageLoop(Montage, Proxy.IsLooping(), Montage_GetCurrentSection());
	}
}

void UAnimSingleNodeInstance::SetPlaying(bool bIsPlaying)
{
	FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();
	Proxy.SetPlaying(bIsPlaying);

	if (FAnimMontageInstance* CurMontageInstance = GetActiveMontageInstance())
	{
		CurMontageInstance->bPlaying = bIsPlaying;
	}
	else if (Proxy.IsPlaying())
	{
		UAnimMontage* Montage = Cast<UAnimMontage>(GetCurrentAsset());
		if (Montage)
		{
			RestartMontage(Montage);
		}
	}
}

bool UAnimSingleNodeInstance::IsPlaying() const
{
	// since setPlaying is setting to montage, we should get it as symmmetry
	if (FAnimMontageInstance* CurMontageInstance = GetActiveMontageInstance())
	{
		return CurMontageInstance->bPlaying;
	}

	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().IsPlaying();
}

bool UAnimSingleNodeInstance::IsReverse() const
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().IsReverse();
}

bool UAnimSingleNodeInstance::IsLooping() const
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().IsLooping();
}

float UAnimSingleNodeInstance::GetPlayRate() const
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().GetPlayRate();
}

void UAnimSingleNodeInstance::SetPlayRate(float InPlayRate)
{
	GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().SetPlayRate(InPlayRate);

	if (FAnimMontageInstance* CurMontageInstance = GetActiveMontageInstance())
	{
		CurMontageInstance->SetPlayRate(InPlayRate);
	}
}

UAnimationAsset* UAnimSingleNodeInstance::GetCurrentAsset()
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().GetCurrentAsset();
}

UVertexAnimation* UAnimSingleNodeInstance::GetCurrentVertexAnimation()
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().GetCurrentVertexAnimation();
}

float UAnimSingleNodeInstance::GetCurrentTime() const
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().GetCurrentTime();
}

void UAnimSingleNodeInstance::SetReverse(bool bInReverse)
{
	GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().SetReverse(bInReverse);
}

void UAnimSingleNodeInstance::SetPosition(float InPosition, bool bFireNotifies)
{
	FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();

	float PreviousTime = Proxy.GetCurrentTime();
	Proxy.SetCurrentTime(FMath::Clamp<float>(InPosition, 0.f, GetLength()));

	if (FAnimMontageInstance* CurMontageInstance = GetActiveMontageInstance())
	{
		CurMontageInstance->SetPosition(Proxy.GetCurrentTime());
	}

	// Handle notifies
	// the way AnimInstance handles notifies doesn't work for single node because this does not tick or anything
	// this will need to handle manually, emptying, it and collect it, and trigger them at once. 
	if (bFireNotifies)
	{
		UAnimSequenceBase * SequenceBase = Cast<UAnimSequenceBase> (Proxy.GetCurrentAsset());
		if (SequenceBase)
		{
			NotifyQueue.Reset(GetSkelMeshComponent());

			TArray<const FAnimNotifyEvent*> Notifies;
			SequenceBase->GetAnimNotifiesFromDeltaPositions(PreviousTime, Proxy.GetCurrentTime(), Notifies);
			if ( Notifies.Num() > 0 )
			{
				// single node instance only has 1 asset at a time
				NotifyQueue.AddAnimNotifies(Notifies, 1.0f);
			}

			TriggerAnimNotifies(0.f);

		}
	}
}

void UAnimSingleNodeInstance::SetBlendSpaceInput(const FVector& InBlendInput)
{
	GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().SetBlendSpaceInput(InBlendInput);
}

float UAnimSingleNodeInstance::GetLength()
{
	return GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>().GetLength();
}

void UAnimSingleNodeInstance::StepForward()
{
	if (UAnimSequence* Sequence = Cast<UAnimSequence>(GetCurrentAsset()))
	{
		FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();

		float KeyLength = Sequence->SequenceLength/Sequence->NumFrames+SMALL_NUMBER;
		float Fraction = (Proxy.GetCurrentTime()+KeyLength)/Sequence->SequenceLength;
		int32 Frames = FMath::Clamp<int32>((float)(Sequence->NumFrames*Fraction), 0, Sequence->NumFrames);
		SetPosition(Frames*KeyLength);
	}	
}

void UAnimSingleNodeInstance::StepBackward()
{
	if (UAnimSequence* Sequence = Cast<UAnimSequence>(GetCurrentAsset()))
	{
		FAnimSingleNodeInstanceProxy& Proxy = GetProxyOnGameThread<FAnimSingleNodeInstanceProxy>();

		float KeyLength = Sequence->SequenceLength/Sequence->NumFrames+SMALL_NUMBER;
		float Fraction = (Proxy.GetCurrentTime()-KeyLength)/Sequence->SequenceLength;
		int32 Frames = FMath::Clamp<int32>((float)(Sequence->NumFrames*Fraction), 0, Sequence->NumFrames);
		SetPosition(Frames*KeyLength);
	}
}

FAnimInstanceProxy* UAnimSingleNodeInstance::CreateAnimInstanceProxy()
{
	return new FAnimSingleNodeInstanceProxy(this);
}
