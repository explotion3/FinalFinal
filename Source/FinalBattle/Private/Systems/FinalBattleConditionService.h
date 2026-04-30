#pragma once

#include "CoreMinimal.h"
#include "Systems/FinalBattleConditionTypes.h"

class UFinalBattleConditionDefinition;
class UFinalBattleEffectDefinition;
struct FFinalBattleCardInstance;

class FFinalBattleConditionService
{
public:
	bool SatisfiesConditions(
		const TArray<TObjectPtr<UFinalBattleConditionDefinition>>& Conditions,
		const FFinalBattleConditionEvaluationContext& Context) const;

	bool SatisfiesSourceAndChainConditions(
		const UFinalBattleEffectDefinition* EffectDefinition,
		const FFinalBattleConditionEvaluationContext& Context) const;

	bool SatisfiesTargetConditions(
		const UFinalBattleEffectDefinition* EffectDefinition,
		const FFinalBattleConditionEvaluationContext& Context) const;

	bool SatisfiesAllEffectConditions(
		const UFinalBattleEffectDefinition* EffectDefinition,
		const FFinalBattleConditionEvaluationContext& Context) const;

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
	void RecordResourceConsumed(
		FFinalBattleEffectChainRecordContext& ChainRecords,
		FName OwnerUnitId,
		const FFinalStatusId& StatusId,
		int32 ConsumedStacks) const;

private:
	bool EvaluateCondition(
		const UFinalBattleConditionDefinition* Condition,
		const FFinalBattleConditionEvaluationContext& Context) const;
};
