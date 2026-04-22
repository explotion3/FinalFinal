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

bool HasRetainKeyword(const FGameplayTagContainer& Keywords)
{
	return Keywords.HasTagExact(GetRetainKeyword());
}

bool HasExpendKeyword(const FGameplayTagContainer& Keywords)
{
	return Keywords.HasTagExact(GetExpendKeyword());
}

int32 ResolveInitialRecycleCount(const FGameplayTagContainer& Keywords)
{
	return 0;
}

void InitializeRuntimeKeywordState(FFinalBattleCardInstance& CardInstance)
{
	CardInstance.RuntimeBehavior.bRetained = HasRetainKeyword(CardInstance.RuntimeKeywords);
	CardInstance.RuntimeBehavior.bConsumeOnPlay = HasExpendKeyword(CardInstance.RuntimeKeywords);
	CardInstance.RuntimeBehavior.RecycleCount = ResolveInitialRecycleCount(CardInstance.RuntimeKeywords);
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

TArray<FGuid>* ResolveZoneArray(FFinalTeamDeckState& DeckState, const EFinalBattleCardZone Zone)
{
	switch (Zone)
	{
	case EFinalBattleCardZone::Hand:
		return &DeckState.HandCardInstanceIds;

	case EFinalBattleCardZone::DrawPileTop:
	case EFinalBattleCardZone::DrawPileBottom:
		return &DeckState.DrawPileCardInstanceIds;

	case EFinalBattleCardZone::DiscardPile:
		return &DeckState.DiscardPileCardInstanceIds;

	case EFinalBattleCardZone::OngoingZone:
		return &DeckState.OngoingZoneCardInstanceIds;

	case EFinalBattleCardZone::ConsumePile:
		return &DeckState.ConsumePileCardInstanceIds;

	default:
		return nullptr;
	}
}

const TArray<FGuid>* ResolveZoneArray(const FFinalTeamDeckState& DeckState, const EFinalBattleCardZone Zone)
{
	switch (Zone)
	{
	case EFinalBattleCardZone::Hand:
		return &DeckState.HandCardInstanceIds;

	case EFinalBattleCardZone::DrawPileTop:
	case EFinalBattleCardZone::DrawPileBottom:
		return &DeckState.DrawPileCardInstanceIds;

	case EFinalBattleCardZone::DiscardPile:
		return &DeckState.DiscardPileCardInstanceIds;

	case EFinalBattleCardZone::OngoingZone:
		return &DeckState.OngoingZoneCardInstanceIds;

	case EFinalBattleCardZone::ConsumePile:
		return &DeckState.ConsumePileCardInstanceIds;

	default:
		return nullptr;
	}
}

void InsertCardInstanceIntoZoneArray(TArray<FGuid>& ZoneArray, const FGuid& CardInstanceId, const EFinalBattleCardZone Zone)
{
	if (Zone == EFinalBattleCardZone::DrawPileTop)
	{
		ZoneArray.Insert(CardInstanceId, 0);
		return;
	}

	ZoneArray.Add(CardInstanceId);
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
		const FGuid CardInstanceId = CreateCardInstance(
			BattleState,
			CardDefinition,
			RuntimeOwnerUnitId,
			false,
			false);
		if (CardInstanceId.IsValid())
		{
			MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::DrawPileBottom);
		}
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

int32 FFinalBattleCardService::CountMatchingCardsInHand(
	const FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword,
	const bool bGeneratedOnly) const
{
	TArray<FGuid> MatchingCardInstanceIds;
	CollectMatchingCardInstanceIdsInZone(
		BattleState,
		RuntimeOwnerUnitId,
		EFinalBattleCardZone::Hand,
		RequiredCardId,
		RequiredKeyword,
		MAX_int32,
		bGeneratedOnly,
		MatchingCardInstanceIds);
	return MatchingCardInstanceIds.Num();
}

bool FFinalBattleCardService::SatisfiesHandCardRequirement(
	const FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const FFinalBattleHandCardRequirement& Requirement) const
{
	if (!Requirement.bRequireInHand)
	{
		return true;
	}

	return CountMatchingCardsInHand(
		BattleState,
		RuntimeOwnerUnitId,
		Requirement.RequiredCardId,
		Requirement.RequiredKeyword,
		Requirement.bGeneratedOnly) >= FMath::Max(Requirement.MinimumCount, 1);
}

FGuid FFinalBattleCardService::CreateCardInstance(
	FFinalBattleState& BattleState,
	UFinalCardDefinition* CardDefinition,
	const FName RuntimeOwnerUnitId,
	const bool bGeneratedCard,
	const bool bTemporaryCard) const
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
	InitializeRuntimeKeywordState(CardInstance);

	BattleState.CardInstances.Add(CardInstance);
	return CardInstance.CardInstanceId;
}

bool FFinalBattleCardService::MoveCardInstanceToZone(
	FFinalBattleState& BattleState,
	const FGuid& CardInstanceId,
	const EFinalBattleCardZone Zone) const
{
	if (!CardInstanceId.IsValid() || FindCardInstance(BattleState, CardInstanceId) == nullptr)
	{
		return false;
	}

	TArray<FGuid>* ZoneArray = ResolveZoneArray(BattleState.DeckState, Zone);
	if (ZoneArray == nullptr)
	{
		return false;
	}

	RemoveCardInstanceFromAllZones(BattleState, CardInstanceId);
	InsertCardInstanceIntoZoneArray(*ZoneArray, CardInstanceId, Zone);
	return true;
}

int32 FFinalBattleCardService::MoveMatchingCardsBetweenZones(
	FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const EFinalBattleCardZone SourceZone,
	const EFinalBattleCardZone DestinationZone,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword,
	const int32 MoveCount,
	const bool bGeneratedOnly,
	TArray<FGuid>* OutMovedCardInstanceIds) const
{
	const int32 TargetMoveCount = FMath::Max(MoveCount, 0);
	if (TargetMoveCount <= 0 || RuntimeOwnerUnitId.IsNone())
	{
		return 0;
	}

	if (OutMovedCardInstanceIds != nullptr)
	{
		OutMovedCardInstanceIds->Reset();
	}

	TArray<FGuid> MatchedCardInstanceIds;
	CollectMatchingCardInstanceIdsInZone(
		BattleState,
		RuntimeOwnerUnitId,
		SourceZone,
		RequiredCardId,
		RequiredKeyword,
		TargetMoveCount,
		bGeneratedOnly,
		MatchedCardInstanceIds);

	int32 MovedCount = 0;
	for (const FGuid& MatchedCardInstanceId : MatchedCardInstanceIds)
	{
		if (!MoveCardInstanceToZone(BattleState, MatchedCardInstanceId, DestinationZone))
		{
			continue;
		}

		if (OutMovedCardInstanceIds != nullptr)
		{
			OutMovedCardInstanceIds->Add(MatchedCardInstanceId);
		}
		++MovedCount;
	}

	return MovedCount;
}

void FFinalBattleCardService::MoveHandCardAfterPlay(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	if (CardInstance != nullptr && CardInstance->RuntimeBehavior.bConsumeOnPlay)
	{
		MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::ConsumePile);
		return;
	}

	MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::DiscardPile);
}

int32 FFinalBattleCardService::DrawCards(FFinalBattleState& BattleState, const int32 DrawCount) const
{
	int32 DrawnCount = 0;

	for (int32 DrawIndex = 0; DrawIndex < DrawCount; ++DrawIndex)
	{
		if (BattleState.DeckState.DrawPileCardInstanceIds.Num() == 0)
		{
			if (!RefillDrawPileFromDiscard(BattleState))
			{
				return DrawnCount;
			}
		}

		const FGuid DrawnCardId = BattleState.DeckState.DrawPileCardInstanceIds[0];
		MoveCardInstanceToZone(BattleState, DrawnCardId, EFinalBattleCardZone::Hand);
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
		CardView.bRetained = CardInstance->RuntimeBehavior.bRetained;
		if (const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(BattleState, CardInstance->RuntimeOwnerUnitId))
		{
			CardView.bCollapsedCard = OwnerCharacterState->bCollapsed;
		}

		OutViews.Add(MoveTemp(CardView));
	}
}

bool FFinalBattleCardService::RefillDrawPileFromDiscard(FFinalBattleState& BattleState) const
{
	if (BattleState.DeckState.DiscardPileCardInstanceIds.Num() == 0)
	{
		return false;
	}

	BattleState.DeckState.DrawPileCardInstanceIds.Append(BattleState.DeckState.DiscardPileCardInstanceIds);
	BattleState.DeckState.DiscardPileCardInstanceIds.Reset();
	return BattleState.DeckState.DrawPileCardInstanceIds.Num() > 0;
}

void FFinalBattleCardService::RemoveCardInstanceFromAllZones(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	RemoveCardInstanceId(BattleState.DeckState.DrawPileCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.HandCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.DiscardPileCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.OngoingZoneCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.ConsumePileCardInstanceIds, CardInstanceId);
}

void FFinalBattleCardService::CollectMatchingCardInstanceIdsInZone(
	const FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const EFinalBattleCardZone SourceZone,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword,
	const int32 MaxCount,
	const bool bGeneratedOnly,
	TArray<FGuid>& OutCardInstanceIds) const
{
	OutCardInstanceIds.Reset();

	if (RuntimeOwnerUnitId.IsNone() || MaxCount <= 0)
	{
		return;
	}

	const TArray<FGuid>* SourceZoneArray = ResolveZoneArray(BattleState.DeckState, SourceZone);
	if (SourceZoneArray == nullptr)
	{
		return;
	}

	for (int32 CardIndex = SourceZoneArray->Num() - 1;
		CardIndex >= 0 && OutCardInstanceIds.Num() < MaxCount;
		--CardIndex)
	{
		const FGuid CandidateCardInstanceId = (*SourceZoneArray)[CardIndex];
		const FFinalBattleCardInstance* CandidateCardInstance = FindCardInstance(BattleState, CandidateCardInstanceId);
		if (CandidateCardInstance == nullptr
			|| !MatchesGeneratedCardFilter(*CandidateCardInstance, RuntimeOwnerUnitId, RequiredCardId, RequiredKeyword, bGeneratedOnly))
		{
			continue;
		}

		OutCardInstanceIds.Add(CandidateCardInstanceId);
	}
}
