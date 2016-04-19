// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
#pragma once


#include "UTLocalMessage.generated.h"

UCLASS(Blueprintable, Abstract, NotPlaceable, meta = (ShowWorldContextPin))
class UNREALTOURNAMENT_API UUTLocalMessage : public ULocalMessage
{
	GENERATED_UCLASS_BODY()

	/** Message area on HUD (index into UTHUD.MessageOffset[]) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	FName MessageArea;

	/** Message area on HUD (index into UTHUD.MessageOffset[]) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
		FName MessageSlot;

	/** Can be used by a message zone to stylize an incoming localized message */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	FName StyleTag;

	/** If true, this is a Game Status Announcement */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Message)
	uint32 bIsStatusAnnouncement : 1;

	// If true, don't add to normal queue.  
	UPROPERTY(EditDefaultsOnly, Category = Message)
	uint32 bIsSpecial:1;

	// If true and special, only one can be in the HUD queue at a time.
	UPROPERTY(EditDefaultsOnly, Category = Message)
	uint32 bIsUnique:1;    

	// If true and special, only one can be in the HUD queue with the same switch value
	UPROPERTY(EditDefaultsOnly, Category = Message)
	uint32 bIsPartiallyUnique:1;    

	// If true, put a GetString on the console.
	UPROPERTY(EditDefaultsOnly, Category = Message)
	uint32 bIsConsoleMessage:1;    

	// if true, allow overriding by anything else 
	UPROPERTY(EditDefaultsOnly, Category = Message)
	uint32 bOptionalSpoken : 1;

	// for announcements - will request a bot reaction after playing this announcement. 
	UPROPERTY(EditDefaultsOnly, Category = Message)
	uint32 bWantsBotReaction : 1;

	/** Use special deathmessage handling. */
	UPROPERTY(EditDefaultsOnly, Category = Message)
		uint32 bDrawAsDeathMessage : 1;

	/** Continue to display this message at intermission. */
	UPROPERTY(EditDefaultsOnly, Category = Message)
		uint32 bDrawAtIntermission : 1;

	// # of seconds to stay in HUD message queue.
	UPROPERTY(EditDefaultsOnly, Category = Message)
	float Lifetime;    

	// how long to delay playing this announcement
	UPROPERTY(EditDefaultsOnly, Category = Message)
	float AnnouncementDelay;

	UPROPERTY(EditDefaultsOnly, Category = Message)
		float ScaleInSize;

	/** 0=smallest font, 3=largest font. */
	UPROPERTY(EditDefaultsOnly, Category = Message)
		int32 FontSizeIndex;

	virtual int32 GetFontSizeIndex(int32 MessageIndex) const;

	virtual float GetAnnouncementDelay(int32 Switch);

	virtual bool ShouldPlayAnnouncement(const FClientReceiveData& ClientData) const;

	virtual void ClientReceive(const FClientReceiveData& ClientData) const override;

	virtual void GetEmphasisText(FText& PrefixText, FText& EmphasisText, FText& PostfixText, FLinearColor& EmphasisColor, int32 Switch, class APlayerState* RelatedPlayerState_1, class APlayerState* RelatedPlayerState_2, class UObject* OptionalObject) const;

	/** Returns full string built from the parts generated by GetEmphasisText(). */
	virtual FText BuildEmphasisText(int32 Switch, class APlayerState* RelatedPlayerState_1, class APlayerState* RelatedPlayerState_2, class UObject* OptionalObject) const;

	UFUNCTION(BlueprintImplementableEvent, Category = Message)
	void OnClientReceive(APlayerController* LocalPC, int32 Switch, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Message)
	FText ResolveMessage(int32 Switch = 0, bool bTargetsPlayerState1 = false, class APlayerState* RelatedPlayerState_1 = NULL, class APlayerState* RelatedPlayerState_2 = NULL, class UObject* OptionalObject = NULL) const;

	/** return the spacing to wait before playing this announcement if directly following another announcement. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	float GetAnnouncementSpacing(int32 Switch, const UObject* OptionalObject) const;

	/** return the name of announcement to play for this message (if any); UTAnnouncer will map to an actual sound */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	FName GetAnnouncementName(int32 Switch, const UObject* OptionalObject) const;

	/** return the sound to play for the announcement, if GetAnnouncementName() returned NAME_Custom. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	USoundBase* GetAnnouncementSound(int32 Switch, const UObject* OptionalObject) const;

	/** Return true if local player is viewing first or second playerstate. */
	virtual bool IsLocalForAnnouncement(const FClientReceiveData& ClientData, bool bCheckFirstPS, bool bCheckSecondPS) const;

	/** Return color of displayed message. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	FLinearColor GetMessageColor(int32 MessageIndex) const;

	/** How long to scale to 1 from initial message scale. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	float GetScaleInTime(int32 MessageIndex) const;

	/** Initial Message scale */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	float GetScaleInSize(int32 MessageIndex) const;

	/** if true, if sent to HUD multiple times, count up instances (only if bIsUnique) */
	/** return whether this announcement should be cancelled by the passed in announcement */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	bool ShouldCountInstances(int32 MessageIndex, UObject* OptionalObject) const;

	/** return whether this announcement should interrupt/cancel the passed in announcement */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	bool InterruptAnnouncement(int32 Switch, const UObject* OptionalObject, TSubclassOf<UUTLocalMessage> OtherMessageClass, int32 OtherSwitch, const UObject* OtherOptionalObject) const;

	/** return whether this announcement should be cancelled by the passed in announcement */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = Message)
	bool CancelByAnnouncement(int32 Switch, const UObject* OptionalObject, TSubclassOf<UUTLocalMessage> OtherMessageClass, int32 OtherSwitch, const UObject* OtherOptionalObject) const;

	/** called when the UTAnnouncer plays the announcement sound - can be used to e.g. display HUD text at the same time */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Message)
	void OnAnnouncementPlayed(int32 Switch, const UObject* OptionalObject) const;

	/** called to precache announcer sounds at startup, since they're dynamically loaded to enable custom announcer packs */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Message)
	void PrecacheAnnouncements(class UUTAnnouncer* Announcer) const;

	virtual void GetArgs(FFormatNamedArguments& Args, int32 Switch = 0, bool bTargetsPlayerState1 = false, class APlayerState* RelatedPlayerState_1 = NULL, class APlayerState* RelatedPlayerState_2 = NULL, class UObject* OptionalObject = NULL) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Message)
	virtual FText GetText(int32 Switch = 0, bool bTargetsPlayerState1 = false, class APlayerState* RelatedPlayerState_1 = NULL, class APlayerState* RelatedPlayerState_2 = NULL, class UObject* OptionalObject = NULL) const;

	/**	Give Blueprints a way to override the text generated by this local message. */
	UFUNCTION(BlueprintNativeEvent, Category = Message)
	FText Blueprint_GetText(int32 Switch, bool bTargetsPlayerState1, class APlayerState* RelatedPlayerState_1, class APlayerState* RelatedPlayerState_2, class UObject* OptionalObject) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Message)
	virtual float GetLifeTime(int32 Switch) const;

	/** Range of 0 to 1, affects where announcement is inserted into pending announcements queue. */
	virtual float GetAnnouncementPriority(int32 Switch) const;

	/**	Give Blueprints a way to override the lifetime for this message */
	UFUNCTION(BlueprintNativeEvent, Category = Message)
	float Blueprint_GetLifeTime(int32 Switch) const;

	virtual bool IsConsoleMessage(int32 Switch) const;
	bool PartiallyDuplicates(int32 Switch1, int32 Switch2, class UObject* OptionalObject1, class UObject* OptionalObject2 ) const;
};



