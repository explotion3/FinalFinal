#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Ids/FinalIds.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleStatusInstance.h"
#include "Runtime/FinalTeamDeckState.h"

struct FFinalBattleState
{
	FGuid BattleId;
	FFinalEncounterId EncounterId;
	FFinalRuleConfigId RuleConfigId;
	int32 CurrentRound = 0;
	int32 CurrentAP = 0;
	int32 CurrentEP = 0;
	int32 TeamCurrentHP = 0;
	int32 TeamMaxHP = 0;
	int32 TeamShield = 0;
	bool bBattleEnded = false;
	bool bPlayerVictory = false;
	FName CurrentTargetUnitId = NAME_None;
	TArray<FFinalBattleCharacterState> Characters;
	TArray<FFinalBattleEnemyState> Enemies;
	TArray<FFinalBattleCardInstance> CardInstances;
	TArray<FFinalBattleStatusInstance> StatusInstances;
	FFinalTeamDeckState DeckState;
	TArray<FFinalBattleEvent> BattleLogEntries;
};
