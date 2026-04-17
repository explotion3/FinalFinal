#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalBattleCharacterStatusesViewData;
struct FFinalBattleState;
struct FFinalBattleStatusInstance;
struct FFinalBattleStatusViewData;
class UFinalStatusDefinition;

class FFinalBattleStatusService
{
public:
	void TickStatusWindows(FFinalBattleState& BattleState) const;
	int32 AddStatusStacks(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		FName SourceUnitId,
		const FFinalStatusId& StatusId,
		const UFinalStatusDefinition* StatusDefinition,
		int32 StacksToAdd,
		int32 DurationOverride = 0) const;
	int32 RemoveStatusStacks(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		const FFinalStatusId& StatusId,
		int32 StacksToRemove) const;
	int32 GetStatusStacks(const FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalStatusId& StatusId) const;
	const FFinalBattleStatusInstance* FindStatusInstance(const FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalStatusId& StatusId) const;
	FFinalBattleStatusInstance* FindStatusInstance(FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalStatusId& StatusId) const;
	void BuildStatusSnapshotData(
		const FFinalBattleState& BattleState,
		TArray<FFinalBattleCharacterStatusesViewData>& OutCharacterStatuses,
		TArray<FFinalBattleStatusViewData>& OutTeamStatuses,
		TArray<FFinalBattleStatusViewData>& OutStatuses) const;
};
