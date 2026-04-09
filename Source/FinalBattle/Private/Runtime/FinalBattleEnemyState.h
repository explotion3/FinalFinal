#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalBattleEnemyState
{
	FName RuntimeUnitId = NAME_None;
	FFinalEnemyId EnemyId;
	FText DisplayName;
	int32 PositionIndex = 0;
	int32 SpawnWave = 1;
	int32 CurrentHP = 0;
	int32 CurrentBreakValue = 0;
	int32 CurrentInitiative = 0;
	FText CurrentIntentText;
	bool bActedThisRound = false;
};
