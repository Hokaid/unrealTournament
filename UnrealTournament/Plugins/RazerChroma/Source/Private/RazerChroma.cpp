// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "RazerChroma.h"

#include "UnrealTournament.h"
#include "UTPlayerController.h"
#include "UTGameState.h"

#ifdef _WIN64
#define CHROMASDKDLL        _T("RzChromaSDK64.dll")
#else
#define CHROMASDKDLL        _T("RzChromaSDK.dll")
#endif

using namespace ChromaSDK;
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace ChromaSDK::Headset;

DEFINE_LOG_CATEGORY_STATIC(LogUTKBLightShow, Log, All);

ARazerChroma::ARazerChroma(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FRazerChroma::FRazerChroma()
{
	FrameTimeMinimum = 0.03f;
	DeltaTimeAccumulator = 0;
	bIsFlashingForEnd = false;
	bInitialized = false;
	bChromaSDKEnabled = false;

	FlashSpeed = 100;
	
	TextScrollerFrame = 0;
	TextScrollerDeltaTimeAccumulator = 0;
	TextScrollerFrameTimeMinimum = 0.10f;
}

IMPLEMENT_MODULE(FRazerChroma, RazerChroma)

void FRazerChroma::StartupModule()
{
	HMODULE ChromaSDKModule = nullptr;

	ChromaSDKModule = LoadLibrary(CHROMASDKDLL);
	if (ChromaSDKModule == NULL)
	{
		return;
	}

	// GetProcAddress will throw 4191 because it's an unsafe type cast, but I almost know what I'm doing here
#pragma warning(disable: 4191)
	RZRESULT Result = RZRESULT_INVALID;
	Init = (INIT)GetProcAddress(ChromaSDKModule, "Init");
	if (Init)
	{
		Result = Init();
		if (Result == RZRESULT_SUCCESS)
		{
			CreateEffect = (CREATEEFFECT)GetProcAddress(ChromaSDKModule, "CreateEffect");
			CreateKeyboardEffect = (CREATEKEYBOARDEFFECT)GetProcAddress(ChromaSDKModule, "CreateKeyboardEffect");
			CreateMouseEffect = (CREATEMOUSEEFFECT)GetProcAddress(ChromaSDKModule, "CreateMouseEffect");
			CreateHeadsetEffect = (CREATEHEADSETEFFECT)GetProcAddress(ChromaSDKModule, "CreateHeadsetEffect");
			CreateMousepadEffect = (CREATEMOUSEPADEFFECT)GetProcAddress(ChromaSDKModule, "CreateMousepadEffect");
			CreateKeypadEffect = (CREATEKEYPADEFFECT)GetProcAddress(ChromaSDKModule, "CreateKeypadEffect");
			SetEffect = (SETEFFECT)GetProcAddress(ChromaSDKModule, "SetEffect");
			DeleteEffect = (DELETEEFFECT)GetProcAddress(ChromaSDKModule, "DeleteEffect");
			QueryDevice = (QUERYDEVICE)GetProcAddress(ChromaSDKModule, "QueryDevice");
			UnInit = (UNINIT)GetProcAddress(ChromaSDKModule, "UnInit");

			if (CreateEffect &&
				CreateKeyboardEffect &&
				CreateMouseEffect &&
				CreateHeadsetEffect &&
				CreateMousepadEffect &&
				CreateKeypadEffect &&
				SetEffect &&
				DeleteEffect &&
				QueryDevice &&
				UnInit)
			{
				bChromaSDKEnabled = true;
			}
		}
	}
#pragma warning(default: 4191)

//	QueryDevice(RZDEVICEID)

	if (bChromaSDKEnabled)
	{
		FWorldDelegates::FWorldInitializationEvent::FDelegate OnWorldCreatedDelegate = FWorldDelegates::FWorldInitializationEvent::FDelegate::CreateRaw(this, &FRazerChroma::OnWorldCreated);
		FDelegateHandle OnWorldCreatedDelegateHandle = FWorldDelegates::OnPostWorldInitialization.Add(OnWorldCreatedDelegate);

		ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE Effect = {};

		// U
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_Q)][LOBYTE(RZKEY_Q)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_A)][LOBYTE(RZKEY_A)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_Z)][LOBYTE(RZKEY_Z)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_X)][LOBYTE(RZKEY_X)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_C)][LOBYTE(RZKEY_C)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_V)][LOBYTE(RZKEY_V)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_F)][LOBYTE(RZKEY_F)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_R)][LOBYTE(RZKEY_R)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = RED;
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = RED;
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = RED;
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = RED;
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = RED;
		Effect.Color[HIBYTE(RZKEY_I)][LOBYTE(RZKEY_I)] = RED;
		Effect.Color[HIBYTE(RZKEY_K)][LOBYTE(RZKEY_K)] = RED;
		Effect.Color[HIBYTE(RZKEY_OEM_9)][LOBYTE(RZKEY_OEM_9)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[0]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_Z)][LOBYTE(RZKEY_Z)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_X)][LOBYTE(RZKEY_X)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_C)][LOBYTE(RZKEY_C)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_D)][LOBYTE(RZKEY_D)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_E)][LOBYTE(RZKEY_E)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = RED;
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = RED;
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = RED;
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = RED;
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = RED;
		Effect.Color[HIBYTE(RZKEY_U)][LOBYTE(RZKEY_U)] = RED;
		Effect.Color[HIBYTE(RZKEY_J)][LOBYTE(RZKEY_J)] = RED;
		Effect.Color[HIBYTE(RZKEY_M)][LOBYTE(RZKEY_M)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[1]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_Z)][LOBYTE(RZKEY_Z)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_X)][LOBYTE(RZKEY_X)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_S)][LOBYTE(RZKEY_S)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_W)][LOBYTE(RZKEY_W)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = BLUE;

		// Second U
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_P)][LOBYTE(RZKEY_P)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_7)][LOBYTE(RZKEY_OEM_7)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_11)][LOBYTE(RZKEY_OEM_11)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = RED;
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = RED;
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = RED;
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = RED;
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = RED;
		Effect.Color[HIBYTE(RZKEY_Y)][LOBYTE(RZKEY_Y)] = RED;
		Effect.Color[HIBYTE(RZKEY_H)][LOBYTE(RZKEY_H)] = RED;
		Effect.Color[HIBYTE(RZKEY_N)][LOBYTE(RZKEY_N)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[2]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_Q)][LOBYTE(RZKEY_Q)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_A)][LOBYTE(RZKEY_A)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_Z)][LOBYTE(RZKEY_Z)] = BLUE;

		// Second U
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_O)][LOBYTE(RZKEY_O)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_L)][LOBYTE(RZKEY_L)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_10)][LOBYTE(RZKEY_OEM_10)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = RED;
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = RED;
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = RED;
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = RED;
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = RED;
		Effect.Color[HIBYTE(RZKEY_T)][LOBYTE(RZKEY_T)] = RED;
		Effect.Color[HIBYTE(RZKEY_G)][LOBYTE(RZKEY_G)] = RED;
		Effect.Color[HIBYTE(RZKEY_B)][LOBYTE(RZKEY_B)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[3]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_I)][LOBYTE(RZKEY_I)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_K)][LOBYTE(RZKEY_K)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_9)][LOBYTE(RZKEY_OEM_9)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_10)][LOBYTE(RZKEY_OEM_10)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_11)][LOBYTE(RZKEY_OEM_11)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = RED;
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = RED;
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = RED;
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = RED;
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = RED;
		Effect.Color[HIBYTE(RZKEY_R)][LOBYTE(RZKEY_R)] = RED;
		Effect.Color[HIBYTE(RZKEY_F)][LOBYTE(RZKEY_F)] = RED;
		Effect.Color[HIBYTE(RZKEY_V)][LOBYTE(RZKEY_V)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[4]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_U)][LOBYTE(RZKEY_U)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_J)][LOBYTE(RZKEY_J)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_M)][LOBYTE(RZKEY_M)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_9)][LOBYTE(RZKEY_OEM_9)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_10)][LOBYTE(RZKEY_OEM_10)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_11)][LOBYTE(RZKEY_OEM_11)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_7)][LOBYTE(RZKEY_OEM_7)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_P)][LOBYTE(RZKEY_P)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = RED;
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = RED;
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = RED;
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = RED;
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = RED;
		Effect.Color[HIBYTE(RZKEY_E)][LOBYTE(RZKEY_E)] = RED;
		Effect.Color[HIBYTE(RZKEY_D)][LOBYTE(RZKEY_D)] = RED;
		Effect.Color[HIBYTE(RZKEY_C)][LOBYTE(RZKEY_C)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[5]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_Y)][LOBYTE(RZKEY_Y)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_H)][LOBYTE(RZKEY_H)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_N)][LOBYTE(RZKEY_N)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_M)][LOBYTE(RZKEY_M)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_9)][LOBYTE(RZKEY_OEM_9)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_10)][LOBYTE(RZKEY_OEM_10)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_L)][LOBYTE(RZKEY_L)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_O)][LOBYTE(RZKEY_O)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = RED;
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = RED;
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = RED;
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = RED;
		Effect.Color[HIBYTE(RZKEY_W)][LOBYTE(RZKEY_W)] = RED;
		Effect.Color[HIBYTE(RZKEY_S)][LOBYTE(RZKEY_S)] = RED;
		Effect.Color[HIBYTE(RZKEY_X)][LOBYTE(RZKEY_X)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[6]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_T)][LOBYTE(RZKEY_T)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_G)][LOBYTE(RZKEY_G)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_B)][LOBYTE(RZKEY_B)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_N)][LOBYTE(RZKEY_N)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_M)][LOBYTE(RZKEY_M)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_OEM_9)][LOBYTE(RZKEY_OEM_9)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_K)][LOBYTE(RZKEY_K)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_I)][LOBYTE(RZKEY_I)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = RED;
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = RED;
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = RED;
		Effect.Color[HIBYTE(RZKEY_Q)][LOBYTE(RZKEY_Q)] = RED;
		Effect.Color[HIBYTE(RZKEY_A)][LOBYTE(RZKEY_A)] = RED;
		Effect.Color[HIBYTE(RZKEY_Z)][LOBYTE(RZKEY_Z)] = RED;

		// Second T
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[7]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_4)][LOBYTE(RZKEY_4)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_R)][LOBYTE(RZKEY_R)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_F)][LOBYTE(RZKEY_F)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_V)][LOBYTE(RZKEY_V)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_B)][LOBYTE(RZKEY_B)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_N)][LOBYTE(RZKEY_N)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_M)][LOBYTE(RZKEY_M)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_J)][LOBYTE(RZKEY_J)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_U)][LOBYTE(RZKEY_U)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = RED;
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = RED;

		// Second T
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = RED;
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[8]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_3)][LOBYTE(RZKEY_3)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_E)][LOBYTE(RZKEY_E)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_D)][LOBYTE(RZKEY_D)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_C)][LOBYTE(RZKEY_C)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_V)][LOBYTE(RZKEY_V)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_B)][LOBYTE(RZKEY_B)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_N)][LOBYTE(RZKEY_N)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_Y)][LOBYTE(RZKEY_Y)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_H)][LOBYTE(RZKEY_H)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_6)][LOBYTE(RZKEY_6)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_1)][LOBYTE(RZKEY_1)] = RED;

		// Second T
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = RED;
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = RED;
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = RED;
		Effect.Color[HIBYTE(RZKEY_P)][LOBYTE(RZKEY_P)] = RED;
		Effect.Color[HIBYTE(RZKEY_OEM_7)][LOBYTE(RZKEY_OEM_7)] = RED;
		Effect.Color[HIBYTE(RZKEY_OEM_11)][LOBYTE(RZKEY_OEM_11)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[9]);

		FMemory::Memzero(Effect);

		// U
		Effect.Color[HIBYTE(RZKEY_2)][LOBYTE(RZKEY_2)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_W)][LOBYTE(RZKEY_W)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_S)][LOBYTE(RZKEY_S)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_X)][LOBYTE(RZKEY_X)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_C)][LOBYTE(RZKEY_C)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_V)][LOBYTE(RZKEY_V)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_B)][LOBYTE(RZKEY_B)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_T)][LOBYTE(RZKEY_T)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_G)][LOBYTE(RZKEY_G)] = BLUE;
		Effect.Color[HIBYTE(RZKEY_5)][LOBYTE(RZKEY_5)] = BLUE;

		// T
		Effect.Color[HIBYTE(RZKEY_7)][LOBYTE(RZKEY_7)] = RED;
		Effect.Color[HIBYTE(RZKEY_8)][LOBYTE(RZKEY_8)] = RED;
		Effect.Color[HIBYTE(RZKEY_9)][LOBYTE(RZKEY_9)] = RED;
		Effect.Color[HIBYTE(RZKEY_0)][LOBYTE(RZKEY_0)] = RED;
		Effect.Color[HIBYTE(RZKEY_O)][LOBYTE(RZKEY_O)] = RED;
		Effect.Color[HIBYTE(RZKEY_L)][LOBYTE(RZKEY_L)] = RED;
		Effect.Color[HIBYTE(RZKEY_OEM_10)][LOBYTE(RZKEY_OEM_10)] = RED;

		CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, &UnrealTextScroller[10]);
	}

}

void FRazerChroma::ShutdownModule()
{
	FWorldDelegates::FWorldEvent::FDelegate OnWorldDestroyedDelegate = FWorldDelegates::FWorldEvent::FDelegate::CreateRaw(this, &FRazerChroma::OnWorldDestroyed);
	FDelegateHandle OnWorldDestroyedDelegateHandle = FWorldDelegates::OnPreWorldFinishDestroy.Add(OnWorldDestroyedDelegate);

	if (bChromaSDKEnabled)
	{
		for (int i = 0; i < UNREALTEXTSCROLLERFRAMES; i++)
		{
			DeleteEffect(UnrealTextScroller[i]);
		}

		UnInit();
	}
}

void FRazerChroma::OnWorldCreated(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (IsRunningCommandlet() || IsRunningDedicatedServer())
	{
		return;
	}

	WorldList.Add(World);
}

void FRazerChroma::OnWorldDestroyed(UWorld* World)
{
	if (IsRunningCommandlet() || IsRunningDedicatedServer())
	{
		return;
	}

	WorldList.Remove(World);
}


void FRazerChroma::Tick(float DeltaTime)
{
	if (!bChromaSDKEnabled)
	{
		return;
	}

	if (GIsEditor)
	{
		return;
	}

	// Avoid double ticking
	if (LastFrameCounter > 0 && LastFrameCounter == GFrameCounter)
	{
		return;
	}

	LastFrameCounter = GFrameCounter;

	// We may be going 120hz, don't spam the device
	DeltaTimeAccumulator += DeltaTime;
	if (DeltaTimeAccumulator < FrameTimeMinimum)
	{
		return;
	}
	DeltaTimeAccumulator = 0;

	AUTPlayerController* UTPC = nullptr;
	AUTGameState* GS = nullptr;
	for (int32 i = 0; i < WorldList.Num(); i++)
	{
		if (WorldList[i]->WorldType == EWorldType::Game)
		{
			UTPC = Cast<AUTPlayerController>(GEngine->GetFirstLocalPlayerController(WorldList[i]));
			if (UTPC)
			{
				UUTLocalPlayer* UTLP = Cast<UUTLocalPlayer>(UTPC->GetLocalPlayer());
				if (UTLP == nullptr || UTLP->IsMenuGame())
				{
					UTPC = nullptr;
					continue;
				}

				GS = WorldList[i]->GetGameState<AUTGameState>();
				break;
			}
		}
	}

	if (!UTPC || !GS)
	{
		return;
	}

	RZRESULT Result = RZRESULT_INVALID;

	if (GS->HasMatchEnded())
	{
		if (GS->WinningTeam != nullptr)
		{
			if (GS->WinningTeam->GetTeamNum() == UTPC->GetTeamNum())
			{
			}
			else
			{
			}
		}
		else if (GS->WinnerPlayerState != nullptr)
		{
			if (GS->WinnerPlayerState == UTPC->PlayerState)
			{
			}
			else
			{
			}
		}

		PlayTextScroller(DeltaTime);
	}
	else if (GS->HasMatchStarted())
	{
		ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE Effect = {};
		
		if (UTPC->GetUTCharacter() && !UTPC->GetUTCharacter()->IsDead())
		{
			float HealthPct = (float)UTPC->GetUTCharacter()->Health / 100.0f;
			HealthPct = FMath::Clamp(HealthPct, 0.0f, 1.0f);
			int32 KeysToFill = FMath::Clamp((int)(HealthPct * 12), 0, 12);
			
			uint32 HealthColor = GREEN;
			if (HealthPct <= 0.5f)
			{
				HealthColor = RED;
			}
			else if (HealthPct <= 0.75f)
			{
				HealthColor = YELLOW;
			}

			for (int32 i = 0; i < KeysToFill; i++)
			{
				Effect.Color[HIBYTE(RZKEY_F1 + i)][LOBYTE(RZKEY_F1 + i)] = HealthColor;
			}

			Effect.Color[HIBYTE(RZKEY_W)][LOBYTE(RZKEY_W)] = BLUE;
			Effect.Color[HIBYTE(RZKEY_A)][LOBYTE(RZKEY_A)] = BLUE;
			Effect.Color[HIBYTE(RZKEY_S)][LOBYTE(RZKEY_S)] = BLUE;
			Effect.Color[HIBYTE(RZKEY_D)][LOBYTE(RZKEY_D)] = BLUE;
			Effect.Color[HIBYTE(RZKEY_SPACE)][LOBYTE(RZKEY_SPACE)] = BLUE;

			bool bFoundWeapon = false;
			for (int32 i = 0; i < 10; i++)
			{
				bFoundWeapon = false;

				int32 CurrentWeaponGroup = -1;
				if (UTPC->GetUTCharacter()->GetWeapon())
				{
					CurrentWeaponGroup = UTPC->GetUTCharacter()->GetWeapon()->DefaultGroup;
				}

				for (TInventoryIterator<AUTWeapon> It(UTPC->GetUTCharacter()); It; ++It)
				{
					AUTWeapon* Weap = *It;
					if (Weap->HasAnyAmmo())
					{
						if (Weap->DefaultGroup == (i + 1))
						{
							bFoundWeapon = true;
							break;
						}
					}
				}

				if ((i + 1) == CurrentWeaponGroup)
				{
					Effect.Color[HIBYTE(RZKEY_1 + i)][LOBYTE(RZKEY_1 + i)] = RED;
				}
				else if (bFoundWeapon)
				{
					Effect.Color[HIBYTE(RZKEY_1 + i)][LOBYTE(RZKEY_1 + i)] = GREEN;
				}
			}
		}

		Result = CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_CUSTOM, &Effect, NULL);
	}
	else
	{
		PlayTextScroller(DeltaTime);
	}
}

void FRazerChroma::PlayTextScroller(float DeltaTime)
{
	SetEffect(UnrealTextScroller[TextScrollerFrame]);

	TextScrollerDeltaTimeAccumulator += DeltaTime;
	if (TextScrollerDeltaTimeAccumulator > TextScrollerFrameTimeMinimum)
	{
		TextScrollerDeltaTimeAccumulator = 0;
		TextScrollerFrame++;
		TextScrollerFrame %= UNREALTEXTSCROLLERFRAMES;
	}
}