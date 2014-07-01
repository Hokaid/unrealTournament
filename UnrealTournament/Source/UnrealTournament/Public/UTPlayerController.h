// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTPlayerController.generated.h"

UCLASS(dependson=UTCharacter, dependson=UTPlayerState, config=Game)
class AUTPlayerController : public APlayerController, public IUTTeamInterface
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY()
	AUTCharacter* UTCharacter;

public:

	UFUNCTION(BlueprintCallable, Category = PlayerController)
	virtual AUTCharacter* GetUTCharacter();

	UPROPERTY()
	AUTPlayerState* UTPlayerState;

	UPROPERTY()
	class AUTHUD* MyUTHUD;

	// the announcer types are split as we use different voices for the two types;
	// this allows them independent queues with limited talking over each other, which is better than the long queues that can sometimes happen
	// more configurability for mods and such doesn't hurt either

	UPROPERTY(Config, BlueprintReadWrite, Category = Announcer)
	FStringClassReference RewardAnnouncerPath;

	/** announcer for reward announcements (multikill, etc) - only set on client */
	UPROPERTY(BlueprintReadWrite, Category = Announcer)
	class UUTAnnouncer* RewardAnnouncer;

	UPROPERTY(Config, BlueprintReadWrite, Category = Announcer)
	FStringClassReference StatusAnnouncerPath;

	/** announcer for status announcements (red flag taken, etc) - only set on client */
	UPROPERTY(BlueprintReadWrite, Category = Announcer)
	class UUTAnnouncer* StatusAnnouncer;

	virtual void InitInputSystem() OVERRIDE;
	virtual void InitPlayerState();
	virtual void OnRep_PlayerState();
	virtual void SetPawn(APawn* InPawn);
	virtual void SetupInputComponent() OVERRIDE;

	virtual void CheckAutoWeaponSwitch(AUTWeapon* TestWeapon);

	/** check if sound is audible to this player and call ClientHearSound() if so to actually play it
	 * SoundPlayer may be NULL
	 */
	virtual void HearSound(USoundBase* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, bool bStopWhenOwnerDestroyed);

	/** plays a heard sound locally
	 * SoundPlayer may be NULL for an unattached sound
	 * if SoundLocation is zero then the sound should be attached to SoundPlayer
	 */
	UFUNCTION(client, unreliable)
	void ClientHearSound(USoundBase* TheSound, AActor* SoundPlayer, FVector SoundLocation, bool bStopWhenOwnerDestroyed, bool bOccluded);

	virtual void SwitchToBestWeapon();

	inline void AddWeaponPickup(class AUTPickupWeapon* NewPickup)
	{
		// insert new pickups at the beginning so the order should be newest->oldest
		// this makes iteration and removal faster when deciding whether the pickup is still hidden in the per-frame code
		RecentWeaponPickups.Insert(NewPickup, 0);
	}

	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& HiddenComponents);

	virtual void SetName(const FString& S);

	UFUNCTION(exec)
	virtual void ToggleScoreboard(bool bShow);

	UFUNCTION(client, reliable)
	virtual void ClientToggleScoreboard(bool bShow);

	UFUNCTION(client, reliable)
	virtual void ClientSetHUDAndScoreboard(TSubclassOf<class AHUD> NewHUDClass, TSubclassOf<class UUTScoreboard> NewScoreboardClass);

	/**	We overload ServerRestartPlayer so that we can set the bReadyToPlay flag if the game hasn't begun	 **/
	virtual void ServerRestartPlayer_Implementation();

	/**  Added a check to see if the player's RespawnTimer is > 0	 **/
	virtual bool CanRestartPlayer();

	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) OVERRIDE;

	UFUNCTION(exec)
	virtual void BehindView(bool bWantBehindView);

	virtual bool IsBehindView();
	virtual void SetCameraMode( FName NewCamMode );
	virtual void ClientSetCameraMode_Implementation( FName NewCamMode ) OVERRIDE;
	virtual void ClientGameEnded_Implementation(AActor* EndGameFocus, bool bIsWinner) OVERRIDE;

	virtual void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

	// TEMP FIXMESTEVE - change gravity and update all jump properties to keep same jump heights etc.
	UFUNCTION(exec)
	virtual void SetGravity(float NewGravity);

	// A quick function so I don't have to keep adding one when I want to test something.  @REMOVEME: Before the final version
	UFUNCTION(exec)
	virtual void DebugTest();

	/**
	 *	We override player tick to keep updating the player's rotation when the game is over.
	 **/
	virtual void PlayerTick(float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly)
	void NotifyTakeHit(AController* InstigatedBy, int32 Damage, FVector Momentum, const FDamageEvent& DamageEvent);
	UFUNCTION(Client, Unreliable)
	void ClientNotifyTakeHit(APlayerState* InstigatedBy, int32 Damage, FVector Momentum, FVector RelHitLocation, TSubclassOf<UDamageType> DamgaeType);

	/**	Will popup the in-game menu	 **/
	UFUNCTION(exec)
	virtual void ShowMenu();

	UFUNCTION()
	virtual void ShowMessage(FText MessageTitle, FText MessageText, uint16 Buttons, UObject* Host, FName ResultFunction);

	/** blueprint hook */
	UFUNCTION(BlueprintCallable, Category = Message)
	void K2_ReceiveLocalizedMessage(TSubclassOf<ULocalMessage> Message, int32 Switch = 0, class APlayerState* RelatedPlayerState_1 = NULL, class APlayerState* RelatedPlayerState_2 = NULL, class UObject* OptionalObject = NULL);

	virtual uint8 GetTeamNum() const;

protected:

	// If set, this will be the final viewtarget this pawn can see.
	AActor* FinalViewTarget;

	/** list of weapon pickups that my Pawn has recently picked up, so we can hide the weapon mesh per player */
	UPROPERTY()
	TArray<class AUTPickupWeapon*> RecentWeaponPickups;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	// @TODO FIXMESTEVE make config
	UPROPERTY(EditAnywhere, Category = Dodging)
	float MaxDodgeClickTime;

	UPROPERTY(BluePrintReadOnly, Category = Dodging)
	float LastTapLeftTime;

	UPROPERTY(BluePrintReadOnly, Category = Dodging)
	float LastTapRightTime;

	UPROPERTY(BluePrintReadOnly, Category = Dodging)
	float LastTapForwardTime;

	UPROPERTY(BluePrintReadOnly, Category = Dodging)
	float LastTapBackTime;

	UPROPERTY(config, BlueprintReadOnly, Category = Weapon)
	bool bAutoWeaponSwitch;

	UFUNCTION(exec)
	virtual void ChangeTeam(uint8 NewTeamIndex);

	UFUNCTION(exec)
	virtual void Suicide();

	/** weapon selection */
	void PrevWeapon();
	void NextWeapon();
	virtual void SwitchWeaponInSequence(bool bPrev);
	UFUNCTION(Exec)
	virtual void SwitchWeapon(int32 Group);

	/** weapon fire input handling -- NOTE: Just forward to the pawn */
	virtual void OnFire();
	virtual void OnStopFire();
	virtual void OnAltFire();
	virtual void OnStopAltFire();

	/** Handles moving forward/backward */
	virtual void MoveForward(float Val);
	/** Handles stafing movement, left and right */
	virtual void MoveRight(float Val);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	virtual void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	virtual void LookUpAtRate(float Rate);

	/** called to set the jump flag from input */
	virtual void Jump();

	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** If double tap, tell pawn to dodge */
	bool CheckDodge(float LastTapTime, bool bForward, bool bBack, bool bLeft, bool bRight);

	/** Dodge tap input handling */
	void OnTapLeft();
	void OnTapRight();
	void OnTapForward();
	void OnTapBack();

	virtual void OnShowScores();
	virtual void OnHideScores();

};



