#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleHandCardRequirement.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"

class UFinalCardDefinition;
struct FFinalBattleCardInstance;
struct FFinalBattleCardViewData;
struct FFinalBattleState;
struct FFinalTeamDeckState;

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
	FGuid AddGeneratedCardToHand(
		FFinalBattleState& BattleState,
		UFinalCardDefinition* CardDefinition,
		FName RuntimeOwnerUnitId,
		bool bGeneratedCard,
		bool bTemporaryCard) const;
	int32 ConsumeMatchingCardsFromHand(
		FFinalBattleState& BattleState,
		FName RuntimeOwnerUnitId,
		const FFinalCardId& RequiredCardId,
		const FGameplayTag& RequiredKeyword,
		int32 ConsumeCount,
		bool bGeneratedOnly,
		TArray<FGuid>* OutConsumedCardInstanceIds = nullptr) const;
	void MoveHandCardAfterPlay(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const;
	int32 DrawCards(FFinalBattleState& BattleState, int32 DrawCount) const;
	int32 DrawUpToHandSize(FFinalBattleState& BattleState, int32 TargetHandSize) const;
	void BuildHandCardViews(const FFinalBattleState& BattleState, TArray<FFinalBattleCardViewData>& OutViews) const;
};
