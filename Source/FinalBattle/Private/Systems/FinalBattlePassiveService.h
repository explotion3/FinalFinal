#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"

class FFinalBattleConditionService;
class FFinalBattleEffectExecutionService;
class FFinalBattleUnitService;
class UFinalPassiveDefinition;
struct FFinalBattleEffectExecutionSummary;
struct FFinalBattlePassiveInstance;
struct FFinalBattlePassiveViewData;
struct FFinalBattleState;

class FFinalBattlePassiveService
{
public:
	int32 ApplyPassive(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		FName SourceUnitId,
		const FFinalPassiveId& PassiveId,
		const UFinalPassiveDefinition* PassiveDefinition,
		int32 StacksToAdd,
		int32 DurationOverride = 0) const;

	void ResolveOwnerTookHealthDamagePassives(
		FFinalBattleState& BattleState,
		const FFinalBattleUnitService& UnitService,
		const FFinalBattleConditionService& ConditionService,
		const FFinalBattleEffectExecutionService& EffectExecutionService,
		FFinalBattleEffectExecutionSummary& InOutSummary) const;

	void ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const;
	void ResolvePlayerTurnEndPassives(FFinalBattleState& BattleState) const;
	void BuildPassiveSnapshotData(const FFinalBattleState& BattleState, TArray<FFinalBattlePassiveViewData>& OutPassives) const;

	const FFinalBattlePassiveInstance* FindPassiveInstance(const FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalPassiveId& PassiveId) const;
	FFinalBattlePassiveInstance* FindPassiveInstance(FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalPassiveId& PassiveId) const;
};
