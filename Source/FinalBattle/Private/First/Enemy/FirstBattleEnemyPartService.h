#pragma once

#include "CoreMinimal.h"
#include "First/Core/FirstBattleState.h"

class FFirstBattleEventLog;

class FFirstBattleEnemyPartService
{
public:
	FFirstEnemyPartState* FindEnemyPart(FFirstBattleState& State, FName PartId) const;
	void EnsureIntentSequence(FFirstEnemyPartState& Part) const;
	void ResolveInitiativeAfterCard(FFirstBattleState& State, const FFirstCardInstance& Card, const TMap<FName, int32>& PreInitiativeByPart, const FFirstBattleEventLog& EventLog) const;
	void ResolveQueuedEnemyPartActions(FFirstBattleState& State, const TArray<FName>& QueuedPartIds, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const;
	void ResolveEnemyPartAction(FFirstBattleState& State, FFirstEnemyPartState& Part, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const;
	void ResolveVictory(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const;
	void ResolvePlayerDefeat(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const;

	static bool IsAlivePart(const FFirstEnemyPartState& Part);

private:
	void ExecuteIntentEffects(FFirstBattleState& State, const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const;
	void ApplyIntentDamageEffect(FFirstBattleState& State, const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FFirstCardEffectInstance& Effect, const FGuid& SourceCardInstanceId, const FFirstBattleEventLog& EventLog) const;
	void ApplyIntentFromSequence(FFirstEnemyPartState& Part) const;
};
