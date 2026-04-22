#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Ids/FinalIds.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleRelicRuntimeState.h"
#include "Runtime/FinalBattleStatusInstance.h"
#include "Runtime/FinalTeamDeckState.h"

struct FFinalBattleState
{
	FGuid BattleId;
	FFinalEncounterId EncounterId;
	FFinalRuleConfigId RuleConfigId;
	FText EncounterDisplayName;
	int32 CurrentRound = 0;
	int32 CurrentAP = 0;
	int32 CurrentEP = 0;
	int32 MaxEP = 0;
	int32 TeamCurrentHP = 0;
	int32 TeamMaxHP = 0;
	int32 TeamShield = 0;
	bool bBattleEnded = false;
	bool bPlayerVictory = false;
	FName CurrentTargetUnitId = NAME_None;
	int32 LastEventSequence = 0;
	TArray<FFinalBattleCharacterState> Characters;
	TArray<FFinalBattleStartRelicInput> ActiveRelics;
	TArray<FFinalBattleRelicRuntimeState> RelicRuntimeStates;
	TArray<FFinalBattleEnemyState> Enemies;
	TArray<FFinalBattleCardInstance> CardInstances;
	TMap<FGuid, int32> CardInstanceIndexById;
	TArray<FFinalBattleStatusInstance> StatusInstances;
	FFinalTeamDeckState DeckState;
	TArray<FFinalBattleEvent> BattleLogEntries;
};
