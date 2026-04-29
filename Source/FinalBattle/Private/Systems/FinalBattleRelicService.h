#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"

struct FFinalBattleState;
class FFinalBattleConditionService;
class FFinalBattleEffectExecutionService;
class FFinalBattleTriggerService;
class FFinalBattleUnitService;

class FFinalBattleRelicService
{
public:
	void InitializeRelics(
		FFinalBattleState& BattleState,
		TArray<FFinalBattleStartRelicInput> ActiveRelics,
		const FFinalBattleTriggerService& TriggerService,
		const FFinalBattleConditionService& ConditionService,
		const FFinalBattleEffectExecutionService& EffectExecutionService,
		const FFinalBattleUnitService& UnitService,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;

	void ApplyPlayerTurnStartRelicEffects(
		FFinalBattleState& BattleState,
		const FFinalBattleTriggerService& TriggerService,
		const FFinalBattleConditionService& ConditionService,
		const FFinalBattleEffectExecutionService& EffectExecutionService,
		const FFinalBattleUnitService& UnitService,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
	void ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const;
};
