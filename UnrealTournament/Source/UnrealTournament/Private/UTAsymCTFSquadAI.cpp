// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTAsymCTFSquadAI.h"
#include "UTDefensePoint.h"
#include "UTFlagRunGameState.h"

void AUTAsymCTFSquadAI::Initialize(AUTTeamInfo* InTeam, FName InOrders)
{
	Super::Initialize(InTeam, InOrders);

	AUTFlagRunGameState* GS = GetWorld()->GetGameState<AUTFlagRunGameState>();
	if (GS != NULL && GS->FlagBases.Num() >= 2 && GS->FlagBases[0] != NULL && GS->FlagBases[1] != NULL)
	{
		GameObjective = GS->FlagBases[GS->bRedToCap ? 1 : 0];
		Objective = GameObjective;
		Flag = GS->FlagBases[GS->bRedToCap ? 0 : 1]->GetCarriedObject();
		TotalFlagRunDistance = (Objective->GetActorLocation() - Flag->GetActorLocation()).Size();
	}
	LastFlagNode = nullptr;
	SquadRoutes.Empty();
}

bool AUTAsymCTFSquadAI::IsAttackingTeam() const
{
	AUTFlagRunGameState* GS = GetWorld()->GetGameState<AUTFlagRunGameState>();
	return (GS != NULL && GetTeamNum() == (GS->bRedToCap ? 0 : 1));
}

bool AUTAsymCTFSquadAI::MustKeepEnemy(APawn* TheEnemy)
{
	// must keep enemy flag holder
	AUTCharacter* UTC = Cast<AUTCharacter>(TheEnemy);
	return (UTC != NULL && UTC->GetCarriedObject() != NULL);
}

bool AUTAsymCTFSquadAI::ShouldUseTranslocator(AUTBot* B)
{
	if (Super::ShouldUseTranslocator(B))
	{
		return true;
	}
	else if (B->RouteCache.Num() < 3)
	{
		return false;
	}
	else
	{
		// prioritize translocator when chasing enemy flag carrier
		AUTCharacter* CharGoal = Cast<AUTCharacter>(B->RouteCache.Last().Actor.Get());
		AUTCharacter* CharHuntTarget = Cast<AUTCharacter>(B->HuntingTarget);
		return ((CharGoal != NULL && CharGoal->GetCarriedObject() != NULL) || (CharHuntTarget != NULL && CharHuntTarget->GetCarriedObject() != NULL)) &&
			(B->GetEnemy() != CharGoal || !B->IsEnemyVisible(B->GetEnemy()) || (B->CurrentAggression > 0.0f && (B->GetPawn()->GetActorLocation() - B->GetEnemy()->GetActorLocation()).Size() > 3000.0f));
	}
}

float AUTAsymCTFSquadAI::ModifyEnemyRating(float CurrentRating, const FBotEnemyInfo& EnemyInfo, AUTBot* B)
{
	if (EnemyInfo.GetUTChar() != NULL && EnemyInfo.GetUTChar()->GetCarriedObject() != NULL && B->CanAttack(EnemyInfo.GetPawn(), EnemyInfo.LastKnownLoc, false))
	{
		if ((B->GetPawn()->GetActorLocation() - EnemyInfo.LastKnownLoc).Size() < 3500.0f || (B->GetUTChar() != NULL && B->GetUTChar()->GetWeapon() != NULL && B->GetUTChar()->GetWeapon()->bSniping) ||
			(EnemyInfo.LastKnownLoc - Objective->GetActorLocation()).Size() < 4500.0f )
		{
			return CurrentRating + 6.0f;
		}
		else
		{
			return CurrentRating + 1.5f;
		}
	}
	// prioritize enemies that target friendly flag carrier
	else if (Flag->HoldingPawn != NULL && IsAttackingTeam() && (Flag->HoldingPawn->LastHitBy == EnemyInfo.GetPawn()->GetController() || !GetWorld()->LineTraceTestByChannel(EnemyInfo.LastKnownLoc, Flag->HoldingPawn->GetActorLocation(), ECC_Pawn, FCollisionQueryParams::DefaultQueryParam, WorldResponseParams)))
	{
		return CurrentRating + 1.5f;
	}
	else
	{
		return CurrentRating;
	}
}

bool AUTAsymCTFSquadAI::TryPathTowardObjective(AUTBot* B, AActor* Goal, bool bAllowDetours, const FString& SuccessGoalString)
{
	bool bResult = Super::TryPathTowardObjective(B, Goal, bAllowDetours, SuccessGoalString);
	if (bResult && Cast<AUTCarriedObject>(Goal) != NULL && B->GetRouteDist() < 2500 && B->LineOfSightTo(Goal))
	{
		B->SendVoiceMessage(StatusMessage::IGotFlag);
	}
	return bResult;
}

bool AUTAsymCTFSquadAI::SetFlagCarrierAction(AUTBot* B)
{
	// TODO: wait for allies, maybe double back, etc
	return TryPathTowardObjective(B, Objective, false, "Head to enemy base with flag");
}

void AUTAsymCTFSquadAI::GetPossibleEnemyGoals(AUTBot* B, const FBotEnemyInfo* EnemyInfo, TArray<FPredictedGoal>& Goals)
{
	Super::GetPossibleEnemyGoals(B, EnemyInfo, Goals);
	if (!IsAttackingTeam() && Flag != NULL && Objective != NULL)
	{
		if (Flag->HoldingPawn == NULL)
		{
			Goals.Add(FPredictedGoal(Flag->GetActorLocation() - FVector(0.0f, 0.0f, Flag->GetSimpleCollisionHalfHeight()), true));
		}
		else
		{
			Goals.Add(FPredictedGoal(Objective->GetActorLocation(), true));
		}
	}
}

bool AUTAsymCTFSquadAI::HuntEnemyFlag(AUTBot* B)
{
	if (Flag->HoldingPawn != nullptr)
	{
		// if the enemy FC has never been seen, use the alternate path logic
		// this prevents the AI from abandoning the alternate approach routes prematurely when the flag is picked up
		const FBotEnemyInfo* CarrierInfo = B->GetEnemyInfo(Flag->HoldingPawn, true);
		if (CarrierInfo == nullptr || CarrierInfo->LastSeenTime <= 0.0f)
		{
			if ((B->GetPawn()->GetActorLocation() - Flag->HoldingPawn->GetActorLocation()).Size() < TotalFlagRunDistance * (0.33f - 0.1f * B->Personality.Aggressiveness))
			{
				B->GoalString = TEXT("Wait here for enemy assault to begin");
				B->DoCamp();
				return true;
			}
			else if (FollowAlternateRoute(B, Flag->HoldingPawn, SquadRoutes, true, true, "Continue prior route to flag carrier"))
			{
				return true;
			}
		}
		
		if (Flag->HoldingPawn == B->GetEnemy())
		{
			B->GoalString = "Fight flag carrier";
			// fight enemy
			return false;
		}
		else
		{
			B->GoalString = "Hunt down enemy flag carrier";
			B->DoHunt(Flag->HoldingPawn);
			return true;
		}
	}
	else if ((Flag->GetActorLocation() - B->GetPawn()->GetActorLocation()).Size() < FMath::Min<float>(3000.0f, (Flag->GetActorLocation() - Objective->GetActorLocation()).Size()) && B->UTLineOfSightTo(Flag))
	{
		// fight/camp here
		return false;
	}
	else
	{
		// use alternate route to reach flag so that defenders approach from multiple angles
		const UUTPathNode* FlagNode = NavData->FindNearestNode(Flag->GetActorLocation(), NavData->GetPOIExtent(Flag));
		if (FlagNode != LastFlagNode)
		{
			SquadRoutes.Reset();
			LastFlagNode = FlagNode;
		}
		return FollowAlternateRoute(B, Flag, SquadRoutes, true, true, "Camp dropped flag") || B->TryPathToward(Flag, true, true, "Camp dropped flag");
	}
}

bool AUTAsymCTFSquadAI::CheckSquadObjectives(AUTBot* B)
{
	// make bot with the flag Leader if possible
	if (B->GetUTChar() != nullptr && B->GetUTChar()->GetCarriedObject() != nullptr && Cast<APlayerController>(Leader) == nullptr)
	{
		SetLeader(B);
	}

	FName CurrentOrders = GetCurrentOrders(B);

	if (Flag == NULL || Objective == NULL)
	{
		return Super::CheckSquadObjectives(B);
	}
	else if (IsAttackingTeam())
	{
		B->SetDefensePoint(NULL);
		if (B->GetUTChar() != NULL && B->GetUTChar()->GetCarriedObject() != NULL)
		{
			return SetFlagCarrierAction(B);
		}
		else if (B->NeedsWeapon() && (GameObjective == NULL || GameObjective->GetCarriedObject() == NULL || (B->GetPawn()->GetActorLocation() - GameObjective->GetCarriedObject()->GetActorLocation()).Size() > 3000.0f) && B->FindInventoryGoal(0.0f))
		{
			B->GoalString = FString::Printf(TEXT("Get inventory %s"), *GetNameSafe(B->RouteCache.Last().Actor.Get()));
			B->SetMoveTarget(B->RouteCache[0]);
			B->StartWaitForMove();
			return true;
		}
		else if (Flag->HoldingPawn == NULL && (Flag->bFriendlyCanPickup || CurrentOrders == NAME_Defend)) 
		{
			// amortize generation of alternate routes during delay before flag can be picked up
			if (!Flag->bFriendlyCanPickup && SquadRoutes.Num() < MaxSquadRoutes && (GetLeader() == B || Cast<APlayerController>(GetLeader()) != nullptr))
			{
				FollowAlternateRoute(B, Objective, SquadRoutes, false, false, TEXT(""));
				// clear cached values since we're not actually following the route right now
				B->UsingSquadRouteIndex = INDEX_NONE;
				B->bDisableSquadRoutes = false;
				CurrentSquadRouteIndex = INDEX_NONE;
			}
			return B->TryPathToward(Flag, true, false, "Get flag");
		}
		else if (CurrentOrders == NAME_Defend)
		{
			if ((B->GetPawn()->GetActorLocation() - Flag->HoldingPawn->GetActorLocation()).Size() < 2000.0f)
			{
				return false; // fight enemies around FC
			}
			else
			{
				return B->TryPathToward(Flag->HoldingPawn, true, false, "Find flag carrier");
			}
		}
		else
		{
			// priorize fighting and powerups
			if (B->GetEnemy() != NULL && !B->LostContact(2.0f + 1.5f * B->Personality.Aggressiveness))
			{
				return false;
			}
			else if (CheckSuperPickups(B, 10000))
			{
				return true;
			}
			else if (Team->GetEnemyList().Num() == 0 && B->FindInventoryGoal(0.0f))
			{
				B->GoalString = FString::Printf(TEXT("Initial rush: Head to inventory %s"), *GetNameSafe(B->RouteCache.Last().Actor.Get()));
				B->SetMoveTarget(B->RouteCache[0]);
				B->StartWaitForMove();
				return true;
			}
			else
			{
				return B->TryPathToward(Flag->HoldingPawn, true, false, "Find flag carrier");
			}
		}
	}
	else
	{
		if (CurrentOrders == NAME_Defend)
		{
			SetDefensePointFor(B);
		}
		else
		{
			B->SetDefensePoint(NULL);
		}

		if (B->NeedsWeapon() && B->FindInventoryGoal(0.0f))
		{
			B->GoalString = FString::Printf(TEXT("Get inventory %s"), *GetNameSafe(B->RouteCache.Last().Actor.Get()));
			B->SetMoveTarget(B->RouteCache[0]);
			B->StartWaitForMove();
			return true;
		}
		else if (CurrentOrders == NAME_Defend)	
		{
			if (B->GetEnemy() != NULL)
			{
				// prioritize defense point if haven't actually encountered enemy since respawning
				bool bPrioritizeEnemy = B->GetEnemy() != nullptr && (B->GetDefensePoint() == nullptr || B->GetEnemyInfo(B->GetEnemy(), false)->LastFullUpdateTime > B->LastRespawnTime);
				if (bPrioritizeEnemy && B->GetEnemy() == Flag->HoldingPawn)
				{
					return HuntEnemyFlag(B);
				}
				else if (bPrioritizeEnemy && (!B->LostContact(3.0f) || MustKeepEnemy(B->GetEnemy())))
				{
					B->GoalString = "Fight attacker";
					return false;
				}
				else if (CheckSuperPickups(B, 5000))
				{
					return true;
				}
				else if (B->GetDefensePoint() != NULL)
				{
					return B->TryPathToward(B->GetDefensePoint(), true, false, "Go to defense point");
				}
				else
				{
					B->GoalString = "Fight attacker";
					return false;
				}
			}
			else if (Super::CheckSquadObjectives(B))
			{
				return true;
			}
			else if (B->FindInventoryGoal(0.0003f))
			{
				B->GoalString = FString::Printf(TEXT("Get inventory %s"), *GetNameSafe(B->RouteCache.Last().Actor.Get()));
				B->SetMoveTarget(B->RouteCache[0]);
				B->StartWaitForMove();
				return true;
			}
			else if (B->GetDefensePoint() != NULL)
			{
				return B->TryPathToward(B->GetDefensePoint(), true, false, "Go to defense point");
			}
			// TODO: decide between defending flag or defending goal based on situation
			else if (Flag != NULL)
			{
				return HuntEnemyFlag(B);
			}
			else if (Objective != NULL)
			{
				return B->TryPathToward(Objective, true, true, "Defend objective");
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (B->GetEnemy() != NULL && MustKeepEnemy(B->GetEnemy()) && !B->LostContact(2.0f))
			{
				return HuntEnemyFlag(B);
			}
			else
			{
				const FBotEnemyInfo* EnemyFCTeamData = (Flag->HoldingPawn != nullptr) ? Team->GetEnemyList().FindByPredicate([this](const FBotEnemyInfo& TestItem) { return TestItem.GetPawn() == Flag->HoldingPawn; }) : nullptr;
				// prioritize finding FC if no eyes on it for a while
				if (EnemyFCTeamData != nullptr && GetWorld()->TimeSeconds - EnemyFCTeamData->LastFullUpdateTime > 3.0f)
				{
					return HuntEnemyFlag(B);
				}
				else if (CurrentOrders == NAME_Attack)
				{
					if (CheckSuperPickups(B, 8000))
					{
						return true;
					}
					else
					{
						return HuntEnemyFlag(B);
					}
				}
				else
				{
					// freelance, just fight whoever's around
					return false;
				}
			}
		}
	}
}

int32 AUTAsymCTFSquadAI::GetDefensePointPriority(AUTBot* B, AUTDefensePoint* Point)
{
	// prioritize defense points closer to start of map
	// but outright reject those that the enemy FC has passed
	const float PointDist = (Point->GetActorLocation() - Objective->GetActorLocation()).Size();
	const FVector FlagLoc = (Flag->HoldingPawn != nullptr) ? B->GetEnemyLocation(Flag->HoldingPawn, true) : Flag->GetActorLocation();
	const float FlagDist = (FlagLoc - Objective->GetActorLocation()).Size();
	// hard reject if flag has passed the point or it's too far away from the action
	if ((FlagDist - 4000.0f > PointDist && !Point->bSniperSpot) || FlagDist + 2000.0f < PointDist || (FlagDist < PointDist && GetWorld()->LineTraceTestByChannel(Point->GetActorLocation(), FlagLoc, ECC_Visibility)))
	{
		return 0;
	}
	else
	{
		return Super::GetDefensePointPriority(B, Point) + FMath::Min<int32>(33, FMath::TruncToInt(33.0f * (PointDist / TotalFlagRunDistance)));
	}
}

void AUTAsymCTFSquadAI::NotifyObjectiveEvent(AActor* InObjective, AController* InstigatedBy, FName EventName)
{
	AUTGameObjective* InGameObjective = Cast<AUTGameObjective>(InObjective);
	for (AController* C : Members)
	{
		AUTBot* B = Cast<AUTBot>(C);
		if (B != NULL)
		{
			if (B->GetUTChar() != NULL && B->GetUTChar()->GetCarriedObject() != NULL)
			{
				// retask flag carrier immediately
				B->WhatToDoNext();
			}
			else if (B->GetMoveTarget().Actor != NULL && (B->GetMoveTarget().Actor == InObjective || (InGameObjective != NULL && B->GetMoveTarget().Actor == InGameObjective->GetCarriedObject())))
			{
				SetRetaskTimer(B);
			}
		}
	}

	Super::NotifyObjectiveEvent(InObjective, InstigatedBy, EventName);
}

bool AUTAsymCTFSquadAI::HasHighPriorityObjective(AUTBot* B)
{
	return ((B->GetUTChar() != NULL && B->GetUTChar()->GetCarriedObject() != NULL) || (B->GetEnemy() != NULL && MustKeepEnemy(B->GetEnemy())));
}