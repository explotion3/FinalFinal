#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleTypes.h"

struct FFirstEnemyPartState
{
	FName PartId = NAME_None;
	FText DisplayName;
	int32 PositionIndex = 0;
	int32 MaxHP = 0;
	int32 CurrentHP = 0;
	bool bDestroyed = false;
	FName CurrentIntentId = NAME_None;
	FText CurrentIntentDisplayName;
	TArray<FFirstEnemyPartIntentInstance> IntentSequence;
	int32 CurrentIntentIndex = 0;
	int32 CurrentInitiative = 0;
};

struct FFirstBattleState
{
	FGuid BattleId;
	int32 CurrentRound = 0;
	bool bInitialized = false;
	bool bBattleEnded = false;
	bool bPlayerVictory = false;
	TArray<FFirstCardInstance> HandCards;
	TArray<FFirstCardInstance> DiscardPile;
	TArray<FFirstEnemyPartState> EnemyParts;
	TArray<FFirstBattleEvent> Events;
};
