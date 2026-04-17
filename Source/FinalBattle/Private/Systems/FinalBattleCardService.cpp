#include "Systems/FinalBattleCardService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalTeamDeckState.h"

namespace
{
FGameplayTag GetRetainKeyword()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Retain"));
}

FGameplayTag GetExpendKeyword()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Expend"));
}

const FFinalBattleCharacterState* FindCharacterState(const FFinalBattleState& BattleState, const FName RuntimeUnitId)
{
	return BattleState.Characters.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

bool RemoveCardInstanceId(TArray<FGuid>& CardInstanceIds, const FGuid& CardInstanceId)
{
	return CardInstanceIds.RemoveSingle(CardInstanceId) > 0;
}

bool MatchesGeneratedCardFilter(
	const FFinalBattleCardInstance& CardInstance,
	const FName RuntimeOwnerUnitId,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword,
	const bool bGeneratedOnly)
{
	if (CardInstance.RuntimeOwnerUnitId != RuntimeOwnerUnitId)
	{
		return false;
	}

	if (bGeneratedOnly && !CardInstance.bGeneratedCard)
	{
		return false;
	}

	if (RequiredCardId.IsValid() && CardInstance.CardId != RequiredCardId)
	{
		return false;
	}

	if (RequiredKeyword.IsValid() && !CardInstance.RuntimeKeywords.HasTagExact(RequiredKeyword))
	{
		return false;
	}

	return true;
}
}

void FFinalBattleCardService::InitializeDeckState(FFinalTeamDeckState& DeckState) const
{
	DeckState = FFinalTeamDeckState{};
}

void FFinalBattleCardService::InitializeDeckCards(
	FFinalBattleState& BattleState,
	const TArray<UFinalCardDefinition*>& DeckDefinitions,
	const TMap<FName, FName>& TemplateToRuntimeUnitMap) const
{
	for (UFinalCardDefinition* CardDefinition : DeckDefinitions)
	{
		if (CardDefinition == nullptr || !CardDefinition->CardId.IsValid())
		{
			continue;
		}

		const FName* RuntimeOwnerUnitIdPtr = TemplateToRuntimeUnitMap.Find(CardDefinition->OwnerUnitId);
		const FName RuntimeOwnerUnitId = RuntimeOwnerUnitIdPtr != nullptr
			? *RuntimeOwnerUnitIdPtr
			: CardDefinition->OwnerUnitId;
		const FGuid CardInstanceId = AddGeneratedCardToHand(
			BattleState,
			CardDefinition,
			RuntimeOwnerUnitId,
			false,
			false,
			CardDefinition->Keywords.HasTagExact(GetRetainKeyword()),
			CardDefinition->Keywords.HasTagExact(GetExpendKeyword()));
		RemoveCardInstanceId(BattleState.DeckState.HandCardInstanceIds, CardInstanceId);
		BattleState.DeckState.DrawPileCardInstanceIds.Add(CardInstanceId);
	}
}

FFinalBattleCardInstance* FFinalBattleCardService::FindCardInstance(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	return BattleState.CardInstances.FindByPredicate(
		[&CardInstanceId](const FFinalBattleCardInstance& Candidate)
		{
			return Candidate.CardInstanceId == CardInstanceId;
		});
}

const FFinalBattleCardInstance* FFinalBattleCardService::FindCardInstance(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	return BattleState.CardInstances.FindByPredicate(
		[&CardInstanceId](const FFinalBattleCardInstance& Candidate)
		{
			return Candidate.CardInstanceId == CardInstanceId;
		});
}

bool FFinalBattleCardService::IsCardInHand(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	return BattleState.DeckState.HandCardInstanceIds.Contains(CardInstanceId);
}

FGuid FFinalBattleCardService::AddGeneratedCardToHand(
	FFinalBattleState& BattleState,
	UFinalCardDefinition* CardDefinition,
	const FName RuntimeOwnerUnitId,
	const bool bGeneratedCard,
	const bool bTemporaryCard,
	const bool bRetainInHand,
	const bool bConsumeOnPlay) const
{
	if (CardDefinition == nullptr || !CardDefinition->CardId.IsValid())
	{
		return FGuid();
	}

	FFinalBattleCardInstance CardInstance;
	CardInstance.CardInstanceId = FGuid::NewGuid();
	CardInstance.CardId = CardDefinition->CardId;
	CardInstance.RuntimeOwnerUnitId = RuntimeOwnerUnitId;
	CardInstance.RuntimeCostAP = CardDefinition->BaseCostAP;
	CardInstance.RuntimeKeywords = CardDefinition->Keywords;
	CardInstance.SourceDefinition = CardDefinition;
	CardInstance.bGeneratedCard = bGeneratedCard;
	CardInstance.bTemporaryCard = bTemporaryCard;
	CardInstance.bRetained = bRetainInHand || CardInstance.RuntimeKeywords.HasTagExact(GetRetainKeyword());
	CardInstance.bConsumeOnPlay = bConsumeOnPlay || CardInstance.RuntimeKeywords.HasTagExact(GetExpendKeyword());

	BattleState.CardInstances.Add(CardInstance);
	BattleState.DeckState.HandCardInstanceIds.Add(CardInstance.CardInstanceId);
	return CardInstance.CardInstanceId;
}

int32 FFinalBattleCardService::ConsumeMatchingCardsFromHand(
	FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword,
	const int32 ConsumeCount,
	const bool bGeneratedOnly,
	TArray<FGuid>* OutConsumedCardInstanceIds) const
{
	const int32 TargetConsumeCount = FMath::Max(ConsumeCount, 0);
	if (TargetConsumeCount <= 0 || RuntimeOwnerUnitId.IsNone())
	{
		return 0;
	}

	if (OutConsumedCardInstanceIds != nullptr)
	{
		OutConsumedCardInstanceIds->Reset();
	}

	int32 ConsumedCount = 0;
	for (int32 HandIndex = BattleState.DeckState.HandCardInstanceIds.Num() - 1;
		HandIndex >= 0 && ConsumedCount < TargetConsumeCount;
		--HandIndex)
	{
		const FGuid CandidateCardInstanceId = BattleState.DeckState.HandCardInstanceIds[HandIndex];
		const FFinalBattleCardInstance* CandidateCardInstance = FindCardInstance(BattleState, CandidateCardInstanceId);
		if (CandidateCardInstance == nullptr
			|| !MatchesGeneratedCardFilter(*CandidateCardInstance, RuntimeOwnerUnitId, RequiredCardId, RequiredKeyword, bGeneratedOnly))
		{
			continue;
		}

		BattleState.DeckState.HandCardInstanceIds.RemoveAt(HandIndex);
		BattleState.DeckState.ConsumePileCardInstanceIds.Add(CandidateCardInstanceId);
		if (OutConsumedCardInstanceIds != nullptr)
		{
			OutConsumedCardInstanceIds->Add(CandidateCardInstanceId);
		}
		++ConsumedCount;
	}

	return ConsumedCount;
}

void FFinalBattleCardService::MoveHandCardAfterPlay(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	BattleState.DeckState.HandCardInstanceIds.RemoveSingle(CardInstanceId);

	const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	if (CardInstance != nullptr && CardInstance->bConsumeOnPlay)
	{
		BattleState.DeckState.ConsumePileCardInstanceIds.Add(CardInstanceId);
		return;
	}

	BattleState.DeckState.DiscardPileCardInstanceIds.Add(CardInstanceId);
}

int32 FFinalBattleCardService::DrawCards(FFinalBattleState& BattleState, const int32 DrawCount) const
{
	int32 DrawnCount = 0;

	for (int32 DrawIndex = 0; DrawIndex < DrawCount; ++DrawIndex)
	{
		if (BattleState.DeckState.DrawPileCardInstanceIds.Num() == 0)
		{
			if (BattleState.DeckState.DiscardPileCardInstanceIds.Num() == 0)
			{
				return DrawnCount;
			}

			BattleState.DeckState.DrawPileCardInstanceIds.Append(BattleState.DeckState.DiscardPileCardInstanceIds);
			BattleState.DeckState.DiscardPileCardInstanceIds.Reset();
		}

		const FGuid DrawnCardId = BattleState.DeckState.DrawPileCardInstanceIds[0];
		BattleState.DeckState.DrawPileCardInstanceIds.RemoveAt(0);
		BattleState.DeckState.HandCardInstanceIds.Add(DrawnCardId);
		++DrawnCount;
	}

	return DrawnCount;
}

int32 FFinalBattleCardService::DrawUpToHandSize(FFinalBattleState& BattleState, const int32 TargetHandSize) const
{
	return DrawCards(BattleState, FMath::Max(TargetHandSize - BattleState.DeckState.HandCardInstanceIds.Num(), 0));
}

void FFinalBattleCardService::BuildHandCardViews(const FFinalBattleState& BattleState, TArray<FFinalBattleCardViewData>& OutViews) const
{
	for (const FGuid& CardInstanceId : BattleState.DeckState.HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
		if (CardInstance == nullptr)
		{
			continue;
		}

		FFinalBattleCardViewData CardView;
		CardView.CardInstanceId = CardInstance->CardInstanceId;
		CardView.CardId = CardInstance->CardId;
		CardView.RuntimeOwnerUnitId = CardInstance->RuntimeOwnerUnitId;
		CardView.DisplayName = CardInstance->SourceDefinition != nullptr
			? CardInstance->SourceDefinition->DisplayName
			: FText::FromName(CardInstance->CardId.Value);
		CardView.CardType = CardInstance->SourceDefinition != nullptr
			? CardInstance->SourceDefinition->CardType
			: EFinalCardType::Attack;
		CardView.RuntimeCostAP = CardInstance->RuntimeCostAP;
		CardView.RuntimeKeywords = CardInstance->RuntimeKeywords;
		CardView.bRetained = CardInstance->bRetained;
		if (const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(BattleState, CardInstance->RuntimeOwnerUnitId))
		{
			CardView.bCollapsedCard = OwnerCharacterState->bCollapsed;
		}

		OutViews.Add(MoveTemp(CardView));
	}
}
