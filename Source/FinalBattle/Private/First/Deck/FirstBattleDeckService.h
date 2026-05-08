#pragma once

#include "CoreMinimal.h"
#include "First/Core/FirstBattleState.h"

class FFirstBattleEventLog;
class FFirstBattleHandService;

class FFirstBattleDeckService
{
public:
	void ApplyEntryStatBonuses(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const;
	void BeginPlayerTurnDraw(FFirstBattleState& State, const FFirstBattleEventLog& EventLog, const FFirstBattleHandService& HandService) const;

private:
	void ApplyCardEntryStatBonus(FFirstBattleState& State, const FFirstCardInstance& Card, const FFirstBattleEventLog& EventLog) const;
	void PullMissingCoreCards(FFirstBattleState& State, TArray<FFirstCardInstance>& DrawnCards, int32 DrawTargetCount, const FFirstBattleEventLog& EventLog) const;
	bool PullMissingCoreCard(FFirstBattleState& State, EFirstHandRole HandRole, TArray<FFirstCardInstance>& DrawnCards, const FFirstBattleEventLog& EventLog) const;
	bool RemoveFirstCardWithRole(TArray<FFirstCardInstance>& Cards, EFirstHandRole HandRole, FFirstCardInstance& OutCard) const;
	void DrawCardsFromDrawPile(FFirstBattleState& State, TArray<FFirstCardInstance>& DrawnCards, int32 DrawTargetCount, const FFirstBattleEventLog& EventLog) const;
	bool ShuffleDiscardIntoDrawPile(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const;
	void ShuffleCards(FFirstBattleState& State, TArray<FFirstCardInstance>& Cards) const;
};
