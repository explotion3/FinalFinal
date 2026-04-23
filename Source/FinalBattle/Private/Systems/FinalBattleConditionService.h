#pragma once

#include "CoreMinimal.h"
#include "Systems/FinalBattleConditionTypes.h"

class UFinalBattleConditionDefinition;
class UFinalBattleEffectDefinition;
struct FFinalBattleCardInstance;

class FFinalBattleConditionService
{
public:
	bool SatisfiesSourceAndChainConditions(
		const UFinalBattleEffectDefinition* EffectDefinition,
		const FFinalBattleConditionEvaluationContext& Context) const;

	bool SatisfiesTargetConditions(
		const UFinalBattleEffectDefinition* EffectDefinition,
		const FFinalBattleConditionEvaluationContext& Context) const;

	bool SatisfiesAllEffectConditions(
		const UFinalBattleEffectDefinition* EffectDefinition,
		const FFinalBattleConditionEvaluationContext& Context) const;

	bool SatisfiesResolvedCardCondition(
		const FFinalRelicRuntimeCardConditionDefinition& CardCondition,
		const FFinalBattleResolvedCardTriggerContext& CardContext) const;

	void RecordStatusChange(
		FFinalBattleEffectChainRecordContext& ChainRecords,
		FName OwnerUnitId,
		const FFinalStatusId& StatusId,
		EFinalBattleStatusChangeKind ChangeKind,
		int32 ChangedStacks) const;

	void RecordMovedCard(
		FFinalBattleEffectChainRecordContext& ChainRecords,
		FName RuntimeOwnerUnitId,
		const FFinalBattleCardInstance& CardInstance,
		EFinalBattleCardZoneRule SourceZone,
		EFinalBattleCardZoneRule DestinationZone,
		int32 MovedCount) const;

private:
	bool EvaluateCondition(
		const UFinalBattleConditionDefinition* Condition,
		const FFinalBattleConditionEvaluationContext& Context) const;
};
