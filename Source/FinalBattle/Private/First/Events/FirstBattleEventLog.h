#pragma once

#include "CoreMinimal.h"
#include "First/Core/FirstBattleState.h"

class FFirstBattleEventLog
{
public:
	void AppendEvent(FFirstBattleState& State, const FFirstBattleEvent& Event) const;
	void AppendPlayerTurnStartedEvent(FFirstBattleState& State) const;
	void AppendPlayerMaxHPChangedEvent(FFirstBattleState& State, const FFirstCardInstance& Card, int32 PreviousMaxHP, int32 NewMaxHP) const;
	void AppendCardDrawnEvent(FFirstBattleState& State, const FFirstCardInstance& Card, EFirstCardDrawSource DrawSource) const;
	void AppendDrawPileShuffledEvent(FFirstBattleState& State, int32 ShuffledCount) const;
	void AppendCardReturnedToHandEvent(FFirstBattleState& State, const FFirstCardInstance& Card, EFirstHandZone TargetZone) const;
	void AppendHandCardMovedEvent(FFirstBattleState& State, const FFirstCardInstance& SourceCard, const FFirstCardInstance& MovedCard, EFirstHandZone SourceZone, EFirstHandZone TargetZone) const;
	void AppendCardRuntimeCostChangedEvent(FFirstBattleState& State, const FFirstCardInstance& Card, int32 PreviousCost, int32 NewCost) const;
};
