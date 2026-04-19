#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"

struct FFinalBattleState;

class FFinalBattleRelicService
{
public:
	void InitializeRelics(
		FFinalBattleState& BattleState,
		TArray<FFinalBattleStartRelicInput> ActiveRelics,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;

	void ApplyPlayerTurnStartRelicEffects(FFinalBattleState& BattleState, TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
	void ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const;
	void HandlePlayerTeamTookHealthDamage(
		FFinalBattleState& BattleState,
		int32 ActualHealthDamage,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
};
