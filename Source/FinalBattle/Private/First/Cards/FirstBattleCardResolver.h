#pragma once

#include "CoreMinimal.h"
#include "First/Core/FirstBattleState.h"

class FFirstBattleEnemyPartService;
class FFirstBattleEventLog;
class FFirstBattleHandService;

class FFirstBattleCardResolver
{
public:
	void ExecuteCardEffects(FFirstBattleState& State, const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart, const FFirstBattleEnemyPartService& EnemyPartService, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const;
	bool ResolvePerfectReleaseEvents(FFirstBattleState& State, const FFirstCardInstance& Card, const TMap<FName, int32>& PrePlayInitiatives, const FFirstBattleEventLog& EventLog) const;
	void ResolvePlayedCardDestination(FFirstBattleState& State, const FFirstCardInstance& PlayedCard, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const;
	bool ShouldSkipInitiativeReductionForZonePerfectRelease(const FFirstCardInstance& Card, EFirstHandZone PlayedHandZone, bool bTriggeredPerfectRelease) const;
	static bool HasSwiftKeyword(const FFirstCardInstance& Card);

private:
	void ApplyCardDamageEffect(FFirstBattleState& State, const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart, const FFirstCardEffectInstance& Effect, const FFirstBattleEnemyPartService& EnemyPartService, const FFirstBattleEventLog& EventLog) const;
	void ApplyMoveHandCardEffect(FFirstBattleState& State, const FFirstCardInstance& Card, const FFirstCardEffectInstance& Effect, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const;
	void ReturnPlayedCardToHandRandomZone(FFirstBattleState& State, const FFirstCardInstance& PlayedCard, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const;
};
