#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleTypes.h"

struct FINALBATTLE_API FFirstCardViewData
{
	FGuid CardInstanceId;
	FName CardId = NAME_None;
	FText DisplayName;
	int32 BaseCost = 0;
	int32 RuntimeCost = 0;
	FGameplayTagContainer Keywords;
	TArray<FFirstCardEffectInstance> Effects;
};

struct FINALBATTLE_API FFirstEnemyPartViewData
{
	FName PartId = NAME_None;
	FText DisplayName;
	int32 PositionIndex = 0;
	int32 MaxHP = 0;
	int32 CurrentHP = 0;
	bool bDestroyed = false;
	FName CurrentIntentId = NAME_None;
	FText CurrentIntentDisplayName;
	int32 CurrentInitiative = 0;
};

struct FINALBATTLE_API FFirstBattleSnapshot
{
	FGuid BattleId;
	int32 CurrentRound = 0;
	bool bInitialized = false;
	bool bBattleEnded = false;
	bool bPlayerVictory = false;
	int32 DiscardPileCount = 0;
	TArray<FFirstCardViewData> HandCards;
	TArray<FFirstEnemyPartViewData> EnemyParts;
	TArray<FFirstBattleEvent> RecentEvents;
};
