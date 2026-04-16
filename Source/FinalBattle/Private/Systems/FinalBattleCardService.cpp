#include "Systems/FinalBattleCardService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalTeamDeckState.h"

namespace
{
const FFinalBattleCharacterState* FindCharacterState(const FFinalBattleState& BattleState, const FName RuntimeUnitId)
{
	return BattleState.Characters.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
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

		FFinalBattleCardInstance CardInstance;
		CardInstance.CardInstanceId = FGuid::NewGuid();
		CardInstance.CardId = CardDefinition->CardId;
		CardInstance.RuntimeCostAP = CardDefinition->BaseCostAP;
		CardInstance.RuntimeKeywords = CardDefinition->Keywords;
		CardInstance.SourceDefinition = CardDefinition;

		if (const FName* RuntimeOwnerUnitId = TemplateToRuntimeUnitMap.Find(CardDefinition->OwnerUnitId))
		{
			CardInstance.RuntimeOwnerUnitId = *RuntimeOwnerUnitId;
		}
		else
		{
			CardInstance.RuntimeOwnerUnitId = CardDefinition->OwnerUnitId;
		}

		BattleState.CardInstances.Add(CardInstance);
		BattleState.DeckState.DrawPileCardInstanceIds.Add(CardInstance.CardInstanceId);
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

void FFinalBattleCardService::MoveHandCardToDiscard(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	BattleState.DeckState.HandCardInstanceIds.RemoveSingle(CardInstanceId);
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
