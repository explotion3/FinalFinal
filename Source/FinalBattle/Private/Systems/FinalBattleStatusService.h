#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalBattleCharacterStatusesViewData;
class FFinalBattleCardService;
struct FFinalBattleState;
struct FFinalBattleStatusInstance;
struct FFinalBattleStatusViewData;
class UFinalStatusDefinition;

class FFinalBattleStatusService
{
public:
	void ResolvePlayerTurnEndStatuses(FFinalBattleState& BattleState) const;
	int32 AddStatusStacks(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		FName SourceUnitId,
		const FFinalStatusId& StatusId,
		const UFinalStatusDefinition* StatusDefinition,
		int32 StacksToAdd,
		int32 DurationOverride = 0) const;
	int32 GetOutgoingDamageModifierPercent(
		const FFinalBattleState& BattleState,
		FName OwnerUnitId,
		bool bIsAttackCardDamage) const;
	int32 ConsumeOutgoingDamageModifierStacks(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		bool bIsAttackCardDamage) const;
	int32 GetIncomingTeamHealthDamageReductionPercent(const FFinalBattleState& BattleState) const;
	int32 ApplyIncomingTeamHealthDamageProtection(FFinalBattleState& BattleState, int32 IncomingHealthDamage) const;
	int32 RemoveStatusStacks(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		const FFinalStatusId& StatusId,
		int32 StacksToRemove) const;
	bool CanConsumeStatusResource(
		const FFinalBattleState& BattleState,
		FName OwnerUnitId,
		const FFinalStatusId& StatusId,
		int32 StacksToConsume) const;
	int32 ConsumeStatusResource(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		const FFinalStatusId& StatusId,
		int32 StacksToConsume) const;
	int32 GetStatusStacks(const FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalStatusId& StatusId) const;
	const FFinalBattleStatusInstance* FindStatusInstance(const FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalStatusId& StatusId) const;
	FFinalBattleStatusInstance* FindStatusInstance(FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalStatusId& StatusId) const;
	void ResyncProjectedHandCardModifiers(
		FFinalBattleState& BattleState,
		const FFinalBattleCardService& CardService,
		FName OwnerUnitId) const;
	void BuildStatusSnapshotData(
		const FFinalBattleState& BattleState,
		TArray<FFinalBattleCharacterStatusesViewData>& OutCharacterStatuses,
		TArray<FFinalBattleStatusViewData>& OutTeamStatuses,
		TArray<FFinalBattleStatusViewData>& OutStatuses) const;
};
