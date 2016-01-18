// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTAnnouncer.generated.h"

USTRUCT(BlueprintType)
struct FAnnouncementInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = Announcement)
	TSubclassOf<class UUTLocalMessage> MessageClass;
	UPROPERTY(BlueprintReadWrite, Category = Announcement)
	int32 Switch;
	UPROPERTY(BlueprintReadWrite, Category = Announcement)
	const class APlayerState* RelatedPlayerState_1;
	UPROPERTY(BlueprintReadWrite, Category = Announcement)
	const class APlayerState* RelatedPlayerState_2;
	UPROPERTY(BlueprintReadWrite, Category = Announcement)
	const UObject* OptionalObject;

	FAnnouncementInfo()
		: MessageClass(NULL), Switch(0), RelatedPlayerState_1(NULL), RelatedPlayerState_2(NULL), OptionalObject(NULL)
	{}
	FAnnouncementInfo(TSubclassOf<UUTLocalMessage> InMessageClass, int32 InSwitch, const class APlayerState* InRelatedPlayerState_1, const class APlayerState* InRelatedPlayerState_2, const UObject* InOptionalObject)
		: MessageClass(InMessageClass), Switch(InSwitch), RelatedPlayerState_1(InRelatedPlayerState_1), RelatedPlayerState_2(InRelatedPlayerState_2), OptionalObject(InOptionalObject)
	{}
};

USTRUCT(BlueprintType)
struct FAnnouncerSound
{
	GENERATED_USTRUCT_BODY()

	/** the generic name of the sound that will be used to look up the audio */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	FName SoundName;
	/** reference to the actual sound object */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundBase* Sound;
};

UENUM()
enum EAnnouncerType
{
	AT_Reward, // only supports reward announcements (multikill, etc)
	AT_Status, // only supports status announcements (red flag taken, etc)
	AT_All, // supports all announcements
};

UCLASS(Blueprintable, Abstract, Within=UTPlayerController)
class UNREALTOURNAMENT_API UUTAnnouncer : public UObject
{
	GENERATED_UCLASS_BODY()

	virtual class UWorld* GetWorld() const override
	{
		return GetOuter()->GetWorld();
	}

	/** type of announcements supported */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	TEnumAsByte<EAnnouncerType> Type;

	/** audio path containing the announcer audio; all audio in this path must match the SoundName used by the various message types in order to be found */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	FString RewardAudioPath;

	/** additional prefix for all sound names (since it needs to be applied twice - to file name and to asset name) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	FString RewardAudioNamePrefix;

	/** audio path containing the announcer audio; all audio in this path must match the SoundName used by the various message types in order to be found */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	FString StatusAudioPath;

	/** additional prefix for all sound names (since it needs to be applied twice - to file name and to asset name) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	FString StatusAudioNamePrefix;

	/** array allowing manually matching SoundName to sound in case the naming convention wasn't followed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	TArray<FAnnouncerSound> RewardAudioList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Announcer)
	TArray<FAnnouncerSound> StatusAudioList;

	/** amount of time between the end of one announcement and the start of the next when there is a queue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Announcer)
	float Spacing;

	FTimerHandle PlayNextAnnouncementHandle;

	/** fast lookup to audio we've used previously */
	TMap<FName, USoundBase*> RewardCachedAudio;

	/** fast lookup to audio we've used previously */
	TMap<FName, USoundBase*> StatusCachedAudio;

	/** currently playing announcement */
	UPROPERTY(Transient)
	FAnnouncementInfo CurrentAnnouncement;

	/** list of queued announcements */
	UPROPERTY(Transient)
	TArray<FAnnouncementInfo> QueuedAnnouncements;

	/** Set if the announcement wants a reaction from bots after being played. */
	UPROPERTY(Transient)
	FAnnouncementInfo ReactionAnnouncement;

	/** Find best bot to provide reaction. */
	virtual void RequestReaction();

	/** component used to control the audio */
	UPROPERTY()
	UAudioComponent* AnnouncementComp;

	virtual void PostInitProperties() override;

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
	{
		Super::AddReferencedObjects(InThis, Collector);

		UUTAnnouncer* AnnThis = Cast<UUTAnnouncer>(InThis);
		for (TMap<FName, USoundBase*>::TIterator It(AnnThis->RewardCachedAudio); It; ++It)
		{
			Collector.AddReferencedObject(It.Value(), AnnThis);
		}
		for (TMap<FName, USoundBase*>::TIterator It(AnnThis->StatusCachedAudio); It; ++It)
		{
			Collector.AddReferencedObject(It.Value(), AnnThis);
		}
	}

	UFUNCTION(BlueprintCallable, Category = Announcement)
	virtual void PlayAnnouncement(TSubclassOf<UUTLocalMessage> MessageClass, int32 Switch, const APlayerState* PlayerState1, const APlayerState* PlayerState2, const UObject* OptionalObject);

	/** play next announcement in queue (if any)
	 * cancels any currently playing announcement
	 */
	UFUNCTION(BlueprintCallable, Category = Announcement)
	virtual void PlayNextAnnouncement();

	/** load and cache reference to announcement sound (if it exists) without playing it */
	UFUNCTION(BlueprintCallable, Category = Announcement)
	virtual void PrecacheAnnouncement(FName SoundName);

protected:
	/** called when announcement audio ends (set to AudioComponent delegate) */
	UFUNCTION()
	virtual void AnnouncementFinished();

	USoundBase* LoadAudio(FString NewAudioPath, FString NewAudioNamePrefix, FName SoundName);

};