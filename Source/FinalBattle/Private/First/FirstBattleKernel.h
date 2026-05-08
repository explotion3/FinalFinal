#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleCommand.h"
#include "First/FirstBattleSnapshot.h"
#include "First/FirstBattleState.h"

class FFirstBattleKernel
{
public:
	void Initialize(const FFirstBattleStartParams& StartParams);
	FFirstBattleCommandResult SubmitCommand(const FFirstBattleCommand& Command);
	FFirstBattleSnapshot BuildSnapshot() const;

private:
	FFirstBattleState State;

	FFirstBattleCommandResult ResolvePlayCard(const FFirstBattleCommand& Command);
	FFirstBattleCommandResult ResolveEndTurn();
	void ApplyDamageEffects(const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart);
	void ResolvePerfectReleaseEvents(const FFirstCardInstance& Card, const TMap<FName, int32>& PrePlayInitiatives);
	void ResolveInitiativeAfterCard(const FFirstCardInstance& Card, const TMap<FName, int32>& PreInitiativeByPart);
	void ResolveQueuedEnemyPartActions(const TArray<FName>& QueuedPartIds, const FGuid& SourceCardInstanceId);
	void ResolveEnemyPartAction(FFirstEnemyPartState& Part, const FGuid& SourceCardInstanceId);
	void ExecuteIntentEffects(const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FGuid& SourceCardInstanceId);
	void ApplyIntentDamageEffect(const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FFirstCardEffectInstance& Effect, const FGuid& SourceCardInstanceId);
	void ResolveVictory();
	void ResolvePlayerDefeat();
	void AppendEvent(const FFirstBattleEvent& Event);

	FFirstCardInstance* FindHandCard(const FGuid& CardInstanceId);
	FFirstEnemyPartState* FindEnemyPart(FName PartId);
	static void EnsureIntentSequence(FFirstEnemyPartState& Part);
	static void ApplyIntentFromSequence(FFirstEnemyPartState& Part);
	static bool IsAlivePart(const FFirstEnemyPartState& Part);
	static bool HasSwiftKeyword(const FFirstCardInstance& Card);
	static FFirstBattleCommandResult MakeRejectedResult(FName ReasonTag, const FText& Message);
	static FFirstBattleCommandResult MakeAcceptedResult(const FText& Message);
};
