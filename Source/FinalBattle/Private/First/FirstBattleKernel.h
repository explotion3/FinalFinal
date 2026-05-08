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
	void ExecuteCardEffects(const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart);
	void ApplyCardDamageEffect(const FFirstCardInstance& Card, FFirstEnemyPartState& TargetPart, const FFirstCardEffectInstance& Effect);
	void ApplyMoveHandCardEffect(const FFirstCardInstance& Card, const FFirstCardEffectInstance& Effect);
	bool ResolvePerfectReleaseEvents(const FFirstCardInstance& Card, const TMap<FName, int32>& PrePlayInitiatives);
	void ResolveInitiativeAfterCard(const FFirstCardInstance& Card, const TMap<FName, int32>& PreInitiativeByPart);
	void ResolveQueuedEnemyPartActions(const TArray<FName>& QueuedPartIds, const FGuid& SourceCardInstanceId);
	void ResolveEnemyPartAction(FFirstEnemyPartState& Part, const FGuid& SourceCardInstanceId);
	void ExecuteIntentEffects(const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FGuid& SourceCardInstanceId);
	void ApplyIntentDamageEffect(const FFirstEnemyPartState& Part, const FFirstEnemyPartIntentInstance& Intent, const FFirstCardEffectInstance& Effect, const FGuid& SourceCardInstanceId);
	void BeginPlayerTurnDraw();
	void ApplyEntryStatBonuses();
	void ApplyCardEntryStatBonus(const FFirstCardInstance& Card);
	void PullMissingCoreCards(TArray<FFirstCardInstance>& DrawnCards, int32 DrawTargetCount);
	bool PullMissingCoreCard(EFirstHandRole HandRole, TArray<FFirstCardInstance>& DrawnCards);
	bool RemoveFirstCardWithRole(TArray<FFirstCardInstance>& Cards, EFirstHandRole HandRole, FFirstCardInstance& OutCard);
	void DrawCardsFromDrawPile(TArray<FFirstCardInstance>& DrawnCards, int32 DrawTargetCount);
	bool ShuffleDiscardIntoDrawPile();
	void ShuffleCards(TArray<FFirstCardInstance>& Cards);
	void RebuildHandWithRandomZones(TArray<FFirstCardInstance>& Cards);
	int32 RollZoneBucket(bool bHasLeftCore, bool bHasRightCore);
	void ResolveVictory();
	void ResolvePlayerDefeat();
	void AppendEvent(const FFirstBattleEvent& Event);
	void AppendPlayerTurnStartedEvent();
	void AppendPlayerMaxHPChangedEvent(const FFirstCardInstance& Card, int32 PreviousMaxHP, int32 NewMaxHP);
	void AppendCardDrawnEvent(const FFirstCardInstance& Card, EFirstCardDrawSource DrawSource);
	void AppendDrawPileShuffledEvent(int32 ShuffledCount);
	void ResolvePlayedCardDestination(const FFirstCardInstance& PlayedCard);
	void ReturnPlayedCardToHandRandomZone(const FFirstCardInstance& PlayedCard);
	TArray<EFirstHandZone> ResolveValidHandMoveTargetZones() const;
	void AppendCardReturnedToHandEvent(const FFirstCardInstance& Card, EFirstHandZone TargetZone);

	FFirstCardInstance* FindHandCard(const FGuid& CardInstanceId);
	FFirstEnemyPartState* FindEnemyPart(FName PartId);
	EFirstHandZone ResolveCurrentHandZoneForCardIndex(int32 CardIndex) const;
	bool MoveRandomHandCard(const FFirstCardInstance& SourceCard, const FFirstCardEffectInstance& Effect);
	TArray<int32> CollectMoveCandidateIndices(const FFirstCardEffectInstance& Effect) const;
	TArray<EFirstHandZone> ResolveMoveTargetZones(const FFirstCardEffectInstance& Effect, EFirstHandZone SourceZone) const;
	bool InsertCardIntoZone(FFirstCardInstance&& Card, EFirstHandZone TargetZone);
	void AppendHandCardMovedEvent(const FFirstCardInstance& SourceCard, const FFirstCardInstance& MovedCard, EFirstHandZone SourceZone, EFirstHandZone TargetZone);
	int32 ApplyRuntimeCostDelta(FFirstCardInstance& Card, int32 CostDelta);
	void ApplyTransferredCostReductionToSourceCard(const FFirstCardInstance& SourceCard, int32 ActualCostReduction);
	void AppendCardRuntimeCostChangedEvent(const FFirstCardInstance& Card, int32 PreviousCost, int32 NewCost);
	bool ShouldSkipInitiativeReductionForZonePerfectRelease(const FFirstCardInstance& Card, EFirstHandZone PlayedHandZone, bool bTriggeredPerfectRelease) const;
	static void ResolveHandAnchorIndices(const TArray<FFirstCardInstance>& HandCards, int32& OutLeftHandIndex, int32& OutRightHandIndex);
	static bool IsValidHandMoveTargetZone(const TArray<FFirstCardInstance>& HandCards, EFirstHandZone Zone);
	static void EnsureIntentSequence(FFirstEnemyPartState& Part);
	static void ApplyIntentFromSequence(FFirstEnemyPartState& Part);
	static EFirstHandZone ResolveHandZoneForCard(const TArray<FFirstCardInstance>& HandCards, int32 CardIndex, int32 LeftHandIndex, int32 RightHandIndex);
	static bool IsAlivePart(const FFirstEnemyPartState& Part);
	static bool HasSwiftKeyword(const FFirstCardInstance& Card);
	static FFirstBattleCommandResult MakeRejectedResult(FName ReasonTag, const FText& Message);
	static FFirstBattleCommandResult MakeAcceptedResult(const FText& Message);
};
