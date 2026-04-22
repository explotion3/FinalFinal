#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleHandCardRequirement.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Systems/FinalBattleCardMatchCriteria.h"

class UFinalCardDefinition;
struct FFinalBattleCardInstance;
struct FFinalBattleCardViewData;
struct FFinalBattleState;
struct FFinalTeamDeckState;

enum class EFinalBattleCardZone : uint8
{
	Hand,
	DrawPileTop,
	DrawPileBottom,
	DiscardPile,
	OngoingZone,
	ConsumePile
};

class FFinalBattleCardService
{
public:
	void InitializeDeckState(FFinalTeamDeckState& DeckState) const;
	void InitializeDeckCards(
		FFinalBattleState& BattleState,
		const TArray<UFinalCardDefinition*>& DeckDefinitions,
		const TMap<FName, FName>& TemplateToRuntimeUnitMap) const;
	FFinalBattleCardInstance* FindCardInstance(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
	const FFinalBattleCardInstance* FindCardInstance(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
	bool IsCardInHand(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
	int32 CountMatchingCardsInHand(
		const FFinalBattleState& BattleState,
		FName RuntimeOwnerUnitId,
		const FFinalCardId& RequiredCardId,
		const FGameplayTag& RequiredKeyword,
		bool bGeneratedOnly) const;
	bool SatisfiesHandCardRequirement(
		const FFinalBattleState& BattleState,
		FName RuntimeOwnerUnitId,
		const FFinalBattleHandCardRequirement& Requirement) const;
	FGuid CreateCardInstance(
		FFinalBattleState& BattleState,
		UFinalCardDefinition* CardDefinition,
		FName RuntimeOwnerUnitId,
		bool bGeneratedCard = false,
		bool bTemporaryCard = false) const;
	bool MoveCardInstanceToZone(
		FFinalBattleState& BattleState,
		const FGuid& CardInstanceId,
		EFinalBattleCardZone Zone) const;
	int32 MoveMatchingCardsBetweenZones(
		FFinalBattleState& BattleState,
		EFinalBattleCardZone SourceZone,
		EFinalBattleCardZone DestinationZone,
		const FFinalBattleCardMatchCriteria& Criteria,
		int32 MoveCount,
		TArray<FGuid>* OutMovedCardInstanceIds = nullptr) const;
	void MoveHandCardAfterPlay(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
	int32 DrawCards(FFinalBattleState& BattleState, int32 DrawCount) const;
	int32 DrawUpToHandSize(FFinalBattleState& BattleState, int32 TargetHandSize) const;
	void BuildHandCardViews(const FFinalBattleState& BattleState, TArray<FFinalBattleCardViewData>& OutViews) const;

private:
	bool RefillDrawPileFromDiscard(FFinalBattleState& BattleState) const;
	void CollectMatchingCardInstanceIdsInZone(
		const FFinalBattleState& BattleState,
		EFinalBattleCardZone SourceZone,
		const FFinalBattleCardMatchCriteria& Criteria,
		int32 MaxCount,
		TArray<FGuid>& OutCardInstanceIds) const;
	void RemoveCardInstanceFromAllZones(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
};
