#pragma once

#include "CoreMinimal.h"
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
	FGuid AddGeneratedCardToHand(
		FFinalBattleState& BattleState,
		UFinalCardDefinition* CardDefinition,
		FName RuntimeOwnerUnitId,
		bool bGeneratedCard,
		bool bTemporaryCard,
		bool bRetainInHand,
		bool bConsumeOnPlay) const;
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
