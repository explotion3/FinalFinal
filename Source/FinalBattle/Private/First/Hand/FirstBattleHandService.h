#pragma once

#include "CoreMinimal.h"
#include "First/Core/FirstBattleState.h"

class FFirstBattleEventLog;

class FFirstBattleHandService
{
public:
	FFirstCardInstance* FindHandCard(FFirstBattleState& State, const FGuid& CardInstanceId) const;
	EFirstHandZone ResolveCurrentHandZoneForCardIndex(const FFirstBattleState& State, int32 CardIndex) const;
	bool MoveRandomHandCard(FFirstBattleState& State, const FFirstCardInstance& SourceCard, const FFirstCardEffectInstance& Effect, const FFirstBattleEventLog& EventLog) const;
	TArray<EFirstHandZone> ResolveValidHandMoveTargetZones(const FFirstBattleState& State) const;
	bool InsertCardIntoZone(FFirstBattleState& State, FFirstCardInstance&& Card, EFirstHandZone TargetZone) const;
	void RebuildHandWithRandomZones(FFirstBattleState& State, TArray<FFirstCardInstance>& Cards) const;

	static void ResolveHandAnchorIndices(const TArray<FFirstCardInstance>& HandCards, int32& OutLeftHandIndex, int32& OutRightHandIndex);
	static bool IsValidHandMoveTargetZone(const TArray<FFirstCardInstance>& HandCards, EFirstHandZone Zone);
	static EFirstHandZone ResolveHandZoneForCard(const TArray<FFirstCardInstance>& HandCards, int32 CardIndex, int32 LeftHandIndex, int32 RightHandIndex);

private:
	TArray<int32> CollectMoveCandidateIndices(const FFirstBattleState& State, const FFirstCardEffectInstance& Effect) const;
	TArray<EFirstHandZone> ResolveMoveTargetZones(const FFirstBattleState& State, const FFirstCardEffectInstance& Effect, EFirstHandZone SourceZone) const;
	int32 ApplyRuntimeCostDelta(FFirstCardInstance& Card, int32 CostDelta) const;
	void ApplyTransferredCostReductionToSourceCard(FFirstBattleState& State, const FFirstCardInstance& SourceCard, int32 ActualCostReduction, const FFirstBattleEventLog& EventLog) const;
	int32 RollZoneBucket(FFirstBattleState& State, bool bHasLeftCore, bool bHasRightCore) const;
};
