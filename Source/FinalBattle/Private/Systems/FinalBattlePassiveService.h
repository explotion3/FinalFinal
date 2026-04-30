#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"

class UFinalPassiveDefinition;
struct FFinalBattlePassiveInstance;
struct FFinalBattlePassiveViewData;
struct FFinalBattleState;

struct FFinalBattlePassiveApplyResult
{
	bool bApplied = false;
	bool bCreatedNewInstance = false;
	FGuid PassiveInstanceId;
	FFinalPassiveId PassiveId;
	FText DisplayName;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	int32 CurrentStacks = 0;
	int32 RemainingDuration = 0;
};

struct FFinalBattlePassiveRemovalResult
{
	FGuid PassiveInstanceId;
	FFinalPassiveId PassiveId;
	FText DisplayName;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	int32 CurrentStacksBeforeRemoval = 0;
	int32 RemainingDurationBeforeRemoval = 0;
	FName RemovalReasonTag = NAME_None;
};

class FFinalBattlePassiveService
{
public:
	FFinalBattlePassiveApplyResult ApplyPassive(
		FFinalBattleState& BattleState,
		FName OwnerUnitId,
		FName SourceUnitId,
		const FFinalPassiveId& PassiveId,
		const UFinalPassiveDefinition* PassiveDefinition,
		int32 StacksToAdd,
		int32 DurationOverride = 0) const;

	void ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const;
	TArray<FFinalBattlePassiveRemovalResult> ResolvePlayerTurnEndPassives(FFinalBattleState& BattleState) const;
	void BuildPassiveSnapshotData(const FFinalBattleState& BattleState, TArray<FFinalBattlePassiveViewData>& OutPassives) const;

	const FFinalBattlePassiveInstance* FindPassiveInstance(const FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalPassiveId& PassiveId) const;
	FFinalBattlePassiveInstance* FindPassiveInstance(FFinalBattleState& BattleState, FName OwnerUnitId, const FFinalPassiveId& PassiveId) const;
};
